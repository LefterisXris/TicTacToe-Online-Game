/* echoserver.c */

#include <stdlib.h>
#include <stdio.h>
#include <cnaiapi.h>

#define BUFFSIZE		256

/*-----------------------------------------------------------------------
 *
 * Program: echoserver
 * Purpose: wait for a connection from an echoclient and echo data
 * Usage:   echoserver <appnum>
 *
 *-----------------------------------------------------------------------
 */
int
main(int argc, char *argv[])
{
	system("cls");
	connection	conn;
	int		len;
	char		buff[BUFFSIZE];

	if (argc != 2) {
		(void) fprintf(stderr, "usage: %s <appnum>\n", argv[0]);
		exit(1);
	}

	/* wait for a connection from an echo client */
	printf("ANAMONI GIA SINDESI");
	conn = await_contact((appnum) atoi(argv[1]));
	if (conn < 0)
		exit(1);

	printf("EGINE I SINDESI");

	/* iterate, echoing all data received until end of file */
	(void)send(conn, "Geia sou\n", BUFFSIZE, 0);

	(void)send(conn, "Ti kaneis? \n", BUFFSIZE, 0);

	(void)send(conn, "Theleis na ginoume filoi?\n", BUFFSIZE, 0);
	while((len = recv(conn, buff, BUFFSIZE, 0)) > 0){
		(void) printf(buff);
		(void) printf("Yolooo");
		(void) send(conn, buff, len, 0);
	}
	send_eof(conn);
	return 0;
}
