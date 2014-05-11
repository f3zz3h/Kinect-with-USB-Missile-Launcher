#Working command line
#g++ xbdrive.c joystick.h launcher.cpp launcher.h -lusb-1.0 -o xbdrive

CC=g++

LDFLAGS=-lusb-1.0

CPPFLAGS	=  -I.#		# for joystick.h

PROGRAMS	= xbdrive

compile:  $(PROGRAMS)

install: compile

xbdrive: xbdrive.o 
	$(CC) $(cflags) $(cppflags) launcher.cpp $^ -o $@ $(LDFLAGS)
	#$(CC) xbdrive.c launcher.cpp -lusb-1.0 -o xbdrive

clean:
	-$(RM) *.o $(PROGRAMS)
	
xbdrive.o: xbdrive.c joystick.h launcher.cpp launcher.h 
