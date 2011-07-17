/**
 * EclipseLovesCinder example application
 *
 * * On first run, run Project -> Clean...
 * * If you change your project name go into debug configurations (arrow next to bug icon), and modify where the debug application will run from
 *
 * This project is released under public domain, do whatever with it.
 *
 *
 * Mario Gonzalez
 * http://onedayitwillmake
 */
#include "cinder/Camera.h"
#include "cinder/TriMesh.h"
#include "cinder/Rand.h"
#include "cinder/MayaCamUI.h"
#include "cinder/Surface.h"
#include "cinder/app/AppBasic.h"
#include "cinder/app/Renderer.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/Vbo.h"
#include <float.h>

class RibbonProjection : public ci::app::AppBasic {
public:
	void prepareSettings( ci::app::AppBasic::Settings *settings );
	void setup();
	void setupCamera();
	void setupQuadSprites();

	void	resize( ci::app::ResizeEvent event );
	void	mouseDown( ci::app::MouseEvent event );
	void	mouseMove( ci::app::MouseEvent event );
	void	mouseDrag( ci::app::MouseEvent event );
	void	mouseUp( ci::app::MouseEvent event );

	void update();
	void draw();
	void addQuad( ci::TriMesh& mesh, const ci::Vec3f& p1, const ci::Vec3f& p2, const ci::Vec3f& p3, const ci::Vec3f& p4, float minU, float maxU, float minV, float maxV );

	ci::gl::Texture		_texture;
	ci::MayaCamUI		_mayaCam;
	ci::TriMesh*		_particleMesh;
};

void RibbonProjection::prepareSettings( ci::app::AppBasic::Settings *settings )
{
	settings->setWindowSize( 800, 600 );
}

void RibbonProjection::setup()
{
	setupQuadSprites();
	getFocus();
}


/**
 * Creates a cube out of 6 quads
 * https://github.com/drojdjou/J3D/blob/master/src/engine/Primitives.js
 */
void RibbonProjection::setupQuadSprites()
{
	_particleMesh = new ci::TriMesh();
	_particleMesh->clear();

	float w = 5;
	float h = 5;
	float d = 5;

	addQuad(*_particleMesh, ci::Vec3f(-w, h, d),ci::Vec3f(w, h, d), ci::Vec3f(w, -h, d), ci::Vec3f(-w, -h, d), 0.0f, 1.0f, 0.0f, 1.0f);
	addQuad(*_particleMesh, ci::Vec3f(w, h, -d), ci::Vec3f(-w, h, -d), ci::Vec3f(-w, -h, -d), ci::Vec3f(w, -h, -d), 0.0f, 1.0f, 0.0f, 1.0f);

	addQuad(*_particleMesh, ci::Vec3f(-w, h, -d), ci::Vec3f(-w, h, d), ci::Vec3f(-w, -h, d), ci::Vec3f(-w, -h, -d), 0.0f, 1.0f, 0.0f, 1.0f);
	addQuad(*_particleMesh, ci::Vec3f(w, h, d), ci::Vec3f(w, h, -d), ci::Vec3f(w, -h, -d), ci::Vec3f(w, -h, d), 0.0f, 1.0f, 0.0f, 1.0f);

	addQuad(*_particleMesh, ci::Vec3f(w, h, d), ci::Vec3f(-w, h, d), ci::Vec3f(-w, h, -d), ci::Vec3f(w, h, -d), 0.0f, 1.0f, 0.0f, 1.0f);
	addQuad(*_particleMesh, ci::Vec3f(w, -h, d), ci::Vec3f(w, -h, -d), ci::Vec3f(-w, -h, -d), ci::Vec3f(-w, -h, d), 0.0f, 1.0f, 0.0f, 1.0f);
}

/**
 * Adds a single quad to a mesh
 * https://github.com/drojdjou/J3D/blob/master/src/engine/Primitives.js
 */
void RibbonProjection::addQuad( ci::TriMesh& mesh, const ci::Vec3f& p1, const ci::Vec3f& p2, const ci::Vec3f& p3, const ci::Vec3f& p4, float minU, float maxU, float minV, float maxV ) {
	const std::vector<ci::Vec3f>& vertices = mesh.getVertices();
	const std::vector<uint32_t>& indices = mesh.getIndices();

	ci::Vec3f n = (p1-p2).cross( p2-p3 ).normalized();
	std::cout << n << std::endl;
	uint32_t p = mesh.getNumVertices();

	float nu = minU;
	float xu = maxU;
	float nv = minV;
	float xv = maxV;

	mesh.appendVertex( p1 );
	mesh.appendVertex( p2 );
	mesh.appendVertex( p3 );
	mesh.appendVertex( p4 );

	mesh.appendNormal( n );
	mesh.appendNormal( n );
	mesh.appendNormal( n );
	mesh.appendNormal( n );

	ci::Vec3f normalColor = ci::Vec3f(n.x, n.y, n.z)*0.5;
	normalColor += 0.5;

	mesh.appendColorRGB( ci::Color(normalColor.x, normalColor.y, normalColor.z) );
	mesh.appendColorRGB( ci::Color(normalColor.x, normalColor.y, normalColor.z) );
	mesh.appendColorRGB( ci::Color(normalColor.x, normalColor.y, normalColor.z) );
	mesh.appendColorRGB( ci::Color(normalColor.x, normalColor.y, normalColor.z) );

	mesh.appendTexCoord( ci::Vec2f(nu, xv) );
	mesh.appendTexCoord( ci::Vec2f(xu, xv) );
	mesh.appendTexCoord( ci::Vec2f(xu, nv) );
	mesh.appendTexCoord( ci::Vec2f(nu, nv) );

	mesh.appendTriangle( p, p+1, p+2 );
	mesh.appendTriangle( p, p+2, p+3 );
}


void RibbonProjection::setupCamera()
{
	// Camera perspective properties
	float cameraFOV			= 60.0f;
	float cameraNear		= 1.0f;
	float cameraFar			= 10000;

	ci::Vec3f p = ci::Vec3f::one() * 2000.0f;// Start off this far away from the center
	ci::CameraPersp cam = ci::CameraPersp( getWindowWidth(), getWindowHeight(), cameraFOV );

	cam.setWorldUp( ci::Vec3f(0, 1, 0) );
	cam.setEyePoint( ci::Vec3f(0, 0, 0 ) );
	cam.setCenterOfInterestPoint( ci::Vec3f::zero() );
	cam.setPerspective( cameraFOV, getWindowAspectRatio(), cameraNear, cameraFar );
	cam.setViewDirection( ci::Vec3f(0, 0, 1 ) );

	// Set mayacamera
	_mayaCam.setCurrentCam( cam );
}


void RibbonProjection::mouseDown( ci::app::MouseEvent event )
{
	_mayaCam.mouseDown( event.getPos() );
}

void RibbonProjection::mouseDrag( ci::app::MouseEvent event )
{
	_mayaCam.mouseDrag( event.getPos(), event.isLeftDown(), event.isMetaDown(), event.isRightDown() );
}

void RibbonProjection::mouseMove( ci::app::MouseEvent event )
{
	_mayaCam.mouseDrag( event.getPos(), event.isLeftDown(), event.isMetaDown(), event.isRightDown() );
}

void RibbonProjection::mouseUp( ci::app::MouseEvent event )
{
	_mayaCam.mouseDown( event.getPos() );
}

void RibbonProjection::resize( ci::app::ResizeEvent event )
{
	ci::CameraPersp cam = _mayaCam.getCamera();
	cam.setPerspective( 60,  event.getAspectRatio(), 1, std::numeric_limits<int>::max());
	_mayaCam.setCurrentCam( cam );
}

void RibbonProjection::update() {

}

void RibbonProjection::draw()
{
	// clear out the window with black
	ci::gl::clear( ci::Color( 0, 0, 0 ) );
	ci::gl::enableDepthRead();
	ci::gl::setMatrices( _mayaCam.getCamera() );

	ci::gl::enableAlphaBlending();
//	ci::gl::enableWireframe();
	ci::gl::draw( *_particleMesh );
}

CINDER_APP_BASIC( RibbonProjection, ci::app::RendererGl )
