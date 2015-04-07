// Student ID : 20093279
// Name : Kim Hyun-Bong

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
#include <fcntl.h> //open���� O_RDONLY�� ����ϱ� ���� ���

#define PROMPT() {printf("WEBSC> ");fflush(stdout);}
#define GETCMD "download"
#define QUITCMD "exit"

void error_handling(char *message);

int main(int argc, char *argv[])
{
	struct sockaddr_in server, remote;
	int request_sock, cli_sock = -1;
	int nfound, fd, maxfd, bytesread, addrlen;
	fd_set rmask, mask;

	char *comm, *html, *port, *file;
	char buf[BUFSIZ], message[BUFSIZ];
	char save[BUFSIZ], file_name[BUFSIZ];

	if (argc != 2) {
		(void) fprintf(stderr,"usage: %s portnum\n",argv[0]);
		exit(1);
	}

	// change the student information
	printf("Student ID : 20093279\n");
	printf("Name : Kim Hyun-Bong\n");

	// create a socket that waits for HTTP request message
	
	if((request_sock = socket(PF_INET, SOCK_STREAM, 0)) == -1)
		error_handling("socket() error");

	// Bind the socket

	memset( (void *)&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(atoi(argv[1]));
	server.sin_addr.s_addr = INADDR_ANY;

	if(bind(request_sock, (struct sockaddr*)&server, sizeof(server)) == -1)
		error_handling("bind() error");

	// Listen the socket

	if(listen(request_sock, SOMAXCONN) < 0)
		error_handling("listen() error");

	// prepare the masks for selec() function
	FD_ZERO(&mask);
	FD_SET(fileno(stdin), &mask);
	FD_SET(request_sock, &mask);
	maxfd = request_sock;
	PROMPT(); 

	for (;;) {
		rmask = mask;
		// call select()
		
		if((nfound = select(maxfd + 1, &rmask, 0, 0, NULL)) == -1)
			error_handling("select() error");

		// check for stdin

		if(FD_ISSET(fileno(stdin), &rmask))
		{
			fgets(buf, sizeof(buf), stdin);
			FD_CLR(fileno(stdin), &rmask);

			comm = strtok(buf, " ");
			html = strtok(NULL, " ");
			port = strtok(NULL, " ");
			file = strtok(NULL, "\n");


			if(!strcmp(comm, GETCMD)){
				struct hostent* host;
				close(cli_sock);

				if((cli_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
					error_handling("socket() error");
			
				
			
				if((host = gethostbyname(html)) == NULL)
					error_handling("gethostbyhame() error");
				
				memset((void *)&remote, 0, sizeof(remote));
				remote.sin_family = AF_INET;
				remote.sin_port = htons(atoi(port));
				memcpy((void*)&remote.sin_addr, host->h_addr, host->h_length);

				if(connect(cli_sock, (struct sockaddr *) &remote, sizeof(remote)) == -1) //�Է¹��� �ּҿ� ��Ʈ�ѹ��� ���� ����õ�
					error_handling("connect() error");

				maxfd = cli_sock;

				strcpy(message, "GET "); //������ ���������� ����Ǹ� ��û�޼��� �߼�
				strcat(message, file);
				strcat(message, " HTTP/1.0\nHost: ");
				strcat(message, html);
				strcat(message, "\r\n\r\n");
				write(cli_sock, message, strlen(message));

				memset(message, 0, sizeof(message));
				while((bytesread=read(cli_sock, message, BUFSIZ))>0) //��û �� ���� �޼����� �޾� ȭ�鿡 ���
				{
					printf("%s", message);
					memset(message, 0, sizeof(message));
				}
				PROMPT();
			}
			else if(!strcmp(comm, QUITCMD))
			{
				close(request_sock);
				close(cli_sock);
				exit(1);
			}
		}
		// check for request socket
		else if(FD_ISSET(request_sock, &rmask))
		{
			addrlen = sizeof remote;
						
			if((cli_sock = accept(request_sock, (struct sockaddr *)&remote, &addrlen))== -1) 
				error_handling("accept() error");
			
			//�ܺ� ��û�� �޾Ƽ� ����
			FD_SET(cli_sock, &mask);
			maxfd = cli_sock;
			printf("connection from host %s, port %d, socket %d\n", inet_ntoa(remote.sin_addr), ntohs(remote.sin_port), cli_sock);

		}
		// check for the connection to the server
		else if(FD_ISSET(cli_sock, &rmask))
		{
			bytesread = read(cli_sock, buf, BUFSIZ);
			
			strcpy(save, buf);

			strtok(save, " ");
			file = strtok(NULL, " ");

			if(*file == '/') file++;
			strcpy(file_name, file);
			fd = 0;
			if((fd = open(file_name, O_RDONLY)) < 0) //������ ���� ���ϸ�(�������� ������) �ش� �޼��� ����
			{
				sprintf(save, "HTTP/1.0 404 NOT FOUND\nConnection: close\nContent-len: %d\nContent-Type: text/html\n\n", 60);
				write(cli_sock, save, strlen(save));
			}
			else
			{
				//������ ����ϰ� ���� �޽����� ����
				long int length = 0; //(long) (lseek(fd, 0L, SEEK_END) - lseek(fd, 0L, SEEK_SET));
				while((addrlen = read(fd, save, BUFSIZ)) > 0) //���� ���� ����
					length += addrlen;

				lseek(fd, 0L, SEEK_SET);
				sprintf(message, "HTTP/1.0 200 OK\r\nConnection: close\r\nContent-Length: %ld\r\nContent-Type: text/html\r\n\r\n", length);
				write(cli_sock, message, strlen(message));
				while((addrlen = read(fd, save, BUFSIZ)) > 0)
				{
					write(cli_sock, save, addrlen);				
				}
			}

			//��û �޽��� ���
			if(bytesread!=0)
			{
				buf[bytesread]='\0';
				printf("%s", buf);
			}
			FD_CLR(cli_sock, &mask); //������ ���´�.
			close(cli_sock);
			maxfd = request_sock;
			PROMPT();
		}
	}
} 

void error_handling(char *message) //���� ���
{
	fputs(message, stderr);
	fputc('\n',stderr);
	exit(1);
}
