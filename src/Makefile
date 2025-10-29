CC=g++
OPTS=-g -Werror

all: main.o predictor.o
	$(CC) $(OPTS) -lm -o predictor main.o predictor.o

main.o: main.cpp predictor.h
	$(CC) $(OPTS) -c main.cpp

predictor.o: predictor.h predictor.cpp
	$(CC) $(OPTS) -c predictor.cpp

clean:
	rm -f *.o predictor;

tournament:
	bunzip2 -kc ../traces/U1_Blender.bz2 | ./predictor --tournament
	bunzip2 -kc ../traces/U2_Leela.bz2 | ./predictor --tournament
	bunzip2 -kc ../traces/U3_GCC.bz2 | ./predictor --tournament
	bunzip2 -kc ../traces/U4_Cam4.bz2 | ./predictor --tournament

gshare:
	bunzip2 -kc ../traces/U1_Blender.bz2 | ./predictor --gshare
	bunzip2 -kc ../traces/U2_Leela.bz2 | ./predictor --gshare
	bunzip2 -kc ../traces/U3_GCC.bz2 | ./predictor --gshare
	bunzip2 -kc ../traces/U4_Cam4.bz2 | ./predictor --gshare

test: test.o predictor.o
	$(CC) $(OPTS) -lm -o test test.o predictor.o

test.o: predictor.h test.cpp
	$(CC) $(OPTS) -c test.cpp
