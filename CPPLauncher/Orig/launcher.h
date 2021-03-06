#ifndef MISSILE_LAUNCHER_H_
#define MISSILE_LAUNCHER_H_

#include <iostream>
#include <libusb-1.0/libusb.h>
#include <usb.h>
#include <time.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

typedef enum
{
	FIRE = 0x10, LEFT = 0x04, RIGHT = 0x08, UP = 0x01, DOWN = 0x02, STOP = 0x00
} MissileCmd;

class MissileLauncher
{
public:
	//Setup and teardown
	int init();
	void deinit();
	//Motion commands
	void fire();
	void stop();
	void turn(MissileCmd direction, double duration);
private:
	int sendMsg(MissileCmd cmd);
	libusb_device **devs; //pointer to pointer of device, used to retrieve a list of devices
	libusb_device_handle *dev_handle; //a device handle
	libusb_context *ctx; //a libusb session
	
};
 
#endif /* MISSILE_LAUNCHER_H_ */
