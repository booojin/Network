// Student ID : 20123426
// Name : 최진성

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>    
#include <unistd.h>
#include <pthread.h>

#define PROMPT() {printf("WEBSC> ");fflush(stdout);}
#define GETCMD "download"
#define QUITCMD "exit"

#define BUF_SIZE 1024
#define SMALL_BUF 100

void request_handler(int sock);
void send_data(FILE* fp, char* ct, char* file_name);
char* content_type(char* file);
void send_error(FILE* fp);

int main(int argc, char *argv[])
{
	struct sockaddr_in server, remote;
	int request_sock, new_sock, cli_sock = -1;
	int nfound, fd, maxfd, bytesread, addrlen;
	fd_set rmask, mask;
	char command[1024];
	char req_msg[1024];
	char req_msg2[1024];
	char *token = NULL;
	char str[]=" \n";

	char buf[BUF_SIZE];
	if (argc != 2) {
		(void) fprintf(stderr,"usage: %s portnum\n",argv[0]);
		exit(1);
	}

	// change the student information
	printf("Student ID : 20123426\n");
	printf("Name : Choi Jinsung\n");

	// create a socket that waits for HTTP request message
	request_sock=socket(PF_INET, SOCK_STREAM, 0);
	memset(&server, 0, sizeof(server));
	server.sin_family=AF_INET;
	server.sin_addr.s_addr=htonl(INADDR_ANY);
	server.sin_port=htons(atoi(argv[1]));

	// Bind the socket
	if(bind(request_sock, (struct sockaddr*)&server, sizeof(server))==-1)
		printf("bind() error");

	// Listen the socket
	if(listen(request_sock, 20)==-1)
		printf("listen() error");

	// prepare the masks for select() function
	FD_ZERO(&mask);
	FD_SET(fileno(stdin), &mask);
	FD_SET(request_sock, &mask);
	maxfd=request_sock;

	PROMPT(); 

	while(1) {
		char hostname[50];
		char *portno;
		char *download;
		char tempCmd[1024];
		rmask = mask;

		// call select()
		nfound = select(maxfd+1, &rmask, 0, 0, NULL);

		// check for stdin
		if(FD_ISSET(fileno(stdin), &rmask)) {
			fgets(command, sizeof(command), stdin);
			FD_CLR(fileno(stdin), &rmask);

			strcpy(tempCmd, command);
			token = strtok(tempCmd, str);
			if(!strcmp(tempCmd, QUITCMD)) {
				close(cli_sock);
				cli_sock = socket(PF_INET, SOCK_STREAM, 0);
				exit(0);
			}
			else if(!strcmp(token, GETCMD)) {
				token = strtok(NULL, str);
				if(!strcmp(token, "localhost"))
					strcpy(hostname,"127.0.0.1");
				else 
					strcpy(hostname, token);
				portno = strtok(NULL, str);
				download = strtok(NULL, str);

				cli_sock = socket(PF_INET, SOCK_STREAM, 0);
				memset(&remote, 0, sizeof(remote));
				remote.sin_family=AF_INET;
				remote.sin_addr.s_addr=inet_addr(hostname);
				remote.sin_port=htons(atoi(portno));

				if(!strcmp(hostname, "127.0.0.1"))
					strcpy(hostname,"localhost");
				
				strcpy(req_msg, "Get /");
				strcat(req_msg, download);
				strcat(req_msg, " HTTP/1.0\n");
				strcpy(req_msg2, "Host: ");
				strcat(req_msg2, hostname);
				strcat(req_msg2, "\n");

				if(connect(cli_sock, (struct sockaddr*)&remote, sizeof(remote))==-1)
					printf("connect() error\n");
				else {
					FD_SET(cli_sock, &mask);
					if(maxfd<cli_sock)
						maxfd=cli_sock;
					write(cli_sock, req_msg, strlen(req_msg));
					write(cli_sock, req_msg2, strlen(req_msg2));
				}
			}
			PROMPT(); 
		}
		// check for request socket
		else if(FD_ISSET(request_sock, &rmask)) {
			addrlen=sizeof(remote);
			cli_sock=accept(request_sock, (struct sockaddr*)&remote, &addrlen);
			FD_SET(cli_sock, &mask);
			if(maxfd<cli_sock)
				maxfd=cli_sock;
			printf("connection from host %s, port %d, socket %d\n",
				inet_ntoa(remote.sin_addr), ntohs(remote.sin_port), cli_sock);
			request_handler(cli_sock);
		}
		else if(FD_ISSET(cli_sock, &rmask)) {
			//request_handler(cli_sock);
		}
	}
	close(request_sock);
	return 0;
} /* main - websc.c */

void request_handler(int sock) {
	int clnt_sock=sock;
	char req_line[SMALL_BUF];
	FILE* clnt_read;
	FILE* clnt_write;

	char method[10];
	char ct[15];
	char file_name[30];

	clnt_read=fdopen(clnt_sock, "r");
	clnt_write=fdopen(dup(clnt_sock), "w");
	fgets(req_line, SMALL_BUF, clnt_read);
	
	printf(req_line);
	if(strstr(req_line, "HTTP/")==NULL) {
		printf("not http error\n");
		send_error(clnt_write);
		fclose(clnt_read);
		fclose(clnt_write);
		return;
	}
	strcpy(method, strtok(req_line, " /"));
	strcpy(file_name, strtok(NULL, " /"));
	strcpy(ct, content_type(file_name));
	if(strcmp(method, "Get")!=0) {
		printf(method);
		printf("not get error\n");
		send_error(clnt_write);
		fclose(clnt_read);
		fclose(clnt_write);
		return;
	}

	fclose(clnt_read);
	send_data(clnt_write, ct, file_name);
}

void send_data(FILE* fp, char* ct, char* file_name) {
	char protocol[]="HTTP/1.0 200 OK\r\n";
	char server[]="Server:Linux Web Server \r\n";
	char cnt_len[]="Content-length:2048\r\n";
	char cnt_type[SMALL_BUF];
	char buf[BUF_SIZE];
	FILE* send_file;

	sprintf(cnt_type, "Content-type:%s\r\n\r\n", ct);
	send_file=fopen(file_name, "r");
	if(send_file==NULL) {
		send_error(fp);
		return;
	}

	fputs(protocol, fp);
	fputs(server, fp);
	fputs(cnt_len, fp);
	fputs(cnt_type, fp);
	
	while(fgets(buf, BUF_SIZE, send_file)!=NULL) {
		fputs(buf, fp);
		fflush(fp);
	}
	fflush(fp);
	fclose(fp);
}

char* content_type(char* file) {
	char extension[SMALL_BUF];
	char file_name[SMALL_BUF];
	strcpy(file_name, file);
	strtok(file_name, ".");
	strcpy(extension, strtok(NULL, "."));

	if(!strcmp(extension, "html")||!strcmp(extension, "htm"))
		return "text/html";
	else
		return "text/plain";
}

void send_error(FILE* fp) {
	char protocol[]="HTTP/1.0 400 Bad Request\r\n";
	char server[]="Server:Linux Web Server \r\n";
	char cnt_len[]="Content-length:2048\r\n";
	char cnt_type[]="Content-type:text/html\r\n\r\n";
	char content[]="<html><head><title>NETWORK</title></head>"
		"<body><font size=+5><br>오류 발생! 요청 파일명 및 요청 방식 확인!"
		"</font></body></html>";

	fputs(protocol, fp);
	fputs(server, fp);
	fputs(cnt_len, fp);
	fputs(cnt_type, fp);
	fflush(fp);
}

void error_handling(char* message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}