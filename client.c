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

        printf("Zadajte spr??vu pre %s: ", contact);
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

    printf("Posielam ??ifrovan?? spr??vu \n");
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
        printf("Pou????vate?? sa nena??iel.");
        hlavneMenu();
    }

    printf("Zadajte spr??vu pre %s: ", contact);
    printf("%s", "> ");
    fflush(stdout);
    scanf("%s", buffer);
    n = write(sockfd, buffer, strlen(buffer));

    if (n < 0) {
        perror("Error writing to socket");
        return NULL;
    }

    int posun;
    printf("Zadajte ????slo: \n ");
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
    printf("Skupinov?? konverz??cia: \n");

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
        printf("Pros??m zadajte meno s??boru: ");
        fgets(nazovSuboru, 99, stdin);
        trim(nazovSuboru, 100);
        printf("%s \n", nazovSuboru);
        subor = fopen(nazovSuboru, "rb");
        if (subor == NULL) {
            printf("S??bor neexistuje! Zadajte in?? s??bor \n");
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
    printf("Teraz m????ete zada?? jeden alebo viac pr??jemcov s??boru. Pre ukon??enie nap????te exit. \n");
    while (1) {
        printf("Pros??m zadajte kontakt: ");
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

        printf("zadan?? kontakt %s \n", contact);
        int nasielSa;
        n = read(sockfd, &nasielSa, sizeof(nasielSa));
        if (n < 0) {
            perror("Error writing to socket");
            return NULL;
        }

        if (nasielSa == 1) {
            printf("Pou????vate?? je prihl??sen?? a s??bor mu bude odoslan??. \n");
        } else {
            printf("Pou????vate?? nie je prihl??sen?? alebo neexistuje. \n");

        }
    }
    hlavneMenu();

}

void *prijatieSuboru() {
    char nazovSuboru[100];
    char buffer[256];
    bzero(buffer, 256);
    bzero(nazovSuboru, 100);

    printf("Pros??m zadajte n??zov, pod ktor??m sa s??bor ulo????: ");
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

    printf("S??bor prijat??: %d\n");
    if (n < 0) {
        perror("Receiving");
    }
    hlavneMenu();
}

void skupinovaKonverzacia() {
    int skonci = 0;
    printf("Pre n??vrat do menu nap????te - exit \n");

    while (skonci == 0) {
        int n;
        char contact[100];
        char buffer[256];

        bzero(contact, 100);

        printf("Zadaj kontakt, ktor?? chce?? prida?? do skupiny: ");
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
            printf("Pou????vate?? sa na??iel a bol pridan?? do skupiny\n");
        } else {
            printf("Pou????vate?? sa nena??iel\n");
        }
    }
    hlavneMenu();

}

void prihlasenie() {
    int n;
    char password[100];
    char buffer[256];
    printf("Pre n??vrat do menu nap????te - exit \n");

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

    printf("S??bor prijat??: %d\n");

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
    printf("Pre n??vrat do menu nap????te - exit \n");

    while (1) {
        printf("Zadajte v???? login: ");
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
            printf("Zadan?? login sa u?? pou????va.\n");
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
    printf("Teraz sa m????ete prihl??si??! \n");
    hlavneMenu();

}

void odhlasenie() {
    if (jePrihlaseny == 1) {
        jePrihlaseny = 0;
    }
    printf("Boli ste odhl??sen??.\n");
    hlavneMenu();

}

void zrusitUcet() {
    printf("Pre n??vrat do menu nap????te - exit \n");
    char password[100];
    int n;
    if (jePrihlaseny == 1) {
        printf("Pre zru??enie ????tu zadajte heslo: ");
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
                printf("V???? ????et bol zru??en??.\n");
                jePrihlaseny = 0;
                break;
            } else {
                printf("Nespr??vne heslo. \n");
                break;
            }

        }
    }
    hlavneMenu();
}

void poslatZiadost() {
    printf("Pre n??vrat do menu nap????te - exit \n");
    char contact[100];
    int n;
    printf("Zadajte login pou????vate??a: \n");
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
        printf("Po??iadavka o pridanie do priate??ov bola odoslan??.\n");
    } else {
        printf("Pou????vate?? neexistuje.\n");
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
        printf("??iados?? o priate??stvo od pou????vate??a %s: 0/1", contact);
        scanf("%d", &poziadavka);
        getchar();
        n = write(sockfd, &poziadavka, sizeof(poziadavka));
        if (n < 0) {
            perror("Error writing to socket");
            return;
        }

        if (poziadavka == 1) {
            printf("??iados?? bola prijat??.\n");
            strcpy(priatelia[pocetPriatelov], contact);
            pocetPriatelov++;
        } else {
            printf("??iados?? bola ignorovan??.\n");
        }
        n = read(sockfd, &nasielSa, sizeof(nasielSa));
        if (n < 0) {
            perror("Error reading from socket");
            return;
        }
        printf("%d \n", nasielSa);
    }

    printf("Nem??te u?? ??iadne in?? ??iadosti.\n");
    hlavneMenu();


}

void odobratPriatela() {
    int n;

    if (pocetPriatelov == 0) {
        printf("Nem??te ??iadn??ch priate??ov. \n");
        hlavneMenu();
        return;
    }
    printf("V???? zoznam priate??ov: \n");
    for (int i = 0; i < pocetPriatelov; ++i) {
        printf("[%d.] %s \n", i + 1, priatelia[i]);
    }
    printf("Pre n??vrat do menu nap????te - -1 \n");
    char contact[100];

    printf("Zadajte ????slo priate??a, ktor??ho chcete odobra??: \n");
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
    printf("Zadan?? kontakt %s \n", contact);
    trim(contact, 100);

    printf("Po??iadavka bola spracovan??.");

    hlavneMenu();
}

void desifruj() {
    printf("De??ifrovanie spr??vy\n ");

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

        printf("Spr??va od priate??a %s: %s \n", contact, line);
        printf("De??ifrovanie spr??vy: \n");
        char sifra[256];
        for (int i = 0; i < strlen(line); ++i) {
            char znak = line[i];
            char zasifrovanyZnak = 'a' + ((znak - 'a') - posun) % ('z' - 'a');
            sifra[i] = zasifrovanyZnak;
            printf("Od??ifrovanie znaku %c na znak %c\n", znak, zasifrovanyZnak);
        }
        printf("De??ifrovan?? spr??va: %s\n", sifra);
    }

    hlavneMenu();
}

int ukon??enie() {
    close(sockfd);
    cinnost = -2;
    return 0;
}

void konverzaciaSpriatelom() {
    int skonci = 0;
    char buffer[256];
    printf("Som tu \n");
    if (pocetPriatelov == 0) {
        printf("Nem??te ??iadn??ch priate??ov. \n");
        hlavneMenu();
        return;
    }
    printf("V???? zoznam priate??ov: \n");
    for (int i = 0; i < pocetPriatelov; ++i) {
        printf("[%d.] %s \n", i + 1, priatelia[i]);
    }
    printf("Zadajte ????slo priate??a: \n");
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
        // printf("N??zov s??boru: %s \n", nazovSuboru);
    } else {
        sprintf(nazovSuboru, "%s%s.txt", login, contact);
        // printf("N??zov s??boru: %s \n", nazovSuboru);
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

        printf("Predch??dzaj??ce spr??vy s pou????vate??om %s : \n", contact);
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

        printf("Zadajte spr??vu pre %s: ", contact);
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
        printf("[0.] UKON??I \n");
        printf("[1.] Registr??cia \n");
        printf("[2.] Prihl??senie \n");
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

            ukon??enie();
        } else {
            hlavneMenu();
        }

    } else {
        printf("*****????ET*****\n");
        printf("[3.] Odhl??senie \n");
        printf("[4.] Zmaza?? ????et \n");
        printf("\n*****POSIELANIE SPR??V*****\n");
        printf("[5.] Posielanie spr??v \n");
        printf("[6.] Konverz??cie s priate??mi \n");
        printf("[7.] Vytvor skupinu \n");
        printf("[8.] Skupinov?? konverz??cia \n");
        printf("[9.] Posielanie d??t \n");
        printf("[10.] Prijatie d??t \n");
        printf("[11.] Odoslanie ??ifrovanej spr??vy \n");
        printf("[12.] De??ifrovanie spr??vy \n");
        printf("\n*****PRIATELIA*****\n");
        printf("[13.] Prida?? si ??????vate??a do priate??ov\n");
        printf("[14.] Pozrie?? si ??iadosti o priate??stvo\n");
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
