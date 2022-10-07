.PHONY:
	claen

CC = g++
CFLAGS = -Wall -g 

src =  01echosrv 02echocli

all:$(src)

%:%.c
	$(CC) $(CFLAGS)   $< -o $@   
	

clean:
	rm -f $(src:=.o) $(src) 
