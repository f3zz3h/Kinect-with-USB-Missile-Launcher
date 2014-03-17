/*******************************************************************************
*                                                                              *
*   PrimeSense NITE 1.3 - Players Sample                                       *
*   Copyright (C) 2010 PrimeSense Ltd.                                         *
*                                                                              *
*******************************************************************************/

#include <XnOpenNI.h>
#include <XnCodecIDs.h>
#include <XnCppWrapper.h>
#include "SceneDrawer.h"

void SaveCalibration();
void LoadCalibration();

xn::Context g_Context;
xn::ScriptNode g_ScriptNode;
xn::DepthGenerator g_DepthGenerator;
xn::UserGenerator g_UserGenerator;
xn::Recorder* g_pRecorder;

XnUserID g_nPlayer = 0;
XnBool g_bCalibrated = FALSE;

#include <GL/glut.h>
#include "kbhit.h"

XnChar g_strPose[20] = "";
XnBool g_bNeedPose = FALSE;

#define GL_WIN_SIZE_X 720
#define GL_WIN_SIZE_Y 480
#define START_CAPTURE_CHECK_RC(rc, what)												\
if (nRetVal != XN_STATUS_OK)														\
{																					\
	printf("Failed to %s: %s\n", what, xnGetStatusString(rc));				\
	StopCapture();															\
	return ;																	\
}

XnBool g_bPause = false;
XnBool g_bRecord = false;
XnBool g_bQuit = false;

void StopCapture()
{
	g_bRecord = false;
	if (g_pRecorder != NULL)
	{
		g_pRecorder->RemoveNodeFromRecording(g_DepthGenerator);
		g_pRecorder->Release();
		delete g_pRecorder;
	}
	g_pRecorder = NULL;
}

void CleanupExit()
{
	if (g_pRecorder)
		g_pRecorder->RemoveNodeFromRecording(g_DepthGenerator);
	StopCapture();

	exit (1);
}


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

	if(g_UserGenerator.GetSkeletonCap().NeedPoseForCalibration())
		printf("Yes, pose required\n");
	else
		printf("NO POSES!");

	//THIS IS THE CALIBRATION MODE....
//	if (!g_bCalibrated) // check on player0 is enough
//	{
//		printf("Look for pose\n");
//		g_UserGenerator.GetPoseDetectionCap().StartPoseDetection("Psi", user);
//		return;
//	}

	XnUInt32 epochTime = 0;
	xnOSGetEpochTime(&epochTime);
	printf("%d New User %d\n", epochTime, user);

	// New user found
	if (g_bNeedPose)
	{
		g_UserGenerator.GetPoseDetectionCap().StartPoseDetection(g_strPose, user);
		printf("Pose needed!\n");
	}
	else
	{
		g_UserGenerator.GetSkeletonCap().RequestCalibration(user, TRUE);
		printf("No posed needed right?\n");
	}

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
	g_UserGenerator.GetSkeletonCap().RequestCalibration(user, FALSE);
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

	#ifdef USE_GLUT
	glOrtho(0, depthMD.XRes(), depthMD.YRes(), 0, -1.0, 1.0);
	#else
	glOrthof(0, depthMD.XRes(), depthMD.YRes(), 0, -1.0, 1.0);
	#endif

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

	#ifdef USE_GLUT
	glutSwapBuffers();
	#endif
}

void glutIdle (void)
{
	if (g_bQuit) {
		CleanupExit();
	}

	// Display the frame
	glutPostRedisplay();
}

void glutKeyboard (unsigned char key, int x, int y)
{
	switch (key)
	{
	case 27:
		CleanupExit();
	case'p':
		g_bPause = !g_bPause;
		break;
	case 'S':
			SaveCalibration();
			break;
	case 'L':
		LoadCalibration();
		break;
	/* case 'k':
		if (g_pRecorder == NULL)
			StartCapture();
		else
			StopCapture();
		printf("Record turned %s\n", g_pRecorder ? "on" : "off");
		break; */
	}
}
void glInit (int * pargc, char ** argv)
{
	glutInit(pargc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(GL_WIN_SIZE_X, GL_WIN_SIZE_Y);
	glutCreateWindow ("Sentry Project");
	//glutFullScreen();
	glutSetCursor(GLUT_CURSOR_NONE);

	glutKeyboardFunc(glutKeyboard);
	glutDisplayFunc(glutDisplay);
	glutIdleFunc(glutIdle);

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);

	glEnableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
}

#define SAMPLE_XML_PATH "../../../Data/Sample-User.xml"

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

#define XN_CALIBRATION_FILE_NAME "UserCalibration.bin"

// Save calibration to file
void SaveCalibration()
{
	XnUserID aUserIDs[20] = {0};
	XnUInt16 nUsers = 20;
	g_UserGenerator.GetUsers(aUserIDs, nUsers);
	for (int i = 0; i < nUsers; ++i)
	{
		// Find a user who is already calibrated
		if (g_UserGenerator.GetSkeletonCap().IsCalibrated(aUserIDs[i]))
		{
			// Save user's calibration to file
			g_UserGenerator.GetSkeletonCap().SaveCalibrationDataToFile(aUserIDs[i], XN_CALIBRATION_FILE_NAME);
			break;
		}
	}
}
// Load calibration from file
void LoadCalibration()
{
	printf("LOADING CALIB DATA!\n");
	XnUserID aUserIDs[20] = {0};
	XnUInt16 nUsers = 20;
	g_UserGenerator.GetUsers(aUserIDs, nUsers);
	for (int i = 0; i < nUsers; ++i)
	{
		// Find a user who isn't calibrated or currently in pose
		if (g_UserGenerator.GetSkeletonCap().IsCalibrated(aUserIDs[i])) continue;
		if (g_UserGenerator.GetSkeletonCap().IsCalibrating(aUserIDs[i])) continue;

		// Load user's calibration from file
		XnStatus rc = g_UserGenerator.GetSkeletonCap().LoadCalibrationDataFromFile(aUserIDs[i], XN_CALIBRATION_FILE_NAME);
		if (rc == XN_STATUS_OK)
		{
			// Make sure state is coherent
			g_UserGenerator.GetPoseDetectionCap().StopPoseDetection(aUserIDs[i]);
			g_UserGenerator.GetSkeletonCap().StartTracking(aUserIDs[i]);
		}
		break;
	}
}


int main(int argc, char **argv)
{
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
		if(!g_UserGenerator.IsCapabilitySupported(XN_CAPABILITY_SKELETON))
		{
			printf("User generator doesn't skeleton detection.\n");
		}
		printf("User generator doesn't support either skeleton or pose detection.\n");
		return XN_STATUS_ERROR;
	}

	g_UserGenerator.GetSkeletonCap().SetSkeletonProfile(XN_SKEL_PROFILE_ALL);

	rc = g_Context.StartGeneratingAll();
	CHECK_RC(rc, "StartGenerating");

	XnCallbackHandle hUserCBs, hCalibrationStartCB, hCalibrationCompleteCB, hPoseCBs;
	g_UserGenerator.RegisterUserCallbacks(NewUser, LostUser, NULL, hUserCBs);
	rc = g_UserGenerator.GetSkeletonCap().RegisterToCalibrationStart(CalibrationStarted, NULL, hCalibrationStartCB);
	CHECK_RC(rc, "Register to calibration start");
	rc = g_UserGenerator.GetSkeletonCap().RegisterToCalibrationComplete(CalibrationCompleted, NULL, hCalibrationCompleteCB);
	CHECK_RC(rc, "Register to calibration complete");
	rc = g_UserGenerator.GetPoseDetectionCap().RegisterToPoseDetected(PoseDetected, NULL, hPoseCBs);
	CHECK_RC(rc, "Register to pose detected");


	glInit(&argc, argv);
	glutMainLoop();

}
