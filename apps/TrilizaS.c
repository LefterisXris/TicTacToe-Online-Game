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
 ** Purpose: ����� ���� �����������, � ������ ��������� �� ��������� 2 ������� ��� �� ������� �������.		 **
 ** Usage:   TrilizaS																						 **
 **																											 **
 **************************************************************************************************************/

int
main(int argc, char *argv[])
{
	system("cls");

	connection	connPlayer1, connPlayer2; // �� ��������� �� ���� ��� ������ 1 ��� 2.
	int			len;
	char		buff[BUFFSIZE];
	char		port[] = "20000"; // � ����� ���� ����� �� ������ � �����������.
	int			turn = 0; // � ����� ��� �������. 1--> ������ � ������ �������, 2--> ������ � ��������.
	char		board[] =  "000000000"; // ���� �������� ����������� �� ���� ���� ��� ��������.

	/* ������� ��� ������� ��� ��� Player 1. */
	(void) printf("Game Server Waiting For Player 1.\n");

	connPlayer1 = await_contact((appnum) atoi(port));
	if (connPlayer1 < 0)
		exit(1);
	
	(void) printf("Connection Established for Player 1.\n");
	(void) fflush(stdout);

	(void) send(connPlayer1, "Wait for Player 2\n", BUFFSIZE, 0);


	/* ������� ��� ������� ��� ��� Player 2. */
	(void)printf("Game Server Waiting For Player 2.\n");

	connPlayer2 = await_contact((appnum)atoi(port));
	if (connPlayer2 < 0)
		exit(1);

	(void)printf("Connection Established for Player 2.\n");
	(void)printf("Ready to start the game...\n");
	(void)fflush(stdout);

	/* ����� ����� �������� �������� �� �������. ������������� �� ������� ������� ��� ����� �����, ���� �� ��������� �� ��������. */
	(void)send(connPlayer1, board, BUFFSIZE, 0);
	(void)send(connPlayer2, board, BUFFSIZE, 0);

	/* �������� �������������� ��������� ����� �������. */
	(void)send(connPlayer1, "READY\n", BUFFSIZE, 0);
	(void)send(connPlayer2, "READY\n", BUFFSIZE, 0);
		
	printf("GAME Started!!!\n");

	/* ��������� ������� �������� ��� �� ���������� ����� ������� ������� ��� ���������. */
	turn = 1;	// � ������� 1 �� ������ ������.
	char turnMsg[] = "It's your turn. Add a number (1-9): \n";
	char waitMsg[] = "Wait";
	char winMsg[] = "Congratulations, you have won the game!!! THE GAME IS OVER";
	char loseMsg[] = "Sorry, you have been defeated. THE GAME IS OVER";
	char drawMsg[] = "The game has ended with Draw! Well done players! THE GAME IS OVER";
	char invalidMsg[] = "Error.. your choice is invalid. Add a number (1-9): \n";

	int player1FoulCount = 0; // ����������� ����������� ��� ������ 1. (��� 3)
	int player2FoulCount = 0; // ����������� ����������� ��� ������ 2. (��� 3)
	boolean OK = TRUE; // ��������� ��� ������� �� ������ �� ������� � ����� (�� ����� ������ ��� ���� ������, ���� �� ������ � ����� ����).

	boolean gameHasEnded = FALSE; // ��������� ��� ������� �� ���� ��������� �� ��������.

	/* � ��������� ���� ����� ���� ����� ��� �� ��������. ���� ���� ��� ��� ���������, ������� ��� �����������. */
	while (!gameHasEnded)
	{
		switch (turn)
		{
		case 1:
			printf("It's player's %i turn\n",turn);

			(void)send(connPlayer2, waitMsg, BUFFSIZE, 0); // ��������� ���� ������� ������  �� ������ ��� ������ �� ���������.
			(void)send(connPlayer1, turnMsg, BUFFSIZE, 0); // ��������� ���� ����� ������ �� ������ ��� ����� � ����� ���.

			/* ���������� � �������� ��� ������ ��� �������� ������� ��� �������� ���. */
			while ((len = recv(connPlayer1, buff, BUFFSIZE, 0)) > 0) {
				if (strstr(buff, TIME_IDENTIFIER)) // �� ��������� ��� � ������� ���� ������� �� ������� ���� (��������� ���� client, ��� ���� ����������) �� �������� ���������.
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

					if (!checkMove(turn, buff[0], &board)){ // ��������� ��� ���������� (�� ����� ������) � ������ ��� ������.
						OK = FALSE;  
						player1FoulCount = player1FoulCount +1;
						(void)send(connPlayer2, waitMsg, BUFFSIZE, 0);
						(void)send(connPlayer1, invalidMsg, BUFFSIZE, 0);						
					}
					else {
						player1FoulCount = 0;
						/* ��������� ����� ������� � ������� board ��� �������� ��� ����� ������� �������� ��� �������. */
						(void)send(connPlayer1, board, BUFFSIZE, 0);
						(void)send(connPlayer2, board, BUFFSIZE, 0);

						/* ��������� �� ���� ����������� ������� ��� ��� ������. */
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

					/* ��� � ������� ����� ����� ������ 3 ����� ����������, ���� �������� �����. */
					if (player1FoulCount >= 3) {
						printf("Player %i have lost, because he did 3 invalid choices in row.\n", turn);
						(void)send(connPlayer1, loseMsg, BUFFSIZE, 0);
						(void)send(connPlayer2, winMsg, BUFFSIZE, 0);
						gameHasEnded = TRUE;
					}
				}
				
			}
			if(OK) turn = 2; // ������� � �����.
			break;
		case 2:
			printf("It's player's %i turn\n", turn);

			(void)send(connPlayer1, waitMsg, BUFFSIZE, 0);  // ��������� ���� ������� ������  �� ������ ��� ������ �� ���������.
			(void)send(connPlayer2, turnMsg, BUFFSIZE, 0); // ��������� ���� ����� ������ �� ������ ��� ����� � ����� ���.

			/* ���������� � �������� ��� ������ ��� �������� ������� ��� �������� ���. */
			while ((len = recv(connPlayer2, buff, BUFFSIZE, 0)) > 0) {
				if (strstr(buff, TIME_IDENTIFIER)) // �� ��������� ��� � ������� ���� ������� �� ������� ���� (��������� ���� client, ��� ���� ����������) �� �������� ���������.
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
					
					if (!checkMove(turn, buff[0], &board)) { // ��������� ��� ���������� (�� ����� ������) � ������ ��� ������.
						OK = FALSE;  
						player2FoulCount = player2FoulCount + 1;
						(void)send(connPlayer1, waitMsg, BUFFSIZE, 0);
						(void)send(connPlayer2, invalidMsg, BUFFSIZE, 0);
					}
					else {
						player2FoulCount = 0;
						/* ��������� ����� ������� � ������� board ��� �������� ��� ����� ������� �������� ��� �������. */
						(void)send(connPlayer2, board, BUFFSIZE, 0);
						(void)send(connPlayer1, board, BUFFSIZE, 0);

						/* ��������� �� ���� ����������� ������� ��� ��� ������. */
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

					/* ��� � ������� ����� ����� ������ 3 ����� ����������, ���� �������� �����. */
					if (player2FoulCount >= 3) {
						printf("Player %i have lost, because he did 3 invalid choices in row.\n", turn);
						(void)send(connPlayer2, loseMsg, BUFFSIZE, 0);
						(void)send(connPlayer1, winMsg, BUFFSIZE, 0);
						gameHasEnded = TRUE;
					}
				}
				
			}

			if (OK) turn = 1; // ������� � �����.
			break;
		default:
			break;
		} // ����� ��� switch.

		/* ��������� ��� �� �������� ���� ������ �� ��������. */
		if (isDraw(&board)) {
			(void)send(connPlayer2, drawMsg, BUFFSIZE, 0);
			(void)send(connPlayer1, drawMsg, BUFFSIZE, 0);
			printf("The game is Draw!!! \n");
			gameHasEnded = TRUE;
		}
	} // ����� ��� ���������� ��� ����������.

	printf("THE GAME IS OVER! \n\n");

	/* ����� ����������. ������������ � �������. */
	(void) send_eof(connPlayer1); (void)send_eof(connPlayer2);
	(void) printf("\nGAME Connection Closed.\n\n");
	return 0;
}

/**************************************************************
**															 **
**				����� ���������� �����������				 **
**															 **
***************************************************************/

/* ��������� checkMove:
* ������� �� ������ �� ����� ��� ������ ��� ���� ������, ��� �� ������ �� �����, ���� ��� �������.
* ����������:
* turn: � ����� ��� �������. 1-> ������ � ������ �������, 2-> ������ � ��������.
* b: ���������� �� buff ��� �������� ��� ������� ��� ������.
* *board: ������� ���� ��� ������ board ��� �������� ��� �������� ��������� ��� ��������.
* ���������� ������ �� � ����������� � ������.
*/
boolean checkMove(int turn, char b, char *board ) {

	int cell = b - '0'; // ������������ � ������� ��� ������ �� �������.
	char turnChar = turn + '0'; // ������������ � ����� �� ���������.

	if (board[cell - 1] == '0'){
		board[cell - 1] = turnChar;
		return TRUE;
	}

	return FALSE;
}

/* ��������� hasWin:
* ������� �� ���� ������� ���� ���������� �������.
* ����������:
* turn: � ����� ��� �������. 1-> ������ � ������ �������, 2-> ������ � ��������.
* *board: ������� ���� ��� ������ board ��� �������� ��� �������� ��������� ��� ��������.
* ���������� ������ �� ���� ����������� �������.
*/
boolean hasWin(int turnInt, char *board) {

	char turn = turnInt + '0';

	/* ��������� ������� */
	if (board[0] == turn && board[1] == turn && board[2] == turn)
		return TRUE;
	if (board[3] == turn && board[4] == turn && board[5] == turn)
		return TRUE;
	if (board[6] == turn && board[7] == turn && board[8] == turn)
		return TRUE;

	/* ������ ������� */
	if (board[0] == turn && board[3] == turn && board[6] == turn)
		return TRUE;
	if (board[1] == turn && board[4] == turn && board[7] == turn)
		return TRUE;
	if (board[2] == turn && board[5] == turn && board[8] == turn)
		return TRUE;

	/* �������� ������� */
	if (board[0] == turn && board[4] == turn && board[8] == turn)
		return TRUE;
	if (board[2] == turn && board[4] == turn && board[6] == turn)
		return TRUE;

	return FALSE;
}

/* ��������� isDraw:
* ������� �� ����� �������� �� ��������.
* ����������:
* *board: ������� ���� ��� ������ board ��� �������� ��� �������� ��������� ��� ��������.
* ���������� ������ �� ���� �� ������ ��������� ���� 1 ���� 2 (���� �������� ��� ��� ����  
* ������� ������ ��� �� �������� ��������� �� ��������.
*/
boolean isDraw(char *board) {
	for (int i = 0; i < strlen(board); i++)
	{
		if (board[i] == '0')
			return FALSE;
	}
	return TRUE;
}