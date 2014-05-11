#include "launcher.h"

using namespace std;

int MissileLauncher::fire()
{
	int retval = 1;
	cout << "Fire!" << endl;
	//need to wait 6.77 seconds to fire
	struct timespec toWait;
	toWait.tv_sec = 6;
	toWait.tv_nsec = 770000000;

	const struct timespec* delay = &toWait;
	struct timespec* remainder;

	//Start launch sequence and wait for launch
	if (sendMsg(FIRE) < 0)
	{
		cerr << "Could not fire." << endl;
		retval = -1;
	}
	else
	{
		nanosleep(delay, remainder);
	}

	//Stop, so no more missiles launch
	stop();

	return retval;
}

int MissileLauncher::turn(MissileCmd direction, double delay)
{
	int retval = 1;
	if (direction == FIRE)
	{
		retval = fire();
	}
	else if (direction == STOP)
	{
		retval = stop();
	}
	else
	{
		if (delay == 0)
		{
			/* Start moving. This means it keeps moving until you tell it to stop */
			retval = sendMsg(direction);
		}
		else
		{
			/* Move for the specified time period, then stop. */
			struct timespec toWait;
			toWait.tv_sec = (int)delay;
			toWait.tv_nsec = (delay - toWait.tv_sec) * 1000000000;

			const struct timespec* timedelay = &toWait;
			struct timespec* remainder;

			//Start turn and wait for movement
			retval = sendMsg(direction);
			nanosleep(timedelay, remainder);

			//Stop turning
			stop();
		}
	}
	return retval;
}

int MissileLauncher::stop()
{
	int ret = sendMsg(STOP);
	if (ret < 0)
	{
		cerr << "Cannot stop, error: " << ret << endl;
	}
	return ret;
}

int MissileLauncher::sendMsg(MissileCmd control)
{
	unsigned char *data = new unsigned char[8];

	for (int i = 0; i < 8; i++)
	{
		data[i] = 0x00;
	}

	//send control message
	data[0] = 0x02;
	data[1] = control;

	int ret = libusb_control_transfer(dev_handle, 0x21,0x09,0,0,data, 2, 0);
	
	usleep(10000);
	delete[] data;

	if (ret < 0)
	{
		cerr << "Cannot send command, error:" << ret << "Cmd:" << control << endl;
		return -1;
	}

	return ret;
}

int MissileLauncher::deinit()
{
	int r = libusb_release_interface(dev_handle, 0); //release the claimed interface

	if(r!=0)
	{
		cout<<"Cannot Release Interface"<<endl;
	}
	cout<<"Released Interface"<<endl;

	libusb_close(dev_handle); //close the device we opened
	libusb_exit(ctx); //needs to be called to end the
	return r;
}

int MissileLauncher::init()
{
	ctx = NULL;

	int r; //for return values
	ssize_t cnt; //holding number of devices in list
	r = libusb_init(&ctx); //initialize the library for the session we just declared

	if(r < 0) {
		cout<<"Init Error "<<r<<endl; //there was an error
		return 1;
	}

	libusb_set_debug(ctx, 3); //set verbosity level to 3, as suggested in the documentation

	cnt = libusb_get_device_list(ctx, &devs); //get the list of devices

	//Check how many devices
	if(cnt < 0) {
		cout<<"Get Device Error"<<endl; //there was an error
		return 1;
	}

	//Open ML device  //idVendor=0x2123, idProduct=0x1010
	dev_handle = libusb_open_device_with_vid_pid(ctx, 0x2123, 0x1010);

	if(dev_handle == NULL)
		cout<<"Cannot open device"<<endl;
	else
		cout<<"Device Opened"<<endl;

	//free the list, unref the devices in it
	libusb_free_device_list(devs, 1); 

	//find out if kernel driver is attached
	if(libusb_kernel_driver_active(dev_handle, 0) == 1) 
	{ 
		cout<<"Kernel Driver Active"<<endl;
		//detach it
		if(libusb_detach_kernel_driver(dev_handle, 0) == 0) cout<<"Kernel Driver Detached!"<<endl;
	}
	
	r = libusb_claim_interface(dev_handle, 0); //claim interface 0 (the first) of device (mine had jsut 1)
	if(r < 0) {
		cout<<"Cannot Claim Interface"<<endl;
		return 1;
	}
	cout<<"Claimed Interface"<<endl;

	return 0;
}

