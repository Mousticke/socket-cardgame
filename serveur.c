/**
 * @file serveur.c
 * @author Doral - Jbara - Forey
 * @brief fichier serveur
 * @version 0.1
 * @date 2018-12-22
 * 
 * @copyright Copyright (c) 2018
 * 
 */

#include "config.h"

//card currentCard;
UnoCard currentCard;
int currentPlayer = -1;
int nbJoueur = 0;
pid_t pid,pidR;
int se;
pthread_t t_client[5];
int currentThread = 0;
char* sending;
char* dealing;
int dataToInt;
int next = 0;
UnoCard deck[] = {
	{ZERO,GREEN},
	{ONE,GREEN},
	{TWO,GREEN},
	{THREE,GREEN},
	{FOUR,GREEN},
	{FIVE,GREEN},
	{SIX,GREEN},
	{SEVEN,GREEN},
	{EIGHT,GREEN},
	{NINE,GREEN},
	{ZERO,RED},
	{ONE,RED},
	{TWO,RED},
	{THREE,RED},
	{FOUR,RED},
	{FIVE,RED},
	{SIX,RED},
	{SEVEN,RED},
	{EIGHT,RED},
	{NINE,RED},
	{ZERO,YELLOW},
	{ONE,YELLOW},
	{TWO,YELLOW},
	{THREE,YELLOW},
	{FOUR,YELLOW},
	{FIVE,YELLOW},
	{SIX,YELLOW},
	{SEVEN,YELLOW},
	{EIGHT,YELLOW},
	{NINE,YELLOW},	
	{ZERO,BLUE},
	{ONE,BLUE},
	{TWO,BLUE},
	{THREE,BLUE},
	{FOUR,BLUE},
	{FIVE,BLUE},
	{SIX,BLUE},
	{SEVEN,BLUE},
	{EIGHT,BLUE},
	{NINE,BLUE},
	{TEN, JOKER},
	{TEN, JOKER}
};

/**
 * @brief Fermeture de la socket et arrêt des thread
 * 
 * @param signal 
 */
void deroute (int signal){
	int status;
	switch (signal){
		case SIGCHLD:
			CHECK(pidR=wait(&status),"Problem while waiting");
			if(WIFEXITED(status)){
				printf("\033[01;32mFin du fils [PID=%i] par un exit (normal) avec le status=%i\033[00m\n",pidR,WEXITSTATUS(status));
			}
			if(WIFSIGNALED(status)){
				printf("\033[01;31mFin du fils [PID=%i] par un signal (anormal) avec le signal=%i\033[00m\n",pidR,WTERMSIG(status));
			}
			break;
		case SIGINT:
			//il faut fermer les sd
			for (int i = 0; i < currentThread; i++){
				pthread_join (t_client[i], NULL);
			}
			printf("%s\n", "closing se");
			CHECK(close(se),"erreur close(se)");
			sleep(1);
			exit(0);
	}
}

/**
 * @brief Conversion de la chaine de caractère en UnoCard
 * 
 * @param str 
 * @return UnoCard 
 */
UnoCard strToUnoCard(char *str)
{
    UnoCard temp = {str[0] - 48,str[2] - 48};
	return temp;
}



/**
 * @brief Vérifier si l'on peut jouer la carte. La premier carte côté serveur sera considérée comme un joker.
 * 
 * @param c 
 * @return int 
 */
int canPlayCard(UnoCard c){
	if(c.uc_face == currentCard.uc_face || 
    c.uc_color == currentCard.uc_color || 
    c.uc_face == 10 || currentCard.uc_face == 10){
        printf("uc face %d\n", c.uc_face);
        printf("uc color %d\n", c.uc_color);
        currentCard.uc_face = c.uc_face;
        currentCard.uc_color = c.uc_color;
        printf("current face %d\n", currentCard.uc_face);
        printf("current color %d\n", currentCard.uc_color);
        printf("\n");
		return 1;
	}
	return 0;
}

/**
 * @brief Envoie de la requete au client
 * 
 * @param sock 
 * @param msg 
 * @param usr 
 */
void sendWithAck(int* sock, char * msg, int usr){
	char reponse[255];
	CHECK(write(sock[usr], msg, strlen(msg)+1), "Can't send"); 
	
}

/**
 * @brief Conversin de la UnoCard en chaine de caractère pour l'envoie
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
 * @brief Dialogue avec le client en fonction du code requête passé
 * 
 * @param param 
 */
void dialogueClt (void* param) { 
	struct arg_struct *args = param;
	//int sd = (int *) param;
	int* sd = args->arg1;
	int usr = args->arg2;
	int noJoueur = -1;
	printf("\033[01;33m---------------New thread [socket = %d]---------------\033[00m\n",sd[usr]);

	char requete[MAX_BUFF];
	char data[MAX_BUFF];
    // BUFFER = REQUETE:DATA
	do { 
		read(sd[usr], buffer, sizeof(buffer)); 
		printf("%s\n", buffer);
		sscanf(buffer, "%[^:]:%[^:]:%s",requete, buffer,data);
		printf("DATA : %s\n", data);
		if(atoi(buffer) == atoi("201")){
			next++;
			dataToInt = atoi(data)+((currentThread+1)*11)+next;
		}else{
			dataToInt = atoi(data)+((usr)*11);
		}
		
		printf("DATA TO INT %d\n", dataToInt);
		switch(atoi(requete)) { //atoi convertit la chaine de charactère en entier
			case 600:
				if(dataToInt > 42)
				{
					printf("NOT OK. Plus de carte disponible\n");
					write(sd[usr], NOK, strlen(NOK)+1);
				}else
				{
					dealing = malloc(100);
					strcpy(dealing, DEALINGCARD);
					strcat(dealing,":");
					strcat(dealing,"20");
					strcat(dealing,":");
					strcat(dealing, card2str(deck[dataToInt]));
					sendWithAck(sd, dealing, usr);
					char* color;
					int face; 
					switch (deck[dataToInt].uc_color)
					{
						case 0:
							color = "Bleu";
							face = deck[dataToInt].uc_face;
							break;
						case 1:
							color = "Jaune";
							face = deck[dataToInt].uc_face;
							break;
						case 2:
							color = "Vert";
							face = deck[dataToInt].uc_face;
							break;
						case 3:
							color = "Rouge";
							face = deck[dataToInt].uc_face;
							break;
						case 4:
							color = "Joker";
							face = -1;
							break;										
					}
					printf("\n --------------------- Card [%d - %s] Sent to %d\n -------------------", face, color, sd[usr]);
					free(dealing);
				}
				break;
			case 0 : 
				write(sd[usr], BYE, strlen(BYE)+1); 
				printf("%s\n",buffer);
				break; 
			case 100 : 
				if(nbJoueur<=MAX_PLAYERS){ //joueurMax
					noJoueur = nbJoueur++;
					write(sd[usr], SOK, strlen(SOK)+1);
					printf("OK : Numéro Joueur : %d\n", noJoueur); 
				}
				else{
					write(sd[usr], NOK, strlen(NOK)+1);
					printf("NOT OK : deja 4 joueurs \n"); 
				}
				break;
			case 300 : 
				if(canPlayCard(strToUnoCard(data)) == 1){
					printf("OK\n");
					//write(sd[usr], SOK, strlen(SOK)+1); 
					for(int i = 0; i < currentThread; i++)
					{
						sending = malloc(100);
						strcpy(sending, PRINTCARD);
						strcat(sending,":");
						strcat(sending,"20");
						strcat(sending,":");
						strcat(sending, card2str(currentCard));
						sendWithAck(sd, sending, i);
						printf("\n --------------------- Sent to %d\n -------------------", sd[i]);
						free(sending);
					}

					//REMOVE CARD FROM HAND

					
				}else{
					printf("NOT OK\n");
					write(sd[usr], NOK, strlen(NOK)+1); 
				}
				printf("CARTE : message recu  : \t %s\n", buffer); 
				break; 
			default: 
				write(sd[usr], NOK, strlen(NOK)+1); 
				printf("NOT OK : message recu : \t %s\n", buffer);
				break; 
		}
	}while ( atoi(requete) != 0 );
	close(sd[usr]);
	pthread_exit (NULL);
}

int main() {
    currentCard.uc_face = 10;
	currentCard.uc_color = 4;
	int sd[3];
 	struct arg_struct args;
	

    shuffle(deck);
    printCard(deck);
	printf("\n");


	struct sigaction newact;
	newact.sa_handler = deroute;
	newact.sa_flags = SA_RESTART;
	CHECK (sigemptyset (&newact.sa_mask), "Erreur SIGEMPTYSET");
	CHECK (sigaction (SIGCHLD, &newact, NULL), "Erreur SIGACTION");
	CHECK (sigaction (SIGINT, &newact, NULL), "Erreur SIGACTION");


	//int se,sd;
	struct sockaddr_in svc, clt; 
	socklen_t cltLen;

	// Création de la socket de réception d’écoute des appels 
	CHECK(se=socket(PF_INET, SOCK_STREAM, 0), "Can't create"); // Préparation de l’adressage du service (d’appel) 
	svc.sin_family = PF_INET; 
	svc.sin_port   = htons(PORT_SVC); 
	svc.sin_addr.s_addr = INADDR_ANY;
	memset(&svc.sin_zero, 0, 8); 
	// Association de l’adressage préparé avec la socket d’écoute 
	CHECK(bind(se, (struct sockaddr *) &svc, sizeof(svc)) , "Can't bind"); 
	// Mise en écoute de la socket 
	CHECK(listen(se, 5) , "Can't calibrate");
	// Boucle permanente de service 
	while (1) { //  Attente d’un appel
		cltLen = sizeof(clt); 
		CHECK(sd[currentThread]=accept(se, (struct sockaddr *)&clt, &cltLen) , "Can't connect");
		args.arg1[currentThread] = sd[currentThread];
		args.arg2 = currentThread;
		CHECK(pthread_create(&t_client[++currentThread],NULL,dialogueClt,(void*)&args ),"Problem while threading");
	}
	close(se);
	for (int i = 0; i < 5; i++){
		pthread_join (t_client[i], NULL);
	}
	return 0; 
}