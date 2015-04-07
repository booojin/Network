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
 
   struct Node // Ŭ���̾�Ʈ�� linked-list�� ��Ÿ���� ���� ����ü   
     {  
       int port;  
        char username[10];  
       struct Node *next;  
     };  
   typedef struct Node *ptrtonode;  
   typedef ptrtonode head;  
   typedef ptrtonode addr;  
      void sendtoall(char *,int new_fd); // ���ӵ� Ŭ���̾�Ʈ���� �޼����� �а� ��(ä��)   
      void Quitall( ); // ������ ����Ǹ� Ŭ���̾�Ʈ���� �޼����� ����    
      head MakeEmpty( head h ); // ����Ʈ�� ����  
      void Delete( int port, head h );  // Ŭ���̾�Ʈ�� �����ϸ� Ŭ���̾�Ʈ ������ ���� 
      void Insert(int port,char*,head h,addr a);	// �� Ŭ���̾�Ʈ ������ ����
      void DeleteList( head h );  // ����Ʈ�� ���� 
      void Display( const head h );  // ������ Ŭ���̾�Ʈ ����Ʈ�� ȭ�鿡 ���  
      void *Quitproc( );                
      void *server(void * arg);  // �� Ŭ���̾�Ʈ�� ���� ������ ���� �Լ� 
      void zzz();  
      char      username[10];    
      int sf2;  
      head h;                  
      char     buffer[MAXDATALEN];  
      /******main starts ***********/  
 int main(int argc, char *argv[]) {  
      int       sockfd,new_fd;    // ���� ���ϰ� Ŭ���̾�Ʈ ����  
      int      portnum;           // ��Ʈ��ȣ 
      struct sockaddr_in      server_addr;     // ���� �ּ� 
      struct sockaddr_in      client_addr;    // Ŭ���̾�Ʈ �ּ�
      int      cli_size,z;              // �ּ��� ����  
      pthread_t      thr;               // ������ �ĺ�
      int      yes=1;  
       addr a;                        
      printf("\n\t*-*-*-*SERVER STARTED*-*-*-*\n");  
      // ��Ʈ ��ȣ ����  
  if( argc == 2 )       
       portnum = atoi(argv[1]);  
  else   
      portnum = PORT; // ����Ʈ ��Ʈ ��ȣ 
      printf("PORT NO.:\t%d\n",portnum);  
      h = MakeEmpty( NULL );          // ����Ʈ�� ���
      // ���� ���� ����  
      server_addr.sin_family=AF_INET;            
      server_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
      server_addr.sin_port=htons(portnum);  
       printf("IP ADDRESS:\t%s\n",inet_ntoa(server_addr.sin_addr));  
      // ���� ����  
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
      // ���� ���ε�  
   if(bind(sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr))==-1){  
           printf("binding failed\n");      
      exit(1);}  
   else   
      printf("binding\t\tsuccess.\n\n");  
 printf("\t\tPRESS CTRL+z TO VIEW ONLINE CLIENTS\n\n");  
      // ���� ������  
      listen(sockfd, BACKLOG);  
      printf("waiting for clients......\n");  
   if (signal(SIGINT,(void *)Quitproc)==0)     // �ñ׳� �ڵ鷯  
   if(signal(SIGTSTP, zzz)==0)               // �ñ׳� �ڵ鷯
   while(1){  
             cli_size=sizeof(struct sockaddr_in);   
             new_fd = accept(sockfd, (struct sockaddr *)&client_addr,&cli_size); // Ŭ���̾�Ʈ ������ �޾Ƶ���   
             a =h ;  
           // ����� �̸� �Է�  
           bzero(username,10);            
           if(recv(new_fd,username,sizeof(username),0)>0);  
             username[strlen(username)-1]=':';  
             printf("\t%d->%s JOINED chatroom\n",new_fd,username);  
             sprintf(buffer,"%s IS ONLINE\n",username);  
             Insert( new_fd,username, h, a );  // ���ο� Ŭ���̾�Ʈ ������ ����Ʈ�� ����  
             a = a->next;  
           // ��� Ŭ���̾�Ʈ���� �� Ŭ���̾�Ʈ�� ������ �˸�   
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
            pthread_create(&thr,NULL,server,(void*)&args);   // ������ Ŭ���̾�Ʈ�鿡 ���� ������ ����   
            pthread_detach(thr);  
       }   
  DeleteList(h);  // ������ ���� �� �� ��� Ŭ���̾�Ʈ ����Ʈ�� ����  
  close(sockfd);  
 } 
      
 void *server(void * arguments){  
  struct Node *args=arguments;  
  char     buffer[MAXDATALEN],ubuf[50],uname[10]; // ������ ������ �޼����� ���� ����   
  char *strp;            
  char      *msg = (char *) malloc(MAXDATALEN);  
  int      ts_fd,x,y;  
  int      sfd,msglen;  
  ts_fd = args->port;     
  strcpy(uname,args->username);   
  addr      a;  
       // ������ Ŭ���̾�Ʈ ������ ����
       a =h ;  
           do{  
           a = a->next;  
           sprintf( ubuf," %s is online\n",a->username );  
           send(ts_fd,ubuf,strlen(ubuf),0);  
           } while( a->next != NULL );  
       // ä�� ����  
   while(1){  
        bzero(buffer,256);  
        y=recv(ts_fd,buffer,MAXDATALEN,0);  
       if (y==0)   
       goto jmp;  
        // Ŭ���̾�Ʈ�� ä�ù��� ���� ��� 
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
      // ��� Ŭ���̾�Ʈ���� �޼����� ����
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
