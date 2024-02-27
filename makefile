OBJS	= main.o console.o consoleinput.o
SOURCE	= main.c console.c consoleinput.c
HEADER	= console.h consoleinput.h
OUT	= smallsh
CC	 = gcc
FLAGS	 = -g -c -w -Wall -D_BSD_SOURCE
LFLAGS	 = 

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)

main.o: main.c
	$(CC) $(FLAGS) main.c -std=c99

console.o: console.c
	$(CC) $(FLAGS) console.c -std=c99

consoleinput.o: consoleinput.c
	$(CC) $(FLAGS) consoleinput.c -std=c99


clean:
	rm -f $(OBJS) $(OUT)