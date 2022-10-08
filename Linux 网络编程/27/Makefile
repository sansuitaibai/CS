.PHONY:
	clean all

CC = gcc 
CFLAGS = -Wall -g 
BIN = 01msgget  02msgctl 03msgctl  04msgsend 05msfrcv 05msfrcv 06msgsrv 07msgrcv

all:$(BIN)

%:%.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm $(BIN)
