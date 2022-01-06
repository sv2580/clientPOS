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
char priatelia[100][100];
static int pocetPriatelov;

int vlozitDoSuboruData(char *riadok, int n, char *nazovSuboru) {
    char buffer[1025];
    sprintf(buffer, "%s", riadok);
    FILE *subor;
    subor = fopen(nazovSuboru, "a");
    if (subor == NULL) {
        fputs("Error at opening File!", stderr);
        return -1;
    }
    fwrite(buffer, 1, n, subor);

    fclose(subor);
    return 1;
}

void trim(char *string, int dlzka) {
    int i;
    for (i = 0; i < dlzka; i++) { // trim \n
        if (string[i] == '\n') {
            string[i] = '\0';
            break;
        }

    }

}

void hlavneMenu();

void pozrietZiadosti();

void odobratPriatelaZpola(int index) {
    pocetPriatelov--;

    for (int i = index; i < pocetPriatelov; ++i) {
        strcpy(priatelia[i], priatelia[i + 1]);
    }
    for (int i = 0; i < pocetPriatelov; ++i) {
        printf("%s \n", priatelia[i]);
    }
}

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

void *posielanieSuborov() {
    char nazovSuboru[100];
    char contact[100];
    bzero(nazovSuboru, 100);
    int n;

    printf("Prosím zadajte meno súboru: ");
    fgets(nazovSuboru, 99, stdin);
    trim(nazovSuboru, 100);
    printf("%s \n", nazovSuboru);

    FILE *subor;
    subor = fopen(nazovSuboru, "rb");
    char buffer[256] = {0};
    FILE *fp;

    int counter = 0;
    fp = fopen(nazovSuboru, "r");
    while (fgetc(fp) != EOF)
        counter++;
    printf("there are %d letters", counter);
    fclose(fp);

    n=write(sockfd,&counter,sizeof (counter));
    if (n < 0) {
        perror("Error writing to socket");
        return NULL;
    }
    printf("som tu \n");
    if (subor != NULL) {
        while ((n = fread(buffer, 1, sizeof(buffer), subor)) >= counter) {
            send(sockfd, buffer, n, 0);
            printf("%s %d \n", buffer, n);
        }
    }

    fclose(subor);

    printf("Prosím zadajte kontakt: ");
    bzero(contact, 100);
    scanf("%s", contact);
    n =  send(sockfd, contact, strlen(contact), 0);
    if (n < 0) {
        perror("Error writing to socket");
        return NULL;
    }

    printf("zadaný kontakt %s \n",contact);
    int nasielSa;
    n = read(sockfd, &nasielSa, sizeof(nasielSa));
    if (n < 0) {
        perror("Error writing to socket");
        return NULL;
    }

    if(nasielSa == 1){
        printf("Používateľ je prihlásený a súbor mu bude odoslaný. \n");
    } else {
        printf("Používateľ nie je prihlásený alebo neexistuje. \n");

    }

    hlavneMenu();

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

void *prijatieSuboru() {
    char nazovSuboru[100];
    char buffer[256];
    bzero(buffer, 256);
    bzero(nazovSuboru, 100);

    printf("Prosím zadajte názov, pod ktorým sa súbor uloží: ");
    fgets(nazovSuboru, 99, stdin);
    trim(nazovSuboru, 100);

    int counter;
   int n = read(sockfd, &counter, sizeof(counter));
    if (n < 0) {
        perror("Error writing to socket");
        return NULL;
    }
    printf("%d", counter);
    int tot = 0;

    while (tot < counter) {
        (n = recv(sockfd, buffer, 1024, 0));
        tot += n;
        printf("%s %d \n", buffer, n);
        int a = vlozitDoSuboruData(buffer, n, nazovSuboru);
        if (a != 1) {
            printf("Chyba");
        }
    }

    printf("Súbor prijatý: %d\n");
    if (n < 0) {
        perror("Receiving");
    }
}


void nacitajPolePriatelov() {
    int koniec = 0;
    int n;
    int index = 0;
    printf("%d \n", koniec);

    while (koniec == 0) {
        char contact[100];
        bzero(contact, 100);
        n = read(sockfd, contact, 99);
        if (n < 0) {
            perror("Error reading from socket");
            return;
        }
        index++;
        pocetPriatelov++;
        printf("nacitanie: %s \n", contact);
        strcpy(priatelia[index], contact);

        n = read(sockfd, &koniec, sizeof(koniec));
        if (n < 0) {
            perror("Error reading from socket");
            return;
        }
        printf("%d \n", koniec);

    }

    for (int i = 0; i < pocetPriatelov; i++) {
        printf("%s\n", priatelia[i]);
    }
}


void skupinovaKonverzacia() {
    int skonci = 0;

    while (skonci == 0) {
        int n;
        char contact[100];
        char buffer[256];

        bzero(contact, 100);
        printf("Please enter contact: ");
        scanf("%s", contact);


        n = write(sockfd, contact, strlen(contact));
        if (n < 0) {
            perror("Error writing to socket");
            return;
        }
        if (strcmp(contact, "exit") == 0) {
            skonci = 1;
            break;
        }
        printf("pred naslosa - client");
        int nasielSA = 0;
        n = read(sockfd, &nasielSA, sizeof(nasielSA));
        if (n < 0) {
            perror("Error reading from socket");
            return;
        }
        printf("pred if naslo sa - client");
        if (nasielSA == 1) {
            printf("Pouzivatel sa nasiel");
        } else {
            printf("Pouzivatel sa nenasiel");
        }
    }

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
    //pthread_t klient2;
    //pthread_create(&klient2, NULL, dostatnSpravu, NULL);

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

void pozrietZiadosti() {
    int nasielSa = 0;
    int n;
    n = read(sockfd, &nasielSa, sizeof(nasielSa));
    if (n < 0) {
        perror("Error reading from socket");
        return;
    }
    while (nasielSa == 1) {
        char contact[100];
        bzero(contact, 100);
        n = read(sockfd, contact, 99);
        if (n < 0) {
            perror("Error reading from socket");
            return;
        }
        int poziadavka;
        printf("Žiadosť o priateľstvo od používateľa %s: 0/1", contact);
        scanf("%d", &poziadavka);
        getchar();
        n = write(sockfd, &poziadavka, sizeof(poziadavka));
        if (n < 0) {
            perror("Error writing to socket");
            return;
        }

        if (poziadavka == 1) {
            printf("Žiadosť bola prijatá.\n");
            pocetPriatelov++;
        } else {
            printf("Žiadosť bola ignorovaná.\n");
        }
        n = read(sockfd, &nasielSa, sizeof(nasielSa));
        if (n < 0) {
            perror("Error reading from socket");
            return;
        }
        printf("%d \n", nasielSa);
    }

    printf("Nemáte už žiadne iné žiadosti.\n");
    hlavneMenu();


}

void odobratPriatela() {
    int n;

    if (pocetPriatelov == 0) {
        printf("Nemáte žiadných priateľov. \n");
        return;
    }
    printf("Váš zoznam priateľov: \n");
    for (int i = 0; i < pocetPriatelov; ++i) {
        printf("[%d.] %s \n", i + 1, priatelia[i]);
    }
    printf("Zadajte login priatela, ktorého chcete odobrať: \n");
    int priatel;
    scanf("%d", &priatel);
    getchar();
    char contact[100];
    strcpy(contact, priatelia[priatel - 1]);
    printf("Zadaný kontakt %s \n", contact);
    odobratPriatelaZpola(priatel - 1);
    n = write(sockfd, contact, strlen(contact));
    if (n < 0) {
        perror("Error writing to socket");
        return;
    }
    printf("Požiadavka bola spracovaná.");

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
        printf("[9.] Vytvor skupinu \n");
        printf("[10.] Posielanie dát \n");
        printf("[11.] Prijatie dát \n");
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
        } else if (poziadavka == 9) {
            n = write(sockfd, &poziadavka, sizeof(poziadavka));
            if (n < 0) {
                perror("Error writing to socket");
                return;
            }
            skupinovaKonverzacia();
        } else if (poziadavka == 10) {
            n = write(sockfd, &poziadavka, sizeof(poziadavka));
            if (n < 0) {
                perror("Error writing to socket");
                return;
            }
            posielanieSuborov();
        } else if (poziadavka == 11) {
            n = write(sockfd, &poziadavka, sizeof(poziadavka));
            if (n < 0) {
                perror("Error writing to socket");
                return;
            }
            pthread_t klient4;
            pthread_create(&klient4, NULL, prijatieSuboru, NULL);

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
