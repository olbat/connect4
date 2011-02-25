#ifndef _SERVER_NETWORK_H
#define _SERVER_NETWORK_H

#include "server.h"
#include "puissance4.h"

#include <netinet/in.h> /* struct sockaddr_in */

#define P4_NET_SERVER_MAX_CLIENTS	16


int server_net_init(int *sockfd,int port);
int server_net_wait_client(int sockfd, struct p4_server_player *player);
int server_net_game_list(int sockfd, struct p4_server_player_list *playerlist);
void *server_net_game_list_serv(void *pl);
void *server_net_game_chat_serv(void *pl);
int server_net_game_init(struct p4_server_player *player,
	struct p4_server_player_list *playerlist);
int server_net_game_challenge(struct p4_server_player_list *playerlist,
	struct p4_server_player *player,struct p4_server_game *game);

#endif
