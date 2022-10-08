.PHONY:
	clean all

CC = gcc 
CFLAGS = -Wall -g 
BIN = 01msgget  02msgctl 03msgctl  04msgsend 05msfrcv

all:$(BIN)

%:%.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm $(BIN)
