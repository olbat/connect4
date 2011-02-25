#include "server_game.h"
#include "server_network.h"
#include "server_network_game.h"
#include "server.h"
#include "puissance4.h"

#include <stdio.h> /* fprintf */
#include <string.h> /* memset */
#include <stdlib.h> /* free */

__inline__ void
server_player_cleanup(struct p4_server_player **p)
{
	if ((*p)->nick)
		free((*p)->nick);
	if ((*p)->sain)
		free((*p)->sain);
	free(*p);
}

__inline__ void
server_player_list_cleanup(struct p4_server_player_list_cell *cell)
{
	if (cell->value)
		server_player_cleanup(&(cell->value));
}

struct p4_server_player *
server_get_player(
	struct p4_server_player_list *playerlist,
	char *nick)
{
	__typeof__(playerlist->list) ptr;

	ptr = playerlist->list;

	while (ptr)
	{
		if ((ptr->value) && (ptr->value->nick) && (nick)
			&& !strcmp(ptr->value->nick,nick))
			break;
		ptr = ptr->next;
	}
	
	if (ptr)
		return ptr->value;
	else
		return 0;
}

int
server_game_check_drawn(struct p4_server_game *g)
{
	int i;

	for (i = 0; i < P4_GRID_GET_SIZE_X(g->grid); i++)
	{
		if (P4_GRID_GET_XY(g->grid,i,0) == P4_TOKEN_EMPTY)
			return 0;
	}
	return 1;
}

int
server_game_check_winner(
	struct p4_server_game *g,
	int col,
	int row)
{
	int nbtok, tcol, trow, i;
	enum p4_token curtok;

/* Recherche match nul */
	if (server_game_check_drawn(g))
		return P4_GAME_WIN_NONE;


	nbtok = 1;
	curtok = P4_GRID_GET_XY(g->grid,col,row);
/* Recherche horizontale vers la gauche */
	for (tcol = col-1, i = 0; ((i < 3) && (tcol >= 0)); tcol--, i++)
	{
		if (P4_GRID_GET_XY(g->grid,tcol,row) == curtok)
			nbtok++;
		else
			break;
	}

	if (nbtok >= 4)
		return P4_GAME_WIN_HORIZONTAL;

/* Recherche horizontale vers la droite */
	for (tcol = col+1, i = 0; ((i < 3)
		&& (tcol < P4_GRID_GET_SIZE_X(g->grid))); tcol++, i++)
	{
		if (P4_GRID_GET_XY(g->grid,tcol,row) == curtok)
			nbtok++;
		else
			break;
	}

	if (nbtok >= 4)
		return P4_GAME_WIN_HORIZONTAL;
		
/* Recherche verticale vers le bas */

	nbtok = 1;
	for (trow = row-1, i = 0; ((i < 3) && (trow >= 0)); trow--, i++)
	{
		if (P4_GRID_GET_XY(g->grid,col,trow) == curtok)
			nbtok++;
		else
			break;
	}

	if (nbtok >= 4)
		return P4_GAME_WIN_VERTICAL;

/* Recherche verticale vers le haut */
	for (trow = row+1, i = 0; ((i < 3) 
		&& (trow < P4_GRID_GET_SIZE_Y(g->grid))); trow++, i++)
	{
		if (P4_GRID_GET_XY(g->grid,col,trow) == curtok)
			nbtok++;
		else
			break;
	}

	if (nbtok >= 4)
		return P4_GAME_WIN_VERTICAL;


/* Recherche diagonale vers en bas à gauche */
	nbtok = 1;

	for (trow = row + 1,tcol = col - 1, i = 0; ((i < 3) 
		&& (trow < P4_GRID_GET_SIZE_Y(g->grid)) && (tcol >= 0));
		trow++, tcol--, i++)
	{
		if (P4_GRID_GET_XY(g->grid,tcol,trow) == curtok)
			nbtok++;
		else
			break;
	}

	if (nbtok >= 4)
		return P4_GAME_WIN_DIAGONAL_LEFT;

/* Recherche diagonale vers en haut à droite */
	for (trow = row - 1,tcol = col + 1, i = 0; ((i < 3) 
		&& (trow >= 0) && (tcol < P4_GRID_GET_SIZE_X(g->grid))); 
		trow--, tcol++, i++)
	{
		if (P4_GRID_GET_XY(g->grid,tcol,trow) == curtok)
			nbtok++;
		else
			break;
	}

	if (nbtok >= 4)
		return P4_GAME_WIN_DIAGONAL_LEFT;

/* Recherche diagonale vers en haut à gauche */
	nbtok = 1;

	for (trow = row - 1,tcol = col - 1, i = 0; ((i < 3) 
		&& (trow >= 0) && (tcol >= 0)); 
		trow--, tcol--, i++)
	{
		if (P4_GRID_GET_XY(g->grid,tcol,trow) == curtok)
			nbtok++;
		else
			break;
	}

	if (nbtok >= 4)
		return P4_GAME_WIN_DIAGONAL_RIGHT;

/* Recherche diagonale vers en bas à droite */
	for (trow = row + 1,tcol = col + 1, i = 0; ((i < 3) 
		&& (tcol < P4_GRID_GET_SIZE_X(g->grid)) 
		&& (trow < P4_GRID_GET_SIZE_Y(g->grid))); trow++, tcol--, i++)
	{
		if (P4_GRID_GET_XY(g->grid,tcol,trow) == curtok)
			nbtok++;
		else
			break;
	}

	if (nbtok >= 4)
		return P4_GAME_WIN_DIAGONAL_RIGHT;

	return -1;
}

int
server_game_get_winner_dir(
	struct p4_server_game *g,
	enum p4_game_win_dir dir,
	int *col,
	int *row)
{
	enum p4_token color;

	color = P4_GRID_GET_XY(g->grid,*col,*row);
	
	if (dir == P4_GAME_WIN_HORIZONTAL)
	{
		*col = *col - 1;
		while(P4_GRID_GET_XY(g->grid,*col,*row) == color)
			*col = *col - 1;
		*col = *col + 1;
	}
	else if (dir == P4_GAME_WIN_VERTICAL)
	{
		*row = *row - 1;
		while(P4_GRID_GET_XY(g->grid,*col,*row) == color)
			*row = *row - 1;
		*row = *row + 1;
	}
	else if (dir == P4_GAME_WIN_DIAGONAL_LEFT)
	{
		*row = *row + 1;
		*col = *col + 1;
		while(P4_GRID_GET_XY(g->grid,*col,*row) == color)
		{
			*row = *row + 1;
			*col = *col + 1;
		}
		*row = *row - 1;
		*col = *col - 1;
	}
	else if (dir == P4_GAME_WIN_DIAGONAL_RIGHT)
	{
		*row = *row + 1;
		*col = *col - 1;
		while(P4_GRID_GET_XY(g->grid,*col,*row) == color);
		{
			*row = *row + 1;
			*col = *col - 1;
		}
		*row = *row - 1;
		*col = *col + 1;
	}
	return 0;
}

int
server_game_playable(
	struct p4_server_game *g,
	int *col,
	int *row)
{
	*row = P4_GRID_GET_FIRST_EMPTY(g->grid,*col);
	return (*row >= 0);
}

int
server_game_run(struct p4_server_game *g)
{
	static unsigned int ch_num = 1;
	int col,row,check,num;

	num = ch_num++;

	fprintf(stdout,"challenge (%d) started: %s[R] - %s[B]\n",num,
		g->pred->nick,g->pblack->nick);

	row = 0;
	do
	{
		if ((col = server_net_game_curplay(g)) == -1)
			continue;
		if (col == -2)
			return 0;

		if (server_game_playable(g,&col,&row)) {
			fprintf(stdout,"(%d) %s played in column %d\n",num,
				g->pcur->nick,col);
			P4_GRID_SET_XY(g->grid,col,row,g->pcur->color);
			server_net_game_notify_play(g,col,row);
			P4_SERVER_SWITCH_CURPLAYER(g);
		}
		else
			server_net_game_notify_curinval(g);
	} while((check = server_game_check_winner(g,col,row)) < 0);

	if (check == P4_GAME_WIN_NONE)
		server_net_game_notify_drawn(g);
	else
	{
		P4_SERVER_SWITCH_CURPLAYER(g);
		server_game_get_winner_dir(g,check,&col,&row);
		server_net_game_notify_win(g,col,row,check);
	}

	fprintf(stdout,"challenge (%d) ended\n",num);

	return 0;
}

