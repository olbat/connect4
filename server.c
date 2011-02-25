#include "server.h"
#include "puissance4.h"
#include "server_network.h"
#include "server_game.h"
#include "server_debug.h"

#include <unistd.h> /* close */
#include <stdlib.h> /* free */
#include <string.h> /* memset */
#include <netinet/in.h> /* struct sockaddr_in */
#include <pthread.h> /* pthread_create */
#include <signal.h> /* SIGCONT */

static struct p4_server_player_list *server_playerlist;

int
main(void)
{
	int *sockfd;
	struct p4_server_player *player;
	pthread_t listtid, chattid;
	
	sockfd = (__typeof__(sockfd)) malloc(sizeof(sockfd));
	server_playerlist = (__typeof__(server_playerlist))
		malloc(sizeof(server_playerlist));
	
	P4_LIST_INIT(server_playerlist,server_player_list_cleanup);

	if (server_net_init(sockfd,P4_NET_PORT_DEFAULT))
		goto err;

	/* start listen for the LISTE service */
	pthread_create(&listtid, 0, server_net_game_list_serv, 
		server_playerlist);
	pthread_create(&chattid, 0, server_net_game_chat_serv, 
		server_playerlist);

	/* client thread */
	while (1)
	{
		player = (__typeof__(player)) malloc(sizeof(player));
		P4_SERVER_PLAYER_INIT(player);

		if (server_net_wait_client(*sockfd,player))
			goto err;

		/* server_debug_display_playerlist(server_playerlist,"[main]"); */

		P4_LIST_ADD(server_playerlist,player);

		pthread_create(&(player->tid), 0, server_client_thread,
			(void *) player);
	}

	goto out;	
err:
	return 1;
out:
	P4_LIST_CLEANUP(server_playerlist);
	close(*sockfd);
	free(sockfd);

	return 0;
}

void *
server_client_thread(void *d)
{
	int ret,tmp;
	struct p4_server_game *game;
	struct p4_server_player *player;
	
	game = (__typeof__(game)) malloc(sizeof(game));
	player = (__typeof__(player)) d;
	ret = 0;
	game->grid = 0;

	if (!(player->flags & P4_PLAYER_FLAG_INIT))
	{
		tmp = server_net_game_init(player,server_playerlist);
		if (tmp < 0)
			goto err;
		else if (tmp > 0)
			pthread_exit((void *) 0);
	}
	
	while (1)
	{
		/* server_debug_display_playerlist(server_playerlist,"[thread]"); */

		tmp = server_net_game_challenge(server_playerlist,player,game);
		if (tmp < 0)
			goto err;
		else if (tmp > 0)
			continue;

		grid_init(&(game->grid));

		server_game_run(game);

		if (game->pred == player)
		{
			pthread_create(&(game->pblack->tid),0,
				server_client_thread,(void *)game->pblack);
		}
		else
		{
			pthread_create(&(game->pred->tid),0,
				server_client_thread,(void *)game->pred);
		}

		grid_cleanup(&(game->grid));
	}

	goto out;	
err:
	ret = 1;
	/* >>> TODO: delete player from list */
	/* server_player_cleanup(&player); */
out:
	/* grid_cleanup(&(game->grid)); */
	close(player->sockfd);

	pthread_exit((void *)ret);
}
