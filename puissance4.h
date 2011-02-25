#ifndef _PUISSANCE4_H
#define _PUISSANCE4_H

#define P4_GRID_COLUMNS	7
#define P4_GRID_LINES	6
#define P4_GRID_SIZE	(P4_GRID_COLUMNS * P4_GRID_LINES)
#define P4_TOKEN_NB	21
#define P4_TOKEN_WIN	4
#define P4_NET_PORT_DEFAULT	12345
#define P4_NET_PORT_LIST_DEFAULT	54321
#define P4_NET_PORT_CHAT_DEFAULT	21354

#define P4_GRID_GET_X(G,X)	((G)->cells[X])
#define P4_GRID_SET_X(G,X,V)	((G)->cells[X] = V)
#define P4_GRID_GET_XY(G,X,Y) 	(G)->cells[(((Y) * P4_GRID_COLUMNS) + (X))]
#define P4_GRID_SET_XY(G,X,Y,V) ((G)->cells[(((Y) * P4_GRID_COLUMNS) + (X))] = V)
#define P4_GRID_GET_SIZE(G)	((G)->size_x * (G)->size_y)
#define P4_GRID_GET_SIZE_X(G)	(G)->size_x
#define P4_GRID_GET_SIZE_Y(G)	(G)->size_y

#define P4_GRID_GET_FIRST_EMPTY(G,COL) __extension__ \
({ \
	int _tmp; \
	_tmp = P4_GRID_GET_SIZE_Y(G); \
	while (_tmp--) \
	{ \
		if (P4_GRID_GET_XY(G,COL,_tmp) == P4_TOKEN_EMPTY) \
			break; \
	} \
	_tmp; \
})

#define P4_LIST_INIT(L,CL) \
	(L)->list = 0; \
	(L)->size = 0; \
	(L)->cleanup_cell = CL;

#define P4_LIST_CLEANUP(L) __extension__ \
({ \
	__typeof__((L)->list) _LPTR,_TPTR; \
	_LPTR = (L)->list; \
	if (_LPTR) \
	{ \
		while (_LPTR->next) \
		{ \
			((L)->cleanup_cell)(_LPTR); \
			_TPTR = _LPTR; \
			_LPTR = _LPTR->next; \
			free(_TPTR); \
		} \
	} \
	((L)->cleanup_cell)(_LPTR); \
	free(_LPTR); \
})

#define P4_LIST_ADD(L,V) __extension__ \
({ \
	__typeof__((L)->list) _CELL; \
	_CELL = (__typeof__(_CELL)) malloc(sizeof(_CELL)); \
	_CELL->value = V; \
	_CELL->next = (L)->list; \
	(L)->list = _CELL; \
	(L)->size++; \
})

#define P4_NET_PROTO_CHECK(STR,PTR,PRV,PROT) __extension__ \
({ \
	int _tmp; \
	char _c; \
	PTR = STR; \
	PRV = net_proto_get_proto_id(PROT); \
	_tmp = strlen(PRV->name); \
	_c = STR[_tmp]; \
	STR[_tmp] = 0; \
	PTR = STR + _tmp + 1; \
	PRV = net_proto_get_proto_name(STR); \
	STR[_tmp] = _c; \
	(PRV ? PRV->id != PROT : 1); \
})

enum p4_token
{
	P4_TOKEN_UNKNOWN = -1,
	P4_TOKEN_EMPTY = 0,
	P4_TOKEN_RED = 1,
	P4_TOKEN_BLACK = 2,
	P4_TOKEN_WINER = 4
};

enum p4_net_protocol
{
	P4_NET_PROTO_HELLO,
	P4_NET_PROTO_HELLO_ERR,
	P4_NET_PROTO_LISTE,
	P4_NET_PROTO_DEFI,
	P4_NET_PROTO_DEFI_COUL,
	P4_NET_PROTO_DEFI_ERR,
	P4_NET_PROTO_DEFI_OK,
	P4_NET_PROTO_DEFI_NO,
	P4_NET_PROTO_COUP_COL,
	P4_NET_PROTO_UPDATE_PION,
	P4_NET_PROTO_UPDATE_COUP_INVALIDE,
	P4_NET_PROTO_UPDATE_GAGNE,
	P4_NET_PROTO_UPDATE_NUL,
	P4_NET_PROTO_QUITTE,
	P4_NET_PROTO_MSG_VERS,
	P4_NET_PROTO_MSG_DE,
	P4_NET_PROTO_MSG_ERR,
	P4_NET_PROTO_ERROR,
	P4_NET_PROTO_NULL
};

enum p4_game_win_dir
{
	P4_GAME_WIN_HORIZONTAL = 1,
	P4_GAME_WIN_VERTICAL = 2,
	P4_GAME_WIN_DIAGONAL_RIGHT = 3,
	P4_GAME_WIN_DIAGONAL_LEFT = 4,
	P4_GAME_WIN_NONE = 0
};


struct p4_grid 
{
	enum p4_token *cells;
	int size_x;
	int size_y;
};

struct p4_net_protocol_match
{
	const char *name;
	enum p4_net_protocol id;
};

char token_get_char_value(enum p4_token);
enum p4_token token_get_int_value(char);
__inline__ struct p4_net_protocol_match *net_proto_get_proto_name(char *);
__inline__ struct p4_net_protocol_match *net_proto_get_proto_id(enum p4_net_protocol);
__inline__ int grid_init(struct p4_grid **g);
__inline__ int grid_cleanup(struct p4_grid **g);
char p4_get_win_dir_char_value(enum p4_game_win_dir v);
enum p4_game_win_dir p4_get_win_dir_int_value(char c);

#endif
