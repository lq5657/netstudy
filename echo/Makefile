.PHONY:clean all
CC = gcc
CFLAGS = -Wall -g
BIN = echosrv echocli echosrvmulti echoclimulti p2psrv p2pcli stickpackagecli stickpackagesrv stickpackagecli_2 stickpackagesrv_2
all:$(BIN)
%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm -f *.o $(BIN)
