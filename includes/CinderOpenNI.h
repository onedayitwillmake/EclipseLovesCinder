/*
 * CinderOpenNI.h
 *
 *  Created on: Feb 26, 2011
 *      Author: onedayitwillmake
 */

#ifndef CINDEROPENNI_H_
#define CINDEROPENNI_H_

#pragma once

#include "cinder/app/App.h"
#include "cinder/gl/gl.h"
#include "cinder/Font.h"
#include "cinder/Vector.h"

#include <ni/XnOpenNI.h>
#include <ni/XnTypes.h>
#include <ni/XnCodecIDs.h>
#include <ni/XnCppWrapper.h>
#include <ni/XnFPSCalculator.h>

#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>


using namespace std;
using namespace ci;
using namespace cinder;

// I do so like singletons but I reckon there must be a better way?
#define CINDERSKELETON CinderOpenNISkeleton::getInstance()
#define MAX_DEPTH 10000

#define CHECK_RC(nRetVal, what, isFatal)							\
if (nRetVal != XN_STATUS_OK)										\
{																	\
printf("%s failed: %s\n", what, xnGetStatusString(nRetVal));	\
if(isFatal)														\
exit(-1);													\
}

class CinderOpenNISkeleton {

public:
	static CinderOpenNISkeleton*	getInstance();
	static void shutdown();
	~CinderOpenNISkeleton();

	Surface8u						getDepthSurface();
	ci::Vec3f						getUserJointRealWorld( XnUserID playerID, XnSkeletonJoint jointID);
	ci::Vec2i						getDimensions();

	bool							setup();
	bool							setupFromXML(string path);
	XnStatus						setupCallbacks();
	void							shouldStartUpdating();
	void							update();
	void							shouldStopUpdating();
	//static void						seekToFrame( int nDiff );


	// Pointers to OpenNI stuff
	XnFPSData						xnFPS;
	xn::Context						mContext;
	xn::DepthGenerator				mDepthGenerator;
	xn::UserGenerator				mUserGenerator;
	xn::SceneAnalyzer				mSceneAnalyzer;
	xn::MockDepthGenerator			mMockDepthGenerator;
	xn::Player						mPlayer;

	XnBool							mNeedPose;
	XnUInt32						_slot;
	XnBool							_isFirstCalibrationComplete;
	XnChar							mStrPose[20];
	float							pDepthHist[MAX_DEPTH];
	float							mJointConfidence;


	// Thread
    volatile bool 						_stopRequested;
    boost::shared_ptr<boost::thread> 	_thread;
	// Info
	ci::Vec2i						dimensions;
    ci::Vec3f                       worldOffset;
	// Debug functions
	void							setupGUI();
	void							debugOutputNodeTypes();
	void							debugDrawLabels( Font font, ci::Rectf depthArea );
	void							debugDrawSkeleton(Font font, ci::Rectf depthArea);


	//protected:
	CinderOpenNISkeleton();
	static				CinderOpenNISkeleton* gCinderOpenNISkeleton;

	void				setDepthSurface();
	void				setDimensions( ci::Vec2i aDimensions );
	void				drawLimbDebug(XnUserID player, XnSkeletonJoint eJoint1, XnSkeletonJoint eJoint2);
	XnStatus			getUserJointInformation(XnUserID playerID, XnPoint3D jointPositions[], bool invertY) const;

	unsigned char*		pDepthTexBuf;

	Surface8u			mDepthSurface;
	bool				bInitialized;

	xn::SceneMetaData	mSceneMD;
	xn::DepthMetaData	mDepthMD;

	XnUserID			currentUsers[15];
	XnUInt16			maxUsers;

#pragma mark Callback
	static void XN_CALLBACK_TYPE User_NewUser(xn::UserGenerator& generator, XnUserID nId, void* pCookie);
	static void XN_CALLBACK_TYPE UserCalibration_CalibrationEnd(xn::SkeletonCapability& capability, XnUserID nId, XnBool bSuccess, void* pCookie);
	static void XN_CALLBACK_TYPE UserCalibration_CalibrationStart(xn::SkeletonCapability& capability, XnUserID nId, void* pCookie);
	static void XN_CALLBACK_TYPE UserPose_PoseDetected(xn::PoseDetectionCapability& capability, const XnChar* strPose, XnUserID nId, void* pCookie);
	static void XN_CALLBACK_TYPE User_LostUser(xn::UserGenerator& generator, XnUserID nId, void* pCookie);
};

#endif /* CINDEROPENNI_H_ */
