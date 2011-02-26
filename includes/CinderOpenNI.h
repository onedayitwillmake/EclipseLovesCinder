/*
 * CinderOpenNI.h
 *
 *  Created on: Feb 26, 2011
 *      Author: onedayitwillmake
 */

#ifndef CINDEROPENNI_H_
#define CINDEROPENNI_H_

/*
					  __  .__              ________
  ______ ____   _____/  |_|__| ____   ____/   __   \
 /  ___// __ \_/ ___\   __\  |/  _ \ /    \____    /
 \___ \\  ___/\  \___|  | |  (  <_> )   |  \ /    /
 /____  >\___  >\___  >__| |__|\____/|___|  //____/  .co.uk
 \/     \/     \/                    \/

 THE GHOST IN THE CSH


 CinderOpenNI.h | Part of PhantomLimb | Created 18/01/2011

 Copyright (c) 2010 Benjamin Blundell, www.section9.co.uk
 *** Section9 ***
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Section9 nor the names of its contributors
 *       may be used to endorse or promote products derived from this software
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***********************************************************************/

/**
 ####  #####  ##### ####    ###  #   # ###### ###### ##     ##  #####  #     #      ########    ##    #  #  #####
 #   # #   #  ###   #   #  #####  ###    ##     ##   ##  #  ##    #    #     #     #   ##   #  #####  ###   ###
 ###  #   #  ##### ####   #   #   #   ######   ##   #########  #####  ##### ##### #   ##   #  #   #  #   # #####
 --
 Mario Gonzalez
 CinderOpenNIBarebones - https://github.com/onedayitwillmake/CinderOpenNIBarebones/blob/master/CinderOpenNI.cpp
 */
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

	Surface8u						getDepthSurface();
	ci::Vec3f						getUserJointRealWorld( XnUserID playerID, XnSkeletonJoint jointID);
	ci::Vec2i						getDimensions();

	void							shutDown();
	bool							setup();
	bool							setupFromXML(string path);
	XnStatus						setupCallbacks();
	void							update();
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
	XnChar							mStrPose[20];
	float							pDepthHist[MAX_DEPTH];
	float							mJointConfidence;


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
