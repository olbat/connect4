#ifndef _SERVER_NETWORK_GAME_H
#define _SERVER_NETWORK_GAME_H

#include "server.h"
#include "puissance4.h"

int server_net_game_curplay(struct p4_server_game *game);
int server_net_game_notify_play(struct p4_server_game *game,int x,int y);
int server_net_game_notify_curinval(struct p4_server_game *game);
int server_net_game_notify_drawn(struct p4_server_game *game);
int server_net_game_notify_win(struct p4_server_game *game,int x,int y,
	enum p4_game_win_dir dir);


#endif
