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
static int cinnost;

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
    while (cinnost == 5 || cinnost == 8) {
        int n;
        char buffer[256];
        n = recv(sockfd, buffer, 255, 0);
        if (n < 0) {
            perror("Error reading from socket");
            return NULL;
        }
        printf("%s\n", buffer);
    }
}

void *posliSpravu() {
    int skonci = 0;
    printf("Som tu \n");
    while (skonci == 0) {
        int n;
        char contact[100];
        char buffer[500];
        char sprava[256];


        printf("Zadajte kontakt: ");
        bzero(contact, 100);
        fgets(contact, 99, stdin);
        trim(contact, 100);
        n = write(sockfd, contact, strlen(contact));
        if (n < 0) {
            perror("Error writing to socket");
            return NULL;
        }
        if (strcmp(contact, "exit") == 0) {
            skonci = 1;
            break;
        }

        printf("Zadajte správu pre %s: ", contact);
        fgets(sprava, 255, stdin);
        trim(sprava, 256);
        sprintf(buffer, "%s: %s", login, sprava);
        printf("%s \n", buffer);
        n = write(sockfd, buffer, strlen(buffer));

        if (n < 0) {
            perror("Error writing to socket");
            return NULL;
        }
        if (strcmp(sprava, "exit") == 0) {
            skonci = 1;
            break;
        }

    }
    hlavneMenu();

}

void *posliSifSpravu() {
    int skonci = 0;

    printf("Posielam šifrovanú správu \n");
    int n;
    char contact[100];
    char buffer[256];

    printf("Zadajte kontakt: ");
    bzero(contact, 100);
    printf("%s", "> ");
    fflush(stdout);
    scanf("%s", contact);

    n = write(sockfd, contact, strlen(contact));
    if (n < 0) {
        perror("Error writing to socket");
        return NULL;
    }

    int nasielSa;
    n = read(sockfd, &nasielSa, sizeof(nasielSa));
    if (n < 0) {
        perror("Error writing to socket");
        return NULL;
    }
    if (nasielSa == 0) {
        printf("Používateľ sa nenašiel.");
        hlavneMenu();
    }

    printf("Zadajte správu pre %s: ", contact);
    printf("%s", "> ");
    fflush(stdout);
    scanf("%s", buffer);
    n = write(sockfd, buffer, strlen(buffer));

    if (n < 0) {
        perror("Error writing to socket");
        return NULL;
    }

    int posun;
    printf("Zadajte číslo: \n ");
    scanf("%d", &posun);
    getchar();
    n = write(sockfd, &posun, sizeof(posun));
    if (n < 0) {
        perror("Error writing to socket");
        return NULL;
    }

    hlavneMenu();

}

void *skupKonvSpravy() {
    int skonci = 0;
    printf("Skupinová konverzácia: \n");

    while (skonci == 0) {
        int n;
        char buffer[500];
        char sprava[256];
        trim(login, 100);
        printf("%s: ", login);
        fgets(sprava, 255, stdin);
        sprintf(buffer, "%s: %s", login, sprava);


        n = write(sockfd, buffer, strlen(buffer));
        if (n < 0) {
            perror("Error writing to socket");
            return NULL;
        }
        if (strcmp(sprava, "exit") == 0) {
            skonci = 1;
            break;
        }

    }
    hlavneMenu();
}

void *posielanieSuborov() {
    char nazovSuboru[100];
    char contact[100];
    bzero(nazovSuboru, 100);
    int n;


    FILE *subor;

    while (1) {
        printf("Prosím zadajte meno súboru: ");
        fgets(nazovSuboru, 99, stdin);
        trim(nazovSuboru, 100);
        printf("%s \n", nazovSuboru);
        subor = fopen(nazovSuboru, "rb");
        if (subor == NULL) {
            printf("Súbor neexistuje! Zadajte iný súbor \n");
        } else {
            break;
        }
    }
    char buffer[256] = {0};
    FILE *fp;

    int counter = 0;
    fp = fopen(nazovSuboru, "r");
    while (fgetc(fp) != EOF)
        counter++;
    printf("there are %d letters", counter);
    fclose(fp);

    n = write(sockfd, &counter, sizeof(counter));
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
    printf("Teraz môžete zadať jeden alebo viac príjemcov súboru. Pre ukončenie napíšte exit. \n");
    while (1) {
        printf("Prosím zadajte kontakt: ");
        bzero(contact, 100);
        scanf("%s", contact);
        n = send(sockfd, contact, strlen(contact), 0);
        if (n < 0) {
            perror("Error writing to socket");
            return NULL;
        }
        trim(contact, 100);
        if (strcmp(contact, "exit") == 0) {
            break;
        }

        printf("zadaný kontakt %s \n", contact);
        int nasielSa;
        n = read(sockfd, &nasielSa, sizeof(nasielSa));
        if (n < 0) {
            perror("Error writing to socket");
            return NULL;
        }

        if (nasielSa == 1) {
            printf("Používateľ je prihlásený a súbor mu bude odoslaný. \n");
        } else {
            printf("Používateľ nie je prihlásený alebo neexistuje. \n");

        }
    }
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
    hlavneMenu();
}

void skupinovaKonverzacia() {
    int skonci = 0;
    printf("Pre návrat do menu napíšte - exit \n");

    while (skonci == 0) {
        int n;
        char contact[100];
        char buffer[256];

        bzero(contact, 100);

        printf("Zadaj kontakt, ktorý chceš pridať do skupiny: ");
        scanf("%s", contact);
        trim(contact, 100);

        n = write(sockfd, contact, strlen(contact));
        if (n < 0) {
            perror("Error writing to socket");
            return;
        }
        if (strcmp(contact, "exit") == 0) {
            skonci = 1;
            break;
        }
        int nasielSA = 0;
        n = read(sockfd, &nasielSA, sizeof(nasielSA));
        if (n < 0) {
            perror("Error reading from socket");
            return;
        }
        if (nasielSA == 1) {
            printf("Používateľ sa našiel a bol pridaný do skupiny\n");
        } else {
            printf("Používateľ sa nenašiel\n");
        }
    }
    hlavneMenu();

}

void prihlasenie() {
    int n;
    char password[100];
    char buffer[256];
    printf("Pre návrat do menu napíšte - exit \n");

    while (1) {
        printf("Zadajte prihlasovacie meno: ");
        bzero(login, 100);
        scanf("%s", login);
        trim(login, 100);

        n = write(sockfd, login, strlen(login));
        if (n < 0) {
            perror("Error writing to socket");
            return;
        }
        if (strcmp(login, "exit") == 0) {
            hlavneMenu();
            return;
        }
        printf("Zadajte heslo: ");
        bzero(password, 100);
        scanf("%s", password);
        trim(password, 100);

        n = write(sockfd, password, strlen(password));
        if (n < 0) {
            perror("Error writing to socket");
            return;
        }
        if (strcmp(password, "exit") == 0) {
            hlavneMenu();
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
            printf("Meno alebo heslo neexistuje\n");
        } else {
            jePrihlaseny = 1;

            break;
        }
    }
    int counter;
    n = read(sockfd, &counter, sizeof(counter));
    if (n < 0) {
        perror("Error writing to socket");
        return;
    }
    printf("%d", counter);
    int tot = 0;

    if (counter == 0) {
        hlavneMenu();
        return;
    }
    while (tot < counter) {
        (n = recv(sockfd, buffer, 1024, 0));
        tot += n;
        printf("%s %d \n", buffer, n);
        int a = vlozitDoSuboruData(buffer, n, "friendscopy.txt");
        if (a != 1) {
            printf("Chyba");
        }
    }

    printf("Súbor prijatý: %d\n");

    FILE *subor;

    subor = fopen("friendscopy.txt", "r");
    if (subor == NULL) {
        fputs("Error at opening File!", stderr);
        exit(1);
    }
    char contact[100];

    bzero(contact, 100);
    char meno1[100];
    char meno2[100];
    char line[100];
    int pocetRiadkov = 0;
    while (fscanf(subor, "%s", line) != EOF) {
        pocetRiadkov++;
        if (pocetRiadkov % 2 == 1) {
            strcpy(meno1, line);
        } else {
            strcpy(meno2, line);
            if (strcmp(meno1, login) == 0) {
                printf("%s\n", meno2);
                strcpy(priatelia[pocetPriatelov], meno2);
                pocetPriatelov++;
            } else if (strcmp(meno2, login) == 0) {
                printf("%s\n", meno1);
                strcpy(priatelia[pocetPriatelov], meno1);
                pocetPriatelov++;
            }
            bzero(meno1, 99);
            bzero(meno2, 99);
        }
    }

    fclose(subor);
    remove("friendscopy.txt");
    hlavneMenu();
}

void registracia() {
    int n;
    char password[100];
    char buffer[256];
    printf("Pre návrat do menu napíšte - exit \n");

    while (1) {
        printf("Zadajte váš login: ");
        bzero(login, 100);
        fgets(login, 99, stdin);
        n = write(sockfd, login, strlen(login));
        if (n < 0) {
            perror("Error writing to socket");
            return;
        }
        trim(login, 100);
        if (strcmp(login, "exit") == 0) {
            hlavneMenu();
            return;
        }

        int nasloSa;
        n = read(sockfd, &nasloSa, sizeof(nasloSa));
        if (n < 0) {
            perror("Error reading from socket");
            return;
        }

        if (nasloSa == 1) {
            printf("Zadaný login sa už používa.\n");
        } else {
            break;
        }
    }
    printf("Zadajte heslo. \n");
    bzero(password, 100);
    fgets(password, 99, stdin);
    n = write(sockfd, password, strlen(password));
    if (n < 0) {
        perror("Error writing to socket");
        return;
    }
    trim(password, 100);
    if (strcmp(password, "exit") == 0) {
        hlavneMenu();
        return;
    }
    printf("Teraz sa môžete prihlásiť! \n");
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
    printf("Pre návrat do menu napíšte - exit \n");
    char password[100];
    int n;
    if (jePrihlaseny == 1) {
        printf("Pre zrušenie účtu zadajte heslo: ");
        while (1) {
            bzero(password, 100);
            fgets(password, 99, stdin);
            n = write(sockfd, password, strlen(password));
            if (n < 0) {
                perror("Error writing to socket");
                return;
            }
            trim(password, 100);
            if (strcmp(login, "exit") == 0) {
                hlavneMenu();
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
                printf("Nesprávne heslo. \n");
                break;
            }

        }
    }
    hlavneMenu();
}

void poslatZiadost() {
    printf("Pre návrat do menu napíšte - exit \n");
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
    trim(contact, 100);
    if (strcmp(contact, "exit") == 0) {
        hlavneMenu();
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
    printf("%d ", nasielSa);
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
            strcpy(priatelia[pocetPriatelov], contact);
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
        hlavneMenu();
        return;
    }
    printf("Váš zoznam priateľov: \n");
    for (int i = 0; i < pocetPriatelov; ++i) {
        printf("[%d.] %s \n", i + 1, priatelia[i]);
    }
    printf("Pre návrat do menu napíšte - -1 \n");
    char contact[100];

    printf("Zadajte číslo priateľa, ktorého chcete odobrať: \n");
    int priatel;
    scanf("%d", &priatel);
    getchar();
    if (priatel == -1) {
        strcpy(contact, "exit");
        n = write(sockfd, contact, strlen(contact));
        if (n < 0) {
            perror("Error writing to socket");
            return;
        }

        hlavneMenu();
        return;
    }
    strcpy(contact, priatelia[priatel - 1]);

    odobratPriatelaZpola(priatel - 1);
    n = write(sockfd, contact, strlen(contact));
    if (n < 0) {
        perror("Error writing to socket");
        return;
    }
    printf("Zadaný kontakt %s \n", contact);
    trim(contact, 100);

    printf("Požiadavka bola spracovaná.");

    hlavneMenu();
}

void desifruj() {
    printf("Dešifrovanie správy\n ");

    char contact[100];
    char line[256];
    int posun, p = -1;
    int n = read(sockfd, &p, sizeof(p));
    if (n < 0) {
        perror("Error reading from socket");
        return;
    }


    if (p != -1 && p != 1) {
        n = read(sockfd, contact, 99);
        if (n < 0) {
            perror("Error reading from socket");
        }
        n = read(sockfd, &posun, sizeof(posun));
        if (n < 0) {
            perror("Error reading from socket");
        }
        n = read(sockfd, line, 255);
        if (n < 0) {
            perror("Error reading from socket");
        }

        printf("Správa od priateľa %s: %s \n", contact, line);
        printf("Dešifrovanie správy: \n");
        char sifra[256];
        for (int i = 0; i < strlen(line); ++i) {
            char znak = line[i];
            char zasifrovanyZnak = 'a' + ((znak - 'a') - posun) % ('z' - 'a');
            sifra[i] = zasifrovanyZnak;
            printf("Odšifrovanie znaku %c na znak %c\n", znak, zasifrovanyZnak);
        }
        printf("Dešifrovaná správa: %s\n", sifra);
    }

    hlavneMenu();
}

int ukončenie() {
    close(sockfd);
    cinnost = -2;
    return 0;
}

void konverzaciaSpriatelom() {
    int skonci = 0;
    char buffer[256];
    printf("Som tu \n");
    if (pocetPriatelov == 0) {
        printf("Nemáte žiadných priateľov. \n");
        hlavneMenu();
        return;
    }
    printf("Váš zoznam priateľov: \n");
    for (int i = 0; i < pocetPriatelov; ++i) {
        printf("[%d.] %s \n", i + 1, priatelia[i]);
    }
    printf("Zadajte číslo priateľa: \n");
    int priatel;
    scanf("%d", &priatel);
    getchar();
    char contact[100];
    strcpy(contact, priatelia[priatel - 1]);
    int n = write(sockfd, contact, strlen(contact));
    if (n < 0) {
        perror("Error writing to socket");
        return;
    }
    char nazovSuboru[200];

    if (strcmp(contact, login) > 0) {
        sprintf(nazovSuboru, "%s%s.txt", contact, login);
        // printf("Názov súboru: %s \n", nazovSuboru);
    } else {
        sprintf(nazovSuboru, "%s%s.txt", login, contact);
        // printf("Názov súboru: %s \n", nazovSuboru);
    }

    int counter;
    n = read(sockfd, &counter, sizeof(counter));
    if (n < 0) {
        perror("Error writing to socket");
        return;
    }
    int tot = 0;

    if (counter != 0) {
        while (tot < counter) {
            (n = recv(sockfd, buffer, 1024, 0));
            tot += n;
            int a = vlozitDoSuboruData(buffer, n, nazovSuboru);
            if (a != 1) {
                printf("Chyba");
            }
        }
        char line[256];

        printf("Predchádzajúce správy s používateľom %s : \n", contact);
        FILE *subor;
        subor = fopen(nazovSuboru, "r");
        while (fscanf(subor, "%s", line) != EOF) {
            if (strcmp(login, line) == 0 || strcmp(contact, line) == 0) {
                printf("\n");
            }
            printf("%s ", line);
        }
        fclose(subor);
        remove(nazovSuboru);
        printf("\n");

    }

    while (skonci == 0) {
        char sprava[256];

        printf("Zadajte správu pre %s: ", contact);
        fgets(sprava, 255, stdin);
        trim(sprava, 256);
        n = write(sockfd, sprava, strlen(sprava));
        if (n < 0) {
            perror("Error writing to socket");
            return;
        }
        if (strcmp(sprava, "exit") == 0) {
            skonci = 1;
            break;
        }
    }
    hlavneMenu();
}

void hlavneMenu() {
    int poziadavka;
    int n;
    printf("\n");
    cinnost = 0;
    if (jePrihlaseny == 0) {
        printf("[0.] UKONČI \n");
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
        } else if (poziadavka == 2) {
            n = write(sockfd, &poziadavka, sizeof(poziadavka));
            if (n < 0) {
                perror("Error writing to socket");
                return;
            }
            prihlasenie();
        } else if (poziadavka == 0) {
            n = write(sockfd, &poziadavka, sizeof(poziadavka));
            if (n < 0) {
                perror("Error writing to socket");
                return;
            }

            ukončenie();
        } else {
            hlavneMenu();
        }

    } else {
        printf("*****ÚČET*****\n");
        printf("[3.] Odhlásenie \n");
        printf("[4.] Zmazať účet \n");
        printf("\n*****POSIELANIE SPRÁV*****\n");
        printf("[5.] Posielanie správ \n");
        printf("[6.] Konverzácie s priateľmi \n");
        printf("[7.] Vytvor skupinu \n");
        printf("[8.] Skupinová konverzácia \n");
        printf("[9.] Posielanie dát \n");
        printf("[10.] Prijatie dát \n");
        printf("[11.] Odoslanie šifrovanej správy \n");
        printf("[12.] Dešifrovanie správy \n");
        printf("\n*****PRIATELIA*****\n");
        printf("[13.] Pridať si úžívateľa do priateľov\n");
        printf("[14.] Pozrieť si žiadosti o priateľstvo\n");
        printf("[15.] Odobranie priatela \n");


        scanf("%d", &poziadavka);
        getchar();
        cinnost = poziadavka;
        if (poziadavka == 5) {
            n = write(sockfd, &poziadavka, sizeof(poziadavka));
            if (n < 0) {
                perror("Error writing to socket");
                return;
            }
            pthread_t klient;
            pthread_create(&klient, NULL, posliSpravu, NULL);
            pthread_t klient2;
            pthread_create(&klient2, NULL, dostatnSpravu, NULL);

        } else if (poziadavka == 3) {
            n = write(sockfd, &poziadavka, sizeof(poziadavka));
            if (n < 0) {
                perror("Error writing to socket");
                return;
            }
            odhlasenie();
        } else if (poziadavka == 4) {
            n = write(sockfd, &poziadavka, sizeof(poziadavka));
            if (n < 0) {
                perror("Error writing to socket");
                return;
            }
            zrusitUcet();
        } else if (poziadavka == 13) {
            n = write(sockfd, &poziadavka, sizeof(poziadavka));
            if (n < 0) {
                perror("Error writing to socket");
                return;
            }
            poslatZiadost();
        } else if (poziadavka == 14) {
            n = write(sockfd, &poziadavka, sizeof(poziadavka));
            if (n < 0) {
                perror("Error writing to socket");
                return;
            }
            pozrietZiadosti();
        } else if (poziadavka == 15) {
            n = write(sockfd, &poziadavka, sizeof(poziadavka));
            if (n < 0) {
                perror("Error writing to socket");
                return;
            }
            odobratPriatela();
        } else if (poziadavka == 7) {
            n = write(sockfd, &poziadavka, sizeof(poziadavka));
            if (n < 0) {
                perror("Error writing to socket");
                return;
            }
            skupinovaKonverzacia();
        } else if (poziadavka == 9) {
            n = write(sockfd, &poziadavka, sizeof(poziadavka));
            if (n < 0) {
                perror("Error writing to socket");
                return;
            }
            posielanieSuborov();
        } else if (poziadavka == 10) {
            n = write(sockfd, &poziadavka, sizeof(poziadavka));
            if (n < 0) {
                perror("Error writing to socket");
                return;
            }
            pthread_t klient4;
            pthread_create(&klient4, NULL, prijatieSuboru, NULL);

        } else if (poziadavka == 8) {
            n = write(sockfd, &poziadavka, sizeof(poziadavka));
            if (n < 0) {
                perror("Error writing to socket");
                return;
            }
            pthread_t klient5;
            pthread_create(&klient5, NULL, skupKonvSpravy, NULL);
            pthread_t klient6;
            pthread_create(&klient6, NULL, dostatnSpravu, NULL);

        } else if (poziadavka == 11) {
            n = write(sockfd, &poziadavka, sizeof(poziadavka));
            if (n < 0) {
                perror("Error writing to socket");
                return;
            }
            posliSifSpravu();
        } else if (poziadavka == 12) {
            n = write(sockfd, &poziadavka, sizeof(poziadavka));
            if (n < 0) {
                perror("Error writing to socket");
                return;
            }
            desifruj();

        } else if (poziadavka == 6) {
            n = write(sockfd, &poziadavka, sizeof(poziadavka));
            if (n < 0) {
                perror("Error writing to socket");
                return;
            }
            konverzaciaSpriatelom();

        } else {
            hlavneMenu();
        }
    }
}

int mainKlient(int argc, char *argv[]) {
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
    cinnost = -1;
    hlavneMenu();
    while (cinnost != -2) {

    }
}
