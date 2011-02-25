#include "client_display.h"
#include "client_network.h"
#include "client_network_game.h"
#include "client_game.h"
#include "puissance4.h"
#include "client.h"
#include "basics.h"

#include <unistd.h> /* read */
#include <stdio.h> /* printf */


int
client_disp_display_grid(struct p4_grid *g)
{
	int i;

	for(i = 0; i < P4_GRID_GET_SIZE(g); i++)
	{
		if ((i) && !(i % P4_GRID_COLUMNS))
			printf("\n");
		printf("%c ",token_get_char_value(P4_GRID_GET_X(g,i)));
	}
	printf("\n");

	for(i = 0; i < P4_GRID_GET_SIZE_X(g); i++)
		printf("%d ",i);
	printf("\n");
	
	return 0;
}

int
client_disp_display_playerlist(struct p4_client_player_list *playerlist)
{
	__typeof__(playerlist->list) ptr;
	int i;

	ptr = playerlist->list;
	i = 0;

	printf("List: %d players connected.\n",playerlist->size);
	while (ptr->next)
	{
		if (ptr->value)
			printf("[%d] %s\n",i++,ptr->value->nick);
		ptr = ptr->next;
	}
	if (ptr->value)
		printf("[%d] %s\n",i++,ptr->value->nick);

	return 0;
}

struct p4_client_player *
client_disp_get_player(
	struct p4_client_player_list *playerlist,
	char *msg)
{
	__typeof__(playerlist->list) ptr;
	int tmp;
	
	printf("%s",msg);
	scanf("%d",&tmp);

	tmp = tmp % playerlist->size;

	ptr = playerlist->list;
	
	while (tmp--)
		ptr = ptr->next;

	return ptr->value;
}

int
client_disp_display_player_color(struct p4_game_client *game)
{
	fprintf(stdout,"\nYour color is ");

	if (game->pblack)
		fprintf(stdout,"red\n");
	else
		fprintf(stdout,"black\n");

	return 0;
}

int
client_disp_display_player_color_start(struct p4_game_client *game)
{
	client_disp_display_player_color(game);
	
	if (game->pcur)
		fprintf(stdout,"The other player starts.");
	else
		fprintf(stdout,"You start.");
	
	fprintf(stdout,"\n");
	
	return 0;
}

char
client_disp_get_command()
{
	char buff[32];

	WRITES(1,"> ");
	read(0,buff,sizeof(buff));
	
	return *buff;
}

int
client_get_column()
{
	char buff[32];

	fprintf(stdout,"Column where you want to play: ");
	fflush(stdout);
	read(0,buff,sizeof(buff));

	return atoi(buff);	
}

int
client_disp_display_commandlist_start()
{
	printf("Command list:\n"
		"\tl : display the list of connected players\n"
		"\tc : challenge a player\n"
		"\tm : send a message to a player\n"
		"\th : display the help\n"
		"\tq : quit the game\n");
	return 0;
}

int
client_disp_display_commandlist_challenge()
{
	printf("Command list:\n"
		"\tp : play on a column\n"
		"\tc : display your color\n"
		"\tg : display the grid\n"
		"\tl : display the list of connected players\n"
		"\tm : send a message to a player\n"
		"\th : display the help\n"
		"\tq : quit the game\n");
	return 0;
}

int
client_disp_player_switch_start(
	int sockfd,
	struct p4_client_player_list *playerlist,
	struct p4_game_client *game)
{
	char c;
	int tmp;

	c = client_disp_get_command();
	switch(c)
	{
	case 'l':
		/* >>> TODO: refresh the list */
		client_disp_display_playerlist(playerlist);
		break;
	case 'm':
		client_net_send_message(playerlist);
		break;
	case 'c':

		tmp = client_net_game_challenge(sockfd,
			client_disp_get_player(playerlist,"Challenge player No: "),game);
		if (tmp < 0)
			return 1;
		else if (tmp > 0)
			break;
		else
		{
			client_disp_display_player_color_start(game);
			pthread_cancel(cwaitreqtid);
			client_game_run(game,playerlist);
			return P4_CLIENT_ST_END_GAME;
		}
		break;
	case 'h':
		client_disp_display_commandlist_start();
		break;
	case 'q':
		return P4_CLIENT_ST_QUIT;
		break;
	default:
		printf("choix invalide\n");
		break;
	}
	return 0;
}

int
client_disp_player_switch_challenge(
	struct p4_client_player_list *playerlist,
	struct p4_game_client *game)
{
	char c;

	c = client_disp_get_command();
	switch(c)
	{
	case 'l':
		/* >>> TODO: refresh the list */
		client_disp_display_playerlist(playerlist);
		break;
	case 'm':
		client_net_send_message(playerlist);
		break;
	case 'c':
		client_disp_display_player_color(game);
		break;
	case 'p':
		if (client_game_play(game) == P4_NET_CLIENT_END)
			return P4_NET_CLIENT_END;
		break;
	case 'g':
		client_disp_display_grid(game->grid);		
		break;
	case 'h':
		client_disp_display_commandlist_challenge();
		break;
	case 'q':
		client_net_game_send_quit(game);
		return P4_CLIENT_ST_QUIT;
		break;
	default:
		printf("choix invalide\n");
		break;
	}
	return 0;
}
