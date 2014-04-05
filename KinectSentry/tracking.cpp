#include "tracking.h"
#include "global.h"

MissileLauncher mc;

extern xn::UserGenerator g_UserGenerator;
extern xn::DepthGenerator g_DepthGenerator;

float current_position[2];

int init_ml()
{
	if(mc.init() != 0){
		std::cerr << "Failed to initialize missile launcher" << std::endl;
		return 0;
	}

	//Push past max so we garuntee we reached limits. Then return to centre.
	mc.turn(LEFT, MAX_X_ML+1);
	mc.turn(RIGHT,MID_X_ML);
	mc.turn(DOWN, MAX_Y_ML+1);
	mc.turn(UP, MID_Y_ML);

	//Assign current position to storage arage to keep track of location.
	current_position[X_INDEX] = MID_X_ML;
	current_position[Y_INDEX] = MID_Y_ML;
}
/*void test()
{
	move_to_position(map_x(360));
	move_to_position(map_x(100));
	move_to_position(map_x(620));
	move_to_position(map_x(720));
	move_to_position(map_x(1));
	move_to_position(map_x(360));
}*/
void deinit_ml()
{
	mc.deinit();
}
void* move_to_position(void*)
{
	float mapped_position = mapped_xyz.x;
	/* get difference between current and mapped for amount to move */
	float move_amount =  mapped_position - current_position[X_INDEX];

	//Out of bounds check
	if ((move_amount < -MAX_X_ML) || (move_amount > MAX_X_ML))
	{
		std::cout<<"Out of bounds movement:"<<move_amount<<std::endl;
		goto exit;
	}

	if (move_amount == 0)
	{
		goto exit;
	}
	else if( move_amount < 0 )
	{
		std::cout<<"LEFT:move_amount"<<move_amount<<std::endl;
		mc.turn(LEFT,-1*move_amount);
		current_position[X_INDEX] = mapped_position;
	}
	else if( move_amount > 0 )
	{
		std::cout<<"RIGHT:move_amount"<<move_amount<<std::endl;
		mc.turn(RIGHT,move_amount);
		current_position[X_INDEX] = mapped_position;
	}

	goto exit;

exit:
	pthread_mutex_lock(&moving_mutex);
	moving = false;
	pthread_mutex_unlock(&moving_mutex);

	pthread_exit(NULL);
}

float map_x(float player_x_pos)
{
	float range;
	float x_val;

	player_x_pos = (player_x_pos + MAX_PLAYER_X) / 2;

	range = KINECT_FOV_MAX - KINECT_FOV_MIN;
	x_val = ((range / MAX_PLAYER_X) * player_x_pos) + KINECT_FOV_MIN;

	/*
	if (x_val > 0)
		x_val += KINECT_FOV_MIN;
	else
		x_val -= KINECT_FOV_MIN;
	*/

	std::cout<<"X_VAL::"<<x_val<<std::endl;

	/* Divide by 2 due to pos and negative range of x for kinect */
	return x_val;
}

float* positiontOfTarget(XnUserID player)
{
	float* return_data = new float[2];

	XnSkeletonJoint eJoint1 = XN_SKEL_TORSO;
	XnSkeletonJoint eJoint2 = XN_SKEL_NECK;

	XnUserID aUsers[15];
	XnUInt16 nUsers = 15;

	if (!g_UserGenerator.GetSkeletonCap().IsCalibrated(player))
	{
		printf("not calibrated!\n");
		return NULL;
	}
	if (!g_UserGenerator.GetSkeletonCap().IsTracking(player))
	{
		printf("not tracked!\n");
		return NULL;
	}

	for (int i = 0; i < nUsers; ++i)
	{
		XnPoint3D com;
		g_UserGenerator.GetCoM(aUsers[i], com);
		g_DepthGenerator.ConvertRealWorldToProjective(1, &com, &com);
	}

	if (player == 0)
	{
		return NULL;
	}

	XnSkeletonJointPosition joint1, joint2;
	g_UserGenerator.GetSkeletonCap().GetSkeletonJointPosition(player, eJoint1, joint1);
	g_UserGenerator.GetSkeletonCap().GetSkeletonJointPosition(player, eJoint2, joint2);

	if (joint1.fConfidence < 0.5 || joint2.fConfidence < 0.5)
	{
		return NULL;
	}

	XnPoint3D pt[2];
	pt[0] = joint1.position;
	pt[1] = joint2.position;

	/* Our X and Y positions to return hopefully */
	/* Gives values from -740 <-> +740 on xaxis.. not what was expected but we can deal with it */
	//std::cout<<"X:"<<pt[0].X<<" - Y:"<<pt[0].Y<<std::endl;

	/* Actually only using pt[0] AKA TORSO, should use half way between this and neck for
	 * the widest part of the body as a target
	 */
	return_data[X_INDEX]=pt[0].X;
	return_data[Y_INDEX]=pt[0].Y;

	return return_data;
}
