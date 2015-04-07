 #include <stdio.h>            
 #include <stdlib.h>            
 #include <sys/socket.h>            
 #include <sys/types.h>            
 #include <string.h>           
 #include <netinet/in.h>             
 #include <pthread.h>            
 #include <arpa/inet.h>           
 #include <unistd.h>            
 #include <signal.h>            
 #define BACKLOG 100            
 #define MAXDATALEN 256             
 #define PORT 2012           
 
   struct Node // 클라이언트를 linked-list로 나타내기 위한 구조체   
     {  
       int port;  
        char username[10];  
       struct Node *next;  
     };  
   typedef struct Node *ptrtonode;  
   typedef ptrtonode head;  
   typedef ptrtonode addr;  
      void sendtoall(char *,int new_fd); // 접속된 클라이언트들이 메세지를 읽게 함(채팅)   
      void Quitall( ); // 서버가 종료되면 클라이언트에게 메세지를 보냄    
      head MakeEmpty( head h ); // 리스트를 지움  
      void Delete( int port, head h );  // 클라이언트가 퇴장하면 클라이언트 정보를 지움 
      void Insert(int port,char*,head h,addr a);	// 새 클라이언트 정보를 삽입
      void DeleteList( head h );  // 리스트를 지움 
      void Display( const head h );  // 접속한 클라이언트 리스트를 화면에 띄움  
      void *Quitproc( );                
      void *server(void * arg);  // 각 클라이언트에 대한 쓰레드 메인 함수 
      void zzz();  
      char      username[10];    
      int sf2;  
      head h;                  
      char     buffer[MAXDATALEN];  
      /******main starts ***********/  
 int main(int argc, char *argv[]) {  
      int       sockfd,new_fd;    // 서버 소켓과 클라이언트 소켓  
      int      portnum;           // 포트번호 
      struct sockaddr_in      server_addr;     // 서버 주소 
      struct sockaddr_in      client_addr;    // 클라이언트 주소
      int      cli_size,z;              // 주소의 길이  
      pthread_t      thr;               // 쓰레드 식별
      int      yes=1;  
       addr a;                        
      printf("\n\t*-*-*-*SERVER STARTED*-*-*-*\n");  
      // 포트 번호 지정  
  if( argc == 2 )       
       portnum = atoi(argv[1]);  
  else   
      portnum = PORT; // 디폴트 포트 번호 
      printf("PORT NO.:\t%d\n",portnum);  
      h = MakeEmpty( NULL );          // 리스트를 비움
      // 서버 정보 설정  
      server_addr.sin_family=AF_INET;            
      server_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
      server_addr.sin_port=htons(portnum);  
       printf("IP ADDRESS:\t%s\n",inet_ntoa(server_addr.sin_addr));  
      // 소켓 생성  
      sockfd = socket(AF_INET, SOCK_STREAM, 0);  
   if(sockfd == -1){  
      printf("server- socket() error");       
      exit(1);  
   }else  
      printf("socket\t\tcreated.\n");  
   if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,sizeof(int)) == -1) {  
      printf("setsockopt error");       
      exit(1);  
   }else printf("reusing\t\tport\n");  
      // 소켓 바인딩  
   if(bind(sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr))==-1){  
           printf("binding failed\n");      
      exit(1);}  
   else   
      printf("binding\t\tsuccess.\n\n");  
 printf("\t\tPRESS CTRL+z TO VIEW ONLINE CLIENTS\n\n");  
      // 소켓 리스닝  
      listen(sockfd, BACKLOG);  
      printf("waiting for clients......\n");  
   if (signal(SIGINT,(void *)Quitproc)==0)     // 시그널 핸들러  
   if(signal(SIGTSTP, zzz)==0)               // 시그널 핸들러
   while(1){  
             cli_size=sizeof(struct sockaddr_in);   
             new_fd = accept(sockfd, (struct sockaddr *)&client_addr,&cli_size); // 클라이언트 접속을 받아들임   
             a =h ;  
           // 사용자 이름 입력  
           bzero(username,10);            
           if(recv(new_fd,username,sizeof(username),0)>0);  
             username[strlen(username)-1]=':';  
             printf("\t%d->%s JOINED chatroom\n",new_fd,username);  
             sprintf(buffer,"%s IS ONLINE\n",username);  
             Insert( new_fd,username, h, a );  // 새로운 클라이언트 정보를 리스트에 삽입  
             a = a->next;  
           // 모든 클라이언트에게 새 클라이언트의 접속을 알림   
             a = h ;  
           do{  
             a = a->next;  
             sf2 = a->port;  
           if(sf2!=new_fd)  
           send(sf2,buffer ,sizeof(buffer),0);  
           } while( a->next != NULL );  
              printf("server got connection from %s & %d\n\n",inet_ntoa(client_addr.sin_addr),new_fd);   
            struct Node args;                         
             args.port=new_fd;  
             strcpy(args.username,username);  
            pthread_create(&thr,NULL,server,(void*)&args);   // 접속한 클라이언트들에 대해 쓰레드 생성   
            pthread_detach(thr);  
       }   
  DeleteList(h);  // 서버가 종료 될 때 모든 클라이언트 리스트를 삭제  
  close(sockfd);  
 } 
      
 void *server(void * arguments){  
  struct Node *args=arguments;  
  char     buffer[MAXDATALEN],ubuf[50],uname[10]; // 서버가 보내는 메세지를 담을 버퍼   
  char *strp;            
  char      *msg = (char *) malloc(MAXDATALEN);  
  int      ts_fd,x,y;  
  int      sfd,msglen;  
  ts_fd = args->port;     
  strcpy(uname,args->username);   
  addr      a;  
       // 접속한 클라이언트 정보를 보냄
       a =h ;  
           do{  
           a = a->next;  
           sprintf( ubuf," %s is online\n",a->username );  
           send(ts_fd,ubuf,strlen(ubuf),0);  
           } while( a->next != NULL );  
       // 채팅 시작  
   while(1){  
        bzero(buffer,256);  
        y=recv(ts_fd,buffer,MAXDATALEN,0);  
       if (y==0)   
       goto jmp;  
        // 클라이언트가 채팅방을 나갈 경우 
       if ( strncmp( buffer, "quit", 4) == 0 ){  
      jmp:     printf("%d ->%s left chat deleting from list\n",ts_fd,uname);  
           sprintf(buffer,"%s has left the chat\n",uname);  
           addr a = h ;  
           do{  
                a = a->next;  
                sfd = a->port;  
                if(sfd == ts_fd)   
                 Delete( sfd, h );  
                if(sfd != ts_fd)   
                 send(sfd,buffer,MAXDATALEN,0);  
            }while ( a->next != NULL );  
                Display( h );  
                close(ts_fd);  
                free(msg);  
             break;  
            }  
      // 모든 클라이언트에게 메세지를 보냄
      printf("%s %s\n",uname,buffer);  
      strcpy(msg,uname);  
      x=strlen(msg);  
      strp = msg;  
      strp+= x;  
      strcat(strp,buffer);  
      msglen=strlen(msg);  
        addr a = h ;  
      do{  
       a = a->next;  
       sfd = a->port;  
      if(sfd != ts_fd)   
        send(sfd,msg,msglen,0);  
      } while( a->next != NULL );  
       Display( h );  
       bzero(msg,MAXDATALEN);  
   }  
    return 0;  
 }  
   
 head MakeEmpty( head h )  
      {  
      if( h != NULL )  
           DeleteList( h );  
           h = malloc( sizeof( struct Node ) );  
      if( h == NULL )  
           printf( "Out of memory!" );  
           h->next = NULL;  
      return h;  
      }  
 
 void DeleteList( head h )  
      {  
      addr a, Tmp;  
      a = h->next;   
      h->next = NULL;  
           while( a != NULL )  
           {  
           Tmp = a->next;  
           free( a );  
           a = Tmp;  
           }  
      }  
 
 void Insert( int port,char *username, head h, addr a )  
      {  
      addr TmpCell;  
      TmpCell = malloc( sizeof( struct Node ) );  
           if( TmpCell == NULL )  
           printf( "Out of space!!!" );  
      TmpCell->port = port;  
      strcpy(TmpCell->username,username);  
      TmpCell->next = a->next;  
      a->next = TmpCell;  
      }  
   
 void Display( const head h )  
      {  
        addr a =h ;  
        if( h->next == NULL )  
      printf( "NO ONLINE CLIENTS\n" );  
        else  
        {  
           do  
           {  
             a = a->next;  
             printf( "%d->%s \t", a->port,a->username );  
           } while( a->next != NULL );  
           printf( "\n" );  
        }  
      }  
  
 void Delete( int port, head h ){  
        addr a, TmpCell;  
        a = h;  
    while( a->next != NULL && a->next->port != port )  
       a = a->next;  
   if( a->next != NULL ){             
      TmpCell = a->next;  
      a->next = TmpCell->next;   
      free( TmpCell );  
   }  
  }  
 
 void *Quitproc(){        
      printf("\n\nSERVER SHUTDOWN\n");  
      Quitall( );  
     exit(0);  
  }  
 
 void Quitall(){  
   int sfd;  
   addr a = h ;  
   int i=0;  
    if( h->next == NULL ) {  
     printf( "......BYE.....\nno clients \n\n" );  
      exit(0);  
     } else {  
        do{  
          i++;  
         a = a->next;  
         sfd = a->port;  
         send(sfd,"server down",13,0);  
         } while( a->next != NULL );  
       printf("%d clients closed\n\n",i);         
       }  
  }  
 void zzz(){  
   printf("\rDISPLAYING ONLINE CLIENTS\n\n");  
   Display(h);  
  }  

