#include<stdio.h>  
 #include<sys/types.h>  
 #include<sys/socket.h>  
 #include<netinet/in.h>  
 #include<stdlib.h>  
 #include<string.h>  

struct LLIST {
struct LLNODE *head, *tail;
int size;
};
void list_init(struct LLIST *ll) {
ll->head = ll->tail = NULL;
ll->size = 0;
}
int list_insert(struct LLIST *ll, struct THREADINFO *thr_info) {
if(ll->size == CLIENTS) return -1;
if(ll->head == NULL) {
ll->head = (struct LLNODE *)malloc(sizeof(struct LLNODE));
ll->head->threadinfo = *thr_info;
ll->head->next = NULL;
ll->tail = ll->head;
}
else {
ll->tail->next = (struct LLNODE *)malloc(sizeof(struct LLNODE));
ll->tail->next->threadinfo = *thr_info;
ll->tail->next->next = NULL;
ll->tail = ll->tail->next;
}
ll->size++;
return 0;
}
 
int list_delete(struct LLIST *ll, struct THREADINFO *thr_info) {
struct LLNODE *curr, *temp;
if(ll->head == NULL) return -1;
if(compare(thr_info, &ll->head->threadinfo) == 0) {
temp = ll->head;
ll->head = ll->head->next;
if(ll->head == NULL) ll->tail = ll->head;
free(temp);
ll->size--;
return 0;
}
for(curr = ll->head; curr->next != NULL; curr = curr->next) {
if(compare(thr_info, &curr->next->threadinfo) == 0) {
temp = curr->next;
if(temp == ll->tail) ll->tail = curr;
curr->next = curr->next->next;
free(temp);
ll->size--;
return 0;
}
}
return -1;
}
 
void list_dump(struct LLIST *ll) {
struct LLNODE *curr;
struct THREADINFO *thr_info;
printf("Connection count: %d\n", ll->size);
for(curr = ll->head; curr != NULL; curr = curr->next) {
thr_info = &curr->threadinfo;
printf("[%d] %s\n", thr_info->sockfd, thr_info->alias);
}
}

int sockfd, newfd;
struct THREADINFO thread_info[CLIENTS];
struct LLIST client_list;



 void error(char *msg)  
 {  
 perror(msg);  
 exit(1);  
 }  
 int main(int argc,char *argv[])  
 {  

list_init(&client_list);//init list

 int sockfd,newsockfd,portno,clilen,k;  
 char buffer[256];  
 pid_t childpid;  
 struct sockaddr_in serv_addr,cli_addr;  
 int n;  
 if(argc<2)  
 {  
 fprintf(stderr,"error,no port provided\n");  
 exit(1);  
 }  
 sockfd=socket(AF_INET,SOCK_STREAM,0);  
 if(sockfd<0)  
 {  
 error("error opening socket");  
 }  
 bzero((char *)&serv_addr,sizeof(serv_addr));  
 portno=atoi(argv[1]);  
 serv_addr.sin_family=AF_INET;  
 serv_addr.sin_addr.s_addr=INADDR_ANY;  
 serv_addr.sin_port=htons(portno);  
 if(bind(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) <0)  
 {  
 error("error on binding");  
 }  
 k=listen(sockfd,5);  
 for (;;) {  
 clilen=sizeof(cli_addr);  
 if(k<0) {  
 error("error on inding the network");  
 exit(1);  
 }  
 printf("server is running...........................\n         waiting for the client on the port number: %d\n",portno);   
 newsockfd=accept(sockfd,(struct sockaddr*)&cli_addr,&clilen);  
 if(newsockfd<0)  
 {  
 error("error on accept");  
 }  
 printf("Connection accepted...\n");   
  if ((childpid = fork()) == 0) { //creating a child process   
   close(sockfd);    
 //stop listening for new connections by the main process.    
 //the child will continue to listen.    
 //the main process now handles the connected client.   
   for (;;) {   
 bzero(buffer,256);  
 n=read(newsockfd,buffer,255);  
 if(n<0)  
 {  
 error("error reading from socket");  
 }  
 else  
 {  
 printf("client: %s\n",buffer);  
 }  
 bzero(buffer,256);  
 printf("server->enter your message: %s",buffer);  
 fgets(buffer,256,stdin);  
 n=write(newsockfd,buffer,strlen(buffer));  
 if(n<0)  
 {  
 error("error writing socket");  
 }  
   }   
  }   
  close(newsockfd);   
  }   
 }
