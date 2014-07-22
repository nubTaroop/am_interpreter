AM0_OBJS = am0_interpreter.o am0.o
AM1_OBJS = am1_interpreter.o am0_interpreter.o am1.o
CC = g++
CFLAGS = -std=c++11 -O3 -Wall -c
LFLAGS = -Wall

all : am0 am1

install : all
	sudo mv -f am0 /bin/am0
	sudo mv -f am1 /bin/am1

am0 : $(AM0_OBJS)
	$(CC) $(LFLAGS) $(AM0_OBJS) -o am0

am1 : $(AM1_OBJS)
	$(CC) $(LFLAGS) $(AM1_OBJS) -o am1

am0_interpreter.o : am0_interpreter.hpp am0_interpreter.cpp
	$(CC) $(CFLAGS) am0_interpreter.cpp

am0.o : am0_interpreter.hpp am0.cpp
	$(CC) $(CFLAGS) am0.cpp

am1_interpreter.o : am1_interpreter.hpp am0_interpreter.hpp am1_interpreter.cpp
	$(CC) $(CFLAGS) am1_interpreter.cpp

am1.o : am1_interpreter.hpp am0_interpreter.hpp am1.cpp
	$(CC) $(CFLAGS) am1.cpp

clean:
	rm -f *.o am0 am1
