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
#include "cinder/MayaCamUI.h"
#include "blocks/SimpleGUI/include/SimpleGUI.h"

// App
#include "Konstants.h"
#include "OpenNIThreadRunner.cpp"
#include "CinderOpenNI.h"

using namespace ci;
using namespace ci::app;
using namespace std;

//mowa::sgui::SimpleGUI *GLOBAL_GUI;

class vcApp : public AppBasic
{
public:
	void	prepareSettings( Settings* settings );
	void	setup();
	void	setupCamera();
	void	mouseDown( ci::app::MouseEvent event );
	void	mouseMove( ci::app::MouseEvent event );
	void	mouseDrag( ci::app::MouseEvent event );
	void	mouseUp( ci::app::MouseEvent event );
	void	setupGui();

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

	// GUI
	mowa::sgui::SimpleGUI	*GUI;


private:
	OpenNIThreadRunner *openNIThread;
};

#define __ENABLE_USE_RECORDING
void vcApp::prepareSettings( Settings* settings )
{
	setWindowSize( APP_INITIAL_WIDTH, APP_INITIAL_HEIGHT );
}

void vcApp::setup()
{
    int     maxPathLenth = 255;
    char    temp[maxPathLenth];
    std::string cwd = ( getcwd(temp, maxPathLenth) ? std::string( temp ) : std::string("") );

    std::cout << "CurrentWorkingDirectory is:" << cwd << std::endl;

	bool useRecording = true;

	XnStatus nRetVal = XN_STATUS_OK;
	CinderOpenNISkeleton *skeleton = CINDERSKELETON;

	// shared setup
	skeleton->setup( );

	if(useRecording) {
		nRetVal = skeleton->mContext.OpenFileRecording("resources/SkeletonRec.oni");
		// File opened
		CHECK_RC(nRetVal, "Open File Recording", true);

		// Get recording 'player'
		nRetVal = skeleton->mContext.FindExistingNode(XN_NODE_TYPE_PLAYER, skeleton->mPlayer);
		CHECK_RC(nRetVal, "Find player generator", true);
	} else {
		skeleton->setupFromXML( "resources/configIR.xml" );
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

	openNIThread = new OpenNIThreadRunner();
	openNIThread->go();

	setupGui();
}

void vcApp::setupGui()
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
	float tRange = 4000;		// Range of translate calls

//	GUI->addColumn();		// Place in own column
	GUI->addLabel("OpenNI");
	GUI->addParam("translateX", &CINDERSKELETON->worldOffset.x, -tRange, tRange, CINDERSKELETON->worldOffset.x);
	GUI->addParam("translateY", &CINDERSKELETON->worldOffset.y, -tRange, tRange, CINDERSKELETON->worldOffset.y);
	GUI->addParam("translateZ", &CINDERSKELETON->worldOffset.z, -tRange, tRange, CINDERSKELETON->worldOffset.z);
}

void vcApp::setupCamera()
{
	// Camera perspective propertie
	float cameraFOV			= 65.0f;
	float cameraNear		= 1.0f;
	float cameraFar			= 50000000.0f;


	Vec3f p = Vec3f::one() * 2000.0f;// Start off this far away from the center
	CameraPersp cam = CameraPersp( getWindowWidth(), getWindowHeight(), cameraFOV );
	cam.setEyePoint( Vec3f::zero() );
	cam.setCenterOfInterestPoint( Vec3f::zero() );
	cam.setPerspective( cameraFOV, getWindowAspectRatio(), cameraNear, cameraFar );

	// Set mayacamera
	mMayaCam.setCurrentCam( cam );
}

void vcApp::mouseDown( ci::app::MouseEvent event )
{
	mMayaCam.mouseDown( event.getPos() );
}

void vcApp::mouseDrag( ci::app::MouseEvent event )
{
//	std::cout << event.isMetaDown() << std::endl;
	mMayaCam.mouseDrag( event.getPos(), event.isLeftDown(), event.isMetaDown(), event.isRightDown() );
}

void vcApp::mouseMove( ci::app::MouseEvent event )
{

}

void vcApp::mouseUp( ci::app::MouseEvent event )
{
	mMayaCam.mouseDown( event.getPos() );
}


void vcApp::update()
{
	CinderOpenNISkeleton *skeleton = CINDERSKELETON;
	XnStatus nRetVal = XN_STATUS_OK;
	xnFPSMarkFrame(&skeleton->xnFPS);

	static bool firstRun = true;
	if(firstRun) { // Try to seek
//		seekToFrame( 105 );  // Jump to right before hands go up
		firstRun = false;
	}

	skeleton->update();
}

void vcApp::draw()
{
	// Clear window
	gl::clear( Color( 0, 0, 0 ), true );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	// Draw2D
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
	gl::disableAlphaBlending();
	gl::enableDepthRead();
	gl::enableDepthWrite();
	gl::popMatrices();

	// Reset color
	gl::color(ColorA(1,1,1,1));

	// Set the camera
	gl::setMatrices( mMayaCam.getCamera() );

	gl::color(ColorA(1,1,1,1));

	CinderOpenNISkeleton *skeleton = CINDERSKELETON;
	skeleton->debugDrawSkeleton( Font( "Arial", 18 ), getKinectDepthArea() );

	// Draw a cube at the origin as a visual anchor
	gl::drawStrokedCube( ci::Vec3f::zero(), ci::Vec3f( 10.0f, 10.0f, 10.0f ) );

	GUI->draw();
}

void vcApp::drawKinectDepth()
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
ci::Rectf vcApp::getKinectDepthArea()
{
	int width = 160;
    int height = 120;
    int padding = 10;
    int y1 = getWindowSize().y - height;
    int y2 = y1 + height;

	return ci::Rectf( padding, y1 - padding, width + padding, y2 - padding );
}

#pragma mark Debug
void vcApp::drawFPS()
{
	std::stringstream myString(stringstream::in | stringstream::out);
	double fpsValue = xnFPSCalc(&CINDERSKELETON->xnFPS);
	myString << std::setprecision( 3 ) << fpsValue;

	ci::Vec2f position = ci::Vec2f( 10, getWindowSize().y - 15);
	float	rectGray = 0.05f;
	float	fontGray = 0.75f;

	gl::color( Color( rectGray, rectGray, rectGray) );
	gl::drawSolidRect( ci::Rectf( 0, position.y-5, 50, position.y+20 ) );
	gl::drawString( myString.str(), position, ci::ColorA( fontGray, fontGray, fontGray, 1) );
}





CINDER_APP_BASIC( vcApp, RendererGl )
