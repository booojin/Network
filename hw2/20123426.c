// Student ID : 20123426
// Name : Choi Jinsung

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#define PROMPT() {printf("WEBSC> ");fflush(stdout);}
#define GETCMD "download"
#define QUITCMD "exit"
#define BUF_SIZE 1024

int main(int argc, char *argv[])
{
 struct sockaddr_in servAddr, newAddr;
 int servSock, newSock, clntSock = -1;
 int fd_max, newAddrLen;
 fd_set tempMask, mask;

 char buf[BUF_SIZE]="";

 if (argc != 2) {
  (void) fprintf(stderr,"usage: %s portnum\n",argv[0]);
  exit(1);
 }

 // change the student information
 printf("Student ID : 20123426\n");
 printf("Name : Choi Jinsung\n");

 // create a socket that waits for HTTP request message
 servSock = socket(PF_INET, SOCK_STREAM, 0);
 memset(&servAddr, 0, sizeof(servAddr));
 servAddr.sin_family = AF_INET;
 servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
 servAddr.sin_port = htons(atoi(argv[1]));

 // Bind the socket
 if( bind(servSock, (struct sockaddr*) &servAddr, sizeof(servAddr)) )
 {
  printf("bind() error\n");
  exit(1);
 }

 // Listen the socket
 if( listen(servSock, 5) )
 {
  printf("listen() error\n");
  exit(1);
 }
 
 // prepare the masks for selec() function
 FD_ZERO(&mask);
 FD_SET(0, &mask);
 FD_SET(servSock, &mask);
 fd_max = servSock;

 PROMPT(); 

 while(1) {
  int nfound;
  tempMask = mask;

  // call select()
  nfound = select(fd_max+1, &tempMask, NULL, NULL, NULL);
  // check for stdin
  if( FD_ISSET(fileno(stdin), &tempMask) )
  {
   char cmd[100];
   char* tok;

   fgets(cmd, sizeof(cmd), stdin);

   tok = strtok(cmd, " \n");
   if( !strcmp(strlwr(cmd), GETCMD) )
   {
    char* host = strtok(NULL, " \n");
    char* conPort = strtok(NULL, " \n");
    char* fileName = strtok(NULL, " \n");
    struct hostent* conHost = gethostbyname(host);
    FILE* nWrite;

    newSock = socket(PF_INET, SOCK_STREAM, 0);

    memset(&newAddr, 0, sizeof(newAddr));
    newAddr.sin_family = AF_INET;
    newAddr.sin_addr = *(struct in_addr*)conHost->h_addr_list[0];
    newAddr.sin_port = htons(atoi(conPort));

    if( connect(newSock, (struct sockaddr*)&newAddr, sizeof(newAddr))==-1 )
    {
     printf("connect error() \n");
     exit(1);
    }
    FD_SET(newSock, &mask);
    if(fd_max < newSock)
     fd_max = newSock;

    nWrite = fdopen(dup(newSock), "w");
    fprintf(nWrite, "GET /%s HTTP/1.1\r\n", fileName);
    fprintf(nWrite, "Host: %s\r\n\r\n", host);

    fclose(nWrite);
   }
   else if( !strcmp(strlwr(cmd), QUITCMD) )
    exit(1);
   else
    PROMPT();
  }
  else if( FD_ISSET(servSock, &tempMask) )
  {
   newAddrLen = sizeof(newAddr);
   clntSock = accept(servSock, (struct sockaddr*) &newAddr, &newAddrLen);
   if(clntSock == -1)
   {
    printf("accept() error\n");
    exit(1);
   }
   FD_SET(clntSock, &mask);
   if(fd_max < clntSock)
    fd_max = clntSock;

   printf("connection from host %s, port %d, socket %d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port), clntSock);
  }
  else if( clntSock > 0 && FD_ISSET(clntSock, &tempMask) )
  {
   char* fileName;
   char cType[100];
   int clen=0;
   int reqLen=0;
   FILE* cRead = fdopen(clntSock, "r");
   FILE* cWrite = fdopen(dup(clntSock), "w");
   FILE* send_file;

   reqLen = read(clntSock, buf, BUF_SIZE);
   buf[reqLen]=0;
   printf("%s", buf);

   strtok(buf, " /");
   fileName=strtok(NULL, " /");
   fclose(cRead);

   strcpy(cType, fileName);
   strtok(cType, ".");
   strcpy(cType, strtok(NULL, "."));
   if( !strcmp(strlwr(cType),"html") || !strcmp(strlwr(cType), "htm") )
    strcpy(cType,"text/html");
   else
    strcpy(cType, "text/plain");

   send_file = fopen(fileName, "r");
   if(send_file==NULL)
   {
    fprintf(cWrite, "HTTP/1.1 404 NOT FOUND\r\n");
    fprintf(cWrite, "Connection: close\r\n");
    fprintf(cWrite, "Content-Length: 0\r\n");
    fprintf(cWrite, "Content-Type: %s\r\n\r\n", cType);

    FD_CLR(clntSock, &mask);
    if(!fclose(cWrite) && fd_max==clntSock)
     fd_max--;

    continue;
   }

   while( fgets(buf, BUF_SIZE, send_file)!=NULL )
    clen+= strlen(buf);

   fprintf(cWrite, "HTTP/1.1 200 OK\r\n");
   fprintf(cWrite, "Connection: close\r\n");
   fprintf(cWrite, "Content-Length: %d\r\n", clen);
   fprintf(cWrite, "Content-Type: %s\r\n\r\n", cType);

   fseek(send_file, 0, SEEK_SET);
   while( fgets(buf, BUF_SIZE, send_file)!=NULL )
   {
    fputs(buf, cWrite);
    fflush(cWrite);
   }
   fflush(cWrite);
   fclose(send_file);

   FD_CLR(clntSock, &mask);
   if(!fclose(cWrite) && fd_max==clntSock)
    fd_max--;
   clntSock=-1;

   PROMPT();
  }
  else if( FD_ISSET(newSock, &tempMask) )
  {
   FILE* nRead = fdopen(newSock, "r");
   while( fgets(buf, BUF_SIZE, nRead) )
    printf("%s", buf);

   FD_CLR(newSock, &mask);
   if(!fclose(nRead) && fd_max==newSock)
    fd_max--;
   PROMPT();
  }
 }
} 