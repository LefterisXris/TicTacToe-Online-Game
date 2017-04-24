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
#define TIMEOUT					20 // ������ (������������) �������� ������������ �������.
/* ��������� ������ ���������� �������� ��� ��� ��������������� ��� ��������� ��� ����������. */
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
 ** Purpose: ������� � ������ ��������� �� ���� ���������� �� ���� ��� ������ ��� �������� ��� ������		 **
 **          ������� �� ���� ���� ������ ��� ��������� �� ����� ���� ���� ����������.						 **	
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
	
	/* �������� ����� ���������� (� IP ���������) ��� ����� ��� �� ������ triliza.conf */
	int c;
	FILE *file;
	file = fopen("triliza.conf", "r");
	char server[BUFFSIZE];
	char port[BUFFSIZE];

	if (file){
		fscanf(file, "%s", server); // ������������ � ����� ������ ���� ��������� server.
		fscanf(file, "%s", port); // ���������� ��� � �������.
		
		fclose(file);
	}
	else { // �� ��������� ��������� ��������� ��� �������, ������� �����������.
		printf("Cannot open file triliza.conf. EXIT...\n");
		exit(1);
	}

	
	/* ��������� ��� �������� �� ������� ����� comp. */
	comp = cname_to_comp(server); 
	if (comp == -1) 
		exit(1);		

	/* ����������� �������� �� ��� ���������� ��� ����������. */
	conn = make_contact(comp, (appnum) atoi(port)); 
	if (conn < 0) 	
		exit(1);
		

	(void) printf("Connection With Server Established.\n");

	/* � ������ ������� �� ������ �� ��������� ��� ���� �������. */
	while ((len = recv(conn, buff, BUFFSIZE, 0)) > 0) {
		if ( strstr(buff, WAIT_PLAYER2)){
			printf("%s", buff);
		}
		else if (strlen(buff) == 9) { // �� ������ ������ � ��������� ��� ��������.
			printf("Players are ready. The game is about to Start\n");
			printf("Select a number (1-9) that corresponds to each cell, starting the count from the top left corner.");
			draw(coordDest, buff); // ��������� ��� �������.
		}
		else if (strstr(buff, READY_IDENTIFIER)) { break; } // ��� ���� �������� ��� � �������� ������� ���� ��������.
	}
	

	boolean gameHasEnded = FALSE;

	/* �� ������ ��������. */
	while ((len = recv(conn, buff, BUFFSIZE, 0)) > 0 && !gameHasEnded) {
	
		if (strlen(buff) == 9) { // ���� ���� ���������������� � �������.
			draw(coordDest, buff);
		}
		
		/* ��� ����� � ����� ��� ������ �� ������. */
		if (strstr(buff, YOUR_TURN_IDENTIFIER)){ 
			printf("\r                                                    \r"); // "������" ��� ������ ��� ������ ����.
			printf("%s", buff);
			
			clock_t t0 = clock(); 

			/* ��������� ������ ���� ����� ����� ������� ��� ��� ������. */
			while(TRUE){
				(void)printf(INPUT_PROMPT);
				(void)fflush(stdout);
				len = readln(buff, BUFFSIZE); // �������� ��� ��� ������.
				if (len > 1) { 
					printf(ERROR); 
				}
				else {
					int choice = atoi(buff); // ��������� �� �������.
					
					if (choice < 0 || choice > 9) // ����������� ��� ����� ������� ��� ��� ����� ����������.
					{
						printf(ERROR);
					}
					else { 
						
						clock_t t1 = clock();
						long double time_spent = (long double)(t1 - t0) / CLOCKS_PER_SEC;
										
						if (time_spent < (long double) TIMEOUT) { // ������� �� ���� ������� �� ���� ������������.
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
			clearAllUntil(); // ��������� ��� ������������ �������.
			printf("Wait the other player to make his move...\n");		
		}
		
		/* ��� ���� ��������� �� �������� (���� ����-����-��������, ���� �����������, ���� ������������ �����������).*/
		if (strstr(buff, GAME_OVER_IDENTIFIER)){
			clearAllUntil(); // ��������� ��� ������������ �������.
			printf("%s\n", buff);
			break;
		}

		if (strstr(buff, INVALID_IDENTIFIER)) { clearAllUntil(); } // ������ ������� ������� ���� ����� ����� ��������, ���� ���� �� ����� �������������.
		
	} // ����� ��� ���������� ��� ����������.

	/* ����� ����������. ������������ � �������. */
	(void) printf("\nGame Connection Closed.\n");
	(void) send_eof(conn);
	exit(0);
}

/**************************************************************
**															 **
**				����� ���������� �����������				 **
**															 **
***************************************************************/

/* ��������� gotoxy:
* ��������� ��� cursor ���� ���� (x,y).
* ����������:
* x: �����.
* y: ������.
*/
void gotoxy(int x, int y)
{
	COORD c = { x, y };
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), c);
}

/* ��������� draw:
* ��������� ��� ������� �� ��� �������� ���������.
* ����������:
* coordDest: struct ��� ������������� ��� ����������� ��� ������.
* *b: �� buff �� ����� ������� ��� ��� ���������� ��� ����� � ������� board �� ��� �������� ��������� ��� ��������.
*/
void draw(COORD coordDest, char *b) {
	// 0,0,0,0,0,0,0,0,0 san eisodo b
	
	/* ���������� �������� ��������. */
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

	
	/* ���������� ���������� ���� �������. */
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

/* ��������� clearAllUntil:
* ������ ��� 5 �������� ������� ��� ���������� ���� ��� ����.
*/
void clearAllUntil() {
	
	for (int i = 0; i < 5; i++)
	{
		gotoxy(0, 21+i);
		printf("\r                                                       \r");
	}
	gotoxy(0, 21);
}