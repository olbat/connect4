#include "server_network_game.h"
#include "puissance4.h"
#include "server_game.h"
#include "basics.h"

#include <stdio.h> /* perror, fprintf, sprintf */
#include <stdlib.h> /* atoi */
#include <unistd.h> /* read, write */
#include <string.h> /* memset, strlen */

static char wbuff[P4_NET_SERVER_BUFFER_SIZE_WRITE];
pthread_mutex_t wbuffmutex;

/* returns the number of the column where the current player played */
int
server_net_game_curplay(struct p4_server_game *game)
{
	
	static int start = 1;
	struct p4_net_protocol_match *proto;
	char *ptr;
	int rbufflen;
	
	if (start)
	{
		pthread_mutex_init(&wbuffmutex,0);
		start = 0;
	}

	/* get COUP COL msg */
	if ((rbufflen = read(game->pcur->sockfd,game->rbuff,sizeof(game->rbuff))) <= 0)
		goto err;
	
	game->rbuff[rbufflen] = 0;

	if (P4_NET_PROTO_CHECK(game->rbuff,ptr,proto,P4_NET_PROTO_COUP_COL))
	{
		if (P4_NET_PROTO_CHECK(game->rbuff,ptr,proto,P4_NET_PROTO_QUITTE))
		{
			/* >>> TODO: use the error system */
			WRITES(game->pcur->sockfd,"error: COUP COL expected\n");
			fprintf(stderr,"error: COUP COL\n");
			goto err;
		}
		else
		{
			if (game->pcur == game->pred)
			{
				fprintf(stdout,"client %s leaved the challenge\n",
					game->pblack->nick);
				write(game->pblack->sockfd,game->rbuff,rbufflen);
			}
			else
			{
				fprintf(stdout,"client %s leaved the challenge\n",
					game->pred->nick);
				write(game->pred->sockfd,game->rbuff,rbufflen);
			
			}
			return -2;
		}
	}
	
	goto out;
err:
	return -1;
out:
	return atoi(ptr);
}

int
server_net_game_notify_play(
	struct p4_server_game *g,
	int x,
	int y)
{
	struct p4_net_protocol_match *proto;
	char *ptr;

	/* send UPDATE PION msg */
	pthread_mutex_lock(&wbuffmutex);
	ptr = wbuff;
	proto = net_proto_get_proto_id(P4_NET_PROTO_UPDATE_PION);
	ptr += sprintf(ptr,"%s %c %d %d", proto->name, 
		token_get_char_value(g->pcur->color),x,y);
	*ptr++ = 0;

	write(g->pred->sockfd,wbuff,ptr - wbuff);
	write(g->pblack->sockfd,wbuff,ptr - wbuff);
	pthread_mutex_unlock(&wbuffmutex);

	return 0;
}

int
server_net_game_notify_curinval(struct p4_server_game *g)
{
	struct p4_net_protocol_match *proto;
	char *ptr;

	/* send UPDATE COUP INVALIDE msg */
	pthread_mutex_lock(&wbuffmutex);
	ptr = wbuff;
	proto = net_proto_get_proto_id(P4_NET_PROTO_UPDATE_COUP_INVALIDE);
	ptr += sprintf(ptr,"%s", proto->name);
	*ptr++ = 0;

	write(g->pcur->sockfd,wbuff,ptr - wbuff);
	pthread_mutex_unlock(&wbuffmutex);

	return 0;
}

int
server_net_game_notify_drawn(struct p4_server_game *g)
{
	struct p4_net_protocol_match *proto;
	char *ptr;

	/* send UPDATE NUL msg */
	pthread_mutex_lock(&wbuffmutex);
	ptr = wbuff;
	proto = net_proto_get_proto_id(P4_NET_PROTO_UPDATE_NUL);
	ptr += sprintf(ptr,"%s", proto->name);
	*ptr++ = 0;

	write(g->pred->sockfd,ptr,wbuff - ptr);
	write(g->pblack->sockfd,ptr,wbuff - ptr);
	pthread_mutex_unlock(&wbuffmutex);

	return 0;
}

int
server_net_game_notify_win(
	struct p4_server_game *g,
	int x,
	int y,
	enum p4_game_win_dir dir)
{
	struct p4_net_protocol_match *proto;
	char *ptr;

	/* send UPDATE GAGNE msg */
	pthread_mutex_lock(&wbuffmutex);
	ptr = wbuff;
	proto = net_proto_get_proto_id(P4_NET_PROTO_UPDATE_GAGNE);
	ptr += sprintf(ptr,"%s %c %d %d %c", proto->name, 
		token_get_char_value(g->pcur->color),x,y,
		p4_get_win_dir_char_value(dir));
	*ptr++ = 0;

	write(g->pred->sockfd,wbuff,ptr - wbuff);
	write(g->pblack->sockfd,wbuff,ptr - wbuff);
	pthread_mutex_unlock(&wbuffmutex);

	return 0;
}
