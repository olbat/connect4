#include "client_network.h"
#include "client_display.h"
#include "client_game.h"
#include "puissance4.h"
#include "basics.h"

#include <sys/socket.h> /* socket */
#include <netinet/in.h> /* struct sockaddr_in */
#include <arpa/inet.h> /* inet_aton */
#include <unistd.h> /* read, write */
#include <stdio.h> /* perror */
#include <string.h> /* memset */

static char srbuff[P4_NET_CLIENT_BUFFER_SIZE_READ];
char *rbuff = srbuff;
static char wbuff[P4_NET_CLIENT_BUFFER_SIZE_WRITE];
static int rbufflen;
extern char *serverip;

int
client_net_connection(
	int *sockfd,
	int port,
	char *ipaddr)
{
	struct sockaddr_in sain;

	/* init the networking structures */
	sain.sin_family = AF_INET;
	sain.sin_port = htons(port);
	inet_aton(ipaddr,&(sain.sin_addr));
	memset(&sain.sin_zero,0,sizeof(sain.sin_zero));

	if ((*sockfd = socket(sain.sin_family,SOCK_STREAM,IPPROTO_TCP)) == -1)
	{
		perror("socket");
		goto err;
	}

	if (connect(*sockfd,(struct sockaddr *)&sain,sizeof(sain)) == -1)
	{
		perror("connect");
		goto err;
	}

	goto out;	
err:
	return 1;
out:
	return 0;
}

int
client_net_get_list(struct p4_client_player_list *playerlist)
{
	static char rbufflist[P4_NET_CLIENT_BUFFER_SIZE_READ];
	int sockfd, rbufflistlen,tmp;
	struct p4_net_protocol_match *proto;
	struct p4_client_player *tmpplayer;
	char *ptr, *ptr2;

	/* connect to the list server */
	if (client_net_connection(&sockfd,P4_NET_PORT_LIST_DEFAULT,serverip))
	{
		fprintf(stderr,"error: impossible to connect to list server\n");
		goto err;
	}

	/* get LISTE msg */
	if ((rbufflistlen = read(sockfd,rbufflist,sizeof(rbufflist))) <= 0)
		goto err;

	rbufflist[rbufflistlen] = 0;
	
	close(sockfd);
	
	if (P4_NET_PROTO_CHECK(rbufflist,ptr,proto,P4_NET_PROTO_LISTE))
	{
		/* >>> TODO: use the error system */
		fprintf(stderr,"error: LISTE\n");
		goto err;
	}

	ptr2 = ptr;
	while ((*ptr2) && (*ptr2 != ' '))
		ptr2++;
	*ptr2++ = 0;

	tmp = strtol(ptr,0,10);
	ptr = ptr2;
	while (tmp-- > 0)
	{
		while ((*ptr2) && (*ptr2 != ' '))
			ptr2++;
		*ptr2++ = 0;
		
		/* if the player wasnt already in the list */
		if (client_get_player(playerlist,ptr) == (__typeof__(tmpplayer))-1)
		{
			tmpplayer = (__typeof__(tmpplayer)) 
				malloc(sizeof(tmpplayer));
			tmpplayer->nick = (char *) malloc((ptr2 - ptr) 
				* sizeof(char));
			memcpy(tmpplayer->nick,ptr,ptr2 - ptr);
			P4_LIST_ADD(playerlist,tmpplayer);
		}
		ptr = ptr2;

		if ((!*ptr) && (tmp > 0))
			goto err;	
	}

	goto out;
err:
	return 1;
out:
	return 0;

}

int
client_net_send_message(struct p4_client_player_list *playerlist)
{
	static char wbuffchat[P4_NET_CLIENT_BUFFER_SIZE_WRITE];
	static char rbuffchat[P4_NET_CLIENT_BUFFER_SIZE_READ];
	int sockfd, rbuffchatlen;
	struct p4_net_protocol_match *proto;
	struct p4_client_player *player;
	char *ptr;

	player = client_disp_get_player(playerlist,"Send message to player No: ");
	WRITES(1,"Message: ");
	if ((rbuffchatlen = read(0,rbuffchat,sizeof(rbuffchat))) <= 0)
		return -1;
	rbuffchat[rbuffchatlen] = 0;
	
	/* send MSG VERS msg */
	ptr = wbuffchat;
	proto = net_proto_get_proto_id(P4_NET_PROTO_MSG_VERS);
	ptr += sprintf(ptr,"%s %s %s",proto->name,player->nick,rbuffchat);

	/* connect to the chat server */
	if (client_net_connection(&sockfd,P4_NET_PORT_CHAT_DEFAULT,serverip))
	{
		fprintf(stderr,"error: impossible to connect to list server\n");
		return -1;
	}

	write(sockfd,wbuffchat,ptr - wbuffchat);
	close(sockfd);
	
	return 0;
}

int
client_net_game_connection(
	int sockfd,
	char *nick,
	struct p4_client_player_list *playerlist
)
{
	struct p4_net_protocol_match *proto;
	struct p4_client_player *tmpplayer;
	char *ptr, *ptr2;
	int tmp;

	/* send HELLO msg */
	proto = net_proto_get_proto_id(P4_NET_PROTO_HELLO);

	ptr = wbuff;
	ptr += sprintf(ptr,"%s %s",proto->name,nick);

	write(sockfd,wbuff,ptr - wbuff);

	/* get LISTE msg */
	if ((rbufflen = client_net_read(rbuff,0)) <= 0)
		goto err;
	/* if ((rbufflen = read(sockfd,rbuff,sizeof(rbuff))) <= 0) */

	rbuff[rbufflen] = 0;
	
	if (P4_NET_PROTO_CHECK(rbuff,ptr,proto,P4_NET_PROTO_LISTE))
	{
		if (P4_NET_PROTO_CHECK(rbuff,ptr,proto,P4_NET_PROTO_HELLO_ERR))
		{
			/* >>> TODO: use the error system */
			fprintf(stderr,"error: LISTE\n");
			goto err;
		}
		else
		{
			fprintf(stderr,"error: %s",ptr);
			goto err;
		}
	}

	ptr2 = ptr;
	while ((*ptr2) && (*ptr2 != ' '))
		ptr2++;
	*ptr2++ = 0;

	tmp = strtol(ptr,0,10);
	ptr = ptr2;
	while (tmp-- > 0)
	{
		while ((*ptr2) && (*ptr2 != ' '))
			ptr2++;
		*ptr2++ = 0;
		
		tmpplayer = (__typeof__(tmpplayer)) 
			malloc(sizeof(tmpplayer));
		tmpplayer->nick = (char *) malloc((ptr2 - ptr) * sizeof(char));
		memcpy(tmpplayer->nick,ptr,ptr2 - ptr);
		P4_LIST_ADD(playerlist,tmpplayer);
		ptr = ptr2;

		if ((!*ptr) && (tmp > 0))
			goto err;	
	}

	goto out;
err:
	return 1;
out:
	return 0;	
}

int
client_net_game_get_challenge_color(
	int sockfd,
	struct p4_client_player *player,
	struct p4_game_client *game)
{
	struct p4_net_protocol_match *proto;
	char *ptr;

	/* get DEFI COUL msg */
	if ((rbufflen = client_net_read(rbuff,0)) <= 0)
		goto err;
	/* if ((rbufflen = read(sockfd,rbuff,sizeof(rbuff))) <= 0) */

	rbuff[rbufflen] = 0;
	
	if (P4_NET_PROTO_CHECK(rbuff,ptr,proto,P4_NET_PROTO_DEFI_COUL))
	{
		/* >>> TODO: use the error system */
		fprintf(stderr,"error: DEFI COUL ");

		if (P4_NET_PROTO_CHECK(rbuff,ptr,proto,P4_NET_PROTO_DEFI_ERR))
		{
			fprintf(stderr,"[%s]\n",ptr);
		}
		else
		{
			fprintf(stderr,"\n");
		}
		goto err;
	}

	if (*ptr == token_get_char_value(P4_TOKEN_BLACK)) 
	{
		game->pblack = 0;
		game->pred = player;
	}
	else
	{
		game->pred = 0;
		game->pblack = player;
	}

	if (*(ptr + 2) == token_get_char_value(P4_TOKEN_RED))
		game->pcur = game->pred;
	else
		game->pcur = game->pblack;


	goto out;	
err:
	return 1;
out:
	return 0;
}

/* send challenge req */
int
client_net_game_challenge(
	int sockfd,
	struct p4_client_player *player,
	struct p4_game_client *game)
{
	struct p4_net_protocol_match *proto;
	char *ptr;

	/* send DEFI msg */
	pthread_cancel(cwaitreqtid);

	proto = net_proto_get_proto_id(P4_NET_PROTO_DEFI);
	
	ptr = wbuff;

	ptr += sprintf(ptr,"%s %s",proto->name,player->nick);

	write(sockfd,wbuff,ptr - wbuff);
	fprintf(stdout,"Challenge request sent, waiting answer from %s ... ",
		player->nick);
	fflush(stdout);

	/* get DEFI OK/NO msg */
	if ((rbufflen = client_net_read(rbuff,0)) <= 0)
		goto err;
	/* if ((rbufflen = read(sockfd,rbuff,8)) <= 0) */
	
	rbuff[rbufflen] = 0;

	proto = net_proto_get_proto_name(rbuff);

	if ((proto) && (proto->id == P4_NET_PROTO_DEFI_OK))
	{
		fprintf(stdout,"client accepted\n");
		client_net_game_get_challenge_color(sockfd,player,game);
	}
	else if ((proto) && (proto->id == P4_NET_PROTO_DEFI_NO))
	{
		fprintf(stdout,"client refused\n");
		pthread_create(&cwaitreqtid,0,client_thread_start_waitreq,(void *) game);
		return 1;
	}
	else if (!P4_NET_PROTO_CHECK(rbuff,ptr,proto,P4_NET_PROTO_DEFI_ERR))
	{
		fprintf(stderr,"\nerror: [%s]\n",ptr);
		pthread_create(&cwaitreqtid,0,client_thread_start_waitreq,(void *) game);
		return 1;
	}
	else
	{
		/* >>> TODO: use the error system */
		WRITES(sockfd,"error: DEFI OK/NO expected\n");
		fprintf(stderr,"error: DEFI OK/NO\n");
		goto err;
	}

	goto out;
err:
	return -1;
out:
	return 0;
}

/* get challenge req */
int
client_net_game_wait_challenge_req(
	int sockfd,
	struct p4_client_player_list *playerlist,
	struct p4_game_client *game)
{
	struct p4_net_protocol_match *proto;
	struct p4_client_player *player;
	char *ptr;
	int tmp;

	/* get DEFI msg */
	if ((rbufflen = client_net_read(rbuff,0)) <= 0)
		goto err;
	/* if ((rbufflen = read(sockfd,rbuff,sizeof(rbuff))) <= 0) */
	
	rbuff[rbufflen] = 0;

	if (P4_NET_PROTO_CHECK(rbuff,ptr,proto,P4_NET_PROTO_DEFI))
		goto out;

	pthread_cancel(cswitchtid);

	player = client_get_player(playerlist,ptr);
	/* to be removed */
	if (player == (__typeof__(player)) -1)
		printf("Player NOT found\n");

	printf("\nThe player %s challenged you, accept the challenge ? [y/n] ",ptr);
	fflush(stdout);

	read(0,rbuff,sizeof(rbuff));
	
	/* send DEFI OK/NO msg */
	if (*rbuff == 'y')
		proto = net_proto_get_proto_id(P4_NET_PROTO_DEFI_OK);
	else
		proto = net_proto_get_proto_id(P4_NET_PROTO_DEFI_NO);

	ptr = wbuff;
	tmp = strlen(proto->name);
	ptr += tmp;
	memcpy(wbuff,proto->name,tmp);
	*ptr++ = 0;

	write(sockfd,wbuff,ptr - wbuff);

	if (*rbuff != 'y')
	{
		pthread_create(&cswitchtid,0,client_thread_start_switch,(void *) game);
		return 1;
	}

	client_net_game_get_challenge_color(sockfd,player,game);
	
	goto out;	
err:
	return -1;
out:
	return 0;
}

int lock;
static int srbufflen;

int
client_net_chat_loop(int sockfd)
{
	struct p4_net_protocol_match *proto;
	char *ptr,*ptr2;

	lock = 0;
	while (1)
	{
		if (!lock)
		{
			if ((srbufflen = read(sockfd,srbuff,sizeof(srbuff))) < 0)
				return -1;
			srbuff[srbufflen] = 0;

			if (!P4_NET_PROTO_CHECK(srbuff,ptr,proto,P4_NET_PROTO_MSG_DE))
			{
				ptr2 = ptr;
				while ((*ptr2) && (*ptr2 != ' '))
					ptr2++;
				*ptr2++ = 0;
				fprintf(stdout,"<--- Message from %s: %s\n",ptr,
					ptr2);
			
			}
			else if (!P4_NET_PROTO_CHECK(srbuff,ptr,proto,P4_NET_PROTO_MSG_ERR))
			{
				fprintf(stderr,"chat error: %s\n",ptr);
			}
			else
				lock = 1;	
		}
		else
			usleep(100000);
	}
}


int
client_net_read(
	char *crbuff,
	int clen)
{
	static int split = 0;
	static int lastlen = 0;
	int ret;

	while (!lock)
		usleep(100000);
	if (clen > 0)
	{
		if (split)
		{
			rbuff += lastlen;
			split = 0;
		}
		else
		{
			split = 1;
			lastlen = clen;
		}

		/*
		if (crbuff != rbuff)
			memcpy(crbuff,rbuff,rbufflen);
		*/

		ret = clen;
		if ((rbuff - srbuff + lastlen) >= srbufflen)
		{
			lock = 0;
			split = 0;
			lastlen = 0;
			rbuff = srbuff;
		}
	}
	else
	{
		/*
		if (crbuff != rbuff)
			memcpy(crbuff,rbuff,rbufflen);
		*/
		ret = srbufflen;

		if (split)
		{
			rbuff = srbuff + lastlen;
			split = 0;
		}
		else
			rbuff = srbuff;

		lastlen = 0;
		lock = 0;
	}
	return ret;
}
