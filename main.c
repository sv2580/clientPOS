#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

int sockfd = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static int jePrihlaseny;


void *dostatnSpravu() {
    int skonci = 0;
    printf("Dostávam správy \n");
    while (1) {

        int n;

        char buffer[256];
        n = recv(sockfd, buffer, 255, 0);
        if (n < 0) {
            perror("Error reading from socket");
            return NULL;
        }
        printf("Here is the message: %s\n", buffer);


    }
}

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

void hlavneMenu();

void registracia() {
    int n;
    char login[100];
    char password[100];
    char buffer[256];

    while (1) {
        printf("Please enter your login: ");
        bzero(login, 100);
        fgets(login, 99, stdin);
        n = write(sockfd, login, strlen(login));
        if (n < 0) {
            perror("Error writing to socket");
            return;
        }

        bzero(buffer, 256);
        int nasloSa;
        n = read(sockfd, &nasloSa, sizeof(nasloSa));
        if (n < 0) {
            perror("Error reading from socket");
            return;
        }

        if (nasloSa == 1) {
            printf("Please enter new login, the login is already used.\n");
        } else {
            break;
        }
    }
    printf("Please enter password.\n");
    bzero(password, 100);
    fgets(password, 99, stdin);
    n = write(sockfd, password, strlen(password));
    if (n < 0) {
        perror("Error writing to socket");
        return;
    }
    printf("Teraz sa môžete prihlásiť! \n");
    hlavneMenu();

}

void prihlasenie() {
    int n;
    char login[100];
    char password[100];
    char buffer[256];

    while (1) {

        printf("Please enter your login: ");
        bzero(login, 100);
        fgets(login, 99, stdin);
        n = write(sockfd, login, strlen(login));
        if (n < 0) {
            perror("Error writing to socket");
            return;
        }

        printf("Please enter password.\n");
        bzero(password, 100);
        fgets(password, 99, stdin);
        n = write(sockfd, password, strlen(password));
        if (n < 0) {
            perror("Error writing to socket");
            return;
        }

        bzero(buffer, 256);
        int spravneHeslo;
        n = read(sockfd, &spravneHeslo, sizeof(spravneHeslo));
        if (n < 0) {
            perror("Error reading from socket");
            return;
        }

        if (spravneHeslo == 0) {
            printf("Udaje sa nenasli\n");
        } else {
            jePrihlaseny = 1;

            break;
        }


    }
    pthread_t klient2;
    pthread_create(&klient2, NULL, dostatnSpravu, NULL);

    hlavneMenu();
}

void odhlasenie() {
    if (jePrihlaseny == 1) {
        jePrihlaseny = 0;
    }
    printf("Boli ste odhlásený.\n");
    hlavneMenu();

}

void zrusitUcet() {
    char password[100];
    int n;
    if (jePrihlaseny == 1) {
        printf("Pre zrušenie účtu zadajte heslo \n");
        while (1) {
            bzero(password, 100);
            fgets(password, 99, stdin);
            n = write(sockfd, password, strlen(password));
            if (n < 0) {
                perror("Error writing to socket");
                return;
            }

            int spravneHeslo;
            n = read(sockfd, &spravneHeslo, sizeof(spravneHeslo));
            if (n < 0) {
                perror("Error reading from socket");
                return;
            }
            if(spravneHeslo == 1){
                printf("Váš účet bol zrušený.\n");
                jePrihlaseny = 0;
                break;
            } else {
                printf("Nesprávne heslo \n");
            }

        }
    }
    hlavneMenu();
}

void hlavneMenu() {
    int poziadavka, n;
    if (jePrihlaseny == 0) {
        printf("[1.] Registrácia \n");
        printf("[2.] Prihlásenie \n");
        scanf("%d", &poziadavka);
        getchar();
        if (poziadavka == 1) {
            n = write(sockfd, &poziadavka, sizeof(poziadavka));
            if (n < 0) {
                perror("Error writing to socket");
                return;
            }
            registracia();
        } else {
            n = write(sockfd, &poziadavka, sizeof(poziadavka));
            if (n < 0) {
                perror("Error writing to socket");
                return;
            }
            prihlasenie();
        }

    } else {
        printf("[3.] Písanie správ \n");
        printf("[4.] Odhlásenie \n");
        printf("[5.] Zmazať účet \n");

        scanf("%d", &poziadavka);
        getchar();
        if (poziadavka == 3) {
            n = write(sockfd, &poziadavka, sizeof(poziadavka));
            if (n < 0) {
                perror("Error writing to socket");
                return;
            }
            pthread_t klient;
            pthread_create(&klient, NULL, posliSpravu, NULL);
        } else if (poziadavka == 4) {
            n = write(sockfd, &poziadavka, sizeof(poziadavka));
            if (n < 0) {
                perror("Error writing to socket");
                return;
            }
            odhlasenie();
        } else if (poziadavka == 5) {
            n = write(sockfd, &poziadavka, sizeof(poziadavka));
            if (n < 0) {
                perror("Error writing to socket");
                return;
            }
            zrusitUcet();
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
    jePrihlaseny = 0;

    pthread_t klient2;
    hlavneMenu();
    while (1) {
        usleep(1);
    }
}
