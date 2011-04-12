#include "CinderOpenNI.h"
#include <sstream>

XnFloat Colors[][3] =
{
	{0, 0, 1},
	{1, 0.3,0.2},
	{0,1,0},
	{1,1,0},
	{1,0,0},
	{1,.5,0},
	{.5,1,0},
	{0,.5,1},
	{.5,0,1},
	{1,1,.5},
	{1,1,1}
};
XnUInt32 nColors = 10;

unsigned int getClosestPowerOfTwo(unsigned int n)
{
	unsigned int m = 2;
	while(m < n) m<<=1;

	return m;
}


#pragma mark Singleton
CinderOpenNISkeleton* CinderOpenNISkeleton::gCinderOpenNISkeleton = NULL;
CinderOpenNISkeleton* CinderOpenNISkeleton::getInstance() {
	if (!gCinderOpenNISkeleton){
		gCinderOpenNISkeleton = new CinderOpenNISkeleton();
	}
	return gCinderOpenNISkeleton;
}
void CinderOpenNISkeleton::shutdown(){
	gCinderOpenNISkeleton->mContext.Shutdown();
	gCinderOpenNISkeleton = NULL;
}

#pragma mark CinderOpenNISkeleton
CinderOpenNISkeleton::CinderOpenNISkeleton() {
	_slot = 0;
	mNeedPose = FALSE;
	//gCinderOpenNISkeleton->mStrPose[20];
	pDepthTexBuf = NULL;
	bInitialized = false;
	mJointConfidence = 0.5;
	maxUsers = 15;
}

CinderOpenNISkeleton::~CinderOpenNISkeleton() {
	app::console() << "Shutting down!" << std::endl;
	shouldStopUpdating();
	gCinderOpenNISkeleton->mContext.Shutdown();
}


// Start the thread
void CinderOpenNISkeleton::shouldStartUpdating()
{
	assert(!_thread);

	_thread = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&CinderOpenNISkeleton::update, this)));
}

void CinderOpenNISkeleton::update()
{
	int iteration = 0;

	while (!_stopRequested)
	{
		XnStatus nRetVal = XN_STATUS_OK;

		// nRetVal = skeleton->mContext.WaitOneUpdateAll( skeleton->mDepthGenerator );
		nRetVal = mContext.WaitAndUpdateAll();

		if( nRetVal != XN_STATUS_OK ) { // Update
			printf("WaitAndUpdateAll failed: %s\n", xnGetStatusString(nRetVal));
			continue;
		}
		if( !mUserGenerator ) { // User gen
			std::cout << "No user generator" << std::endl;
			return;
		}

		// Get depthgenerator
		nRetVal = mContext.FindExistingNode(XN_NODE_TYPE_DEPTH, mDepthGenerator);
		if( nRetVal != XN_STATUS_OK ) {
			printf("FindExistingNode failed: %s\n", xnGetStatusString(nRetVal));
			continue;
		}

		// Cannot retrieve FPS or depthGenerator
		if(!&xnFPS) {
			std::cout << "(Oneday) OpenNIThreadRunner Not ready!" << std::endl;
			continue;
		}

		// All clear - update
		maxUsers = 15;
		mUserGenerator.GetUsers(currentUsers, maxUsers);
		mDepthGenerator.GetMetaData( mDepthMD );
		mUserGenerator.GetUserPixels(0, mSceneMD);
		setDepthSurface();


		++iteration;
	}
}

// Stop the thread
void CinderOpenNISkeleton::shouldStopUpdating()
{
	assert(_thread);

	_stopRequested = true;
	_thread->join();
}

// Barebones setup
bool CinderOpenNISkeleton::setup()
{
	XnStatus nRetVal = XN_STATUS_OK;
	xn::EnumerationErrors errors;

	// Init
	nRetVal = mContext.Init();
	CHECK_RC(nRetVal, "Init", true);

	nRetVal = xnFPSInit(&xnFPS, 180);
	CHECK_RC(nRetVal, "FPS Init", true);

    worldOffset.x = -160;
    worldOffset.y = -160;
    worldOffset.z = -2784;

	// Output device production nodes (user, depth, etc)
	debugOutputNodeTypes();

	return true;
}


// Setup SimpleGUI
void CinderOpenNISkeleton::setupGUI()
{
//#ifdef USE_SIMPLE_GUI
//	float tRange = 4000;		// Range of translate calls
//
//	GLOBAL_GUI->addColumn();		// Place in own column
//	GLOBAL_GUI->addLabel("OpenNI");
//	GLOBAL_GUI->addParam("translateX", &gCinderOpenNISkeleton->worldOffset.x, -tRange, tRange, gCinderOpenNISkeleton->worldOffset.x);
//	GLOBAL_GUI->addParam("translateY", &gCinderOpenNISkeleton->worldOffset.y, -tRange, tRange, gCinderOpenNISkeleton->worldOffset.y);
//	GLOBAL_GUI->addParam("translateZ", &gCinderOpenNISkeleton->worldOffset.z, -tRange, tRange, gCinderOpenNISkeleton->worldOffset.z);
//#endif
}


bool CinderOpenNISkeleton::setupFromXML(string path)
{
	XnStatus nRetVal = XN_STATUS_OK;
	xn::EnumerationErrors errors;

	std::cout << "Path:" << path.c_str() << std::endl;
	nRetVal = gCinderOpenNISkeleton->mContext.InitFromXmlFile(path.c_str(), &errors);


	nRetVal = xnFPSInit(&gCinderOpenNISkeleton->xnFPS, 180);
	CHECK_RC(nRetVal, "FPS Init", true);

	// Output device production nodes (user, depth, etc)
	gCinderOpenNISkeleton->debugOutputNodeTypes();

	// No nodes!?
	if (nRetVal == XN_STATUS_NO_NODE_PRESENT) {
		XnChar strError[1024];
		errors.ToString(strError, 1024);
		app::console() << strError << endl;
		return false;
	}
	else if (nRetVal != XN_STATUS_OK)
	{
		app::console() << "Open failed: " << xnGetStatusString(nRetVal) << endl;
		return false;
	}


	// Find depth generator
	nRetVal = gCinderOpenNISkeleton->mContext.FindExistingNode(XN_NODE_TYPE_DEPTH, gCinderOpenNISkeleton->mDepthGenerator);
	CHECK_RC(nRetVal, "Find depth generator", true);

	// Find skeleton / user generator
	nRetVal = gCinderOpenNISkeleton->mContext.FindExistingNode(XN_NODE_TYPE_USER, gCinderOpenNISkeleton->mUserGenerator);
	if (nRetVal != XN_STATUS_OK)
	{
		// Create one
		nRetVal = gCinderOpenNISkeleton->mUserGenerator.Create(gCinderOpenNISkeleton->mContext);
		CHECK_RC(nRetVal, "Find user generator", true);
		return false;
	}

	// Check if user generator can detect skeleton
	if (!gCinderOpenNISkeleton->mUserGenerator.IsCapabilitySupported(XN_CAPABILITY_SKELETON)) {
		app::console() << "Supplied user generator doesn't support skeleton\n" << endl;
		return false;
	}

	// Register callbacks
	nRetVal = gCinderOpenNISkeleton->setupCallbacks();

	// Start generating
	nRetVal = gCinderOpenNISkeleton->mContext.StartGeneratingAll();
	CHECK_RC(nRetVal, "StartGenerating", true);

	return true;
}

#define GESTURE_TO_USE "Click"
XnStatus CinderOpenNISkeleton::setupCallbacks()
{
	XnStatus nRetVal = XN_STATUS_OK;

	// USER CALLBACKS
	XnCallbackHandle hUserCallbacks, hCalibrationCallbacks, hPoseCallbacks;
	nRetVal = gCinderOpenNISkeleton->mUserGenerator.RegisterUserCallbacks(CinderOpenNISkeleton::User_NewUser, User_LostUser, NULL, hUserCallbacks);
	CHECK_RC(nRetVal, "RegisterUserCallbacks", true);

	// CALLABERATION CALLBACKS
	nRetVal = gCinderOpenNISkeleton->mUserGenerator.GetSkeletonCap().RegisterCalibrationCallbacks(UserCalibration_CalibrationStart, UserCalibration_CalibrationEnd, NULL, hCalibrationCallbacks);
	CHECK_RC(nRetVal, "RegisterCalibrationCallbacks", true);

	// POSE CALLBACKS
	if (gCinderOpenNISkeleton->mUserGenerator.GetSkeletonCap().NeedPoseForCalibration())
	{
		gCinderOpenNISkeleton->mNeedPose = TRUE;
		if (!gCinderOpenNISkeleton->mUserGenerator.IsCapabilitySupported(XN_CAPABILITY_POSE_DETECTION))
		{
			app::console() << "Pose required, but not supported" << endl;
			return false;
		}


		gCinderOpenNISkeleton->mUserGenerator.GetPoseDetectionCap().RegisterToPoseCallbacks(UserPose_PoseDetected, NULL, NULL, hPoseCallbacks);
		gCinderOpenNISkeleton->mUserGenerator.GetSkeletonCap().GetCalibrationPose(gCinderOpenNISkeleton->mStrPose);
	}

	gCinderOpenNISkeleton->mUserGenerator.GetSkeletonCap().SetSkeletonProfile(XN_SKEL_PROFILE_ALL);

	return nRetVal;
}

#pragma mark Callbacks
/* Callbacks - Probably un-classable */

void XN_CALLBACK_TYPE CinderOpenNISkeleton::User_NewUser(xn::UserGenerator& generator, XnUserID nId, void* pCookie)
{
	app::console() <<  "(ONEDAY)::OpenNICallback::User_NewUser: " << nId << endl;

	if( gCinderOpenNISkeleton->_isFirstCalibrationComplete
			&& gCinderOpenNISkeleton->mUserGenerator.GetSkeletonCap().IsCalibrationData( gCinderOpenNISkeleton->_slot) ) {

		app::console() <<  "(ONEDAY)::OpenNICallback::User_NewUser - REUSING TRACKING!!: " << nId << endl;
		gCinderOpenNISkeleton->mUserGenerator.GetSkeletonCap().LoadCalibrationData( nId, gCinderOpenNISkeleton->_slot );
		gCinderOpenNISkeleton->mUserGenerator.GetSkeletonCap().StartTracking( nId );

		return;
	}

	// New user found
	if (gCinderOpenNISkeleton->mNeedPose) {
		gCinderOpenNISkeleton->mUserGenerator.GetPoseDetectionCap().StartPoseDetection(gCinderOpenNISkeleton->mStrPose, nId);
	}
	else
	{
		gCinderOpenNISkeleton->mUserGenerator.GetSkeletonCap().RequestCalibration(nId, TRUE);
	}
}

// Callback: An existing user was lost
void XN_CALLBACK_TYPE CinderOpenNISkeleton::User_LostUser(xn::UserGenerator& generator, XnUserID nId, void* pCookie)
{
	app::console() <<  "(ONEDAY)::OpenNICallback::User_LostUser: " << nId;
}

// Callback: Detected a pose
void XN_CALLBACK_TYPE CinderOpenNISkeleton::UserPose_PoseDetected(xn::PoseDetectionCapability& capability, const XnChar* strPose, XnUserID nId, void* pCookie)
{
	app::console() <<  "(ONEDAY)::OpenNICallback::UserPose_PoseDetected: " << gCinderOpenNISkeleton->mStrPose << " detected for user " << nId <<endl;
	gCinderOpenNISkeleton->mUserGenerator.GetPoseDetectionCap().StopPoseDetection(nId);
	gCinderOpenNISkeleton->mUserGenerator.GetSkeletonCap().RequestCalibration(nId, TRUE);
}

// Callback: Started calibration
void XN_CALLBACK_TYPE CinderOpenNISkeleton::UserCalibration_CalibrationStart(xn::SkeletonCapability& capability, XnUserID nId, void* pCookie)
{
	app::console() <<  "(ONEDAY)::OpenNICallback::UserCalibration_CalibrationStart: " << nId << endl;

}

// Callback: Finished calibration
void XN_CALLBACK_TYPE CinderOpenNISkeleton::UserCalibration_CalibrationEnd(xn::SkeletonCapability& capability, XnUserID nId, XnBool bSuccess, void* pCookie)
{
	if (bSuccess)
	{
		// Calibration succeeded

		if( !gCinderOpenNISkeleton->_isFirstCalibrationComplete ) {
			gCinderOpenNISkeleton->mUserGenerator.GetSkeletonCap().SaveCalibrationData( nId, gCinderOpenNISkeleton->_slot );
			gCinderOpenNISkeleton->_isFirstCalibrationComplete = true;
		}

		app::console() <<  "(ONEDAY)::OpenNICallback::UserCalibration_CalibrationEnd: " <<  nId << std::endl;
		float mSkeletonSmoothing = 0.5f;
		gCinderOpenNISkeleton->mUserGenerator.GetSkeletonCap().StartTracking(nId);
		gCinderOpenNISkeleton->mUserGenerator.GetSkeletonCap().SetSmoothing( mSkeletonSmoothing );
	}
	else
	{
		// Calibration failed
		app::console() << "Calibration failed for user " << nId << endl;
		if (gCinderOpenNISkeleton->mNeedPose){
			gCinderOpenNISkeleton->mUserGenerator.GetPoseDetectionCap().StartPoseDetection(gCinderOpenNISkeleton->mStrPose, nId);
		}
		else
		{
			gCinderOpenNISkeleton->mUserGenerator.GetSkeletonCap().RequestCalibration(nId, TRUE);
		}
	}
}

#pragma mark Accessors
void CinderOpenNISkeleton::setDimensions( ci::Vec2i aDimensions ) {
	dimensions = aDimensions;
}
ci::Vec2i CinderOpenNISkeleton::getDimensions() {
	return dimensions;
}

Surface8u CinderOpenNISkeleton::getDepthSurface() {

	return mDepthSurface;
}

void CinderOpenNISkeleton::setDepthSurface()
{
	int texWidth =  gCinderOpenNISkeleton->mDepthMD.XRes();
	int texHeight = gCinderOpenNISkeleton->mDepthMD.YRes();

	const XnDepthPixel* pDepth = gCinderOpenNISkeleton->mDepthMD.Data();
	const XnLabel* pLabels = gCinderOpenNISkeleton->mSceneMD.Data();
	bool hasSceneData = pLabels != 0;

	// Calculate the accumulative histogram -  whatever that means
	memset(gCinderOpenNISkeleton->pDepthHist, 0, MAX_DEPTH*sizeof(float));

	unsigned int nX = 0;
	unsigned int nValue = 0;
	unsigned int nIndex = 0;
	unsigned int nY = 0;
	unsigned int nNumberOfPoints = 0;
	unsigned int nHistValue = 0;

	// First call
	if(!gCinderOpenNISkeleton->bInitialized)
	{
		app::console() << "Initialised Buffer" <<endl;
		gCinderOpenNISkeleton->bInitialized = true;

		gCinderOpenNISkeleton->pDepthTexBuf = new unsigned char[texWidth*texHeight*3];
		gCinderOpenNISkeleton->mDepthSurface = Surface8u( texWidth, texHeight, false ); // width, height, alpha?

		CinderOpenNISkeleton::setDimensions( ci::Vec2i(texWidth, texHeight) );
	}


	for (nY=0; nY<texHeight; nY++)
	{
		for (nX=0; nX<texWidth; nX++)
		{
			nValue = *pDepth;

			if (nValue != 0)
			{
				gCinderOpenNISkeleton->pDepthHist[nValue]++;
				nNumberOfPoints++;
			}

			pDepth++;
		}
	}

	for (nIndex=1; nIndex<MAX_DEPTH; nIndex++)
	{
		gCinderOpenNISkeleton->pDepthHist[nIndex] += gCinderOpenNISkeleton->pDepthHist[nIndex-1];
	}
	if (nNumberOfPoints)
	{
		for (nIndex=1; nIndex<MAX_DEPTH; nIndex++)
		{
			gCinderOpenNISkeleton->pDepthHist[nIndex] =
			(unsigned int)(256 * (1.0f - (gCinderOpenNISkeleton->pDepthHist[nIndex] / nNumberOfPoints)));
		}
	}

	pDepth = gCinderOpenNISkeleton->mDepthMD.Data();

	Area area( 0, 0, texWidth, texHeight );

	Surface::Iter iter = gCinderOpenNISkeleton->mDepthSurface.getIter( area );
	while( iter.line() ) {
		while( iter.pixel() ) {
			iter.r() = 0;
			iter.g() = 0;
			iter.b() = 0;

			if ( hasSceneData && *pLabels != 0) // Buh?
			{
				nValue = *pDepth;
				XnLabel label = *pLabels;
				XnUInt32 nColorID = label % nColors;
				if (label == 0)
				{
					nColorID = nColors;
				}

				if (nValue != 0)
				{
					nHistValue = gCinderOpenNISkeleton->pDepthHist[nValue];

					iter.r() = nHistValue * Colors[nColorID][0];
					iter.g() = nHistValue * Colors[nColorID][1];
					iter.b() = nHistValue * Colors[nColorID][2];
				}
			}
			else {
				nValue = *pDepth;
				if (nValue != 0) {
					nHistValue = gCinderOpenNISkeleton->pDepthHist[nValue];
					iter.r() = nHistValue;
					iter.g() = nHistValue;
					iter.b() = nHistValue;
				}
			}

			pDepth++;
			pLabels++;

		}
	}
}

#pragma mark Controls
//void CinderOpenNI::seekFrame(int nDiff)
//{
//	XnStatus nRetVal = XN_STATUS_OK;
//	if (isPlayerOn())
//	{
//		const XnChar* strNodeName = NULL;
//		if (bInitialized)
//		{
//			strNodeName = g_pPrimary->GetName();
//		}
//		else if (g_Depth.IsValid())
//		{
//			strNodeName = g_Depth.GetName();
//		}
//		else if (g_Image.IsValid())
//		{
//			strNodeName = g_Image.GetName();
//		}
//		else if (g_IR.IsValid())
//		{
//			strNodeName = g_IR.GetName();
//		}
//		else if (g_Audio.IsValid())
//		{
//			strNodeName = g_Audio.GetName();
//		}
//
//
//		displayMessage("Seeked %s to frame %u/%u", strNodeName, nFrame, nNumFrames);
//	}
//}

#pragma mark Debug functions
void CinderOpenNISkeleton::debugOutputNodeTypes()
{
	std::string nodeTypes[13];
	nodeTypes[XN_NODE_TYPE_DEVICE] = "XN_NODE_TYPE_DEVICE";
	nodeTypes[XN_NODE_TYPE_DEPTH] = "XN_NODE_TYPE_DEPTH";
	nodeTypes[XN_NODE_TYPE_IMAGE] = "XN_NODE_TYPE_IMAGE";
	nodeTypes[XN_NODE_TYPE_AUDIO] = "XN_NODE_TYPE_AUDIO";
	nodeTypes[XN_NODE_TYPE_IR] = "XN_NODE_TYPE_IR";
	nodeTypes[XN_NODE_TYPE_USER] = "XN_NODE_TYPE_USER";
	nodeTypes[XN_NODE_TYPE_RECORDER] = "XN_NODE_TYPE_RECORDER";
	nodeTypes[XN_NODE_TYPE_PLAYER] = "XN_NODE_TYPE_PLAYER";
	nodeTypes[XN_NODE_TYPE_GESTURE] = "XN_NODE_TYPE_GESTURE";
	nodeTypes[XN_NODE_TYPE_SCENE] = "XN_NODE_TYPE_SCENE";
	nodeTypes[XN_NODE_TYPE_HANDS] = "XN_NODE_TYPE_HANDS";
	nodeTypes[XN_NODE_TYPE_CODEC] = "XN_NODE_TYPE_CODEC";

	// Find about plugged-in devices
	xn::NodeInfoList list;
	XnStatus nRetVal = XN_STATUS_OK;
	nRetVal = gCinderOpenNISkeleton->mContext.EnumerateExistingNodes( list );
	if( nRetVal == XN_STATUS_OK )
	{
		for (xn::NodeInfoList::Iterator it = list.Begin(); it != list.End(); ++it)
		{
			std::stringstream ss;
			std::cout << "[Oneday] - Found node type: " << nodeTypes[ (*it).GetDescription().Type ] << std::endl;

		}
	}
}

/* Drawing methods */
void CinderOpenNISkeleton::debugDrawLabels( Font font, ci::Rectf depthArea )
{

	// Apply scale based on window size, vs dimensions of depth image ( will be 320 or 640 )
	ci::Vec2i inputSize = CinderOpenNISkeleton::getDimensions();
	ci::Vec3f depthMapScale = ci::Vec3f( depthArea.getWidth() / inputSize.x, depthArea.getHeight() / inputSize.y, 0.5f );

	for (int i = 0; i < maxUsers; ++i)
	{
		// Do the labels first
		XnPoint3D userCenterOfMass;
		mUserGenerator.GetCoM(currentUsers[i], userCenterOfMass);
		mDepthGenerator.ConvertRealWorldToProjective(1, &userCenterOfMass, &userCenterOfMass);

		// Scale and place based on center of 'depthArea' rectangle
		ci::Vec2i labelPosition = ci::Vec2i(depthArea.getX1() + depthMapScale.x*userCenterOfMass.X, depthArea.getY1() + depthMapScale.y*userCenterOfMass.Y);

		glColor4f(1.0f-Colors[i%nColors][0],
				  1.0f-Colors[i%nColors][1],
				  1.0f-Colors[i%nColors][2], 1);


		if ( mUserGenerator.GetSkeletonCap().IsTracking(currentUsers[i]) ) { // Is being tracked
			gl::drawString("Tracking", labelPosition, Color::white(), font );
		} else if ( mUserGenerator.GetSkeletonCap().IsCalibrating(currentUsers[i]) ) { // Is calibrating
			gl::drawString("Calibrating", labelPosition, Color::white(), font );
		} else { // Still waiting for intial pose
			gl::drawString("Waiting for Pose", labelPosition, Color::white(), font );
		}
	}
}
/* Drawing methods */
void CinderOpenNISkeleton::debugDrawSkeleton(Font font, ci::Rectf depthArea)
{
	static bool isTracking = false;
	for (int i = 0; i < maxUsers; ++i)
	{
		bool wasTracking = isTracking;
		if( mUserGenerator.GetSkeletonCap().IsTracking(currentUsers[i]))
		{
			// Was not tracking last frame
			if(isTracking == false) {
				app::console() << "Is Tracking User" << endl;
				isTracking = true;
			}


			glLineWidth(2.0);
			glBegin(GL_LINES);
			glColor4f(1-Colors[currentUsers[i]%nColors][0],
					  1-Colors[currentUsers[i]%nColors][1],
					  1-Colors[currentUsers[i]%nColors][2], 1);


			// HEAD TO NECK
			drawLimbDebug(currentUsers[i], XN_SKEL_HEAD, XN_SKEL_NECK);

			// Left Arm
			drawLimbDebug(currentUsers[i], XN_SKEL_NECK, XN_SKEL_LEFT_SHOULDER);
			drawLimbDebug(currentUsers[i], XN_SKEL_LEFT_SHOULDER, XN_SKEL_LEFT_ELBOW);
			drawLimbDebug(currentUsers[i], XN_SKEL_LEFT_ELBOW, XN_SKEL_LEFT_HAND);
			drawLimbDebug(currentUsers[i], XN_SKEL_LEFT_HAND, XN_SKEL_LEFT_FINGERTIP);

			// RIGHT ARM
			drawLimbDebug(currentUsers[i], XN_SKEL_NECK, XN_SKEL_RIGHT_SHOULDER);
			drawLimbDebug(currentUsers[i], XN_SKEL_RIGHT_SHOULDER, XN_SKEL_RIGHT_ELBOW);
			drawLimbDebug(currentUsers[i], XN_SKEL_RIGHT_ELBOW, XN_SKEL_RIGHT_HAND);
			drawLimbDebug(currentUsers[i], XN_SKEL_RIGHT_HAND, XN_SKEL_RIGHT_FINGERTIP);
			// TORSO
			drawLimbDebug(currentUsers[i], XN_SKEL_LEFT_SHOULDER, XN_SKEL_TORSO);
			drawLimbDebug(currentUsers[i], XN_SKEL_RIGHT_SHOULDER, XN_SKEL_TORSO);

			// LEFT LEG
			drawLimbDebug(currentUsers[i], XN_SKEL_TORSO, XN_SKEL_LEFT_HIP);
			drawLimbDebug(currentUsers[i], XN_SKEL_LEFT_HIP, XN_SKEL_LEFT_KNEE);
			drawLimbDebug(currentUsers[i], XN_SKEL_LEFT_KNEE, XN_SKEL_LEFT_FOOT);

			// RIGHT LEG
			drawLimbDebug(currentUsers[i], XN_SKEL_TORSO, XN_SKEL_RIGHT_HIP);
			drawLimbDebug(currentUsers[i], XN_SKEL_RIGHT_HIP, XN_SKEL_RIGHT_KNEE);
			drawLimbDebug(currentUsers[i], XN_SKEL_RIGHT_KNEE, XN_SKEL_RIGHT_FOOT);
			// PELVIS
			drawLimbDebug(currentUsers[i]	, XN_SKEL_LEFT_HIP, XN_SKEL_RIGHT_HIP);


			glEnd();
		}
	}

	gl::color(ColorA(1,1,1,1));
}

ci::Vec3f CinderOpenNISkeleton::getUserJointRealWorld( XnUserID playerID, XnSkeletonJoint jointID)
{
	if (!gCinderOpenNISkeleton->mUserGenerator.GetSkeletonCap().IsTracking(playerID))
	{
		app::console() << "(Oneday)::getUserJointRealWorld - Not Tracked!" << std::endl;
		return ci::Vec3f::zero();
	}

	// Retrieve joint informaiton
	XnSkeletonJointPosition joint;
	gCinderOpenNISkeleton->mUserGenerator.GetSkeletonCap().GetSkeletonJointPosition(playerID, jointID, joint);

	XnPoint3D point3D[1];
	point3D[0] = joint.position;

	gCinderOpenNISkeleton->mDepthGenerator.ConvertRealWorldToProjective(1, point3D, point3D);
	//	glVertex3i(pt[0].X, pt[0].Y, 0);
	//	glVertex3i(pt[1].X, pt[1].Y, 0);

    // TODO: UPDATE TO ADD WORLD OFFSET
	// Convert and return Vec3f
	return ci::Vec3f( point3D[0].X, point3D[0].Y, point3D[0].Z );
}

XnStatus CinderOpenNISkeleton::getUserJointInformation(XnUserID playerID, XnPoint3D jointPositions[], bool invertY) const
{
	if (!gCinderOpenNISkeleton->mUserGenerator.GetSkeletonCap().IsTracking(playerID)){
		app::console() << "not tracked" << endl;
		return XN_STATUS_NO_MATCH;
	}

	// NOTE:
	//		XN_SKEL_HEAD = 1
	//		XN_SKEL_RIGHT_FOOT = 24
	//		We use those to loop through all the points since they are first and last
	int totalJoints = XN_SKEL_RIGHT_FOOT+1;

	// Since their indexing starts at one, place empty object at first joint slot
	XnPoint3D firstJoint;
	firstJoint.X = firstJoint.Y = firstJoint.Z = 1;
	jointPositions[0] = firstJoint;

	// Loop through all joints 1-24 and fill data
	for (int i = XN_SKEL_HEAD; i < XN_SKEL_RIGHT_FOOT; i++)
	{
		XnSkeletonJointPosition currentJointPosition;
		gCinderOpenNISkeleton->mUserGenerator.GetSkeletonCap().GetSkeletonJointPosition(playerID, (XnSkeletonJoint)i /* current joint - just an interger constant */, currentJointPosition);

		// Place into array
		jointPositions[i] = currentJointPosition.position;
	}

	// convert all points in the array, in place
	gCinderOpenNISkeleton->mDepthGenerator.ConvertRealWorldToProjective(totalJoints, jointPositions, jointPositions);

	// Apply scale based on window size, vs dimensions of depth image ( will be 320 or 640 )
	ci::Vec2i windowSize = ci::app::App::get()->getWindowSize();	// Dimensions of the application window
	ci::Vec2i inputSize = gCinderOpenNISkeleton->getDimensions();	// Dimensions of our image
	ci::Vec3f scale = ci::Vec3f( windowSize.x / inputSize.x, windowSize.y / inputSize.y, 0.5 );

	// Apply offset
	for(int i = 0; i < totalJoints; i++)
	{
		jointPositions[i].X += gCinderOpenNISkeleton->worldOffset.x;
		jointPositions[i].X *= scale.x;

		jointPositions[i].Y += gCinderOpenNISkeleton->worldOffset.y;
		jointPositions[i].Y *= scale.y;

		if(invertY)
			jointPositions[i].Y *= -1;

		jointPositions[i].Z += gCinderOpenNISkeleton->worldOffset.z;
		jointPositions[i].Z *= scale.z;
	}

	return XN_STATUS_OK;
}
void CinderOpenNISkeleton::drawLimbDebug(XnUserID player, XnSkeletonJoint eJoint1, XnSkeletonJoint eJoint2)
{
	if (!gCinderOpenNISkeleton->mUserGenerator.GetSkeletonCap().IsTracking(player)){
		app::console() << "not tracked" << endl;
		return;
	}

	XnSkeletonJointPosition joint1, joint2;
	gCinderOpenNISkeleton->mUserGenerator.GetSkeletonCap().GetSkeletonJointPosition(player, eJoint1, joint1);
	gCinderOpenNISkeleton->mUserGenerator.GetSkeletonCap().GetSkeletonJointPosition(player, eJoint2, joint2);

	// Not sure of joint confidence, just draw previous?
	if (joint1.fConfidence < gCinderOpenNISkeleton->mJointConfidence || joint2.fConfidence < gCinderOpenNISkeleton->mJointConfidence)
			return;

	// Retreive points
	XnPoint3D pt[2];
	pt[0] = joint1.position;
	pt[1] = joint2.position;
	gCinderOpenNISkeleton->mDepthGenerator.ConvertRealWorldToProjective(2, pt, pt);

    pt[0].X += gCinderOpenNISkeleton->worldOffset.x;
    pt[0].Y += gCinderOpenNISkeleton->worldOffset.y;
    pt[0].Z += gCinderOpenNISkeleton->worldOffset.z;
//
    pt[1].X += gCinderOpenNISkeleton->worldOffset.x;
    pt[1].Y += gCinderOpenNISkeleton->worldOffset.y;
    pt[1].Z += gCinderOpenNISkeleton->worldOffset.z;

	// Apply scale based on window size, vs dimensions of depth image ( will be 320 or 640 )
	ci::Vec2i windowSize = ci::app::App::get()->getWindowSize();	// Dimensions of the application window
	ci::Vec2i inputSize = getDimensions();	// Dimensions of our image
	ci::Vec3f scale = ci::Vec3f( windowSize.x / inputSize.x, windowSize.y / inputSize.y, 1.0f );
//	scale = ci::Vec3f(0.2, -0.2,0.2);

	// Draw
	glVertex3i(pt[0].X * scale.x, pt[0].Y * scale.y, pt[0].Z * scale.z);
	glVertex3i(pt[1].X * scale.x, pt[1].Y * scale.y, pt[1].Z * scale.z);
}
