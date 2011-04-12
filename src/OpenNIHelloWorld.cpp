/**
 ####  #####  ##### ####    ###  #   # ###### ###### ##     ##  #####  #     #      ########    ##    #  #  #####
 #   # #   #  ###   #   #  #####  ###    ##     ##   ##  #  ##    #    #     #     #   ##   #  #####  ###   ###
 ###  #   #  ##### ####   #   #   #   ######   ##   #########  #####  ##### ##### #   ##   #  #   #  #   # #####
 --
 Mario Gonzalez
 CinderOpenNIBarebones - https://github.com/onedayitwillmake/CinderOpenNIBarebones/blob/master/CinderOpenNI.cpp
 */
// STL
#include <sstream>
// OpenNI
#include <ni/XnFPSCalculator.h>
// Cinder
#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/Vector.h"
#include "cinder/Rect.h"
#include "cinder/Thread.h"
#include "cinder/MayaCamUI.h"
#include "blocks/SimpleGUI/include/SimpleGUI.h"
#include "cinder/TriMesh.h"
#include "ZoaDebugFunctions.h"
// App
#include "Konstants.h"

// OPEN NI
#if __ENABLE_KINECT
	#include <XnFPSCalculator.h>
	#include "CinderOpenNI.h"
#endif

using namespace ci;
using namespace ci::app;
using namespace std;

//mowa::sgui::SimpleGUI *GLOBAL_GUI;

class OpenNIHelloWorld : public AppBasic
{
public:
	void	prepareSettings( Settings* settings );
	void	setup();
	void	setupCamera();
	void	setupGui();
	void 	resize( ci::app::ResizeEvent event );
	void 	shutdown();
	void	mouseDown( ci::app::MouseEvent event );
	void	mouseMove( ci::app::MouseEvent event );
	void	mouseDrag( ci::app::MouseEvent event );
	void	mouseUp( ci::app::MouseEvent event );

	// Update
	void	update();

	// Draw
	void	draw();
	void	drawKinectDepth();
	void	drawKinectBones();
	void	drawFPS();

	// Helper functions
	ci::Rectf	getKinectDepthArea();

	// Camera
	MayaCamUI				mMayaCam;
	ci::TriMesh				*_floorPlane;

	// GUI
	mowa::sgui::SimpleGUI	*GUI;
};

#define __ENABLE_USE_RECORDING
void OpenNIHelloWorld::prepareSettings( Settings* settings )
{
	settings->setWindowSize( APP_INITIAL_WIDTH, APP_INITIAL_HEIGHT );
}
void OpenNIHelloWorld::shutdown()
{
	CINDERSKELETON->shutdown();
	AppBasic::shutdown();
}

void OpenNIHelloWorld::setup()
{
	// For now we have to manually change to the application path. Bug?
	chdir( getAppPath().c_str() );

    int     maxPathLenth = 255;
    char    temp[maxPathLenth];
    std::string cwd = ( getcwd(temp, maxPathLenth) ? std::string( temp ) : std::string("") );

    std::cout << "CurrentWorkingDirectory is:" << cwd << std::endl;
    std::cout << "AppPath: " << this->getAppPath() << std::endl;
	bool useRecording = false;

	XnStatus nRetVal = XN_STATUS_OK;
	CinderOpenNISkeleton *skeleton = CINDERSKELETON;

	// shared setup
	skeleton->setup( );


	if(useRecording) {
		nRetVal = skeleton->mContext.OpenFileRecording("/SkeletonRec.oni");
		// File opened
		CHECK_RC(nRetVal, "B-Open File Recording", true);

		// Get recording 'player'
		nRetVal = skeleton->mContext.FindExistingNode(XN_NODE_TYPE_PLAYER, skeleton->mPlayer);
		CHECK_RC(nRetVal, "Find player generator", true);
	} else {
		skeleton->setupFromXML( "Contents/Resources/configIR.xml" );
	}

	// Output device production nodes (user, depth, etc)
	skeleton->debugOutputNodeTypes();

	// Find depth generator
	nRetVal = skeleton->mContext.FindExistingNode(XN_NODE_TYPE_DEPTH, skeleton->mDepthGenerator);
	CHECK_RC(nRetVal, "Find depth generator", true);


	// Find skeleton / user generator
	nRetVal = skeleton->mContext.FindExistingNode(XN_NODE_TYPE_USER, skeleton->mUserGenerator);
	if (nRetVal != XN_STATUS_OK) {
		// Create one
		nRetVal = skeleton->mUserGenerator.Create(skeleton->mContext);
		CHECK_RC(nRetVal, "Find user generator", false);
	}

	// Check if user generator can detect skeleton
	if (!skeleton->mUserGenerator.IsCapabilitySupported(XN_CAPABILITY_SKELETON)) {
		app::console() << "Supplied user generator doesn't support skeleton\n" << endl;
	}

	// Register callbacks
	nRetVal = skeleton->setupCallbacks();

	// Start generating
	nRetVal = skeleton->mContext.StartGeneratingAll();
	CHECK_RC(nRetVal, "StartGenerating", true);

	skeleton->shouldStartUpdating();
	getFocus();
	setupGui();

	_floorPlane = new ci::TriMesh();
	_floorPlane->clear();
	ZoaDebugFunctions::createPlane( *_floorPlane, ci::Vec3f(0, -15, 0), 4000.0f, 4000.0f, 8, 8, 40 );
}

void OpenNIHelloWorld::setupGui()
{
	//	Ranges
	float	cameraRange = 10000;
	float	quaternionRange = 0.999;
	float	lightDirectionRange = M_PI*2;


	// create
	GUI = new mowa::sgui::SimpleGUI(this);

	// Intialiaze
	GUI->textColor = ColorA(1,1,1,1);
	GUI->lightColor = ColorA(1, 0, 1, 1);
	GUI->darkColor = ColorA(0.05,0.05,0.05, 1);
	GUI->bgColor = ColorA(0.15, 0.15, 0.15, 1.0);
	GUI->addColumn();
//
//	GUI->addSeparator();

	//	CINDERSKELETON->setupGUI();
	float tRange = 8000;		// Range of translate calls

//	GUI->addColumn();		// Place in own column
	GUI->addLabel("OpenNI");
	GUI->addParam("translateX", &CINDERSKELETON->worldOffset.x, -tRange, tRange, CINDERSKELETON->worldOffset.x);
	GUI->addParam("translateY", &CINDERSKELETON->worldOffset.y, -tRange, tRange, CINDERSKELETON->worldOffset.y);
	GUI->addParam("translateZ", &CINDERSKELETON->worldOffset.z, -tRange, tRange, CINDERSKELETON->worldOffset.z);
}

void OpenNIHelloWorld::setupCamera()
{
	// Camera perspective properties
	float cameraFOV			= 60.0f;
	float cameraNear		= 1.0f;
	float cameraFar			= FLT_MAX;

	ci::Vec3f p = ci::Vec3f::one() * 2000.0f;// Start off this far away from the center
	ci::CameraPersp cam = ci::CameraPersp( getWindowWidth(), getWindowHeight(), cameraFOV );

	cam.setWorldUp( ci::Vec3f(0, 1, 0) );
	cam.setEyePoint( ci::Vec3f(0, 0, 0 ) );
	cam.setCenterOfInterestPoint( ci::Vec3f::zero() );
	cam.setPerspective( cameraFOV, getWindowAspectRatio(), cameraNear, cameraFar );
	cam.setViewDirection( ci::Vec3f(0, 0, 1 ) );

	// Set mayacamera
	mMayaCam.setCurrentCam( cam );
}

void OpenNIHelloWorld::mouseDown( ci::app::MouseEvent event )
{
//	CINDERSKELETON->shouldStartUpdating();

	mMayaCam.mouseDown( event.getPos() );
}

void OpenNIHelloWorld::resize( ci::app::ResizeEvent event )
{
	ci::CameraPersp cam = mMayaCam.getCamera();
	cam.setPerspective( 60,  event.getAspectRatio(), 1, 6500);
	mMayaCam.setCurrentCam( cam );
}

void OpenNIHelloWorld::mouseDrag( ci::app::MouseEvent event )
{
//	std::cout << event.isMetaDown() << std::endl;
	mMayaCam.mouseDrag( event.getPos(), event.isLeftDown(), event.isMetaDown(), event.isRightDown() );
}

void OpenNIHelloWorld::mouseMove( ci::app::MouseEvent event )
{

}

void OpenNIHelloWorld::mouseUp( ci::app::MouseEvent event )
{
	mMayaCam.mouseDown( event.getPos() );
}


void OpenNIHelloWorld::update()
{

}

void OpenNIHelloWorld::drawFPS()
{
	std::stringstream myString(stringstream::in | stringstream::out);
	myString << std::setprecision( 3 ) << getAverageFps();

	ci::Vec2f position = ci::Vec2f( 10, getWindowSize().y - 15);
	float	rectGray = 0.05f;
	float	fontGray = 0.75f;


	gl::color( Color( rectGray, rectGray, rectGray) );
	gl::drawSolidRect( ci::Rectf( 0, position.y-5, 50, position.y+20 ) );
	gl::drawString( myString.str(), position, ci::ColorA( fontGray, fontGray, fontGray, 1) );
}





void OpenNIHelloWorld::draw()
{
	// Clear window
	// clear out the window with black
	ci::gl::clear( ci::Color( 0, 0, 0 ), true );

//	// Draw2D
	gl::pushMatrices();
	gl::setMatricesWindow( getWindowSize() );
	// Disable depth buffer
	gl::disableDepthRead();
	gl::disableDepthWrite();
	// Enable alpha blending
	gl::enableAlphaBlending();

	// Draw2D components
	drawKinectDepth();
	drawFPS();

	// Reset alpha blending and enable the depth buffer then draw3D components
	gl::enableAlphaBlending();
	gl::enableDepthRead();
	gl::enableDepthWrite();
	gl::popMatrices();

	ci::gl::setMatrices( mMayaCam.getCamera() );


	// Reset color
	ci::gl::color(ColorA(1,1,1,1));

	// Set the camera
	gl::setMatrices( mMayaCam.getCamera() );

	CinderOpenNISkeleton *skeleton = CINDERSKELETON;
	skeleton->debugDrawSkeleton( Font( "Arial", 18 ), getKinectDepthArea() );

	// Draw a cube at the origin as a visual anchor
	gl::drawStrokedCube( ci::Vec3f::zero(), ci::Vec3f( 10.0f, 10.0f, 10.0f ) );

	ci::gl::color(ColorA(1,1,1,1));
	ci::gl::draw( *_floorPlane );
	GUI->draw();
}

void OpenNIHelloWorld::drawKinectDepth()
{
	CinderOpenNISkeleton *skeleton = CINDERSKELETON;

	// Get surface
	Surface8u depthSurface = skeleton->getDepthSurface();

	// Not ready yet
	if( depthSurface == NULL ) {
		return;
	}

	// Convert to texture
	ci::Rectf depthArea = getKinectDepthArea();
	gl::draw( gl::Texture( depthSurface ), depthArea );

	// Debug draw
	skeleton->debugDrawLabels( Font( "Arial", 10 ), depthArea );
}


// Returns the area where the kinect depth map is drawn
// This is used when drawing labels, to draw the labels at the relative location by scaling, then translating the values returned by the kinect
ci::Rectf OpenNIHelloWorld::getKinectDepthArea()
{
	int width = 160;
    int height = 120;
    int padding = 10;
    int y1 = getWindowSize().y - height;
    int y2 = y1 + height;

	return ci::Rectf( padding, y1 - padding, width + padding, y2 - padding );
}

#pragma mark Debug
CINDER_APP_BASIC( OpenNIHelloWorld, RendererGl )
