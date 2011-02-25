#ifndef _CLIENT_DISPLAY_H
#define _CLIENT_DISPLAY_H

#include "puissance4.h"
#include "client.h"

int client_disp_display_grid(struct p4_grid *g);
int client_disp_display_playerlist(struct p4_client_player_list *playerlist);
struct p4_client_player *client_disp_get_player(
	struct p4_client_player_list *playerlist,char *msg);
int client_disp_display_player_color(struct p4_game_client *game);
int client_disp_display_player_color_start(struct p4_game_client *game);
char client_disp_get_command();
int client_get_column();
int client_disp_display_commandlist_start();
int client_disp_display_commandlist_challenge();
int client_disp_player_switch_start(int sockfd,
	struct p4_client_player_list *playerlist, struct p4_game_client *game);
int client_disp_player_switch_challenge(
	struct p4_client_player_list *playerlist, struct p4_game_client *game);

#endif
