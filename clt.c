/*
    Simple udp client
    Silver Moon (m00n.silv3r@gmail.com)
 */
#include<stdio.h> //printf
#include<string.h> //memset
#include<stdlib.h> //exit(0);
#include<arpa/inet.h>
#include<sys/socket.h>

#define SERVER "127.0.0.1"
#define longbuff 512  //max buffer length
#define PORT 8888   //port number to send 
struct sockaddr_in *autre;
int len = sizeof (struct sockaddr_in);

void die(char *s) {
    perror(s);
    exit(1);
}

int main(void) {
    struct sockaddr_in si_autre;
    int s, i, slen = sizeof (si_autre);
    char buf[longbuff];
    char message[longbuff];

    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        die("socket");
    }

    memset((char *) &si_autre, 0, sizeof (si_autre));
    si_autre.sin_family = AF_INET;
    si_autre.sin_port = htons(PORT);

    if (inet_aton(SERVER, &si_autre.sin_addr) == 0) {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }

    while (1) {


        printf("Enter message : ");
        char* fgets(char *string, int length, FILE * stream);

        //send the message
        if (sendto(s, message, strlen(message), 0, (struct sockaddr *) &si_autre, slen) == -1) {
            die("sendto()");
        }

        //receive a reply and print it
        //clear the buffer by filling null, it might have previously received data
        memset(buf, '\0', longbuff);
        //try to receive some data, this is a blocking call
        if (recvfrom(s, buf, longbuff, 0, (struct sockaddr *) &si_autre, &slen) == -1) {
            die("recvfrom()");
        }

        puts(buf);


        close(s);
        return 0;
    }
}

void
TraitementClavier(int sock) /* Socket E/S */ {
    char buf[256];
    int taillemessage;

    taillemessage = read(0, buf, 256);
    /*  Envoyer a tous les client la taille du message puis le message */
    sendto(sock, &taillemessage, sizeof (taillemessage), 0,
            (struct sockaddr *) autre, sizeof (struct sockaddr));

    sendto(sock, buf, taillemessage, 0,
            (struct sockaddr *) autre, sizeof (struct sockaddr));

}
