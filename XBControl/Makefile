#Working command line
#g++ xbdrive.c joystick.h launcher.cpp launcher.h -lusb-1.0 -o xbdrive


CC		= g++

CPPFLAGS	=  -lusb-1.0 \
				-I.#		# for joystick.h

PROGRAMS	= xbdrive launcher

compile:  $(PROGRAMS)

install: compile

xbdrive: xbdrive.o
	$(CC) $(cflags) $(cppflags) $^ -lusb-1.0 -o $@
	#$(CC) xbdrive.c joystick.h launcher.cpp launcher.h -lusb-1.0 -o xbdrive


clean:
	-$(RM) *.o $(PROGRAMS)
	
xbdrive.o: xbdrive.c joystick.h 
