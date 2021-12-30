#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
//neviem zatial co tu ideme robit


char login[100];
int sockfd = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int posliSpravu() {
    int n;
    char contact[100];
    char buffer[256];
    printf("Please enter contact: ");
    bzero(contact,100);
    fgets(contact, 99, stdin);
    n = write(sockfd, contact, strlen(contact));
    if (n < 0)
    {
        perror("Error writing to socket");
        return 5;
    }


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


    return 0;


}

int main(int argc, char *argv[])
{
    int sockfd, n;
    struct sockaddr_in serv_addr;
    struct hostent* server;

    char buffer[256];

    if (argc < 3)
    {
        fprintf(stderr,"usage %s hostname port\n", argv[0]);
        return 1;
    }

    server = gethostbyname(argv[1]);
    if (server == NULL)
    {
        fprintf(stderr, "Error, no such host\n");
        return 2;
    }

    bzero((char*)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy(
            (char*)server->h_addr,
            (char*)&serv_addr.sin_addr.s_addr,
            server->h_length
    );
    serv_addr.sin_port = htons(atoi(argv[2]));

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("Error creating socket");
        return 3;
    }

    if(connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Error connecting to socket");
        return 4;
    }

    //

    printf("Please enter login: ");
    bzero(login,100);
    fgets(login, 99, stdin);
    n = write(sockfd, login, strlen(login));
    if (n < 0)
    {
        perror("Error writing to socket");
        return 5;
    }

    printf("Pre poslanie spravy zadajte - 1");
    char zadane = getchar();
    pthread_t klient;
    pthread_create(&klient, NULL, posliSpravu, NULL);

    if(zadane == '1'){
        pthread_join(klient,NULL);
    }

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
