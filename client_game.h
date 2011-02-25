#ifndef _CLIENT_GAME_H
#define _CLIENT_GAME_H

#include "client.h"

void client_player_list_cleanup(struct p4_client_player_list_cell *cell);
struct p4_client_player *client_get_player(
	struct p4_client_player_list *playerlist, char *nick);
int client_game_run(struct p4_game_client *g,
	struct p4_client_player_list *playerlist);
int client_game_play(struct p4_game_client *g);
int client_game_set_win_grid(struct p4_game_client *g,enum p4_game_win_dir dir,
	int col,int row);

#endif
