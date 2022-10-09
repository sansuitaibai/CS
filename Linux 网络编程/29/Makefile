.PHONY:
	clean all

CC = gcc 
CFLAGS = -Wall -g 
BIN = 01shmwrite 02shmread 
all:$(BIN)

%:%.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm $(BIN)
	
