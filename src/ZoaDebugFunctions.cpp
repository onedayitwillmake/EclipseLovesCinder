/**
 * ZoaDebugFunctions.h
 * The place where commented out functions go to live
 *
 * @author Mario Gonzalez
 */
#include "ZoaDebugFunctions.h"
#include "cinder/Rand.h"

void ZoaDebugFunctions::trimeshDrawNormals( ci::TriMesh &mesh )
{
	std::vector<ci::Vec3f> meshVertices = mesh.getVertices();
	std::vector<ci::Vec3f> meshNormals = mesh.getNormals();
	for( size_t i = 3; i < meshNormals.size(); i+=4 )
	{
		float t = 0.5f; // Because T is 0.5 it's not really necessary but the we could project along the plane this way
		ci::Vec3f midPoint = ci::Vec3f( (1.0f - t) * ( meshVertices[i-2] ) + t * ( meshVertices[i] ) );
		ci::Vec3f normal = meshNormals[i]*10;
		ci::gl::drawVector( midPoint, midPoint+normal, 10, 2.5);
	}
}

//
void ZoaDebugFunctions::cameraDrawBillboard( const ci::CameraPersp &camera, ci::Vec2f position )
{
	ci::Vec3f mRight, mUp;
	camera.getBillboardVectors(&mRight, &mUp);
	ci::gl::drawBillboard( ci::Vec3f::zero(), position, 0.0f, mRight, mUp);
}

void ZoaDebugFunctions::drawFloorPlane( float floorSize )
{
	// Draw floor plane
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f,1.0f); glVertex3f(-floorSize, 0.0f, floorSize);
		glTexCoord2f(1.0f,1.0f); glVertex3f( floorSize, 0.0f, floorSize);
		glTexCoord2f(1.0f,0.0f); glVertex3f( floorSize, 0.0f,-floorSize);
		glTexCoord2f(0.0f,0.0f); glVertex3f(-floorSize, 0.0f,-floorSize);
	glEnd();
}

// Modifies the 4 passed vertices to create a quad of 'size' dimensions
void ZoaDebugFunctions::createQuadAtPosition( ci::Vec3f position,
		ci::Vec3f& v1, ci::Vec3f& v2, ci::Vec3f& v3, ci::Vec3f& v4,
		float size,
		float noise,
		float rotationY )
{
	#define quadNoise() (size + ci::Rand::randFloat(-noise, noise))

	v1 = ci::Vec3f::zero();
	v1.x -= quadNoise(), v1.y += quadNoise();
	v1.rotateY(rotationY);
	v1 += position;

	v2 = ci::Vec3f::zero();;
	v2.x += quadNoise(), v2.y += quadNoise();
	v2.rotateY(rotationY);
	v2 += position;

	v3 = ci::Vec3f::zero();;
	v3.x += quadNoise(), v3.y -= quadNoise();
	v3.rotateY(rotationY);
	v3 += position;

	v4 = ci::Vec3f::zero();;
	v4.x -= quadNoise(), v4.y -= quadNoise();
	v4.rotateY(rotationY);
	v4 += position;
}

// Creates a plane of 'width' with 'sW', 'sH' segements at 'offset' and places geometry into 'mesh'
void ZoaDebugFunctions::createPlane( ci::TriMesh& mesh, ci::Vec3f offset, float width, float height, int segmentsW, int segmentsH, float noiseScale = 0 )
{
	std::vector< std::vector<ci::Vec3f> > grid;

	// Create W*H vertices
	for(int i = 0; i <= segmentsW; ++i)
	{
		std::vector<ci::Vec3f> row;
		grid.push_back( row );
		for(int j = 0; j <= segmentsH; ++j)
		{
			ci::Vec3f pos = ci::Vec3f(((float)i / (float)segmentsW - 0.5f) * width, 0, ((float)j / (float)segmentsH - 0.5f) *  height);
			pos += offset;

			// Apply noise to the points if passed in
			if(noiseScale != 0) {
				pos += ci::Rand::randVec3f() * noiseScale;
			}

			grid[i].push_back( pos );
		}
	}

	// Use 2D array-ness to create a quad
	for(int i = 0; i < segmentsW; ++i) {
		for( int j = 0; j < segmentsH; ++j) {
			ci::Vec3f a = grid[i  ][j  ];
			ci::Vec3f b = grid[i+1][j  ];
			ci::Vec3f c = grid[i  ][j+1];
			ci::Vec3f d = grid[i+1][j+1];

			float gray =  ci::Rand::randFloat() * 0.75 + 0.25;
			ci::ColorA color = ci::ColorA(gray, gray, gray, 0.5);

//			std::cout << a << b << c << d << std::endl;

			// d, b, c, a
			addQuadToMesh( mesh, a, c, d, b, color );
		}
	}
}

void ZoaDebugFunctions::addQuadToMesh( ci::TriMesh& mesh, const ci::Vec3f& P0, const ci::Vec3f& P1, const ci::Vec3f& P2, const ci::Vec3f& P3, const ci::ColorA& color )
{
	ci::Vec3f e0 = P2 - P0;
	ci::Vec3f e1 = P2 - P1;
	ci::Vec3f n = e0.cross(e1).normalized();

	mesh.appendVertex( P0 );
	mesh.appendColorRGBA( color );
	mesh.appendNormal( n );

	mesh.appendVertex( P1 );
	mesh.appendColorRGBA( color );
	mesh.appendNormal( n );

	mesh.appendVertex( P2 );
	mesh.appendColorRGBA( color );
	mesh.appendNormal( n );

	mesh.appendVertex( P3 );
	mesh.appendColorRGBA( color );
	mesh.appendNormal( n );

	int vert0 = mesh.getNumVertices() - 4;
	int vert1 = mesh.getNumVertices() - 1;
	int vert2 = mesh.getNumVertices() - 2;
	int vert3 = mesh.getNumVertices() - 3;

	mesh.appendTriangle( vert0, vert3, vert1 );
	mesh.appendTriangle( vert3, vert2, vert1 );
}

void ZoaDebugFunctions::calculateTriMeshNormals( ci::TriMesh &mesh )
{
	const std::vector<ci::Vec3f>& vertices = mesh.getVertices();
	const std::vector<size_t>& indices = mesh.getIndices();

	// remove all current normals
	std::vector<ci::Vec3f>& normals = mesh.getNormals();
	normals.reserve( mesh.getNumVertices() );
	normals.clear();

	// set the normal for each vertex to (0, 0, 0)
	for(size_t i=0; i < mesh.getNumVertices(); ++i)
		normals.push_back( ci::Vec3f::zero() );

	// Average out the normal for each vertex at an index
	for(size_t i=0; i< mesh.getNumTriangles(); ++i)
	{
		ci::Vec3f v0 = vertices[ indices[i * 3] ];
		ci::Vec3f v1 = vertices[ indices[i * 3 + 1] ];
		ci::Vec3f v2 = vertices[ indices[i * 3 + 2] ];

		// calculate normal and normalize it, so each of the normals equally contributes to the final result
		ci::Vec3f e0 = v2 - v0;
		ci::Vec3f e1 = v2 - v1;
		ci::Vec3f n = e0.cross(e1).normalized();

		// add the normal to the final result, so we get an average of the normals of each triangle
		normals[ indices[i * 3] ] += n;
		normals[ indices[i * 3 + 1] ] += n;
		normals[ indices[i * 3 + 2] ] += n;
	}

	// the normals are probably not normalized by now, so make sure their lengths will be 1.0 as expected
	for(size_t i=0;i< normals.size();++i) {
		normals[i].normalize();
	}
}

