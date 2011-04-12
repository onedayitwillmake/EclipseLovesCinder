/*
 * ZoaDebugFunctions.h
 * The place where commented out functions go to live
 *
 * TODO: Creating naming scheme for functions
 *
 * @author Mario Gonzalez
 * http://onedayitwillmake.com
 */

#ifndef ZOADEBUGFUNCTIONS_H_
#define ZOADEBUGFUNCTIONS_H_

#include "cinder/TriMesh.h"
#include "cinder/Camera.h"
#include "cinder/gl/gl.h"
#include "cinder/Vector.h"

class ZoaDebugFunctions
{
public:
	// Draw arrows projecting from each normal in a TriMesh
	static void trimeshDrawNormals( ci::TriMesh &mesh );

	// Draw a billboard sprite relative to given a camera
	static void cameraDrawBillboard( const ci::CameraPersp &camera, ci::Vec2f position );

	// Draw crude quad representing floor plane
	static void drawFloorPlane( float floorSize );

	// Modifies the 4 passed vertices to create a quad of 'size' dimensions
	static void createQuadAtPosition( ci::Vec3f position,
			ci::Vec3f& v1, ci::Vec3f& v2, ci::Vec3f& v3, ci::Vec3f& v4,
			float size, float noise, float rotationY );

	// Add a quad to a mesh. ( Calculates normals, creates then references indices )
	static void addQuadToMesh( ci::TriMesh& mesh, const ci::Vec3f& P0, const ci::Vec3f& P1, const ci::Vec3f& P2, const ci::Vec3f& P3, const ci::ColorA& color );

	// Creates a plane of 'width' with 'sW', 'sH' segements at 'offset' and places geometry into 'mesh'
	static void createPlane( ci::TriMesh& mesh, ci::Vec3f offset, float width, float height, int segmentsW, int segmentsH, float noiseScale );

	// Removes all normals if exist, and re-calculates all vertex normals for a mesh by averaging out in triangles the vertex is used - from sansumbrella
	static void calculateTriMeshNormals( ci::TriMesh &mesh );
};

#endif /* ZOADEBUGFUNCTIONS_H_ */
