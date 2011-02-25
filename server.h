#ifndef _SERVER_H
#define _SERVER_H

#include "puissance4.h"
#include "compiler.h"

#include <netinet/in.h> /* struct sockaddr_in */
#include <pthread.h> /* pthread_t */

extern pthread_t ctid;

#define P4_NET_SERVER_BUFFER_SIZE_READ 	64
#define P4_NET_SERVER_BUFFER_SIZE_WRITE	256


#define P4_SERVER_PLAYER_INIT(P) \
	(P)->color = P4_TOKEN_EMPTY; \
	(P)->nick = (__typeof__((P)->nick)) 0; \
	(P)->sain = (__typeof__((P)->sain)) 0; \
	(P)->sockfd = (__typeof__((P)->sockfd)) 0; \
	(P)->lock = 0; \
	(P)->flags = P4_PLAYER_FLAG_NOINIT;
	
#define P4_SERVER_SWITCH_CURPLAYER(G) \
	if ((G)->pcur == (G)->pred) \
		(G)->pcur = (G)->pblack;\
	else \
		(G)->pcur = (G)->pred; \


enum p4_server_state
{
	P4_SERVER_STATE_OK,
	P4_SERVER_STATE_ERROR,
	P4_SERVER_STATE_ERROR_INVAL
};

enum p4_net_state_server
{
	P4_NET_STATE_SERVER_START,
	P4_NET_STATE_SERVER_STOP,
	P4_NET_STATE_SERVER_ERROR,
	P4_NET_STATE_SERVER_ERROR_INVAL,

	P4_NET_STATE_SERVER_HELLO,
	P4_NET_STATE_SERVER_DEFI,
	P4_NET_STATE_SERVER_COUP,
	P4_NET_STATE_SERVER_MSG_VERS,
	P4_NET_STATE_SERVER_QUITTE
};

enum p4_server_player_flags
{
	/* last bit is always 1 */
	P4_PLAYER_FLAG_NOINIT = 0x00000000,
	P4_PLAYER_FLAG_INIT = 0x00000001
};

struct p4_server_player
{
	enum p4_token color;
	char *nick;
	struct sockaddr_in *sain;
	int sockfd;
	pthread_t tid;
	int flags;
	int lock;
	char rbuff[P4_NET_SERVER_BUFFER_SIZE_READ];
};

struct p4_server_player_list
{
	struct p4_server_player_list_cell *list;
	unsigned int size;
	void (*cleanup_cell) (struct p4_server_player_list_cell *);
};

struct p4_server_player_list_cell
{
	struct p4_server_player *value;
	struct p4_server_player_list_cell *next;
};

struct p4_net_protocol_server
{
	enum p4_net_protocol id;
	int argc;
	int (*func)(char **);
};

struct p4_server_game
{
	char rbuff[P4_NET_SERVER_BUFFER_SIZE_READ];
	struct p4_grid *grid;
	struct p4_server_player *pred;
	struct p4_server_player *pblack;
	struct p4_server_player *pcur;
};

void *server_client_thread(void *d);

#endif
