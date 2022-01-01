#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

char login[100];
int sockfd = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *posliSpravu() {
    int skonci = 0;
    while (skonci == 0) {
        int n;
        char contact[100];
        char buffer[256];

        printf("Please enter contact: ");
        bzero(contact, 100);
        printf("%s", "> ");
        fflush(stdout);
        scanf("%s", contact);
        n = write(sockfd, contact, strlen(contact));
        if (n < 0) {
            perror("Error writing to socket");
            return NULL;
        }
        if (strcmp(contact, "exit") == 0) {
            skonci = 1;
            break;
        }

        printf("Please enter a message to send to %s: ", contact);
        printf("%s", "> ");
        fflush(stdout);
        scanf("%s", buffer);

        n = write(sockfd, buffer, strlen(buffer));
        if (n < 0) {
            perror("Error writing to socket");
            return NULL;
        }
        if (strcmp(contact, "exit") == 0) {
            skonci = 1;
            break;
        }

    }
}

int main(int argc, char *argv[]) {
    int n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];

    if (argc < 3) {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        return 1;
    }

    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr, "Error, no such host\n");
        return 2;
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy(
            (char *) server->h_addr,
            (char *) &serv_addr.sin_addr.s_addr,
            server->h_length
    );
    serv_addr.sin_port = htons(atoi(argv[2]));

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error creating socket");
        return 3;
    }

    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("Error connecting to socket");
        return 4;
    }

    //

    printf("Please enter login: ");
    bzero(login, 100);
    fgets(login, 99, stdin);
    n = write(sockfd, login, strlen(login));
    if (n < 0) {
        perror("Error writing to socket");
        return 5;
    }
    pthread_t klient;

    pthread_create(&klient, NULL, posliSpravu, NULL);
    pthread_join(klient, NULL);


    /* printf("Pre poslanie spravy zadajte - 1");
     char zadane = getchar();

     if(zadane == '1'){
         pthread_join(klient,NULL);
     }*/



    /*
    printf("Please enter a message: ");


    bzero(buffer,256);
    fgets(buffer, 255, stdin);

    n = write(sockfd, buffer, strlen(buffer));
    if (n < 0)
    {
        perror("Error writing to socket");
        return 5;
    }

    bzero(buffer,256);
    n = read(sockfd, buffer, 255);
    if (n < 0)
    {
        perror("Error reading from socket");
        return 6;
    }

    printf("%s\n",buffer);
    close(sockfd);

    return 0;*/

}
