#ifndef _CLIENT_NETWORK_H
#define _CLIENT_NETWORK_H

#include "client.h"

#define P4_NET_CLIENT_BUFFER_SIZE_READ 128
#define P4_NET_CLIENT_BUFFER_SIZE_WRITE 128

#define P4_NET_CLIENT_END 8

int client_net_connection(int *sockfd, int port, char *ipaddr);
int client_net_get_list(struct p4_client_player_list *playerlist);
int client_net_send_message(struct p4_client_player_list *playerlist);
int client_net_game_connection(int sockfd, char *nick, 
	struct p4_client_player_list *playerlist);
int client_net_game_challenge(int sockfd,struct p4_client_player *player,
	struct p4_game_client *game);
int client_net_game_get_challenge_color(int sockfd,
	struct p4_client_player *player,struct p4_game_client *game);
int client_net_game_wait_challenge_req(int sockfd,
	struct p4_client_player_list *playerlist, struct p4_game_client *game);
int client_net_chat_loop(int sockfd);
int client_net_read(char *crbuff, int clen);

#endif
