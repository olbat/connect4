#include "client.h"
#include "client_network.h"
#include "client_display.h"
#include "client_game.h"
#include "puissance4.h"
#include "compiler.h"
#include "basics.h"

#include <stdio.h> /* printf */
#include <stdlib.h> /* free */
#include <unistd.h>

#define BUFFER_SIZE 256

static char rbuff[BUFFER_SIZE], wbuff[BUFFER_SIZE];
static int rbufflen, wbufflen;
pthread_t cswitchtid, cwaitreqtid, clisttid, cmaintid,cchattid;
char *serverip;

int testclient_loop_write(int *sockfd);
int testclient_loop_read(int *sockfd);
void *client_thread_start_waitreq(void *v) { return (void *)0; }
void *client_thread_start_switch(void *v) { return (void *)0; }

int
main(
	int argc,
	char **argv)
{
	int ret,sockfd,tmp;
	struct p4_client_player_list playerlist;
	struct p4_game_client game;
	char *nick;

	ret = 0;
	game.grid = 0;

	P4_LIST_INIT(&playerlist,client_player_list_cleanup);
	
	if (argc < 2)
		nick = "toto";
	else
		nick = *(argv + 1);
	
	if (client_net_connection(&sockfd,P4_NET_PORT_LIST_DEFAULT,"127.0.0.1"))
		goto err;

	if (client_net_game_connection(sockfd,nick,&playerlist))
		goto err;

	fprintf(stdout,"connection established\n");
	client_disp_display_playerlist(&playerlist);

	pthread_create(&tmp, 0, 
		(void *(*)(void *)) testclient_loop_read,
		(void *) &sockfd);

	testclient_loop_write(&sockfd);

	goto out;	
err:
	ret = 1;
out:
	grid_cleanup(&(game.grid));
	P4_LIST_CLEANUP(&playerlist);
	return ret;
}

int
testclient_loop_read(int *sockfd)
{
	while (1)
	{
		if ((rbufflen = read(*sockfd,rbuff,sizeof(rbuff))) <= 0)
			return 1;
		rbuff[rbufflen] = 0;
		WRITES(1,"\nrecv:");
		write(1,rbuff,rbufflen);
		WRITES(1,"\nsend:");
	}
	pthread_exit((void *)0);
}

int
testclient_loop_write(int *sockfd)
{
	while (1)
	{
		WRITES(1,"\nsend:");

		if ((wbufflen = read(0,wbuff,sizeof(wbuff))) <= 0)
			return 1;
		wbuff[--wbufflen] = 0;
		write(*sockfd,wbuff,wbufflen);
	}
	pthread_exit((void *)0);
}
