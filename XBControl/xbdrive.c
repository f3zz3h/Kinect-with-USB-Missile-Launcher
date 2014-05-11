#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "joystick.h"
#include "launcher.h"

#define NAME_LENGTH 128

int main (int argc, char **argv)
{
	int fd;
	unsigned char axes = 2;
	unsigned char buttons = 2;
	int version = 0x000800;
	char name[NAME_LENGTH] = "Unknown";
	MissileLauncher mc;

	int *axis;
	int *button;
	int i;
	struct js_event js;


	if (argc != 2) {
		puts("");
		puts("Usage: xbdrive <device>");
		puts("");
		exit(1);
	}
	if ((fd = open(argv[argc - 1], O_RDONLY)) < 0) {
		perror("xbdrive");
		exit(1);
	}
	if(mc.init() != 0){
		std::cerr << "Failed to initialise missile launcher" << std::endl;
		exit(1);
	}

	ioctl(fd, JSIOCGVERSION, &version);
	ioctl(fd, JSIOCGAXES, &axes);
	ioctl(fd, JSIOCGBUTTONS, &buttons);
	ioctl(fd, JSIOCGNAME(NAME_LENGTH), name);

	printf("Joystick (%s) has %d axes and %d buttons. Driver version is %d.%d.%d.\n",
		name, axes, buttons, version >> 16, (version >> 8) & 0xff, version & 0xff);
	printf("Testing ... (interrupt to exit)\n");

	axis = new int[axes]();
	button = new int[buttons]();

	while (1) {
		if (read(fd, &js, sizeof(struct js_event)) != sizeof(struct js_event)) {
			perror("\njstest: error reading");
			exit (1);
		}

		switch(js.type & ~JS_EVENT_INIT) {
		case JS_EVENT_BUTTON:
			button[js.number] = js.value;
			break;
		case JS_EVENT_AXIS:
			axis[js.number] = js.value;
			break;
		}

		if (axis[0] > 0)
		{
			mc.turn(LEFT,0.05);
		}
		else if (axis[0] < 0)
		{
			mc.turn(RIGHT,0.05);
		}

		if (axis[1] > 0)
		{
			mc.turn(UP,0.05);
		}
		else if (axis[1] < 0)
		{
			mc.turn(DOWN,0.05);
		}

		if (button[0] > 0)
		{
			mc.turn(FIRE,0);
		}
		printf("\n");

		if (axes) {
			printf("Axes: ");
			for (i = 0; i < axes; i++)
				printf("%2d:%6d ", i, axis[i]);
		}

		if (buttons) {
			printf("Buttons: ");
			for (i = 0; i < buttons; i++)
				printf("%2d:%s ", i, button[i] ? "on " : "off");
		}

		fflush(stdout);
	}
}
