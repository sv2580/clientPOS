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
char login[100];
char *priatelia[100];
static int pocetPriatelov;

void hlavneMenu();

void pozrietZiadosti();

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


void registracia() {
    int n;
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

void nacitajPolePriatelov() {
    FILE *subor;
    subor = fopen("requests.txt", "r");
    if (subor == NULL) {
        fputs("Error at opening File!", stderr);
        exit(1);
    }
    printf("Súbor otvorený \n");
    int nasloSa = 0;
    int indexRiadku;
    char line[256];
    char riadok1[256];
    char riadok2[256];
    int pocetRiadkov = 0;
    while (fscanf(subor, "%s", line) != EOF) {
        if (pocetRiadkov % 2 == 0) {
            strcpy(riadok1, line);
        } else {
            strcpy(riadok2, line);
            if (strcmp(riadok1, login) == 0) {
                priatelia[pocetPriatelov] = riadok2;
                pocetPriatelov++;
            }
            if (strcmp(riadok2, login) == 0) {
                priatelia[pocetPriatelov] = riadok1;
                pocetPriatelov++;
            }
        }
        pocetRiadkov++;
    }
    fclose(subor);
}

void prihlasenie() {
    int n;
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
    //nacitajPolePriatelov();
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
            if (spravneHeslo == 1) {
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

void poslatZiadost() {
    char contact[100];
    int n;
    printf("Zadajte login používateľa: \n");
    bzero(contact, 100);
    fgets(contact, 99, stdin);
    n = write(sockfd, contact, strlen(contact));
    if (n < 0) {
        perror("Error writing to socket");
        return;
    }

    int nasielSa;
    n = read(sockfd, &nasielSa, sizeof(nasielSa));
    if (n < 0) {
        perror("Error reading from socket");
        return;
    }
    if (nasielSa == 1) {
        printf("Požiadavka o pridanie do priateľov bola odoslaná.\n");
    } else {
        printf("Používateľ neexistuje.\n");
    }

    hlavneMenu();
}

void pridatDoPriatelov(char *contact) {
    FILE *subor;
    subor = fopen("friends.txt", "a");
    if (subor == NULL) {
        fputs("Error at opening File!", stderr);
        exit(1);
    }
    fprintf(subor, "%s\n", contact);
    fprintf(subor, "%s\n", login);

    fclose(subor);

    priatelia[pocetPriatelov] = contact;
    pocetPriatelov++;
}

void pozrietZiadosti() {
    while (1) {
        FILE *subor;
        subor = fopen("requests.txt", "r");
        if (subor == NULL) {
            fputs("Error at opening File!", stderr);
            exit(1);
        }
        printf("Súbor otvorený \n");

        int nasloSa = 0;
        int indexRiadku;
        char line[256];
        char contact[256];
        int pocetRiadkov = 0;
        while (fscanf(subor, "%s", line) != EOF) {
            if (pocetRiadkov % 2 == 0) {
                strcpy(contact, line);
            } else {
                if (strcmp(contact, login) == 0) {
                    printf("%s vás žiada o priateľstvo", line);
                    indexRiadku = pocetRiadkov - 1;
                    nasloSa = 1;
                    break;
                }
            }
            pocetRiadkov++;
        }
        fclose(subor);
        if (nasloSa == 0) {
            printf("Nemáte už žiadne nové žiadosti.\n");
            break;
        }
        int poziadavka;
        printf("Chcete prijať túto žiadosť? 0/1");
        scanf("%d", &poziadavka);
        getchar();
        if (poziadavka == 1) {
            pridatDoPriatelov(line);
        }

        FILE *povodnySubor, *novySubor;

        int riadok = 0;
        char string[256];
        povodnySubor = fopen("requests.txt", "r");
        if (!povodnySubor) {
            printf("Error at opening File!\n");
            return;
        }
        novySubor = fopen("docasny.txt", "w");
        if (!novySubor) {
            printf("Error at opening File!\n");
            fclose(povodnySubor);
            return;
        }

        while (!feof(povodnySubor)) {
            strcpy(string, "\0");
            fgets(string, 256, povodnySubor);
            if (!feof(povodnySubor)) {
                riadok++;
                if (riadok != indexRiadku && riadok != indexRiadku + 1) {
                    fprintf(novySubor, "%s", string);
                }
            }
        }
        fclose(povodnySubor);
        fclose(novySubor);
        remove("requsts.txt");        // remove the original file
        rename("docasny.txt", "requsts.txt");    // rename the temporary file to original name
    }
}

void odobratPriatela() {
    int n;

    if (pocetPriatelov == 0) {
        printf("Nemáte žiadných priateľov. \n");
        return;
    }
    printf("Váš zoznam priateľov: \n");
    for (int i = 0; i < pocetPriatelov; ++i) {
        printf("%d. %s", i + 1, priatelia[i]);
    }
    printf("Zadajte číslo priatela, ktorého chcete odobrať: \n");
    int priatel;
    scanf("%d", &priatel);
    getchar();
    char contact[100];
    strcpy(contact,priatelia[priatel-1]);
    n = write(sockfd, priatelia[priatel - 1], strlen(priatelia[priatel - 1]));
    if (n < 0) {
        perror("Error writing to socket");
        return;
    }
    printf("Požiadavka bola spracovaná.");

    int nasielSa;
    n = read(sockfd, &nasielSa, sizeof(nasielSa));
    if (n < 0) {
        perror("Error reading from socket");
        return;
    }
    if (nasielSa == 1) {
        FILE *subor;
        subor = fopen("friends.txt", "r");
        if (subor == NULL) {
            fputs("Error at opening File!", stderr);
            exit(1);
        }
        printf("Súbor otvorený \n");

        int nasloSa = 0;
        int indexRiadku1;
        int indexRiadku2;

        char line[256];
        char riadok1[256];
        char riadok2[256];
        int pocetRiadkov = 0;
        while (fscanf(subor, "%s", line) != EOF) {
            if (pocetRiadkov % 2 == 0) {
                strcpy(riadok1, line);
            } else {
                strcpy(riadok2, line);
                if (strcmp(riadok1, login) == 0 && strcmp(riadok2,contact) == 0) {
                    indexRiadku1 = pocetRiadkov - 1;
                    indexRiadku2 = pocetRiadkov;
                } else if (strcmp(riadok2, login) == 0 && strcmp(riadok1,contact) == 0) {
                    indexRiadku1 = pocetRiadkov - 1;
                    indexRiadku2 = pocetRiadkov;
                }
            }
            pocetRiadkov++;
        }
        fclose(subor);

        FILE *povodnySubor, *novySubor;

        int riadok = 0;
        char string[256];
        povodnySubor = fopen("requests.txt", "r");
        if (!povodnySubor) {
            printf("Error at opening File!\n");
            return;
        }
        novySubor = fopen("docasny.txt", "w");
        if (!novySubor) {
            printf("Error at opening File!\n");
            fclose(povodnySubor);
            return;
        }

        while (!feof(povodnySubor)) {
            strcpy(string, "\0");
            fgets(string, 256, povodnySubor);
            if (!feof(povodnySubor)) {
                riadok++;
                if (riadok != indexRiadku1 && riadok != indexRiadku2) {
                    fprintf(novySubor, "%s", string);
                }
            }
        }
        fclose(povodnySubor);
        fclose(novySubor);
        remove("requsts.txt");        // remove the original file
        rename("docasny.txt", "requsts.txt");    // rename the temporary file to original name

        printf("Používateľ bol odobraný z priateľov.\n");
    } else {
        printf("Používateľ neexistuje.\n");
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
        printf("[6.] Pridať si úžívateľa do priateľov\n");
        printf("[7.] Pozrieť si žiadosti o priateľstvo\n");
        printf("[8.] Odobranie priatela \n");
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
        } else if (poziadavka == 6) {
            n = write(sockfd, &poziadavka, sizeof(poziadavka));
            if (n < 0) {
                perror("Error writing to socket");
                return;
            }
            poslatZiadost();
        } else if (poziadavka == 7) {
            n = write(sockfd, &poziadavka, sizeof(poziadavka));
            if (n < 0) {
                perror("Error writing to socket");
                return;
            }
            pozrietZiadosti();
        } else if (poziadavka == 8) {
            n = write(sockfd, &poziadavka, sizeof(poziadavka));
            if (n < 0) {
                perror("Error writing to socket");
                return;
            }
            odobratPriatela();
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
    pocetPriatelov = 0;
    pthread_t klient2;
    hlavneMenu();
    while (1) {
        usleep(1);
    }
}
