/**
 * EclipseLovesCinder example application
 *
 **********************************************************
 **********************************************************
 ********************* IMPORTANT **************************
 * On first run:
 * 	- Select Project -> Clean...
 * 	- Then press the 'bug' icon to build. When asked select the top option of the three GDB debugger types
 **********************************************************
 **********************************************************
 *
 * This project is released under public domain, do whatever with it.
 *
 *
 * Mario Gonzalez
 * http://onedayitwillmake
 */

#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/ImageIo.h"
#include "cinder/Vector.h"
#include "cinder/app/MouseEvent.h"
#include "cinder/Rand.h"
#include "Resources.h"

class HelloWorldApp : public ci::app::AppBasic {
public:
	void setup();
	void prepareSettings( ci::app::AppBasic::Settings *settings );
	void update();
	void draw();
	void shutdown();

	void mouseDown( ci::app::MouseEvent event );
	void mouseMove( ci::app::MouseEvent event );
	void mouseDrag( ci::app::MouseEvent event );
	void mouseUp( ci::app::MouseEvent event );
	void resize( ci::app::ResizeEvent event );

	void keyDown( ci::app::KeyEvent event );

	ci::gl::Texture texture;
};

void HelloWorldApp::prepareSettings( ci::app::AppBasic::Settings *settings ) {
	settings->setWindowSize( 800, 600 );
}

void HelloWorldApp::setup() {
	// Hack to force our app window to show up above other windows on debug launch
	setAlwaysOnTop( true );

	// Test loading an image
	std::string path = ci::app::App::get()->getResourcePath().string() + "/" + "wheel.png";
	texture = ci::gl::Texture( ci::loadImage( path ) );
}


void HelloWorldApp::mouseDown( ci::app::MouseEvent event ) {
}

void HelloWorldApp::mouseDrag( ci::app::MouseEvent event ) {
}

void HelloWorldApp::mouseMove( ci::app::MouseEvent event ) {
}

void HelloWorldApp::mouseUp( ci::app::MouseEvent event ) {
}

void HelloWorldApp::resize( ci::app::ResizeEvent event ) {
	ci::gl::setMatricesWindow( event.getSize() );
}

void HelloWorldApp::keyDown( ci::app::KeyEvent event ) {
	if( event.getChar() == ci::app::KeyEvent::KEY_q )
		quit();
}

// All logic updates here
// Called every frame
void HelloWorldApp::update() {
	// This is a work hacky work around, to force our application to be above other windows when launched
	static bool hasBecomeFirstResponder = false;
	if( !hasBecomeFirstResponder && getElapsedSeconds() > 2 ) { // After 2 seconds, resume normal behavior
		hasBecomeFirstResponder = true;
		setAlwaysOnTop( false );
	}
}

// Only drawing here, do not place logic during drawing or it will slow down OpenGL
// Called every frame
void HelloWorldApp::draw() {

	// Clear screen every frame with black
	ci::gl::clear( ci::Color( 0, 0, 0 ) );

	// Create a random color based on the number of frames passed
	ci::Color aColor = ci::Color( 0, 0, 0 );
	aColor.r = fabs( cosf(getElapsedFrames() * 0.008) );
	aColor.g = fabs( sinf(getElapsedFrames() * 0.01) );
	aColor.b = (float) getMousePos().x / getWindowWidth();

	// Set the color and draw a simple line to the mouse
	ci::gl::color( aColor );
	ci::gl::drawLine( ci::Vec2f(getMousePos()), ci::Vec2f( getWindowCenter() ) );


	// Test drawing a texture...
	// Always check if texture loaded before drawing to avoid race condition on app start
	if ( texture ) {
		ci::gl::color( ci::ColorA(1.0f, 1.0f, 1.0f, 1.0f) );
		ci::gl::draw( texture, getWindowCenter() );
	}

}

void HelloWorldApp::shutdown() {
	std::cout << "Shutdown..." << std::endl;
	AppBasic::shutdown();
}


CINDER_APP_BASIC( HelloWorldApp, ci::app::RendererGl )
