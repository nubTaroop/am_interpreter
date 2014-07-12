OBJS = am0_interpreter.o am0.o
CC = g++
CFLAGS = -std=c++11 -O3 -Wall -c
LFLAGS = -Wall
NAME = -o am0

am0 : $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) $(NAME)

am0_interpreter.o : am0_interpreter.hpp am0_interpreter.cpp
	$(CC) $(CFLAGS) am0_interpreter.cpp

am0.o : am0_interpreter.hpp am0.cpp
	$(CC) $(CFLAGS) am0.cpp

clean:
	rm -f *.o am0
