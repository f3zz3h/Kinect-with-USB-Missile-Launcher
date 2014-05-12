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

	reset();
}
void reset()
{
	//Push past max so we garuntee we reached limits. Then return to centre.
	usleep(10000);
	mc.turn(LEFT, MAX_X_ML+1);
	usleep(10000);
	mc.turn(RIGHT,MID_X_ML);
	usleep(10000);
	mc.turn(UP, MAX_Y_ML+1);
	usleep(10000);
	mc.turn(DOWN, MID_Y_ML);
	usleep(10000);

	//Assign current position to storage array to keep track of location.
	current_position[X_INDEX] = MID_X_ML;
	current_position[Y_INDEX] = MID_Y_ML;
	current_position[Z_INDEX] = 0;
}
void deinit_ml()
{
	mc.deinit();
}
void fire()
{
	mc.turn(FIRE,0);
}
int move_to_position()
{
	int ret = -1;
	/* get difference between current and mapped for amount to move */
	float move_amount_x =  mapped_xyz.x - current_position[X_INDEX];
	float move_amount_y =  mapped_xyz.y - current_position[Y_INDEX];

	//Out of bounds check
	if (((mapped_xyz.x < -MAX_X_ML) || (mapped_xyz.x > MAX_X_ML)) &&
		((mapped_xyz.y < -MAX_Y_ML) || (mapped_xyz.y > MAX_Y_ML))	)
	{
		return OOB;
	}
	//Check movement amount is not 0
	if ((move_amount_x == 0) && (move_amount_y == 0))
	{
		return NO_MOVE;
	}

	/* Moves X */
	if (move_amount_x != 0)
	{
		usleep(1000);
		/* Check the move to position is valid */
		if ((mapped_xyz.x < KINECT_FOV_MAX_X) && (mapped_xyz.x > KINECT_FOV_MIN_X))
		{
			//printf("Move Amount X:%f\n", move_amount_x);
			//printf("mapped = %f - current = %f", mapped_xyz.x , current_position[X_INDEX]);

			// Move amount give direction
			if( move_amount_x < 0 )
			{
				//Check movememnt amount is worth moving
				if (move_amount_x*-1 > 0.02)
				{
					/* negative is left of Kinect frame thus move right */
					if (mc.turn(RIGHT,0.05))
					{
						//check for successful movement before updating current position.
						current_position[X_INDEX] -= 0.05; //mapped_xyz.x;
						ret = SUCCESS;
					}
				}
				else ret = NO_MOVE;
			}
			else if( move_amount_x > 0 )
			{
				//Check movememnt amount is worth moving
				if (move_amount_x > 0.02)
				{
					if(mc.turn(LEFT,0.05))
					{
						//check for successful movement
						current_position[X_INDEX] += 0.05;//mapped_xyz.x;
						ret = SUCCESS;
					}
				}
				else ret = NO_MOVE;
			}
		}
	}

	/* Moves Y */
	if (move_amount_y != 0)
	{
		usleep(1000);

		//printf("Move Amount Y:%f\n", move_amount_y);
		//printf("Mapped Y = %f - current = %f\n", mapped_xyz.y , current_position[Y_INDEX]);

		/*For now rudemntry FOV for kinect */
		if ((mapped_xyz.y < KINECT_FOV_MAX_Y) && (mapped_xyz.y > KINECT_FOV_MIN_Y))
		{
			//Move amount give direction
			if( move_amount_y < 0 )
			{
				//Check movement amount is worth moving
				if (move_amount_y*-1 > 0.025)
				{
					if(mc.turn(UP,0.025))
					{
						//check for successful movement before updating current position.
						current_position[Y_INDEX] -= 0.05; //mapped_xyz.y;
						ret = SUCCESS;
					}
					//need to check here as is suceeded on horizontal we have moved.
					else if (ret != SUCCESS)
					{
						ret = NO_MOVE;
					}
				}

			}
			else if( move_amount_y > 0 )
			{
				//Check movement amount is worth moving
				if (move_amount_y > 0.025)
				{
					if(mc.turn(DOWN,0.025))
					{
						//check for successful movement
						current_position[Y_INDEX] += 0.05; //mapped_xyz.y;
						ret = SUCCESS;
					}
					else if (ret != SUCCESS)
					{
						ret = NO_MOVE;
					}
				}
			}
			usleep(2500);
		}
	}

	return ret;
}

void map_xyz(float* player_position)
{
	float range;

	/* Mapping of X */
	float player_x_pos = player_position[X_INDEX];
	player_x_pos = (player_x_pos + MAX_PLAYER_X) / 2;

	range = KINECT_FOV_MAX_X - KINECT_FOV_MIN_X;
	player_position[X_INDEX]  = ((range / MAX_PLAYER_X) * player_x_pos) + KINECT_FOV_MIN_X;

	/* Mapping of Y,Z */
	float player_y_pos = player_position[Y_INDEX];
	player_y_pos = (player_y_pos + MAX_PLAYER_Y) / 2;

	/* Range should be based from the z co-ords eg the distance to target.
	 * for now we will use rudemntry values .25 -> 1
	 */
	range = 0.75;
	player_position[Y_INDEX]  = ((range / MAX_PLAYER_Y) * player_y_pos) + 0.25;
	player_position[Z_INDEX] = player_position[Z_INDEX];
}

float* positiontOfTarget(XnUserID player)
{
	float* return_data = new float[3];

	XnSkeletonJoint eJoint1 = XN_SKEL_TORSO;
	XnSkeletonJoint eJoint2 = XN_SKEL_NECK;

	XnUserID aUsers[15];
	XnUInt16 nUsers = 15;

	if (!g_UserGenerator.GetSkeletonCap().IsCalibrated(player))
	{
		//printf("not calibrated!\n");
		return NULL;
	}
	if (!g_UserGenerator.GetSkeletonCap().IsTracking(player))
	{
		//printf("not tracked!\n");
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

	/* Actually only using pt[0] AKA TORSO, should use half way between this and neck for
	 * the widest part of the body as a target
	 */
	return_data[X_INDEX] = pt[0].X;
	return_data[Y_INDEX] = pt[0].Y;
	return_data[Z_INDEX] = pt[0].Z;

	return return_data;
}
