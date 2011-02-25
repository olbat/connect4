#include "client_network_game.h"
#include "client_network.h"
#include "client_game.h"
#include "client_display.h"
#include "client.h"
#include "puissance4.h"

#include <stdio.h> /* fprintf */
#include <unistd.h> /* read */
#include <string.h> /* strlen */

/*
static char rbuff[P4_NET_CLIENT_BUFFER_SIZE_READ];
*/
static char wbuff[P4_NET_CLIENT_BUFFER_SIZE_WRITE];
extern char *rbuff;

int
client_net_game_send_update(
	struct p4_game_client *g,
	int col)
{
	struct p4_net_protocol_match *proto;
	char *ptr;

	/* send COUP COL msg */
	proto = net_proto_get_proto_id(P4_NET_PROTO_COUP_COL);

	ptr = wbuff;
	ptr += sprintf(ptr,"%s %d",proto->name, (col % P4_GRID_GET_SIZE_X(g->grid)));

	write(g->sockfd,wbuff,ptr - wbuff);

	return 0;
}

int
client_net_game_send_quit(struct p4_game_client *g)
{
	struct p4_net_protocol_match *proto;
	char *ptr;

	/* send QUITTE msg */
	proto = net_proto_get_proto_id(P4_NET_PROTO_QUITTE);

	ptr = wbuff;
	ptr += sprintf(ptr,"%s",proto->name);

	write(g->sockfd,wbuff,ptr - wbuff);

	return 0;
}

int
client_net_game_get_check_win(
	struct p4_game_client *g,
	char *str)
{
	struct p4_net_protocol_match *proto;
	char *ptr;

	if (!P4_NET_PROTO_CHECK(str,ptr,proto,P4_NET_PROTO_UPDATE_GAGNE))
	{
		if (*ptr == 'N')
			fprintf(stdout,"Black");
		else
			fprintf(stdout,"Red");
		fprintf(stdout," player win.\n");

		*(ptr + 3) = 0;
		*(ptr + 5) = 0;
		client_game_set_win_grid(g,p4_get_win_dir_int_value(*(ptr + 6)),
			atoi(ptr + 2), atoi(ptr + 4));
		client_disp_display_grid(g->grid);

		return P4_NET_CLIENT_END;
	}
	else if (!P4_NET_PROTO_CHECK(str,ptr,proto,P4_NET_PROTO_UPDATE_NUL))
	{
		fprintf(stdout,"End of the game, no player win.\n");
		return P4_NET_CLIENT_END;
	}
	else if (!P4_NET_PROTO_CHECK(str,ptr,proto,P4_NET_PROTO_QUITTE))
	{
		fprintf(stdout,"The other challenger leaved.\n");
		return P4_NET_CLIENT_END;
	}
	else
	{
		fprintf(stderr,"error: UPDATE expected\n");
		return -1;
	}
}

int
client_net_game_get_update(struct p4_game_client *g)
{
	struct p4_net_protocol_match *proto;
	char *ptr, *ptr2;
	int rbufflen;

	/* get UPDATE msg */
	if ((rbufflen = client_net_read(rbuff,0)) <= 0)
		goto err;
	/* if ((rbufflen = read(g->sockfd,rbuff,sizeof(rbuff))) <= 0) */

	rbuff[rbufflen] = 0;

	if (!P4_NET_PROTO_CHECK(rbuff,ptr,proto,P4_NET_PROTO_UPDATE_PION))
	{
		int x,y,tmp;

		ptr2 = ptr + 2;
		while ((*ptr2) && (*ptr2 != ' '))
			ptr2++;
		*ptr2 = 0;
		
		x = atoi(ptr + 2);
		ptr2++;
		y = atoi(ptr2);
		P4_GRID_SET_XY(g->grid,x,y,token_get_int_value(*ptr));
		fprintf(stdout,"Grid updated:\n");
		client_disp_display_grid(g->grid);
		tmp = strlen(rbuff) + 2;

		if ((tmp + 2) >= rbufflen)
			goto out;
		ptr = rbuff + tmp + 1;

		if (client_net_game_get_check_win(g,ptr) <= 0)
			goto err;
		else
			return P4_NET_CLIENT_END;
	}
	else if (!P4_NET_PROTO_CHECK(rbuff,ptr,proto,P4_NET_PROTO_UPDATE_COUP_INVALIDE))
	{
		fprintf(stdout,"Invalid try !\n");
		goto err;
	}
	else if (!P4_NET_PROTO_CHECK(rbuff,ptr,proto,P4_NET_PROTO_UPDATE_GAGNE))
	{
		if (*ptr == 'N')
			fprintf(stdout,"Black");
		else
			fprintf(stdout,"Red");
		fprintf(stdout," player win.\n");

		*(ptr + 3) = 0;
		*(ptr + 5) = 0;
		client_game_set_win_grid(g,p4_get_win_dir_int_value(*(ptr + 6)),
			atoi(ptr + 2), atoi(ptr + 4));
		client_disp_display_grid(g->grid);

		return P4_NET_CLIENT_END;
	}
	else if (!P4_NET_PROTO_CHECK(rbuff,ptr,proto,P4_NET_PROTO_UPDATE_NUL))
	{
		fprintf(stdout,"End of the game, no player win.\n");
		return P4_NET_CLIENT_END;
	}
	else if (!P4_NET_PROTO_CHECK(rbuff,ptr,proto,P4_NET_PROTO_QUITTE))
	{
		fprintf(stdout,"The other challenger leaved the game.\n");
		return P4_NET_CLIENT_END;
	}
	else
	{
		fprintf(stderr,"error: UPDATE expected\n");
		goto err;
	}

	goto out;
err:
	return 1;
out:
	return 0;
}

