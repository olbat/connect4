#include "server_network.h"
#include "server_game.h"
#include "puissance4.h"
#include "basics.h"

#include <stdlib.h> /* malloc, srandom, random, atoi */
#include <stdio.h> /* perror, fprintf, sprintf */
#include <string.h> /* memset, strlen */
#include <sys/socket.h> /* socket, bind, accept */
#include <netinet/in.h> /* struct sockaddr_in */
#include <arpa/inet.h> /* inet_ntoa */
#include <unistd.h> /* read, write */
#include <time.h> /* time */
#include <signal.h> /* SIGSTOP */
#include <pthread.h> /* pthread_kill */

static char wbuff[P4_NET_SERVER_BUFFER_SIZE_WRITE];
pthread_mutex_t wbuffmutex;

int
server_net_init(
	int *sockfd,
	int port)
{
	struct sockaddr_in sain;

	pthread_mutex_init(&wbuffmutex,0);
	/* init the networking structures */
	sain.sin_family = AF_INET; /* ipv4 or ipv6 */
	sain.sin_port = htons(port);
	sain.sin_addr.s_addr = 0;
	memset(&sain.sin_zero,0,sizeof(sain.sin_zero));

	if ((*sockfd = socket(sain.sin_family,SOCK_STREAM,IPPROTO_TCP)) == -1)
	{
		perror("socket");
		goto err;
	}

	if (bind(*sockfd, (struct sockaddr *) &sain, sizeof(sain)) == -1)
	{
		perror("bind");
		goto err;
	} 

	if (listen(*sockfd,P4_NET_SERVER_MAX_CLIENTS) == -1)
	{
		perror("listen");
		goto err;
	}

	goto out;
err:
	return 1;
out:
	return 0;
}

int
server_net_wait_client(
	int sockfd,
	struct p4_server_player *player
)
{
	socklen_t null;

	fprintf(stdout,"waiting for connections ...\n");

	player->sain = (__typeof__(player->sain)) malloc(sizeof(player->sain));

	null = sizeof(player->sain);
	if ((player->sockfd = accept(sockfd,
		(struct sockaddr *) player->sain, &null)) == -1)
	{
		perror("accept");
		return 1;
	}

	fprintf(stdout,"client connected: %s:%d\n",
		inet_ntoa(player->sain->sin_addr),
		ntohs(player->sain->sin_port));

	return 0;
}

void *
server_net_game_list_serv(void *pl)
{
	int sockfd, csockfd;
	struct sockaddr_in sain;
	socklen_t null;
	struct p4_server_player_list *playerlist;

	playerlist = (__typeof__(playerlist)) pl;

	if (server_net_init(&sockfd,P4_NET_PORT_LIST_DEFAULT))
		return (void *) 1;
	
	while (1)
	{
		null = sizeof(sain);
		if ((csockfd = accept(sockfd,
			(struct sockaddr *) &sain, &null)) == -1)
		{
			perror("accept");
			pthread_exit((void *) 1);
		}

		fprintf(stdout,"client list : %s:%d\n", inet_ntoa(sain.sin_addr),
			ntohs(sain.sin_port));
		
		server_net_game_list(csockfd,playerlist);

		close(csockfd);
	}

	close(sockfd);

	return (void *) 0;
}

int
server_net_game_list(
	int sockfd,
	struct p4_server_player_list *playerlist)
{
	char *ptr;
	static char wbufflist[P4_NET_SERVER_BUFFER_SIZE_WRITE];
	struct p4_net_protocol_match *proto;
	__typeof__(playerlist->list) ptrl;
	int tmp;

	/* send LISTE msg */
	ptr = wbufflist;
	proto = net_proto_get_proto_id(P4_NET_PROTO_LISTE);
	ptr += sprintf(ptr,"%s %d ", proto->name, playerlist->size);

	ptrl = playerlist->list;
	
	if (ptrl)
	{
		while(ptrl->next)
		{
			/* the first element in the list is the new player that
			 * may not yet be allocated, so the nick could be null 
			 */
			if ((ptrl->value) && (ptrl->value->nick))
			{
				tmp = strlen(ptrl->value->nick);
				memcpy(ptr, ptrl->value->nick, tmp);
				ptr += tmp;
				*ptr++ = ' ';
			}
			ptrl = ptrl->next;
		}
		
		if ((ptrl->value) && (ptrl->value->nick))
		{
			tmp = strlen(ptrl->value->nick);
			memcpy(ptr, ptrl->value->nick, tmp);
			ptr += tmp;
		}

		write(sockfd,wbufflist,ptr - wbufflist);
	}
	else
	{
		*(ptr - 1) = 0;
		write(sockfd,wbufflist,ptr - wbufflist - 1);
	}

	return 0;
}

void *
server_net_game_chat_serv(void *pl)
{
	static char rbuffchat[P4_NET_SERVER_BUFFER_SIZE_READ];
	static char wbuffchat[P4_NET_SERVER_BUFFER_SIZE_WRITE];
	int sockfd, csockfd,rbufflen;
	struct sockaddr_in sain;
	socklen_t null;
	struct p4_server_player_list *playerlist;
	struct p4_server_player *player;
	struct p4_net_protocol_match *proto;
	char *ptr, *ptr1, *ptr2;

	playerlist = (__typeof__(playerlist)) pl;

	if (server_net_init(&sockfd,P4_NET_PORT_CHAT_DEFAULT))
		return (void *) 1;
	
	while (1)
	{
		null = sizeof(sain);
		if ((csockfd = accept(sockfd,
			(struct sockaddr *) &sain, &null)) == -1)
		{
			perror("accept");
			pthread_exit((void *) 1);
		}
		if ((rbufflen = read(csockfd,rbuffchat,sizeof(rbuffchat))) < 0)
			return (void *) 1;

		rbuffchat[rbufflen] = 0;

		if (!P4_NET_PROTO_CHECK(rbuffchat,ptr1,proto,P4_NET_PROTO_MSG_VERS))
		{
			ptr2 = ptr1;
			while ((*ptr2) && (*ptr2 != ' '))
				ptr2++;
			*ptr2++ = 0;

			fprintf(stdout,"message from (%s:%d) to %s: %s\n",
				inet_ntoa(sain.sin_addr),ntohl(sain.sin_port)
				,ptr1,ptr2);

			if ((player = server_get_player(playerlist,ptr1)))
			{
				proto = net_proto_get_proto_id(
					P4_NET_PROTO_MSG_DE);
				ptr = wbuffchat;
				ptr += sprintf(ptr,"%s %s %s", proto->name, 
					ptr1,ptr2);
				write(1,wbuffchat,ptr - wbuffchat);
				write(player->sockfd,wbuffchat,ptr - wbuffchat);
			}
			else
			{
				proto = net_proto_get_proto_id(P4_NET_PROTO_MSG_ERR);
				ptr = wbuffchat;
				ptr += sprintf(ptr,"%s %s", proto->name,
					"player not found");
				write(csockfd,wbuffchat,ptr - wbuffchat);
			}
		
		}
		close(csockfd);
	}
	close(sockfd);

	return (void *) 0;
}

int
server_net_game_init(
	struct p4_server_player *player,
	struct p4_server_player_list *playerlist
)
{
	char *ptr;
	struct p4_net_protocol_match *proto;
	int rbufflen;

	/* get HELLO msg */
	if ((rbufflen = read(player->sockfd,player->rbuff,sizeof(player->rbuff))) <= 0)
		goto err;
	
	player->rbuff[rbufflen] = 0;
	
	if (P4_NET_PROTO_CHECK(player->rbuff,ptr,proto,P4_NET_PROTO_HELLO))
	{
		/* >>> TODO: use the error system */
		WRITES(player->sockfd,"error: HELLO expected\n");
		fprintf(stderr,"error: HELLO\n");
		goto err;
	}

	if (server_get_player(playerlist,ptr))
	{
		WRITES(player->sockfd,"HELLO ERR nick already in use\n");
		player->nick = (__typeof__(player->nick)) malloc(2);
		*(player->nick) = '0';
		*(player->nick + 1) = 0;
		fprintf(stderr,"error: nick %s already in use (%s:%d)\n",
			player->nick, inet_ntoa(player->sain->sin_addr),
			ntohs(player->sain->sin_port));
		return 1;
	}

	player->nick = (char *) malloc((rbufflen - (ptr - player->rbuff)) 
		* sizeof(char));
	memcpy(player->nick,ptr,(rbufflen - (ptr - player->rbuff)));

	/* send LISTE msg*/
	server_net_game_list(player->sockfd,playerlist);

	player->flags |= P4_PLAYER_FLAG_INIT;

	goto out;
err:
	return -1;
out:
	return 0;
}

int
server_net_game_challenge(
	struct p4_server_player_list *playerlist,
	struct p4_server_player *player,
	struct p4_server_game *game)
{
	struct p4_net_protocol_match *proto;
	struct p4_server_player *player2;
	char *ptr, *ptr2;
	int tmp, rbufflen;

	/* get DEFI msg */
	if ((rbufflen = read(player->sockfd,player->rbuff,sizeof(player->rbuff))) <= 0)
		goto err;
	
	player->rbuff[rbufflen] = 0;

	if (P4_NET_PROTO_CHECK(player->rbuff,ptr,proto,P4_NET_PROTO_DEFI))
	{
		/* >>> TODO: use the error system */
		WRITES(player->sockfd,"error: DEFI expected\n");
		fprintf(stderr,"error: DEFI\n");
		goto err;
	}

	/* send DEFI ERR msg */
	if (!(player2 = server_get_player(playerlist,ptr)))
	{
		WRITES(player->sockfd,"DEFI ERR challenger not found");
		fprintf(stderr,"error: DEFI challenger not found\n");
		goto err;
	}

	if (player2 == player)
	{
		fprintf(stdout,"error: %s tried to challenge himself\n",
			player->nick);
		WRITES(player->sockfd,"DEFI ERR can't challenge yourself");
		return 1;
	}
	
	fprintf(stdout,"client challenge req: %s -> %s\n",player->nick,
		player2->nick);

	/* send DEFI msg */
	/* send the challenge request to the second player */
	*(ptr - 1) = ' ';

	pthread_cancel(player2->tid);
	write(player2->sockfd,player->rbuff,rbufflen);

	/* get DEFI OK/NO msg */
	if ((rbufflen = read(player2->sockfd,player->rbuff,sizeof(player->rbuff))) <= 0)
		goto err;
	
	player->rbuff[rbufflen] = 0;

	proto = net_proto_get_proto_name(player->rbuff);

	if (proto->id == P4_NET_PROTO_DEFI_OK)
	{
		fprintf(stdout,"client %s accepted\n",player2->nick);
	}
	else if (proto->id == P4_NET_PROTO_DEFI_NO)
	{
		fprintf(stdout,"client %s refused\n",player2->nick);
		*(ptr - 1) = ' ';
		/* send DEFI NO msg */
		/* send the answer to the first player */
		write(player->sockfd,player->rbuff,rbufflen);

		/* launch a new thread for the other player */ 
		pthread_create(&(player2->tid), 0, 
			server_client_thread,
			(void *) player2);
		return 1;
	}
	else
	{
		/* >>> TODO: use the error system */
		WRITES(player->sockfd,"error: DEFI OK/NO expected\n");
		fprintf(stderr,"error: DEFI OK/NO\n");
		goto err;
	}

	/* send DEFI OK msg */
	/* send the answer to the first player */
	*(ptr - 1) = ' ';
	write(player->sockfd,player->rbuff,rbufflen);

	/* send DEFI COUL msg */
	pthread_mutex_lock(&wbuffmutex);
	ptr = wbuff;
	proto = net_proto_get_proto_id(P4_NET_PROTO_DEFI_COUL);

	tmp = strlen(proto->name);
	memcpy(ptr,proto->name,tmp);
	ptr += tmp;

	ptr2 = ptr + 6;

	memcpy(ptr2,proto->name,tmp);
	ptr2 += tmp;
	
	*ptr++ = ' ';
	*ptr2++ = ' ';

	srand(time(0));
	
	if (rand() % 2)
	{
		*ptr++ = token_get_char_value(P4_TOKEN_RED);
		*ptr++ = ' ';
		game->pred = player;
		player->color = P4_TOKEN_RED;

		*ptr2++ = token_get_char_value(P4_TOKEN_BLACK);
		*ptr2++ = ' ';
		game->pblack = player2;
		player2->color = P4_TOKEN_BLACK;
	}
	else
	{
		*ptr++ = token_get_char_value(P4_TOKEN_BLACK);
		*ptr++ = ' ';
		game->pblack = player;
		player->color = P4_TOKEN_BLACK;

		*ptr2++ = token_get_char_value(P4_TOKEN_RED);
		*ptr2++ = ' ';
		game->pred = player2;
		player2->color = P4_TOKEN_RED;
	}

	if (rand() % 2) {
		*ptr++ = token_get_char_value(P4_TOKEN_RED);
		*ptr2++ = token_get_char_value(P4_TOKEN_RED);
		game->pcur = game->pred;
	}
	else
	{
		*ptr++ = token_get_char_value(P4_TOKEN_BLACK);
		*ptr2++ = token_get_char_value(P4_TOKEN_BLACK);
		game->pcur = game->pblack;
	}
	*ptr++ = 0;
	*ptr2 = 0;

	write(player->sockfd,wbuff,ptr - wbuff);
	write(player2->sockfd,ptr + 1,ptr2 - ptr);
	pthread_mutex_unlock(&wbuffmutex);

	goto out;
err:
	return -1;
out:
	return 0;
}
