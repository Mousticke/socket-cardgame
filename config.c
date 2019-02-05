/**
 * @file config.c
 * @author Doral - Jbara - Forey
 * @brief fichier config
 * @version 0.1
 * @date 2018-12-22
 * 
 * @copyright Copyright (c) 2018
 * 
 */
#include "config.h"


/**
 * @brief Affichage des cartes côté serveur
 * 
 * @param uDeck 
 */
void printCard(UnoCard * uDeck)
{
    char* color;
	int face; 
	for(int i = 0; i < 42; i++){
		switch (uDeck[i].uc_color)
		{
			case 0:
				color = "Bleu";
				face = uDeck[i].uc_face;
				break;
			case 1:
				color = "Jaune";
				face = uDeck[i].uc_face;
				break;
			case 2:
				color = "Vert";
				face = uDeck[i].uc_face;
				break;
			case 3:
				color = "Rouge";
				face = uDeck[i].uc_face;
				break;
			case 4:
				color = "Joker";
				face = uDeck[i].uc_face;
				break;										
		}
		printf("[ %d %s ]\n", face, color);
	}
}

/**
 * @brief préparation d'un envoie d'un message du serveur vers le client
 * 
 * @param msg_code 
 * @param payload 
 * @param socket 
 */
void send_msg(int msg_code, const char* payload, int socket) {
	char msg[MSG_SIZE];
	sprintf(msg, "%d %s", msg_code, payload);
	send_prepared_msg(msg, socket);
}

/**
 * @brief Envoie d'un requête préparé
 * 
 * @param pmsg 
 * @param socket 
 */
void send_prepared_msg(char* pmsg, int socket) {
	if (send(socket, pmsg, MSG_SIZE, 0) == -1) {
		perror("Failed to send a mesesage to the serveur");
		exit(EXIT_FAILURE);
	}
}


/**
 * @brief Mélange du deck initial
 * 
 * @param card 
 */
void shuffle(UnoCard* card)
{
    int i;
	UnoCard tmp;
    srand(time(NULL)); //Initialisation de rand

    for(i = 0; i<NB_CARTES; i++)
    {
        int r = i + (rand() % (NB_CARTES-i));
		tmp = card[i];
		card[i] = card[r];
		card[r] = tmp;
    }
}

char* req2str(struct t_frame a){
	char* temp;
	temp = malloc(strlen(a.code)+strlen(a.msg)+1); 
	strcat(temp, a.code);
	strcat(temp,":");
	strcat(temp,a.msg);
	return temp;
}

struct t_frame str2req(char* str){
	struct t_frame a;
	char code[4];
	char msg[20];

	sscanf(str,"%[^:]:%s",code, msg);
	strcpy(a.code, code);
	strcpy(a.msg, msg);
	return a;
}