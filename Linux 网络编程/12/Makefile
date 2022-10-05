.PHONY:
	claen

CC = gcc
CFLAGS = -Wall -g

src = 01echosrv 02echocli 

all:$(src)
	$(CC) $(CFLAGS) 03rdwr.o 01echosrv.o -o 01echosrv 
	$(CC) $(CFLAGS) 03rdwr.o 02echocli.o -o 02echocli

%:%.c
	$(CC) $(CFLAGS) $< -c -o $(@:=.o)  
	

clean:
	rm -f $(src:=.o) $(src) 
