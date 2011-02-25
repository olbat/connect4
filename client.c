#include "client.h"
#include "client_network.h"
#include "client_display.h"
#include "client_game.h"
#include "puissance4.h"
#include "compiler.h"

#include <stdio.h> /* printf */
#include <stdlib.h> /* free */
#include <unistd.h> /* sleep */

struct p4_client_player_list playerlist;
static int sockfd;
pthread_t cswitchtid, cwaitreqtid, clisttid, cmaintid,cchattid;
char *serverip;

int
main(
	int argc,
	char **argv)
{
	int ret;
	char *nick;
	struct p4_game_client game;

	game.grid = 0;

	ret = 0;

	P4_LIST_INIT(&playerlist,client_player_list_cleanup);
	
	if (argc < 2)
		nick = "toto";
	else
		nick = *(argv + 1);
	if (argc >= 3)
		serverip = *(argv + 2);
	else
		serverip = "127.0.0.1";
	
	if (client_net_connection(&sockfd,P4_NET_PORT_DEFAULT,serverip))
		goto err;

	game.sockfd = sockfd;

	pthread_create(&cchattid,0,client_thread_chat,(void *) 0);
	if (client_net_game_connection(sockfd,nick,&playerlist))
		goto err;

	pthread_create(&clisttid,0,client_thread_list,(void *) &playerlist);

	client_thread_start((void *) &game);

	goto out;	
err:
	ret = 1;
out:
	P4_LIST_CLEANUP(&playerlist);
	return ret;
}

void *
client_thread_start(void *g)
{
	struct p4_game_client *game;
	void *tmp;

	game = (__typeof__(game)) g;

	cmaintid = pthread_self();

	while (1)
	{
		grid_init(&(game->grid));

		pthread_create(&cswitchtid,0,client_thread_start_switch,
			(void *) game);
		pthread_create(&cwaitreqtid,0,client_thread_start_waitreq,
			(void *) game);
		pthread_join(cwaitreqtid,&tmp);
		pthread_join(cswitchtid,&tmp);

		grid_cleanup(&(game->grid));
	}

	return (void *) 0;
}

void *
client_thread_start_waitreq(void *g)
{
	struct p4_game_client *game;
	int tmp;

	game = (__typeof__(game)) g;
	
	do {
		tmp = client_net_game_wait_challenge_req(sockfd,&playerlist,game);
	} while (tmp > 0);
	
	if (tmp < 0)
		return (void *) 1;
	
	client_disp_display_player_color_start(game);
	client_game_run(game,&playerlist);

	return (void *) 0;
}

void *
client_thread_start_switch(void *g)
{
	struct p4_game_client *game;
	int tmp,ret;

	ret = 0;
	game = (__typeof__(game)) g;

	client_disp_display_commandlist_start();
	do {
		tmp = client_disp_player_switch_start(sockfd,&playerlist,game);
		if ((tmp) && (tmp != P4_CLIENT_ST_QUIT) && (tmp != P4_CLIENT_ST_END_GAME))
			goto err;
	} while ((tmp != P4_CLIENT_ST_QUIT) && (tmp != P4_CLIENT_ST_END_GAME));
	
	if (tmp == P4_CLIENT_ST_QUIT)
	{
		pthread_cancel(cswitchtid);
		pthread_cancel(cwaitreqtid);
		pthread_cancel(clisttid);
		pthread_cancel(cmaintid);
		exit(0);
	}
	
	pthread_exit((void *) 0);
err:
	pthread_exit((void *) 1);
}

void *
client_thread_list(void *pl)
{
	struct p4_client_player_list *playerlist;

	playerlist = (__typeof__(playerlist)) pl;

	while (1)
	{
		client_net_get_list(playerlist);
		sleep(P4_CLIENT_NET_LIST_REFRESHDELAY);
	}
	
	return (void *) 0;
}

void *
client_thread_chat(void *fd)
{
	client_net_chat_loop(sockfd);

	return (void *) 0;
}
