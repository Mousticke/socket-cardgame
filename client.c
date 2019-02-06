/**
 * @file client.c
 * @author Doral - Jbara - Forey
 * @brief fichier client
 * @version 0.1
 * @date 2018-12-22
 * 
 * @copyright Copyright (c) 2018
 * 
 */

#include "config.h"


UnoCard handGame[42];  /*!< Carte en main pour le joueur */
int addition=0; /*!< Variable utilisé contenant le nombre de carte en plus que la main initiale (11 cartes au début) */
UnoCard unoCardFromServer = {-1, -1}; /*!< Récupéré la carte donnée par le serveur */
UnoCard unoCardFromServerPosed = {-1, -1}; /*!< Récupéré la carte posé sur le paquet nécessaire lorsque l'on va faire un refresh */

/**
 * @brief Menu utilisé pour l'action de tirer une carte ou de quitter
 * 
 */
char *menu[] = {
	"Tirer une carte",
	"Quitter",
};

void chooseRole(int,int);
void draw_card(UnoCard,int);
void draw_game_card(UnoCard,int);
int connectSock();
int sendWithAck(int, char *);
char* card2str(UnoCard);
char* intToStr(int entier);
void func(char *name);
UnoCard strToUnoCard(char *str);
int sendWithAckSimple(int socket, char * msg);
int sendForCards(int socket, char * msg, int index);
void * setchoice (int ch);
void * listener();

int y, x; /*!< Ligne et colonne de la fenêtre */
int ch; /*!< recupération de caractère tapé */
char* temp; /*!< chaine de caractère pour l'envoie de la requete */
char* removingcard; /*!< chaine de caractère pour l'envoie de la requete */
int done = 0; /*!< Variable utilisé pour savoir si on arrête les threads */
ITEM **my_items; /*!< Item du menu : voir menu.h */
int c;		/*!< choix dans le menu */		
MENU *my_menu; /*!< Varibale de menu */
int n_choices, iterator;
ITEM *cur_item;
pthread_mutex_t mutexsum = PTHREAD_MUTEX_INITIALIZER; 
int sock; /*!< socket client */
int current_hand_to_stash = 0; /*!< savoir combien de carte on a en main */

int main(int argc, char const *argv[]){

    /* connect to server */
    sock = connectSock();
    if(argc >= 2)
    	chooseRole(sock,atoi(argv[1]));
    else exit(1);

    /* initialize curses */
	initscr();
    keypad(stdscr, TRUE);
    cbreak();
    noecho();

    /* initialize colors */
    if (has_colors() == FALSE) {
        endwin();
        printf("Your terminal does not support color\n");
        exit(1);
    }

    start_color();
    //init_pait(index, couleur, background)
    init_pair(YELLOW, COLOR_WHITE, COLOR_YELLOW);
    init_pair(RED, COLOR_WHITE, COLOR_RED);
    init_pair(BLUE, COLOR_WHITE, COLOR_BLUE);
    init_pair(GREEN, COLOR_WHITE, COLOR_GREEN);
    init_pair(WHITE, COLOR_BLACK, COLOR_WHITE);
    init_pair(BLACK, COLOR_WHITE, COLOR_BLACK);

	init_pair(8, COLOR_MAGENTA, COLOR_BLACK);

	/* Initialize menu */
    n_choices = ARRAY_SIZE(menu);
    my_items = (ITEM **)calloc(n_choices + 1, sizeof(ITEM *));
    for(iterator = 0; iterator < n_choices; ++iterator)
	{       
		my_items[iterator] = new_item(menu[iterator], menu[iterator]);
		/* Set the user pointer */
		set_item_userptr(my_items[iterator], func);
	}
	my_items[n_choices] = (ITEM *)NULL;
	/* Create menu */
	my_menu = new_menu((ITEM **)my_items);

	/* Set fore ground and back ground of the menu */
	set_menu_fore(my_menu, COLOR_PAIR(RED) | A_REVERSE);
	set_menu_back(my_menu, COLOR_PAIR(GREEN));
	set_menu_grey(my_menu, COLOR_PAIR(8));

	//Draw the menu
	mvprintw(LINES - 10, 0, "Appuyer sur le numéro du menu pour le choix");
	mvprintw(LINES - 8, 0, "(F1 pour Quitter)");
	post_menu(my_menu);
	//refresh();


	for(int i = 0; i < HAND; i++)
	{
		char str[10] = "";
    	sprintf(str, "%d", current_hand_to_stash);
		char* receivingCards;
		receivingCards = malloc(100);
		strcpy(receivingCards, "600");
		strcat(receivingCards,":");
		strcat(receivingCards,"20");
		strcat(receivingCards,":");
		strcat(receivingCards, str);
		sendForCards(sock, receivingCards, i);
		current_hand_to_stash++;
		free(receivingCards);
	}
	for(int i = 0; i < HAND; i++)
	{
		draw_game_card(handGame[i],i);
	}

	/* start player at first card */
    y = LINES - 2;
    x = POSFIRSTCARD;
	
	char requete[MAX_BUFF];
	char data[MAX_BUFF];

	pthread_t threads[2];
    void *status;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    // Spawn the listen/receive deamons
    pthread_create(&threads[0], &attr, setchoice, ch);
    pthread_create(&threads[1], &attr, listener, NULL);

    // Keep alive until finish condition is done
    while(!done);

	unpost_menu(my_menu);
	for(iterator = 0; iterator < n_choices; ++iterator)
		free_item(my_items[iterator]);
	free_menu(my_menu);
    endwin();
	sendWithAckSimple(sock, "0:10:0");
	close(sock);
    exit(0);
}

/**
 * @brief Choisir une carte dans la main, piocher une carte, quitter
 * 
 * @param ch 
 * @return void* 
 */
void * setchoice (int ch)
{
	do {
        move(y, x);
        refresh();

		ch = getch();
        switch (ch) {

			case KEY_RESIZE:
				clear();
				n_choices = ARRAY_SIZE(menu);
				my_items = (ITEM **)calloc(n_choices + 1, sizeof(ITEM *));
				for(iterator = 0; iterator < n_choices; ++iterator)
				{       
					my_items[iterator] = new_item(menu[iterator], menu[iterator]);
					/* Set the user pointer */
					set_item_userptr(my_items[iterator], func);
				}
				my_items[n_choices] = (ITEM *)NULL;
				/* Create menu */
				my_menu = new_menu((ITEM **)my_items);
				for(int i = 0; i < HAND+addition; i++)
				{
					draw_game_card(handGame[i],i);
				}
				y = LINES - 2;
    			x = POSFIRSTCARD;
				set_menu_fore(my_menu, COLOR_PAIR(RED) | A_REVERSE);
				set_menu_back(my_menu, COLOR_PAIR(GREEN));
				set_menu_grey(my_menu, COLOR_PAIR(8));
				mvprintw(LINES - 10, 0, "Appuyer sur le chiffre correspondant pour executer une action du menu");
				mvprintw(LINES - 8, 0, "(F1 pour Quitter)");
				post_menu(my_menu);

				draw_card(unoCardFromServerPosed, 1); 
				refresh();
        		break;

		case '1':
		{
			ITEM *cur;
			void (*p)(char *);
			cur = current_item(my_menu);
			p = item_userptr(cur);
			p((char *)item_name(cur));
			pos_menu_cursor(my_menu);

			temp = malloc(100);
			char str[10] = "";
    		sprintf(str, "%d", current_hand_to_stash);
        	strcpy(temp, "600");
        	strcat(temp,":");
        	strcat(temp,"201");
        	strcat(temp,":");
        	strcat(temp, str);
			current_hand_to_stash++;
			sendWithAck(sock, temp);
			break;
		}
		break;
		case '2':
			done = 1;
		    pthread_mutex_destroy(&mutexsum);
            pthread_exit(NULL);
			unpost_menu(my_menu);
			for(iterator = 0; iterator < n_choices; ++iterator)
				free_item(my_items[iterator]);
			free_menu(my_menu);
			endwin();
			exit(0);
			break;
        case KEY_LEFT:
        case 'q':
        case 'Q':
            if ((x > POSFIRSTCARD)) {
                x-=3;
            }
            break;
        case KEY_RIGHT:
        case 'd':
        case 'D':
            if (x < POSFIRSTCARD + (((HAND+addition)-1)*3)) {
                x+=3;
            }
            break;
        case KEY_ENTER_CUSTOM:
        	
        	temp = malloc(100);
        	strcpy(temp, PLAYCARD);
        	strcat(temp,":");
        	strcat(temp,"20");
        	strcat(temp,":");
        	strcat(temp, card2str(handGame[(x - POSFIRSTCARD)/3]));
			sendWithAck(sock, temp);
		
        	break;
        }
    }
    while ((ch != 'a') && (ch != 'A'));
}


/**
 * @brief Executer des requêtes en fonction des événements arrivants
 * 
 * @return void* 
 */
void * listener ()
{
    int bufsize=1024;
    char *buffer=malloc(bufsize);

	char requete[MAX_BUFF];
	char data[MAX_BUFF];
	while(1)
	{
		read(sock, buffer, bufsize);
		sscanf(buffer, "%[^:]:%[^:]:%s",requete, buffer,data);
		switch(atoi(requete)) 
		{
			case 500:
				mvprintw(LINES-6 ,0 ,buffer);
				mvprintw(LINES-5 ,0 ,requete);
				mvprintw(LINES-4 ,0 ,data);
				unoCardFromServerPosed = strToUnoCard(data); 
				current_hand_to_stash--;
				remove_element(handGame, (x - POSFIRSTCARD)/3, 42);
				addition--;
				
				clear();
				n_choices = ARRAY_SIZE(menu);
				my_items = (ITEM **)calloc(n_choices + 1, sizeof(ITEM *));
				for(iterator = 0; iterator < n_choices; ++iterator)
				{       
					my_items[iterator] = new_item(menu[iterator], menu[iterator]);
					/* Set the user pointer */
					set_item_userptr(my_items[iterator], func);
				}
				my_items[n_choices] = (ITEM *)NULL;
				/* Create menu */
				my_menu = new_menu((ITEM **)my_items);
				for(int i = 0; i < HAND+addition; i++)
				{
					draw_game_card(handGame[i],i);
				}
				y = LINES - 2;
    			x = POSFIRSTCARD;
				set_menu_fore(my_menu, COLOR_PAIR(RED) | A_REVERSE);
				set_menu_back(my_menu, COLOR_PAIR(GREEN));
				set_menu_grey(my_menu, COLOR_PAIR(8));
				mvprintw(LINES - 10, 0, "Appuyer sur le chiffre correspondant pour executer une action du menu");
				mvprintw(LINES - 8, 0, "(F1 pour Quitter)");
				post_menu(my_menu);


				draw_card(unoCardFromServerPosed, 1); 

				refresh();
				break;
			
			case 700:
				mvprintw(LINES-6 ,0 ,buffer);
				mvprintw(LINES-5 ,0 ,requete);
				mvprintw(LINES-4 ,0 ,data);
				unoCardFromServer = strToUnoCard(data);
				addition++;
				handGame[HAND+addition] = unoCardFromServer;
				clear();
				n_choices = ARRAY_SIZE(menu);
				my_items = (ITEM **)calloc(n_choices + 1, sizeof(ITEM *));
				for(iterator = 0; iterator < n_choices; ++iterator)
				{       
					my_items[iterator] = new_item(menu[iterator], menu[iterator]);
					/* Set the user pointer */
					set_item_userptr(my_items[iterator], func);
				}
				my_items[n_choices] = (ITEM *)NULL;
				/* Create menu */
				my_menu = new_menu((ITEM **)my_items);
				for(int i = 0; i < HAND+addition; i++)
				{
					draw_game_card(handGame[i],i);
				}
				y = LINES - 2;
    			x = POSFIRSTCARD;
				set_menu_fore(my_menu, COLOR_PAIR(RED) | A_REVERSE);
				set_menu_back(my_menu, COLOR_PAIR(GREEN));
				set_menu_grey(my_menu, COLOR_PAIR(8));
				mvprintw(LINES - 10, 0, "Appuyer sur le chiffre correspondant pour executer une action du menu");
				mvprintw(LINES - 8, 0, "(F1 pour Quitter)");
				post_menu(my_menu);

				draw_card(unoCardFromServerPosed, 1); 

				refresh();
				break;
		}
	}
}

/**
 * @brief Envoi de la requete au serveur pour avoir les cartes au début de la partie
 * 
 * @param socket 
 * @param msg 
 * @param index 
 * @return int 
 */
int sendForCards(int socket, char * msg, int index)
{
	CHECK(write(socket, msg, strlen(msg)+1), "Can't send"); 
	int bufsize=1024;
    char *buffer=malloc(bufsize);
	char requete[MAX_BUFF];
	char data[MAX_BUFF];
	read(sock, buffer, bufsize);
	sscanf(buffer, "%[^:]:%[^:]:%s",requete, buffer,data);
	switch(atoi(requete)) 
	{
		case 700:
			mvprintw(LINES-15 ,0 ,buffer);
			mvprintw(LINES-14 ,0 ,requete);
			mvprintw(LINES-13 ,0 ,data);
			unoCardFromServer = strToUnoCard(data);
			handGame[index] = unoCardFromServer;
			//draw_card(unoCardFromServer, 1);  
			break;
	}
}

/**
 * @brief Requete pour un envoie avec un accusé de réception du serveur
 * 
 * @param socket 
 * @param msg 
 * @return int 
 */
int sendWithAckSimple(int socket, char * msg){
	char reponse[255];
	CHECK(write(socket, msg, strlen(msg)+1), "Can't send"); 
	CHECK(read(socket, reponse, sizeof(reponse)), "Can't read");
	if(strcmp(reponse,COK) == 0)return 1;
	else return 0;
}

/**
 * @brief Convertir une chaine de caratère en UnoCard
 * 
 * @param str 
 * @return UnoCard 
 */
UnoCard strToUnoCard(char *str)
{
    UnoCard tempuno = {str[0] - 48,str[2] - 48};
	return tempuno;
}

/**
 * @brief Requete d'envoie au serveur
 * 
 * @param socket 
 * @param msg 
 * @return int 
 */
int sendWithAck(int socket, char * msg){
	char requete[MAX_BUFF];
	char data[MAX_BUFF];
	CHECK(write(socket, msg, strlen(msg)+1), "Can't send"); 
	return 1;
}

/**
 * @brief Création de la socket client
 * 
 * @return int 
 */
int connectSock(){
	int socket_client;
	struct sockaddr_in svc,clt; 
	// Création de la socket d’appel et de dialogue 
	CHECK(socket_client=socket(PF_INET, SOCK_STREAM, 0), "Can't create");
	// Préparation de l’adressage du service à contacter 
	svc.sin_family = PF_INET; 
	svc.sin_port = htons(PORT_SVC); 
	svc.sin_addr.s_addr = inet_addr(INADDR_SVC);
	//if(argc>1)svc.sin_addr.s_addr = inet_addr(argv[1]);
	memset(&svc.sin_zero, 0, 8); 
	// Demande d’une connexion au service 
	CHECK(connect(socket_client, (struct sockaddr *) &svc, sizeof(svc)) , "Can't connect"); 
	int cltLen = sizeof(clt);
	CHECK(getsockname(socket_client, (struct sockaddr *) &clt, &cltLen), "Erreur lors de la récupération de l'adresse client");
  	printf("connecté [ADDRESS = %s - PORT = %d ]\n", inet_ntoa(clt.sin_addr), ntohs(clt.sin_port)); 
	// Dialogue avec le serveur 
	return socket_client;
}

/**
 * @brief Choisir le rôle en tant que client spectateur ou joueur
 * 
 * @param socket 
 * @param role 
 */
void chooseRole(int socket,int role){
	switch(role){
		case 0:
			sendWithAckSimple(socket, "100:10:0");
		break;
		case 1:
			if(sendWithAckSimple(socket, "100:10:1") == 0) exit(1);
		break;
	}
}

/**
 * @brief Dessiner les cartes en main
 * 
 * @param carte 
 * @param noCarte 
 */
void draw_game_card(UnoCard carte, int noCarte){
	if(carte.uc_face == 10){
		attron(COLOR_PAIR(RED));
			mvaddch(LINES - 2, POSFIRSTCARD + noCarte*3, EMPTY);//
		attroff(COLOR_PAIR(RED));
		attron(COLOR_PAIR(BLUE));
			mvaddch(LINES - 2, POSFIRSTCARD + noCarte*3 + 1, EMPTY);//
		attroff(COLOR_PAIR(BLUE));
		attron(COLOR_PAIR(YELLOW));
			mvaddch(LINES - 1, POSFIRSTCARD + noCarte*3,EMPTY);//
		attroff(COLOR_PAIR(YELLOW));
		attron(COLOR_PAIR(GREEN));
			mvaddch(LINES - 1, POSFIRSTCARD + noCarte*3 + 1, EMPTY);//
		attroff(COLOR_PAIR(GREEN));
	}else{
		attron(COLOR_PAIR(carte.uc_color));
			mvaddch(LINES - 2, POSFIRSTCARD + noCarte*3, (char)carte.uc_face+48);//
			mvaddch(LINES - 2, POSFIRSTCARD + noCarte*3 + 1, EMPTY);//
			mvaddch(LINES - 1, POSFIRSTCARD + noCarte*3,EMPTY);//
			mvaddch(LINES - 1, POSFIRSTCARD + noCarte*3 + 1, (char)carte.uc_face+48);//
		attroff(COLOR_PAIR(carte.uc_color));
	}
	refresh();
}

/**
 * @brief Dessiner la carte posée en jeu
 * 
 * @param carte 
 * @param normal 
 */
void draw_card(UnoCard carte,int normal){
	Color color = carte.uc_color;
	Face number = carte.uc_face;
	int firstX = COLS/2-4,firstY = LINES/2-4; //center
	if(number < 10){
		attron(COLOR_PAIR((int) color));
		for (int y = firstY; y < firstY+9; y++) {
			mvhline(y, firstX, EMPTY, 9);
		}
		mvaddch(firstY, firstX, (char)number+48);
		mvaddch(firstY+8, firstX+8, (char)number+48);
		attroff(COLOR_PAIR((int) color));
	}

    attron(COLOR_PAIR(WHITE));
    if(normal == 0){
		mvhline(firstX+4, 1, EMPTY, 3); //+
		mvvline(firstX+3, 2, EMPTY, 3); //+
	}

    switch(number){
		case 0:
			mvhline(firstY + 1, firstX + 5-2*normal, EMPTY, 2+1*normal); 	//a
			mvvline(firstY + 2, firstX + 7-1*normal, EMPTY, 2); 			//b
			mvvline(firstY + 5, firstX + 7-1*normal, EMPTY, 2);			//c
			mvhline(firstY + 7, firstX + 5-2*normal, EMPTY, 2+1*normal);	//d
			mvvline(firstY + 5, firstX + 4-2*normal, EMPTY, 2); 			//e
			mvvline(firstY + 2, firstX + 4-2*normal, EMPTY, 2); 			//f
			break;
	   	case 1:
			mvvline(firstY + 2, firstX + 7-1*normal, EMPTY, 2); 			//b
			mvvline(firstY + 5, firstX + 7-1*normal, EMPTY, 2);			//c
			break;
		case 2:
			mvhline(firstY + 1, firstX + 5-2*normal, EMPTY, 2+1*normal); 	//a
			mvvline(firstY + 2, firstX + 7-1*normal, EMPTY, 2); 			//b
			mvhline(firstY + 7, firstX + 5-2*normal, EMPTY, 2+1*normal);	//d
			mvvline(firstY + 5, firstX + 4-2*normal, EMPTY, 2); 			//e
			mvhline(firstY + 4, firstX + 5-2*normal, EMPTY, 2+1*normal); 	//g
			break;
		case 3:
			mvhline(firstY + 1, firstX + 5-2*normal, EMPTY, 2+1*normal); 	//a
			mvvline(firstY + 2, firstX + 7-1*normal, EMPTY, 2); 			//b
			mvvline(firstY + 5, firstX + 7-1*normal, EMPTY, 2);			//c
			mvhline(firstY + 7, firstX + 5-2*normal, EMPTY, 2+1*normal);	//d
			mvhline(firstY + 4, firstX + 5-2*normal, EMPTY, 2+1*normal); 	//g
			break;
	   	case 4:
			mvvline(firstY + 2, firstX + 7-1*normal, EMPTY, 2); 			//b
			mvvline(firstY + 5, firstX + 7-1*normal, EMPTY, 2);			//c
			mvvline(firstY + 2, firstX + 4-2*normal, EMPTY, 2); 			//f
			mvhline(firstY + 4, firstX + 5-2*normal, EMPTY, 2+1*normal); 	//g
			break;
	   	case 5:
			mvhline(firstY + 1, firstX + 5-2*normal, EMPTY, 2+1*normal); 	//a
			mvvline(firstY + 5, firstX + 7-1*normal, EMPTY, 2);			//c
			mvhline(firstY + 7, firstX + 5-2*normal, EMPTY, 2+1*normal);	//d
			mvvline(firstY + 2, firstX + 4-2*normal, EMPTY, 2); 			//f
			mvhline(firstY + 4, firstX + 5-2*normal, EMPTY, 2+1*normal); 	//g
			break;
		case 6:
			//mvhline(1, 5-2*normal, EMPTY, 2+1*normal); 	//a
			mvvline(firstY + 5, firstX + 7-1*normal, EMPTY, 2);			//c
			mvhline(firstY + 7, firstX + 5-2*normal, EMPTY, 2+1*normal);	//d
			mvvline(firstY + 5, firstX + 4-2*normal, EMPTY, 2); 			//e
			mvvline(firstY + 2, firstX + 4-2*normal, EMPTY, 2); 			//f
			mvhline(firstY + 4, firstX + 5-2*normal, EMPTY, 2+1*normal); 	//g
			break;
		case 7:
			mvhline(firstY + 1, firstX + 5-2*normal, EMPTY, 2+1*normal); 	//a
			mvvline(firstY + 2, firstX + 7-1*normal, EMPTY, 2); 			//b
			mvvline(firstY + 5, firstX + 7-1*normal, EMPTY, 2);			//c
			break;
		case 8:
			mvhline(firstY +1, firstX + 5-2*normal, EMPTY, 2+1*normal); 	//a
			mvvline(firstY +2, firstX + 7-1*normal, EMPTY, 2); 			//b
			mvvline(firstY +5, firstX + 7-1*normal, EMPTY, 2);			//c
			mvhline(firstY +7, firstX + 5-2*normal, EMPTY, 2+1*normal);	//d
			mvvline(firstY +5, firstX + 4-2*normal, EMPTY, 2); 			//e
			mvvline(firstY +2, firstX + 4-2*normal, EMPTY, 2); 			//f
			mvhline(firstY +4, firstX + 5-2*normal, EMPTY, 2+1*normal); 	//g
			break;
		case 9:
			mvhline(firstY + 1, firstX + 5-2*normal, EMPTY, 2+1*normal); 	//a
			mvvline(firstY + 2, firstX + 7-1*normal, EMPTY, 2); 			//b
			mvvline(firstY + 5, firstX + 7-1*normal, EMPTY, 2);			//c
			//mvhline(7, 5-2*normal, EMPTY, 2+1*normal);	//d
			mvvline(firstY + 2, firstX + 4-2*normal, EMPTY, 2); 			//f
			mvhline(firstY + 4, firstX + 5-2*normal, EMPTY, 2+1*normal); 	//g
			break;
		case MULTIPLE_COLOR:
			attron(COLOR_PAIR(WHITE));
			for (int y = firstY; y < firstY+9; y++) {
				mvhline(y, firstX, EMPTY, 9);
			}
			attroff(COLOR_PAIR(WHITE));
			for (int y = firstY; y < firstY+4; y++) {
				attron(COLOR_PAIR(RED));
				mvhline(y, firstX, EMPTY, 4);
				attroff(COLOR_PAIR(RED));
				attron(COLOR_PAIR(BLUE));
				mvhline(y, firstX+5, EMPTY, 4);
				attroff(COLOR_PAIR(BLUE));
			}
			for (int y = firstY+5; y < firstY+9; y++) {
				attron(COLOR_PAIR(YELLOW));
				mvhline(y, firstX, EMPTY, 4);
				attroff(COLOR_PAIR(YELLOW));
				attron(COLOR_PAIR(GREEN));
				mvhline(y, firstX+5, EMPTY, 4);
				attroff(COLOR_PAIR(GREEN));
			}
			break;
		default:
			break;
	}
    attroff(COLOR_PAIR(WHITE));
    //move(0,0);
    refresh();
}


/**
 * @brief Conversion de la UnoCard en chaine de caractère
 * 
 * @param carte 
 * @return char* 
 */
char* card2str(UnoCard carte){
	char* temp2;
	temp2=malloc(3);
	//char int2str[1]:
	temp2[0] = (char) carte.uc_face + 48;
	temp2[1] = ',';
	temp2[2] = (char) carte.uc_color + 48;
	
	return temp2;
}

/**
 * @brief Conversion d'un entier en chaine de caractère
 * 
 * @param entier 
 * @return char* 
 */
char* intToStr(int entier)
{
	char* temp2;
	temp2=malloc(1);
	temp2[0] = (char) entier + 48;
	return temp2;
}

/**
 * @brief Vérification de l'item sélectionné dans le menu
 * 
 * @param name 
 */
void func(char *name)
{	move(20, 0);
	clrtoeol();
	mvprintw(20, 0, "Item selectionné : %s", name);
}	

/**
 * @brief Supprimer un élément dans le tableau UnoCard du joueur une fois la carte posée
 * 
 * @param array 
 * @param index 
 * @param array_length 
 */
void remove_element(UnoCard *array, int index, int array_length)
{
   int i;
   for(i = index; i < array_length - 1; i++) array[i] = array[i + 1];
}