#include <iostream>
#include "launcher.h"

using namespace std;

int main(int argc, char* argv[])
{
	MissileLauncher mc;
	if(mc.init() == 0){
		//Turn for a time mode
		cout<< "STARTING COMANDS"<<endl;
		mc.turn(LEFT, 1.5);
		mc.turn(UP, 1);
		//mc.fire();
		mc.turn(RIGHT, 1.5);
		mc.turn(DOWN, 1);
		//Turn until I say mode
		mc.turn(UP, 0.0);
		sleep(1);
		mc.stop();
		//mc.fire();
		mc.turn(DOWN, 0);
		sleep(1);
		mc.stop();
	}else{
		cerr << "Failed to initialize missile launcher" << endl;
		//Don't call deinit unless init worked
		return 1;
	}
	mc.deinit();
	return 0;
}
