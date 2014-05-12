/*******************************************************************************
*   Derrived from: PrimeSense NITE 1.3 - Players Sample
*   Copyright (C) 2010 PrimeSense Ltd.
*
*   Edited by: Luke Hart (2013)
*
*******************************************************************************/
#include <pthread.h>
#include "SceneDrawer.h"
#include "tracking.h"

#include <XnOpenNI.h>
#include <XnCodecIDs.h>
#include <XnCppWrapper.h>

#if (XN_PLATFORM == XN_PLATFORM_MACOSX)
        #include <GLUT/glut.h>
#else
        #include <GL/glut.h>
#endif

#define CHECK_RC(rc, what)											\
	if (rc != XN_STATUS_OK)											\
	{																\
		printf("%s failed: %s\n", what, xnGetStatusString(rc));		\
		return rc;													\
	}

#define CHECK_ERRORS(rc, errors, what)		\
	if (rc == XN_STATUS_NO_NODE_PRESENT)	\
{										\
	XnChar strError[1024];				\
	errors.ToString(strError, 1024);	\
	printf("%s\n", strError);			\
	return (rc);						\
}


#define START_CAPTURE_CHECK_RC(rc, what)												\
	if (nRetVal != XN_STATUS_OK)														\
{																					\
	printf("Failed to %s: %s\n", what, xnGetStatusString(rc));				\
	StopCapture();															\
	return ;																	\
}

#define SAMPLE_XML_PATH "/home/luke/FinalYearProject/KinectSentry/Data/Sample-User.xml"
//#define SAMPLE_XML_PATH "/Data/Sample-User.xml"

xn::Context g_Context;
xn::ScriptNode g_ScriptNode;
xn::DepthGenerator g_DepthGenerator;
xn::UserGenerator g_UserGenerator;
xn::Recorder* g_pRecorder;

XnBool g_bPause = false;
XnBool g_bRecord = false;
XnBool g_bQuit = false;
XnBool ml_track = true;

XnUserID g_nPlayer = 0;
XnBool g_bCalibrated = FALSE;

xyz mapped_xyz;

bool moving = false;

#define GL_WIN_SIZE_X 720
#define GL_WIN_SIZE_Y 480

XnBool AssignPlayer(XnUserID user)
{
	if (g_nPlayer != 0)
		return FALSE;

	XnPoint3D com;
	g_UserGenerator.GetCoM(user, com);
	if (com.Z == 0)
		return FALSE;

	printf("Matching for existing calibration\n");
	g_UserGenerator.GetSkeletonCap().LoadCalibrationData(user, 0);
	g_UserGenerator.GetSkeletonCap().StartTracking(user);
	g_nPlayer = user;
	return TRUE;

}
void XN_CALLBACK_TYPE NewUser(xn::UserGenerator& generator, XnUserID user, void* pCookie)
{
	#ifdef FIRST_RUN
	if (!g_bCalibrated) // check on player0 is enough
	{
		printf("Look for pose\n");
		g_UserGenerator.GetPoseDetectionCap().StartPoseDetection("Psi", user);
		return;
	}
	#endif

	AssignPlayer(user);
 	if (g_nPlayer == 0)
 	{
 		printf("Assigned user\n");
 		g_UserGenerator.GetSkeletonCap().LoadCalibrationData(user, 0);
 		g_UserGenerator.GetSkeletonCap().StartTracking(user);
 		g_nPlayer = user;
 	}
}
void FindPlayer()
{
	if (g_nPlayer != 0)
	{
		return;
	}
	XnUserID aUsers[20];
	XnUInt16 nUsers = 20;
	g_UserGenerator.GetUsers(aUsers, nUsers);

	for (int i = 0; i < nUsers; ++i)
	{
		if (AssignPlayer(aUsers[i]))
			return;
	}
}
void LostPlayer()
{
	g_nPlayer = 0;
	FindPlayer();

}
void XN_CALLBACK_TYPE LostUser(xn::UserGenerator& generator, XnUserID user, void* pCookie)
{
	printf("Lost user %d\n", user);
	if (g_nPlayer == user)
	{
		LostPlayer();
	}
}
void XN_CALLBACK_TYPE PoseDetected(xn::PoseDetectionCapability& pose, const XnChar* strPose, XnUserID user, void* cxt)
{
	printf("Found pose \"%s\" for user %d\n", strPose, user);
	g_UserGenerator.GetSkeletonCap().RequestCalibration(user, TRUE);
	g_UserGenerator.GetPoseDetectionCap().StopPoseDetection(user);
}

void XN_CALLBACK_TYPE CalibrationStarted(xn::SkeletonCapability& skeleton, XnUserID user, void* cxt)
{
	printf("Calibration started\n");
}

void XN_CALLBACK_TYPE CalibrationCompleted(xn::SkeletonCapability& skeleton, XnUserID user, XnCalibrationStatus eStatus, void* cxt)
{
	printf("Calibration done [%d] %ssuccessfully\n", user, (eStatus == XN_CALIBRATION_STATUS_OK)?"":"un");
	if (eStatus == XN_CALIBRATION_STATUS_OK)
	{
		if (!g_bCalibrated)
		{
			g_UserGenerator.GetSkeletonCap().SaveCalibrationData(user, 0);
			g_nPlayer = user;
			g_UserGenerator.GetSkeletonCap().StartTracking(user);
			g_bCalibrated = TRUE;
		}

		XnUserID aUsers[10];
		XnUInt16 nUsers = 10;
		g_UserGenerator.GetUsers(aUsers, nUsers);
		for (int i = 0; i < nUsers; ++i)
			g_UserGenerator.GetPoseDetectionCap().StopPoseDetection(aUsers[i]);
	}
}

void DrawProjectivePoints(XnPoint3D& ptIn, int width, double r, double g, double b)
{
	static XnFloat pt[3];

	pt[0] = ptIn.X;
	pt[1] = ptIn.Y;
	pt[2] = 0;
	glColor4f(r,
		g,
		b,
		1.0f);
	glPointSize(width);
	glVertexPointer(3, GL_FLOAT, 0, pt);
	glDrawArrays(GL_POINTS, 0, 1);

	glFlush();

}
void* sentryFollow(void*)
{
	int noMoveCount = 0;
	int firedCount = 0;
	float* position = NULL;
	int ret;
	init_ml();

	//Clear calibration text
	system("clear");

	while (ml_track = true)
	{
		position = positiontOfTarget(g_nPlayer);
		if (position != NULL)
		{
			map_xyz(position);

			mapped_xyz.x = position[0];
			mapped_xyz.y = position[1];
			mapped_xyz.z = position[2];
			//printf("X = %f, Y = %f, Z = %f\n",mapped_xyz.x, mapped_xyz.y, mapped_xyz.z);
			ret = move_to_position();

			if (ret == 0)
			{
				noMoveCount++;
			}
			else
			{
				noMoveCount = 0;
			}
			if (noMoveCount == 50)
			{
				fire();
				firedCount++;
				noMoveCount = 0;
			}
			if (firedCount >= 4)
			{
				reset();
				//ALLOW RELOAD TIME
				system("clear");
				printf("\n--------------------------------------"
					   "\n-   YOU HAVE 10 SECONDS TO RELOAD    -"
					   "\n--------------------------------------\n");
				sleep(10);
				noMoveCount = 0;
				firedCount = 0;
			}
		}
		usleep(10000);
	}

	delete position;

}
// this function is called each frame
void glutDisplay (void)
{
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Setup the OpenGL viewpoint
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	xn::SceneMetaData sceneMD;
	xn::DepthMetaData depthMD;
	g_DepthGenerator.GetMetaData(depthMD);

	glOrtho(0, depthMD.XRes(), depthMD.YRes(), 0, -1.0, 1.0);

	glDisable(GL_TEXTURE_2D);

	if (!g_bPause)
	{
		// Read next available data
		g_Context.WaitOneUpdateAll(g_DepthGenerator);
	}

	// Process the data
	//DRAW
	g_DepthGenerator.GetMetaData(depthMD);
	g_UserGenerator.GetUserPixels(0, sceneMD);
	DrawDepthMap(depthMD, sceneMD, g_nPlayer);

	if (g_nPlayer != 0)
	{
		XnPoint3D com;
		g_UserGenerator.GetCoM(g_nPlayer, com);
		if (com.Z == 0)
		{
			g_nPlayer = 0;
			FindPlayer();
		}
	}

	glutSwapBuffers();
}

void glutIdle (void)
{
	// Display the frame
	glutPostRedisplay();
}

void glutKeyboard (unsigned char key, int x, int y)
{
	switch (key)
	{
	case'p':
		g_bPause = !g_bPause;
		break;

	}
}
void glInit (int * pargc, char ** argv)
{
	glutInit(pargc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(GL_WIN_SIZE_X, GL_WIN_SIZE_Y);
	glutCreateWindow ("Kinect Sentry Application | FYP Demo 2013");
	//glutFullScreen();
	glutSetCursor(GLUT_CURSOR_NONE);

	/* Could use keyboard function for game controller instead as we
	 * don't need any keyboard inputs in this applicaiton */
	glutKeyboardFunc(glutKeyboard);
	glutDisplayFunc(glutDisplay);
	glutIdleFunc(glutIdle);

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);

	glEnableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
}

int main(int argc, char **argv)
{
	/* TODO: Thread this so that kinect init can begin */


	XnStatus rc = XN_STATUS_OK;
	xn::EnumerationErrors errors;

	rc = g_Context.InitFromXmlFile(SAMPLE_XML_PATH, g_ScriptNode, &errors);
	CHECK_ERRORS(rc, errors, "InitFromXmlFile");
	CHECK_RC(rc, "InitFromXml");

	rc = g_Context.FindExistingNode(XN_NODE_TYPE_DEPTH, g_DepthGenerator);
	CHECK_RC(rc, "Find depth generator");
	rc = g_Context.FindExistingNode(XN_NODE_TYPE_USER, g_UserGenerator);
	CHECK_RC(rc, "Find user generator");

	if (!g_UserGenerator.IsCapabilitySupported(XN_CAPABILITY_SKELETON) ||
		!g_UserGenerator.IsCapabilitySupported(XN_CAPABILITY_POSE_DETECTION))
	{
		printf("User generator doesn't support either skeleton or pose detection.\n");
		return XN_STATUS_ERROR;
	}

	g_UserGenerator.GetSkeletonCap().SetSkeletonProfile(XN_SKEL_PROFILE_ALL);

	rc = g_Context.StartGeneratingAll();
	CHECK_RC(rc, "StartGenerating");

	XnCallbackHandle hUserCBs, hCalibrationStartCB, hCalibrationCompleteCB, hPoseCBs;
	g_UserGenerator.RegisterUserCallbacks(NewUser, LostUser, NULL, hUserCBs);
	rc = g_UserGenerator.GetSkeletonCap().RegisterToCalibrationStart(CalibrationStarted, NULL, hCalibrationStartCB);
	CHECK_RC(rc, "Register to calbiration start");
	rc = g_UserGenerator.GetSkeletonCap().RegisterToCalibrationComplete(CalibrationCompleted, NULL, hCalibrationCompleteCB);
	CHECK_RC(rc, "Register to calibration complete");
	rc = g_UserGenerator.GetPoseDetectionCap().RegisterToPoseDetected(PoseDetected, NULL, hPoseCBs);
	CHECK_RC(rc, "Register to pose detected");

	/* Create thread to always be moving */
	pthread_t thread;

	rc = pthread_create(&thread, NULL, sentryFollow, NULL);

	if (rc){
		std::cout << "Error:unable to create thread," << rc << std::endl;
		exit(-1);
	}

	/* Enter display loop */
	glInit(&argc, argv);
	glutMainLoop();

	/* DIsplay loop finished thus deinit the ML*/
	ml_track = false;
	pthread_join(thread, NULL);
	deinit_ml();
}
