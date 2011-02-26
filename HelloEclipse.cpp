#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/Vector.h"
#include "cinder/app/MouseEvent.h"
#include "cinder/Rand.h"


class HelloWorldApp : public ci::app::AppBasic {
public:
	void setup();
	void mouseDown( ci::app::MouseEvent event );
	void update();
	void draw();
};

void HelloWorldApp::setup()
{
}

void HelloWorldApp::mouseDown( ci::app::MouseEvent event )
{
}

void HelloWorldApp::update()
{
}

void HelloWorldApp::draw()
{
	// clear out the window with black
	ci::Color aColor = ci::Color( 0, 0, 0 );
	aColor.r = fabs( cosf(getElapsedFrames() * 0.008) );
	aColor.g = fabs( sinf(getElapsedFrames() * 0.01) );
	aColor.b = 1.0f;

	ci::gl::clear( aColor );
}


CINDER_APP_BASIC( HelloWorldApp, ci::app::RendererGl )
