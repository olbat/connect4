#include "client_game.h"
#include "client_display.h"
#include "client_network.h"
#include "client_network_game.h"
#include "client.h"
#include "puissance4.h"

#include <stdio.h> /* fprintf */
#include <stdlib.h> /* free */
#include <string.h> /* strcmp */
#include <unistd.h> /* pause */


void
client_player_list_cleanup(struct p4_client_player_list_cell *cell)
{
	if ((cell) && (cell->value))
	{
		free(cell->value->nick);
		free(cell->value);
	}
}

struct p4_client_player *
client_get_player(
	struct p4_client_player_list *playerlist,
	char *nick)
{
	__typeof__(playerlist->list) ptr;

	ptr = playerlist->list;

	while (ptr)
	{
		if ((ptr->value) && (ptr->value->nick) 
			&& !strcmp(ptr->value->nick,nick))
			break;
		ptr = ptr->next;
	}
	
	if (ptr)
		return ptr->value;
	else
		return (struct p4_client_player *) -1;
}

int
client_game_run(
	struct p4_game_client *g,
	struct p4_client_player_list *playerlist)
{
	int tmp;
	
	fprintf(stdout,"\nGame started\n");

	if (g->pcur)
	{
		fprintf(stdout,"The other player is playing now ...\n");
		client_net_game_get_update(g);
	}

	client_disp_display_commandlist_challenge();
	do {
		tmp = client_disp_player_switch_challenge(playerlist,g);
		if ((tmp) && (tmp != P4_CLIENT_ST_QUIT) && (tmp != P4_NET_CLIENT_END))
			goto err;
	} while ((tmp != P4_CLIENT_ST_QUIT) && (tmp != P4_NET_CLIENT_END));

	if (tmp == P4_NET_CLIENT_END)
	{
		fprintf(stdout,"End of the game...\n");
		sleep(1);
	}

	pthread_exit((void *) 0);

	goto out;
err:
	return 1;
out:
	return 0;
}

int
client_game_play(struct p4_game_client *g)
{
	int tmp;

	do {
		client_net_game_send_update(g,client_get_column());
		tmp = client_net_game_get_update(g);
	} while ((tmp) && (tmp != P4_NET_CLIENT_END));


	if (tmp != P4_NET_CLIENT_END) 
	{
		fprintf(stdout,"The other player is playing now ...\n");
		if (client_net_game_get_update(g) == P4_NET_CLIENT_END)
			return P4_NET_CLIENT_END;
		else
			return 0;
	}
	else
		return P4_NET_CLIENT_END;
}

int
client_game_set_win_grid(
	struct p4_game_client *g,
	enum p4_game_win_dir dir,
	int col,
	int row)
{
	int i;

	if (dir == P4_GAME_WIN_HORIZONTAL)
	{
		for (i = 0; i < 4; i++)
			P4_GRID_SET_XY(g->grid,col + i,row,P4_TOKEN_WINER);
	}
	else if (dir == P4_GAME_WIN_VERTICAL)
	{
		for (i = 0; i < 4; i++)
			P4_GRID_SET_XY(g->grid,col,row + i,P4_TOKEN_WINER);
	}
	else if (dir == P4_GAME_WIN_DIAGONAL_LEFT)
	{
		for (i = 0; i < 4; i++)
			P4_GRID_SET_XY(g->grid,col - i,row + i,P4_TOKEN_WINER);
	}
	else if (dir == P4_GAME_WIN_DIAGONAL_RIGHT)
	{
		for (i = 0; i < 4; i++)
			P4_GRID_SET_XY(g->grid,col + i,row + i,P4_TOKEN_WINER);
	}

	return 0;
}
