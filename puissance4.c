#include "puissance4.h"
#include <stdlib.h> /* free */
#include <string.h> /* memset */

#include <string.h>


static struct p4_net_protocol_match proto_match[] = 
{
	{ "HELLO", P4_NET_PROTO_HELLO }, 
	{ "HELLO ERR", P4_NET_PROTO_HELLO_ERR }, 
	{ "LISTE", P4_NET_PROTO_LISTE }, 
	{ "DEFI", P4_NET_PROTO_DEFI }, 
	{ "DEFI COUL", P4_NET_PROTO_DEFI_COUL }, 
	{ "DEFI ERR", P4_NET_PROTO_DEFI_ERR }, 
	{ "DEFI OK", P4_NET_PROTO_DEFI_OK }, 
	{ "DEFI NO", P4_NET_PROTO_DEFI_NO }, 
	{ "COUP COL", P4_NET_PROTO_COUP_COL }, 
	{ "UPDATE PION", P4_NET_PROTO_UPDATE_PION }, 
	{ "UPDATE COUP INVALIDE", P4_NET_PROTO_UPDATE_COUP_INVALIDE }, 
	{ "UPDATE GAGNE", P4_NET_PROTO_UPDATE_GAGNE }, 
	{ "UPDATE NUL", P4_NET_PROTO_UPDATE_NUL }, 
	{ "QUITTE", P4_NET_PROTO_QUITTE }, 
	{ "MSG VERS", P4_NET_PROTO_MSG_VERS }, 
	{ "MSG DE", P4_NET_PROTO_MSG_DE }, 
	{ "MSG ERR", P4_NET_PROTO_MSG_ERR }, 
	{ "ERROR", P4_NET_PROTO_ERROR }, 
	{ "", P4_NET_PROTO_NULL } 
}; 

char
token_get_char_value(enum p4_token v)
{
	switch(v)
	{
		case P4_TOKEN_WINER:
			return 'x';
		case P4_TOKEN_EMPTY:
			return '.';
		case P4_TOKEN_RED:
			return 'R';
		case P4_TOKEN_BLACK:
			return 'N';
		default:
			return '?';
	}
	return '?';
}

enum p4_token
token_get_int_value(char c)
{
	switch(c)
	{
		case 'x':
			return P4_TOKEN_WINER;
		case 'R':
			return P4_TOKEN_RED;
		case 'N':
			return P4_TOKEN_BLACK;
		case '.':
			return P4_TOKEN_EMPTY;
		case '?':
		default:
			return P4_TOKEN_UNKNOWN;
	}
	return P4_TOKEN_UNKNOWN;
}

__inline__ struct p4_net_protocol_match *
net_proto_get_proto_name(char *name)
{
	int i;

	i = 0;

	while ((proto_match[i].name) && (strcmp(proto_match[i].name,name)))
		i++;

	if (proto_match[i].name)
		return &proto_match[i];
	else
		return 0;
}

__inline__ struct p4_net_protocol_match *
net_proto_get_proto_id(enum p4_net_protocol id)
{
	int i;

	i = 0;
	while ((proto_match[i].name) && (proto_match[i].id != id))
		i++;

	if (proto_match[i].name)
		return &proto_match[i];
	else
		return 0;
}

__inline__ int
grid_init(struct p4_grid **g)
{
	*g = (__typeof__(*g)) malloc(sizeof(*g));
	(*g)->cells = (__typeof__((*g)->cells)) 
		malloc(P4_GRID_SIZE * sizeof((*g)->cells));
	(*g)->size_x = P4_GRID_COLUMNS;
	(*g)->size_y = P4_GRID_LINES;
	memset((*g)->cells,P4_TOKEN_EMPTY,
		P4_GRID_GET_SIZE(*g)*sizeof((*g)->cells));

	return 0;
}

__inline__ int
grid_cleanup(struct p4_grid **g)
{
	if (*g)
	{
		if ((*g)->cells)
			free((*g)->cells);	
		free(*g);
	}

	return 0;
}

char
p4_get_win_dir_char_value(enum p4_game_win_dir v)
{
	switch(v)
	{
		case P4_GAME_WIN_HORIZONTAL:
			return '1';
		case P4_GAME_WIN_VERTICAL:
			return '2';
		case P4_GAME_WIN_DIAGONAL_RIGHT:
			return '3';
		case P4_GAME_WIN_DIAGONAL_LEFT:
			return '4';
		default:
			return '?';
	}
}

enum p4_game_win_dir
p4_get_win_dir_int_value(char c)
{
	switch(c)
	{
		case '1':
			return P4_GAME_WIN_HORIZONTAL;
		case '2':
			return P4_GAME_WIN_VERTICAL;
		case '3':
			return P4_GAME_WIN_DIAGONAL_RIGHT;
		case '4':
			return P4_GAME_WIN_DIAGONAL_LEFT;
		default:
			return P4_GAME_WIN_NONE;
	}
}
