/**
 * @file config.h
 * @author Doral - Jbara - Forey
 * @brief 
 * @version 0.1
 * @date 2018-12-22
 * 
 * @copyright Copyright (c) 2018
 * 
 */

#ifndef CONFIG_H
#define CONFIG_H


#include <curses.h>
#include <unistd.h> 
#include <stdlib.h> 
#include <stdio.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <signal.h>
#include <sys/wait.h>
#include <pthread.h>
#include <menu.h>
#include <math.h>

#define CHECK(sts,msg) if ((sts) == -1) {perror(msg);exit(-1);} 
#define PORT_SVC 5000 
#define INADDR_SVC "127.0.0.1"
#define MSG "100:Je dis que \"le fond de l’eau est clair par ici ! Où ça ?\"" 
#define BYE "000:Au revoir et à bientôt ..." 
#define ERROR_REQ "200:Requête ou réponse non reconnue !" 
#define SOK "OK" 
#define NOK  "Not OK" 
#define MAX_BUFF 1024 
#define NB_CARTES 42
#define MAX_PLAYERS 3
#define MSG_SIZE 82

#define COK "OK"
#define PLAYCARD "300"
#define PRINTCARD "500"
#define DEALINGCARD "700"
#define DRAWCARDFROMSERVER "800"
#define KEY_ENTER_CUSTOM 10
#define EMPTY	' '
#define WHITE 5
#define BLACK 6

#define WAIT		0 //server -> client
#define REFUSE		1 //server -> client
#define DEAL		4 //server -> client
#define ASK			5 //server -> client
#define GIVE		7 //server -> client
#define ROUND		9 //server -> client
#define SCORES		11 //server -> client
#define WINNER		12 

#define	MULTIPLE_COLOR 10

#define POSFIRSTCARD 10

#define HAND 11

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#define CTRLD 	4

char buffer[MAX_BUFF]; 


/**
 * @brief Définition de la couleur des cartes
 * 
 */
enum color
{
	BLUE, YELLOW, GREEN, RED, JOKER
};

/**
 * @brief Définir enum color en tant que Color
 * 
 */
typedef enum color Color;

/**
 * @brief Définition des faces des cartes où TEN sera le joker
 * 
 */
enum face
{
	ZERO, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, TEN
};

/**
 * @brief Définition de enum face en tant que Face
 * 
 */
typedef enum face Face;

/**
 * @brief Structure d'une carte de uno composé d'une face et d'une couleur
 * 
 */
struct unocard
{
	Face uc_face;
	Color uc_color;
};
/**
 * @brief Définition de la struct unocard en tant que UnoCard
 * 
 */
typedef struct unocard UnoCard;

typedef struct t_frame{
	char code[3];
	char msg[20];
}t_frame;

/**
 * @brief Définition d'une strcture arguments contenant la socket et le numéro de la socket pour les threads
 * 
 */
struct arg_struct {
    int arg1[3];
    int arg2;
};

void printCard(UnoCard * unocard);
void shuffleCard(UnoCard * unocard);
char* req2str(struct t_frame a);
struct t_frame str2req(char* str);
void send_msg(int msg_code, const char* payload, int socket);
void send_prepared_msg(char* pmsg, int socket);
void shuffle(UnoCard* card);

#endif