# This file is a part of groinc 
# Copyright 2006 Sarzyniec Luc <olbat@xiato.net>
# This software is released under GPL the license
# see the COPYING file for more informations

BINDIR= $(DESTDIR)/usr/bin
MANDIR= $(DESTDIR)/usr/share/man/man1
LBFLAGS= -lpthread
CFLAGS= -Wall -ansi -pedantic -O3 -g
CC= gcc ${CFLAGS}
CLIENT_BNAME= client
SERVER_BNAME= server
TESTSERVER_BNAME= testserver
TESTCLIENT_BNAME= testclient
CLIENT_SRC=client.c client_network.c client_network_game.c client_game.c client_display.c
SERVER_SRC=server.c server_network.c server_network_game.c server_game.c server_debug.c
TESTSERVER_SRC=testserver.c server_network.c server_game.c server_network_game.c
TESTCLIENT_SRC=testclient.c client_network.c client_game.c client_display.c client_network_game.c
SHARED_SRC=puissance4.c
CLIENT_OBJ=$(CLIENT_SRC:.c=.o)
SERVER_OBJ=$(SERVER_SRC:.c=.o)
TESTSERVER_OBJ=$(TESTSERVER_SRC:.c=.o)
TESTCLIENT_OBJ=$(TESTCLIENT_SRC:.c=.o)
SHARED_OBJ=$(SHARED_SRC:.c=.o)

all : shared ${CLIENT_BNAME} ${SERVER_BNAME} clean
	@echo compilation successful

dev : shared ${CLIENT_BNAME} ${SERVER_BNAME}
	@echo success making ${CLIENT_BNAME} and ${SERVER_BNAME}
test : shared ${TESTCLIENT_BNAME} ${TESTSERVER_BNAME}
	@echo success making ${TESTCLIENT_BNAME} and ${TESTSERVER_BNAME}

${CLIENT_BNAME} : shared ${CLIENT_OBJ}
	@echo ${CLIENT_BNAME} files compiled
	@${CC} -o $@ ${CLIENT_OBJ} ${SHARED_OBJ} ${LBFLAGS}

${SERVER_BNAME} : shared ${SERVER_OBJ}
	@echo ${SERVER_BNAME} files compiled
	@${CC} -o $@ ${SERVER_OBJ} ${SHARED_OBJ} ${LBFLAGS}


${TESTSERVER_BNAME} : shared ${TESTSERVER_OBJ}
	@echo ${TESTSERVER_BNAME} files compiled
	@${CC} -o $@ ${TESTSERVER_OBJ} ${SHARED_OBJ} ${LBFLAGS}

${TESTCLIENT_BNAME} : shared ${TESTCLIENT_OBJ}
	@echo ${TESTCLIENT_BNAME} files compiled
	@${CC} -o $@ ${TESTCLIENT_OBJ} ${SHARED_OBJ} ${LBFLAGS}

shared : ${SHARED_OBJ}
	@echo shared binaries compiled

%.o : %.c
	@echo -n '  compiling $< ... '
	@${CC} -o $@ -c $<
	@echo done
install :
	@echo install ...
clean :
	@echo cleaning object files
	@rm -f ${CLIENT_OBJ} ${SERVER_OBJ} ${SHARED_OBJ}
cleanall : clean
	@echo cleaning executable file
	@rm -f ${CLIENT_BNAME} ${SERVER_BNAME}
