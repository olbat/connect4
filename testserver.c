#include "puissance4.h"
#include "server_network.h"
#include "server_game.h"
#include "basics.h"

#include <unistd.h> /* close */
#include <stdlib.h> /* free */
#include <string.h> /* memset */
#include <netinet/in.h> /* struct sockaddr_in */

int testserver_loop(int sockfd);
int server_client_thread2(int sockfd,struct p4_server_player_list *playerlist);
void *server_client_thread(void *d) { return (void *) 0; }


int
main(void)
{
	int sockfd;
	struct p4_server_player_list playerlist;
	
	P4_LIST_INIT(&playerlist,server_player_list_cleanup);

	if (server_net_init(&sockfd))
		goto err;

	if (server_client_thread2(sockfd,&playerlist))
		goto err;

	goto out;	
err:
	return 1;
out:
	P4_LIST_CLEANUP(&playerlist);
	close(sockfd);

	return 0;
}

int
server_client_thread2(
	int sockfd,
	struct p4_server_player_list *playerlist
)
{
	int ret;
	struct p4_grid *g;
	struct p4_server_player *player;

	ret = 0;
	g = 0;	
	player = (__typeof__(player)) malloc(sizeof(player));
	P4_SERVER_PLAYER_INIT(player);


	if (server_net_wait_client(sockfd,player))
		goto err;

	if (testserver_loop(player->sockfd))
		goto err;

	grid_init(&g);

	goto out;	
err:
	ret = 1;
	/* >>> TODO: delete player from list */
	free(player);
out:
	grid_cleanup(&g);
	close(player->sockfd);

	return ret;
}

#define BUFFER_SIZE 256

int
testserver_loop(int sockfd)
{
	static char rbuff[BUFFER_SIZE], wbuff[BUFFER_SIZE];
	static int rbufflen, wbufflen;

	while (1)
	{
		if ((rbufflen = read(sockfd,rbuff,sizeof(rbuff))) <= 0)
			return 1;
		write(1,rbuff,rbufflen);
		WRITES(1,"\nsend:");

		if ((wbufflen = read(0,wbuff,sizeof(wbuff))) <= 0)
			return 1;

		write(sockfd,wbuff,wbufflen);
	}
	return 0;
}	
