.PHONY:
	clean all

CC = gcc 
CFLAGS = -Wall -g 
BIN = 01mmapwrite 02mmapread 
all:$(BIN)

%:%.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm $(BIN)
	
