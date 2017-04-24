/* TrilizaS.c */

#include <stdlib.h>
#include <stdio.h>
#include <cnaiapi.h>
#include <conio.h>

#define BUFFSIZE			256
#define TIME_IDENTIFIER		"TIME"

int recvln(connection, char *, int);
int readln(char *, int);
boolean checkMove(int turn, char b, char *board);
boolean hasWin(int turn, char *board);
boolean isDraw(char *board);


/**************************************************************************************************************
 **																											 **
 ** Program: TrilizaS.c																						 **
 ** Purpose: Είναι ένας διακομιστής, ο οποίος περιμένει να συνδεθούν 2 παίχτες για να παίξουν τρίλιζα.		 **
 ** Usage:   TrilizaS																						 **
 **																											 **
 **************************************************************************************************************/

int
main(int argc, char *argv[])
{
	system("cls");

	connection	connPlayer1, connPlayer2; // οι συνδέσεις με τους τον παίχτη 1 και 2.
	int			len;
	char		buff[BUFFSIZE];
	char		port[] = "20000"; // η πόρτα στην οποία θα ακούει ο διακομιστής.
	int			turn = 0; // η σειρά των παιχτών. 1--> παίζει ο πρώτος παίχτης, 2--> παίζει ο δεύτερος.
	char		board[] =  "000000000"; // κάθε μηδενικό αντιστοιχεί σε κάθε θέση της τρίλιζας.

	/* αναμονή για σύνδεση από τον Player 1. */
	(void) printf("Game Server Waiting For Player 1.\n");

	connPlayer1 = await_contact((appnum) atoi(port));
	if (connPlayer1 < 0)
		exit(1);
	
	(void) printf("Connection Established for Player 1.\n");
	(void) fflush(stdout);

	(void) send(connPlayer1, "Wait for Player 2\n", BUFFSIZE, 0);


	/* αναμονή για σύνδεση από τον Player 2. */
	(void)printf("Game Server Waiting For Player 2.\n");

	connPlayer2 = await_contact((appnum)atoi(port));
	if (connPlayer2 < 0)
		exit(1);

	(void)printf("Connection Established for Player 2.\n");
	(void)printf("Ready to start the game...\n");
	(void)fflush(stdout);

	/* Πλέον έχουν συνδεθεί επιτυχώς οι παίχτες. Αποστέλλονται οι αρχικοί πίνακες που είναι κενοί, ώστε να ξεκινήσει το παιχνίδι. */
	(void)send(connPlayer1, board, BUFFSIZE, 0);
	(void)send(connPlayer2, board, BUFFSIZE, 0);

	/* Αποστολή επιβεβαιωτικού μηνύματος στους παίχτες. */
	(void)send(connPlayer1, "READY\n", BUFFSIZE, 0);
	(void)send(connPlayer2, "READY\n", BUFFSIZE, 0);
		
	printf("GAME Started!!!\n");

	/* Ορίζονται διάφορα μηνύματα που θα στέλνονται στους παίχτες ανάλογα την περίπτωση. */
	turn = 1;	// ο παίχτης 1 θα παίξει πρώτος.
	char turnMsg[] = "It's your turn. Add a number (1-9): \n";
	char waitMsg[] = "Wait";
	char winMsg[] = "Congratulations, you have won the game!!! THE GAME IS OVER";
	char loseMsg[] = "Sorry, you have been defeated. THE GAME IS OVER";
	char drawMsg[] = "The game has ended with Draw! Well done players! THE GAME IS OVER";
	char invalidMsg[] = "Error.. your choice is invalid. Add a number (1-9): \n";

	int player1FoulCount = 0; // Λανθασμένες προσπάθειες του παίχτη 1. (έως 3)
	int player2FoulCount = 0; // Λανθασμένες προσπάθειες του παίχτη 2. (έως 3)
	boolean OK = TRUE; // μεταβλητή που ελέγχει αν μπορεί να αλλάξει η σειρά (αν έγινε κίνηση του ενός παίχτη, ώστε να παίξει ο άλλος μετά).

	boolean gameHasEnded = FALSE; // μεταβλητή που ελέγχει αν έχει τελειώσει το παιχνίδι.

	/* Η επανάληψη αυτή είναι στην ουσία όλο το παιχνίδι. Όταν βγει από την επανάληψη, γίνεται και τερματισμός. */
	while (!gameHasEnded)
	{
		switch (turn)
		{
		case 1:
			printf("It's player's %i turn\n",turn);

			(void)send(connPlayer2, waitMsg, BUFFSIZE, 0); // στέλνεται στον δεύτερο παίχτη  το μήνυμα ότι πρέπει να περιμένει.
			(void)send(connPlayer1, turnMsg, BUFFSIZE, 0); // στέλνεται στον πρώτο παίχτη το μήνυμα ότι είναι η σειρά του.

			/* Λαμβάνεται η απάντηση του παίχτη και γίνονται έλεγχοι της επιλογής του. */
			while ((len = recv(connPlayer1, buff, BUFFSIZE, 0)) > 0) {
				if (strstr(buff, TIME_IDENTIFIER)) // σε περίπτωση που ο παίχτης έχει υπερβεί το χρονικό όριο (ελέγχεται στον client, εδώ απλά ενημερώνει) το παιχνίδι τελειώνει.
				{
					printf("Player %i have lost, because he is too late.\n", turn);
					(void)send(connPlayer2, winMsg, BUFFSIZE, 0);
					(void)send(connPlayer1, loseMsg, BUFFSIZE, 0);
					gameHasEnded = TRUE;
				}
				else
				{
					printf("Player's 1 move: %c\n", buff[0]);
					printf("Try to make the move...\n");

					if (!checkMove(turn, buff[0], &board)){ // ελέγχεται και εκτελείται (αν είναι δυνατό) η κίνηση του παίχτη.
						OK = FALSE;  
						player1FoulCount = player1FoulCount +1;
						(void)send(connPlayer2, waitMsg, BUFFSIZE, 0);
						(void)send(connPlayer1, invalidMsg, BUFFSIZE, 0);						
					}
					else {
						player1FoulCount = 0;
						/* στέλνεται στους παίχτες ο πίνακας board που περιέχει τις μέχρι στιγμής επιλογές των παιχτών. */
						(void)send(connPlayer1, board, BUFFSIZE, 0);
						(void)send(connPlayer2, board, BUFFSIZE, 0);

						/* ελέγχεται αν έχει σχηματιστεί τρίλιζα από τον παίχτη. */
						if (hasWin(turn, &board))
						{
							printf("We have a winner!! Well done player %i!\n", turn);
							(void)send(connPlayer1, winMsg, BUFFSIZE, 0);
							(void)send(connPlayer2, loseMsg, BUFFSIZE, 0);
							gameHasEnded = TRUE;
						}
						
						OK = TRUE;
						break;
					}

					/* Εάν ο παίχτης κάνει λάθος κίνηση 3 φορές συνεχόμενα, τότε αυτόματα χάνει. */
					if (player1FoulCount >= 3) {
						printf("Player %i have lost, because he did 3 invalid choices in row.\n", turn);
						(void)send(connPlayer1, loseMsg, BUFFSIZE, 0);
						(void)send(connPlayer2, winMsg, BUFFSIZE, 0);
						gameHasEnded = TRUE;
					}
				}
				
			}
			if(OK) turn = 2; // αλλάζει η σειρά.
			break;
		case 2:
			printf("It's player's %i turn\n", turn);

			(void)send(connPlayer1, waitMsg, BUFFSIZE, 0);  // στέλνεται στον δεύτερο παίχτη  το μήνυμα ότι πρέπει να περιμένει.
			(void)send(connPlayer2, turnMsg, BUFFSIZE, 0); // στέλνεται στον πρώτο παίχτη το μήνυμα ότι είναι η σειρά του.

			/* Λαμβάνεται η απάντηση του παίχτη και γίνονται έλεγχοι της επιλογής του. */
			while ((len = recv(connPlayer2, buff, BUFFSIZE, 0)) > 0) {
				if (strstr(buff, TIME_IDENTIFIER)) // σε περίπτωση που ο παίχτης έχει υπερβεί το χρονικό όριο (ελέγχεται στον client, εδώ απλά ενημερώνει) το παιχνίδι τελειώνει.
				{
					printf("Player %i have lost, because he is too late.\n",turn);
					(void)send(connPlayer2, loseMsg, BUFFSIZE, 0);
					(void)send(connPlayer1, winMsg, BUFFSIZE, 0);					
					gameHasEnded = TRUE;
				}
				else
				{
					printf("Player's 2 move: %c\n", buff[0]);
					printf("Try to make the move...\n");
					
					if (!checkMove(turn, buff[0], &board)) { // ελέγχεται και εκτελείται (αν είναι δυνατό) η κίνηση του παίχτη.
						OK = FALSE;  
						player2FoulCount = player2FoulCount + 1;
						(void)send(connPlayer1, waitMsg, BUFFSIZE, 0);
						(void)send(connPlayer2, invalidMsg, BUFFSIZE, 0);
					}
					else {
						player2FoulCount = 0;
						/* στέλνεται στους παίχτες ο πίνακας board που περιέχει τις μέχρι στιγμής επιλογές των παιχτών. */
						(void)send(connPlayer2, board, BUFFSIZE, 0);
						(void)send(connPlayer1, board, BUFFSIZE, 0);

						/* ελέγχεται αν έχει σχηματιστεί τρίλιζα από τον παίχτη. */
						if (hasWin(turn, &board))
						{
							printf("We have a winner!! Well done player %i!\n", turn);
							(void)send(connPlayer2, winMsg, BUFFSIZE, 0);
							(void)send(connPlayer1, loseMsg, BUFFSIZE, 0);
							gameHasEnded = TRUE;
						}

						OK = TRUE;
						break;
					}

					/* Εάν ο παίχτης κάνει λάθος κίνηση 3 φορές συνεχόμενα, τότε αυτόματα χάνει. */
					if (player2FoulCount >= 3) {
						printf("Player %i have lost, because he did 3 invalid choices in row.\n", turn);
						(void)send(connPlayer2, loseMsg, BUFFSIZE, 0);
						(void)send(connPlayer1, winMsg, BUFFSIZE, 0);
						gameHasEnded = TRUE;
					}
				}
				
			}

			if (OK) turn = 1; // αλλάζει η σειρά.
			break;
		default:
			break;
		} // τέλος της switch.

		/* Ελέγχεται εάν το παιχνίδι έχει φτάσει σε ισοπαλία. */
		if (isDraw(&board)) {
			(void)send(connPlayer2, drawMsg, BUFFSIZE, 0);
			(void)send(connPlayer1, drawMsg, BUFFSIZE, 0);
			printf("The game is Draw!!! \n");
			gameHasEnded = TRUE;
		}
	} // τέλος της επανάληψης του παιχνιδιού.

	printf("THE GAME IS OVER! \n\n");

	/* Τέλος παιχνιδιού. Τερματίζεται η σύνδεση. */
	(void) send_eof(connPlayer1); (void)send_eof(connPlayer2);
	(void) printf("\nGAME Connection Closed.\n\n");
	return 0;
}

/**************************************************************
**															 **
**				ΕΞΤΡΑ ΒΟΗΘΗΤΙΚΕΣ ΣΥΝΑΡΤΗΣΕΙΣ				 **
**															 **
***************************************************************/

/* ΣΥΝΑΡΤΗΣΗ checkMove:
* ελέγχει αν μπορεί να γίνει μια κίνηση από έναν παίχτη, και αν μπορεί να γίνει, τότε την εκτελεί.
* ΠΑΡΑΜΕΤΡΟΙ:
* turn: η σειρά των παιχτών. 1-> παίζει ο πρώτος παίχτης, 2-> παίζει ο δεύτερος.
* b: συμβολίζει το buff που περιέχει την επιλογή του παίχτη.
* *board: δείκτης προς τον πίνακα board που περιέχει την τρέχουσα κατάσταση της τρίλιζας.
* ΕΠΙΣΤΡΕΦΕΙ αληθής αν η εκτελέστηκε η κίνηση.
*/
boolean checkMove(int turn, char b, char *board ) {

	int cell = b - '0'; // μετατρέπεται η επιλογή του παίχτη σε ακέραιο.
	char turnChar = turn + '0'; // μετατρέπεται η σειρά σε χαρακτήρα.

	if (board[cell - 1] == '0'){
		board[cell - 1] = turnChar;
		return TRUE;
	}

	return FALSE;
}

/* ΣΥΝΑΡΤΗΣΗ hasWin:
* Ελέγχει αν ένας παίχτης έχει σχηματίσει τρίλιζα.
* ΠΑΡΑΜΕΤΡΟΙ:
* turn: η σειρά των παιχτών. 1-> παίζει ο πρώτος παίχτης, 2-> παίζει ο δεύτερος.
* *board: δείκτης προς τον πίνακα board που περιέχει την τρέχουσα κατάσταση της τρίλιζας.
* ΕΠΙΣΤΡΕΦΕΙ αληθής αν έχει σχηματίστεί τρίλιζα.
*/
boolean hasWin(int turnInt, char *board) {

	char turn = turnInt + '0';

	/* ΟΡΙΖΟΝΤΙΑ ΤΡΙΛΙΖΑ */
	if (board[0] == turn && board[1] == turn && board[2] == turn)
		return TRUE;
	if (board[3] == turn && board[4] == turn && board[5] == turn)
		return TRUE;
	if (board[6] == turn && board[7] == turn && board[8] == turn)
		return TRUE;

	/* ΚΑΘΕΤΗ ΤΡΙΛΙΖΑ */
	if (board[0] == turn && board[3] == turn && board[6] == turn)
		return TRUE;
	if (board[1] == turn && board[4] == turn && board[7] == turn)
		return TRUE;
	if (board[2] == turn && board[5] == turn && board[8] == turn)
		return TRUE;

	/* ΔΙΑΓΩΝΙΑ ΤΡΙΛΙΖΑ */
	if (board[0] == turn && board[4] == turn && board[8] == turn)
		return TRUE;
	if (board[2] == turn && board[4] == turn && board[6] == turn)
		return TRUE;

	return FALSE;
}

/* ΣΥΝΑΡΤΗΣΗ isDraw:
* Ελέγχει αν βγήκε ισοπαλία το παιχνίδι.
* ΠΑΡΑΜΕΤΡΟΙ:
* *board: δείκτης προς τον πίνακα board που περιέχει την τρέχουσα κατάσταση της τρίλιζας.
* ΕΠΙΣΤΡΕΦΕΙ αληθής αν όλες οι θέσεις περιέχουν είτε 1 είτε 2 (αυτό σημαίνει ότι δεν έχει  
* νικήσει κανείς και το παιχνίδι τελειώνει με ισοπαλία.
*/
boolean isDraw(char *board) {
	for (int i = 0; i < strlen(board); i++)
	{
		if (board[i] == '0')
			return FALSE;
	}
	return TRUE;
}