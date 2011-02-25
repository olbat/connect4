#ifndef _CLIENT_NETWORK_GAME_h
#define _CLIENT_NETWORK_GAME_h

#include "client.h"


int client_net_game_send_update(struct p4_game_client *g, int col);
int client_net_game_send_quit(struct p4_game_client *g);
int client_net_game_get_update(struct p4_game_client *g);

#endif
