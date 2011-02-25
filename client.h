#ifndef _CLIENT_H
#define _CLIENT_H

#include "puissance4.h"
#include "compiler.h"

#include <stdlib.h>
#include <pthread.h>

extern pthread_t cswitchtid, cwaitreqtid;

#define P4_CLIENT_ST_QUIT 2 
#define P4_CLIENT_ST_END_GAME 3
#define P4_CLIENT_NET_LIST_REFRESHDELAY 10 

enum p4_client_state
{
	P4_CLIENT_STATE_OK,
	P4_CLIENT_STATE_ERROR,
	P4_CLIENT_STATE_ERROR_INVAL
};

enum p4_net_state_client
{
	P4_NET_STATE_CLIENT_START,
	P4_NET_STATE_CLIENT_STOP,
	P4_NET_STATE_CLIENT_ERROR,
	P4_NET_STATE_CLIENT_ERROR_INVAL,

	P4_NET_STATE_CLIENT_LISTE,
	P4_NET_STATE_CLIENT_DEFI_COUL,
	P4_NET_STATE_CLIENT_DEFI_ERR,
	P4_NET_STATE_CLIENT_UPDATE_PION,
	P4_NET_STATE_CLIENT_UPDATE_COUP_INVALIDE,
	P4_NET_STATE_CLIENT_UPDATE_GAGNE,
	P4_NET_STATE_CLIENT_UPDATE_NUL,
	P4_NET_STATE_CLIENT_MSG_ERR,
	P4_NET_STATE_CLIENT_MSG_DE
};

struct p4_client_player
{
	char *nick;
};

struct p4_client_player_list
{
	struct p4_client_player_list_cell *list;
	unsigned int size;
	void (*cleanup_cell) (struct p4_client_player_list_cell *);
};

struct p4_client_player_list_cell
{
	struct p4_client_player *value;
	struct p4_client_player_list_cell *next;
};

struct p4_game_client
{
	struct p4_grid *grid;
	int sockfd;
	struct p4_client_player *pred;
	struct p4_client_player *pblack;
	struct p4_client_player *pcur;
};


int display_grid(struct p4_grid *g);
enum p4_client_state client_play(int sockfd, char *msg, int size);
void *client_thread_start(void *pl);
void *client_thread_start_waitreq(void *g);
void *client_thread_start_switch(void *g);
void *client_thread_list(void *g);
void *client_thread_chat(void *fd);

#endif
