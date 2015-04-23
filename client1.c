#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <netdb.h>
#include <unistd.h>
#include <pthread.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#define SERVERIP "127.0.0.1"
#define SERVERPORT 8080

#define BUFFSIZE 1024
#define USERLEN 32
#define CMNDLEN 16
#define LINEBUFF 2048

struct PACKET {
    char comande[CMNDLEN]; // instruction
    char user[USERLEN]; // user
    char buff[BUFFSIZE]; // payload
};

struct USER {
    int sockfd; // user's socket descriptor
    char user[USERLEN]; // user's name
};

struct THREADINFO {
    pthread_t thread_ID; // thread's pointer
    int sockfd; // socket file descriptor
};

int isconnected, sockfd;
char comande[LINEBUFF];
struct USER me;

int connect_with_server();
void setuser(struct USER *me);
void logout(struct USER *me);
void login(struct USER *me);
void *receiver(void *param);
void sendtoall(struct USER *me, char *msg);
void sendtouser(struct USER *me, char * target, char *msg);

int main(int argc, char **argv) {
    int sockfd, userlen;
    
     printf ("********* Welcome to mini chat cote client *********\n");
  printf( "  Les commande sont : \n"
          "      login puis nom utilisateur pour connecter au serveur \n"
          "       pour envoyer un message a un client taper : send utilisateur puis message \n"
          "      quiter \n\n\n");
printf("-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*Client*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-\n");

    
    
    
    memset(&me, 0, sizeof (struct USER));
    while (gets(comande)) {
        if (!strncmp(comande, "quiter", 4)) {
            logout(&me);
            break;
        }
      /*  if (!strncmp(comande, "help", 4)) {
            FILE *fin = fopen("help.txt", "r");
            if (fin != NULL) {
                while (fgets(comande, LINEBUFF - 1, fin)) puts(comande);
                fclose(fin);
            } else {
                fprintf(stderr, "Help file not found...\n");
            }
        }*/ else if (!strncmp(comande, "login", 5)) {
            char *ptr = strtok(comande, " ");
            ptr = strtok(0, " ");
            memset(me.user, 0, sizeof (char) * USERLEN);
            if (ptr != NULL) {
                userlen = strlen(ptr);
                if (userlen > USERLEN) ptr[USERLEN] = 0;
                strcpy(me.user, ptr);
            } else {
                strcpy(me.user, "Anonymous");
            }
            login(&me);
        } else if (!strncmp(comande, "user", 5)) {
            char *ptr = strtok(comande, " ");
            ptr = strtok(0, " ");
            memset(me.user, 0, sizeof (char) * USERLEN);
            if (ptr != NULL) {
                userlen = strlen(ptr);
                if (userlen > USERLEN) ptr[USERLEN] = 0;
                strcpy(me.user, ptr);
                setuser(&me);
            }
        } else if (!strncmp(comande, "send", 5)) {
            char *ptr = strtok(comande, " ");
            char temp[USERLEN];
            ptr = strtok(0, " ");
            memset(temp, 0, sizeof (char) * USERLEN);
            if (ptr != NULL) {
                userlen = strlen(ptr);
                if (userlen > USERLEN) ptr[USERLEN] = 0;
                strcpy(temp, ptr);
                while (*ptr) ptr++;
                ptr++;
                while (*ptr <= ' ') ptr++;
                sendtouser(&me, temp, ptr);
            }
        } else if (!strncmp(comande, "send", 4)) {
            sendtoall(&me, &comande[5]);
        } else if (!strncmp(comande, "logout", 6)) {
            logout(&me);
        } else fprintf(stderr, "Unknown comande...\n");
    }
    return 0;
}

void login(struct USER *me) {
    int recvd;
    if (isconnected) {
        fprintf(stderr, "You are already connected to server at %s:%d\n", SERVERIP, SERVERPORT);
        return;
    }
    sockfd = connect_with_server();
    if (sockfd >= 0) {
        isconnected = 1;
        me->sockfd = sockfd;
        if (strcmp(me->user, "Anonymous")) setuser(me);
        printf("Logged in as %s\n", me->user);
        printf("Receiver started [%d]...\n", sockfd);
        struct THREADINFO threadinfo;
        pthread_create(&threadinfo.thread_ID, NULL, receiver, (void *) &threadinfo);

    } else {
        fprintf(stderr, "Connection rejected...\n");
    }
}

int connect_with_server() {
    int newfd, err_ret;
    struct sockaddr_in serv_addr;
    struct hostent *to;

    /* generate address */
    if ((to = gethostbyname(SERVERIP)) == NULL) {
        err_ret = errno;
        fprintf(stderr, "gethostbyname() error...\n");
        return err_ret;
    }

    /* open a socket */
    if ((newfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        err_ret = errno;
        fprintf(stderr, "socket() error...\n");
        return err_ret;
    }

    /* set initial values */
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVERPORT);
    serv_addr.sin_addr = *((struct in_addr *) to->h_addr);
    memset(&(serv_addr.sin_zero), 0, 8);

    /* try to connect with server */
    if (connect(newfd, (struct sockaddr *) &serv_addr, sizeof (struct sockaddr)) == -1) {
        err_ret = errno;
        fprintf(stderr, "connect() error...\n");
        return err_ret;
    } else {
        printf("Connected to server at %s:%d\n", SERVERIP, SERVERPORT);
        return newfd;
    }
}

void logout(struct USER *me) {
    int sent;
    struct PACKET packet;
    if (!isconnected) {
        fprintf(stderr, "You are not connected...\n");
        return;
    }
    memset(&packet, 0, sizeof (struct PACKET));
    strcpy(packet.comande, "quiter");
    strcpy(packet.user, me->user);
    /* send request to close this connetion */
    sent = send(sockfd, (void *) &packet, sizeof (struct PACKET), 0);
    isconnected = 0;
}

void setuser(struct USER *me) {
    int sent;
    struct PACKET packet;
    if (!isconnected) {
        fprintf(stderr, "You are not connected...\n");
        return;
    }
    memset(&packet, 0, sizeof (struct PACKET));
    strcpy(packet.comande, "user");
    strcpy(packet.user, me->user);
    /* send request to close this connetion */
    sent = send(sockfd, (void *) &packet, sizeof (struct PACKET), 0);
}

void *receiver(void *param) {
    int recvd;
    struct PACKET packet;
    printf("Waiting here [%d]...\n", sockfd);
    while (isconnected) {
        recvd = recv(sockfd, (void *) &packet, sizeof (struct PACKET), 0);
        if (!recvd) {
            fprintf(stderr, "Connection lost from server...\n");
            isconnected = 0;
            close(sockfd);
            break;
        }
        if (recvd > 0) {
            printf("[%s]: %s\n", packet.user, packet.buff);
        }
        memset(&packet, 0, sizeof (struct PACKET));
    }
    return NULL;
}

void sendtoall(struct USER *me, char *msg) {
    int sent;
    struct PACKET packet;
    if (!isconnected) {
        fprintf(stderr, "You are not connected...\n");
        return;
    }
    msg[BUFFSIZE] = 0;
    memset(&packet, 0, sizeof (struct PACKET));
    strcpy(packet.comande, "send");
    strcpy(packet.user, me->user);
    strcpy(packet.buff, msg);
    /* send request to close this connetion */
    sent = send(sockfd, (void *) &packet, sizeof (struct PACKET), 0);
}

void sendtouser(struct USER *me, char *target, char *msg) {
    int sent, targetlen;
    struct PACKET packet;
    if (target == NULL) {
        return;
    }
    if (msg == NULL) {
        return;
    }
    if (!isconnected) {
        fprintf(stderr, "You are not connected...\n");
        return;
    }
    msg[BUFFSIZE] = 0;
    targetlen = strlen(target);
    memset(&packet, 0, sizeof (struct PACKET));
    strcpy(packet.comande, "send");
    strcpy(packet.user, me->user);
    strcpy(packet.buff, target);
    strcpy(&packet.buff[targetlen], " ");
    strcpy(&packet.buff[targetlen + 1], msg);
    /* send request to close this connetion */
    sent = send(sockfd, (void *) &packet, sizeof (struct PACKET), 0);
}
