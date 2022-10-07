.PHONY:
	claen

CC = gcc
CFLAGS = -Wall -g

src =  01echosrv 02echocli rlimit

all:$(src)

%:%.c
	$(CC) $(CFLAGS)   $< -o $@   
	

clean:
	rm -f $(src:=.o) $(src) 
