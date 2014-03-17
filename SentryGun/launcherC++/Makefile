# Uncomment the following to enable debug.
#DEBUG = y
CC=g++

LDFLAGS=-lusb-1.0 -std=c++0x

default:
	$(CC) launcher.cpp -o launcher $(LDFLAGS)
	$(CC) main.cpp -o main $(LDFLAGS) 
clean:
	rm launcher
	rm main
