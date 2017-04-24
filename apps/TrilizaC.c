/* TrilizaC.c */

#include <stdlib.h>
#include <stdio.h>
#include <cnaiapi.h>
#include <windows.h>
#include <time.h>

#define BUFFSIZE				256
#define INPUT_PROMPT			"Input   > "
#define WAIT_PLAYER2			"Wait for Player 2"
#define ERROR					"Error..select a valid number (1-9)\n"
#define TIMEOUT					20 // χρόνος (δευτερόλεπτα) ανώτατης καθυστέρησης κίνησης.
/* Ορίζονται κάοιες βοηθητικές σταθερές για την αποκωδικοποίηση της απάντησης του διακομιστή. */
#define READY_IDENTIFIER		"READY" 
#define YOUR_TURN_IDENTIFIER	"your" 
#define WAIT_IDENTIFIER			"Wait"
#define GAME_OVER_IDENTIFIER	"THE GAME IS OVER"
#define INVALID_IDENTIFIER		"invalid"
#define TIME_IDENTIFIER			"TIME"

int recvln(connection, char *, int);
int readln(char *, int);
void gotoxy(int x, int y);
void draw(COORD coordDest, char *b);
void clearAllUntil();


/**************************************************************************************************************
 **																											 **
 ** Program: TrilizaC																						 **
 ** Purpose: Πελάτης ο οποίος συνδέεται σε έναν διακομιστή με βάση ένα αρχείο που διαβάζει και παίζει		 **
 **          τρίλιζα με έναν άλλο πελάτη που συνδέεται κι αυτός στον ίδιο διακομιστή.						 **	
 ** Usage:   TrilizaC																						 **
 **																											 **
 /*************************************************************************************************************/

int
main(int argc, char *argv[])
{
	system("cls");

	computer	comp;
	connection	conn;
	char		buff[BUFFSIZE];
	int			len;
	COORD		coordDest;
	
	/* διαβάζει όνομα διακομιστή (ή IP διεύθυνση) και πόρτα από τό αρχείο triliza.conf */
	int c;
	FILE *file;
	file = fopen("triliza.conf", "r");
	char server[BUFFSIZE];
	char port[BUFFSIZE];

	if (file){
		fscanf(file, "%s", server); // αποθηκεύεται η πρώτη γραμμή στην μεταβλητή server.
		fscanf(file, "%s", port); // αντίστοιχα και η επόμενη.
		
		fclose(file);
	}
	else { // σε περίπτωση αποτυχίας ανάγνωσης του αρχείου, γίνεται τερματισμός.
		printf("Cannot open file triliza.conf. EXIT...\n");
		exit(1);
	}

	
	/* μετατροπή του ονόματος σε δυαδική μορφή comp. */
	comp = cname_to_comp(server); 
	if (comp == -1) 
		exit(1);		

	/* εγκαθίδρυση σύνδεσης με τον διακομιστή του παιχνιδιού. */
	conn = make_contact(comp, (appnum) atoi(port)); 
	if (conn < 0) 	
		exit(1);
		

	(void) printf("Connection With Server Established.\n");

	/* Ο πρώτος πελάτης θα πρέπει να περιμένει και έναν δεύτερο. */
	while ((len = recv(conn, buff, BUFFSIZE, 0)) > 0) {
		if ( strstr(buff, WAIT_PLAYER2)){
			printf("%s", buff);
		}
		else if (strlen(buff) == 9) { // αν δηλαδή σταλεί η κατάσταση της τρίλιζας.
			printf("Players are ready. The game is about to Start\n");
			printf("Select a number (1-9) that corresponds to each cell, starting the count from the top left corner.");
			draw(coordDest, buff); // σχεδιάζει την τρίλιζα.
		}
		else if (strstr(buff, READY_IDENTIFIER)) { break; } // εάν έχει συνδεθεί και ο δεύτερος παίχτης τότε συνέχισε.
	}
	

	boolean gameHasEnded = FALSE;

	/* Το κυρίως παιχνίδι. */
	while ((len = recv(conn, buff, BUFFSIZE, 0)) > 0 && !gameHasEnded) {
	
		if (strlen(buff) == 9) { // κάθε φορά επανασχεδιάζεται η τρίλιζα.
			draw(coordDest, buff);
		}
		
		/* Εάν είναι η σειρά του παίχτη να παίξει. */
		if (strstr(buff, YOUR_TURN_IDENTIFIER)){ 
			printf("\r                                                    \r"); // "σβήνει" την γραμμή που υπήρχε πριν.
			printf("%s", buff);
			
			clock_t t0 = clock(); 

			/* επανάληψη μέχρις ότου γίνει σωστή επιλογή από τον παίχτη. */
			while(TRUE){
				(void)printf(INPUT_PROMPT);
				(void)fflush(stdout);
				len = readln(buff, BUFFSIZE); // διαβάζει από τον παίχτη.
				if (len > 1) { 
					printf(ERROR); 
				}
				else {
					int choice = atoi(buff); // μετατροπή σε ακέραιο.
					
					if (choice < 0 || choice > 9) // εξασφαλίζει ότι είναι αριθμός και όχι άλλος χαρακτήρας.
					{
						printf(ERROR);
					}
					else { 
						
						clock_t t1 = clock();
						long double time_spent = (long double)(t1 - t0) / CLOCKS_PER_SEC;
										
						if (time_spent < (long double) TIMEOUT) { // ελέγχει αν έχει περάσει το όριο καθυστέρησης.
							send(conn, buff, len, 0);
							break;
						}
						else{
							printf("OUT OF TIME\n");
							gameHasEnded = TRUE;
							send(conn, TIME_IDENTIFIER, 10, 0);
							break;
						}
						
					}
				}
			}
		}
		else if (strstr(buff, WAIT_IDENTIFIER)) {
			clearAllUntil(); // καθαρίζει τις προηγούμενες γραμμές.
			printf("Wait the other player to make his move...\n");		
		}
		
		/* Εάν έχει τελειώσει το παιχνίδι (είτε νίκη-ήττα-ισοπαλία, είτε καθυστέρηση, είτε αποτυχημένες προσπάθειες).*/
		if (strstr(buff, GAME_OVER_IDENTIFIER)){
			clearAllUntil(); // καθαρίζει τις προηγούμενες γραμμές.
			printf("%s\n", buff);
			break;
		}

		if (strstr(buff, INVALID_IDENTIFIER)) { clearAllUntil(); } // σβήνει κάποιες γραμμές όταν γίνει λάθος εισαγωγή, έτσι ώστε να γίνει επαναεισαγωγή.
		
	} // τέλος της επανάληψης του παιχνιδιού.

	/* Τέλος παιχνιδιού. Τερματίζεται η σύνδεση. */
	(void) printf("\nGame Connection Closed.\n");
	(void) send_eof(conn);
	exit(0);
}

/**************************************************************
**															 **
**				ΕΞΤΡΑ ΒΟΗΘΗΤΙΚΕΣ ΣΥΝΑΡΤΗΣΕΙΣ				 **
**															 **
***************************************************************/

/* ΣΥΝΑΡΤΗΣΗ gotoxy:
* μεταφέρει τον cursor στην θέση (x,y).
* ΠΑΡΑΜΕΤΡΟΙ:
* x: Στήλη.
* y: γραμμή.
*/
void gotoxy(int x, int y)
{
	COORD c = { x, y };
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), c);
}

/* ΣΥΝΑΡΤΗΣΗ draw:
* σχεδιάζει την τρίλιζα με την τρέχουσα κατάσταση.
* ΠΑΡΑΜΕΤΡΟΙ:
* coordDest: struct για συντεταγμένες που συμβολίζουν ένα σημείο.
* *b: το buff το οποίο παίρνει από τον διακομιστή και είναι ο πίνακας board με την τρέχουσα κατάσταση της τρίλιζας.
*/
void draw(COORD coordDest, char *b) {
	// 0,0,0,0,0,0,0,0,0 san eisodo b
	
	/* Σχεδιασμός σκελετού τρίλιζας. */
	int j;

	coordDest.X = 35;
	for (j = 9; j < 17; j++)
	{
		gotoxy(35, j);
		printf("|       |");
	}

	gotoxy(28, 11);
	printf("-----------------------");
	gotoxy(28, 14);
	printf("-----------------------");

	
	/* Σχεδιασμός κατάστασης στην τρίλιζα. */
	int xAxis = 32;
	int yAxis = 10;

	for (int i = 1; i <= strlen(b); i++)
	{
		int value = b[i-1] - '0';
		if (value == 1) {
			gotoxy(xAxis, yAxis); printf("X");
		}
		else if (value == 2) {
			gotoxy(xAxis, yAxis); printf("O");
		}


		if (i % 3 == 0) {
			yAxis = yAxis + 3; 
			xAxis = 32;		
		}
		else {
			xAxis = xAxis + 7;
		}	
		
	}

	printf("\n\n\n");
	gotoxy(0, 21);	
}

/* ΣΥΝΑΡΤΗΣΗ clearAllUntil:
* σβήνει τις 5 επόμενες γραμμές και επιστρέφει εκεί που ήταν.
*/
void clearAllUntil() {
	
	for (int i = 0; i < 5; i++)
	{
		gotoxy(0, 21+i);
		printf("\r                                                       \r");
	}
	gotoxy(0, 21);
}