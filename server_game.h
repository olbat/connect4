#ifndef _SERVER_GAME_H
#define _SERVER_GAME_H

#include "server.h"
#include "puissance4.h"

__inline void server_player_cleanup(struct p4_server_player **p);
__inline__ void server_player_list_cleanup(struct p4_server_player_list_cell *cell);
struct p4_server_player *server_get_player(
	struct p4_server_player_list *playerlist,char *nick);
int server_game_check_drawn(struct p4_server_game *g);
int server_game_check_winner(struct p4_server_game *g, int col, int row);
int server_game_get_winner_dir(struct p4_server_game *g,
	enum p4_game_win_dir dir,int *col,int *row);
int server_game_playable(struct p4_server_game *g,int *col,int *row);
int server_game_run(struct p4_server_game *g);

#endif
