/* 
the basic operations we have to do are:

initialize the library by calling the function libusb_init and creating a session

Call the function libusb_get_device_list to get a list of connected devices. This creates an array of libusb_device containing all usb devices connected to the system.

Loop through all these devices and check their options

Discover the one and open the device either by libusb_open or libusb_open_device_with_vid_pid(when you know vendor and product id of the device) to open the device

Clear the list you got from libusb_get_device_list by using libusb_free_device_list

Claim the interface with libusb_claim_interface (requires you to know the interface numbers of device)

Do desired I/O

Release the device by using libusb_release_interface

Close the device you openedbefore, by using libusb_close

Close the session by using libusb_exit
*/

/*	(	libusb_device_handle * 	dev_handle,
uint8_t 	bmRequestType,
uint8_t 	bRequest,
uint16_t 	wValue,
uint16_t 	wIndex,
unsigned char * data,
uint16_t 	wLength,
unsigned int 	timeout 
) 
*/		


#define DEFAULT_DURATION	50000

#define ML_STOP         0x00
#define ML_UP           0x02
#define ML_DOWN         0x01
#define ML_LEFT         0x04
#define ML_RIGHT        0x08
#define ML_FIRE         0x10

#define ML_FIRE_DELAY	800000

#include <iostream>
#include <libusb-1.0/libusb.h>
#include <usb.h>
#include <time.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
 
int kbhit(void)
{
  struct termios oldt, newt;
  int ch;
  int oldf;
 
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
 
  ch = getchar();
 
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);
 
  if(ch != EOF)
  {
    ungetc(ch, stdin);
    return 1;
  }
 
  return 0;
}

#define KB_UP 119
#define KB_DOWN 115
#define KB_LEFT 97
#define KB_RIGHT 100
#define KB_ESCAPE 27
#define KB_SPACE 32

using namespace std;

unsigned char *data = new unsigned char[8]; //data to write
libusb_device **devs; //pointer to pointer of device, used to retrieve a list of devices
libusb_device_handle *dev_handle; //a device handle
libusb_context *ctx = NULL; //a libusb session

int turretUp()
{
	cout<<("Turret Up.\n");
	data[1]=ML_UP;
	return libusb_control_transfer(dev_handle, 0x21,0x09,0,0,data, 2, 0);
}
int turretDown()
{
	cout<<("Turret Down.\n");
	data[1]=ML_DOWN;
	return libusb_control_transfer(dev_handle, 0x21,0x09,0,0,data, 2, 0);
}
int turretLeft()
{
      	cout<<("Turret Left.\n");	
	data[1]=ML_LEFT;
	return libusb_control_transfer(dev_handle, 0x21,0x09,0,0,data, 2, 0);
}
int turretRight()
{
      cout<<("Turret Right.\n");
	data[1]=ML_RIGHT;
	return libusb_control_transfer(dev_handle, 0x21,0x09,0,0,data, 2, 0);
}
int turretStop()
{
	data[1]=ML_STOP;
	return libusb_control_transfer(dev_handle, 0x21,0x09,0,0,data, 2, 0);
}
int turretFire()
{
	cout<<("FIRE!\n");
	data[1]=ML_FIRE;
	return libusb_control_transfer(dev_handle, 0x21,0x09,0,0,data, 2, 0);

}


int main() {
	
	data[0]=0x02;data[1]=ML_STOP;data[2]=0x00;data[3]=0x00;data[4]=0x00;data[5]=0x00;data[6]=0x00;data[7]=0x00;

	
	int r; //for return values
	ssize_t cnt; //holding number of devices in list
	r = libusb_init(&ctx); //initialize the library for the session we just declared

	if(r < 0) {
		cout<<"Init Error "<<r<<endl; //there was an error
		return 1;
	}

	libusb_set_debug(ctx, 3); //set verbosity level to 3, as suggested in the documentation

	cnt = libusb_get_device_list(ctx, &devs); //get the list of devices

	if(cnt < 0) {
		cout<<"Get Device Error"<<endl; //there was an error
		return 1;
	}

	cout<<cnt<<" Devices in list."<<endl;

	dev_handle = libusb_open_device_with_vid_pid(ctx, 0x2123, 0x1010); //idVendor=0x2123, idProduct=0x1010

	if(dev_handle == NULL)
		cout<<"Cannot open device"<<endl;
	else
		cout<<"Device Opened"<<endl;

	libusb_free_device_list(devs, 1); //free the list, unref the devices in it


	//used to find out how many bytes were written
	int actual; 

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


	 //Move up val

	cout<<"Data->"<<data<<"<-"<<endl; //just to see the data we want to write : abcd
	cout<<"Writing Data..."<<endl;
	 int KB_code=0;

	   while(KB_code != KB_ESCAPE )
	   { 
	     if (kbhit())
	      {
		    KB_code = getchar();
		    printf("KB_code = %i \n",KB_code);

		    switch (KB_code)
		    {
		        case KB_LEFT:
		                turretLeft();
				usleep(30000); 
				turretStop();
		        break;

		        case KB_RIGHT:
		                
		                turretRight();
				usleep(30000); 
				turretStop();
		        break;

		        case KB_UP:
		                
		                turretUp();
				usleep(30000); 
				turretStop();
		        break;

		        case KB_DOWN:
		                turretDown();
				usleep(30000); 
				turretStop();
		        break;
			case KB_SPACE:
		                turretFire();
				usleep(200000); 
				//turretStop();
		        break;
			default :
			 printf("KB_code = %i \n",KB_code);

		    }        

	      }
	}
	if(r == 0 && actual == 4) //we wrote the 4 bytes successfully
		cout<<"Writing Successful!"<<endl;
	else
		cout<<"Write Error"<<endl;
	
	r = libusb_release_interface(dev_handle, 0); //release the claimed interface

	if(r!=0) {
		cout<<"Cannot Release Interface"<<endl;
		return 1;
	}
	cout<<"Released Interface"<<endl;

	libusb_close(dev_handle); //close the device we opened
	libusb_exit(ctx); //needs to be called to end the

	delete[] data; //delete the allocated memory for data
	return 0;
}

