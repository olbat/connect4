#include "server_debug.h"
#include "server.h"

#include <stdio.h>

void
server_debug_display_playerlist(
	struct p4_server_player_list *playerlist,
	char *msg
)
{
	__typeof__(playerlist->list) ptr;
	int i;

	printf("%s\n",msg);
	if (!playerlist->list)
		return;
	ptr = playerlist->list;
	i = 0;
	
	while (ptr->next)
	{
		if (ptr->value)
			printf("[%d] nick:%s/color:%c/sain:%#x/sockfd:%d,tid:%#x\n",
				i++,
				ptr->value->nick,
				token_get_char_value(ptr->value->color),
				(int) ptr->value->sain,
				ptr->value->sockfd,
				(int) ptr->value->tid);
		ptr = ptr->next;
	}
	if (ptr->value)
		printf("[%d] nick:%s/color:%c/sain:%#x/sockfd:%d,tid:%#x\n",
			i++,
			ptr->value->nick,
			token_get_char_value(ptr->value->color),
			(int) ptr->value->sain,
			ptr->value->sockfd,
			(int) ptr->value->tid);
}
