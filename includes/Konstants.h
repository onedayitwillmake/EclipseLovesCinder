/*
 *  Konstants.h
 *  CinderRibbons
 *
 *  Created by Mario Gonzalez on 2/7/11.
 *  Copyright 2011 Ogilvy & Mather. All rights reserved.
 *
 */

// Compiler flags
#define __ENABLE_KINECT 1
#define __DRAW_IMAGE_SOURCE_REF 0
#define __ENABLE_DRAW_KINECT 1
#define __ENABLE_DRAW_RIBBON 1
#define __ENABLE_DEBUG_LIGHT 0

// App info
#define APP_INITIAL_WIDTH 1024.0f
#define APP_INITIAL_HEIGHT 768.0f

// SHORTHAND
#define RED 0
#define GREEN 1
#define BLUE 2
#define ALPHA 3
// Macros
#define traceBone(__U__, __B__) printf("%d: (%f,%f,%f) [%f]\n", __U__, __B__.position.X, __B__.position.Y, __B__.position.Z, __B__.fConfidence)
#define traceVec3f(__X__) printf("Vec3f(%f,%f,%f)\n", __X__.x, __X__.y, __X__.z)
#define __IS_NAN(__X__) (__X__.x != __X__.x || __X__.y != __X__.y || __X__.z != __X__.z)
#define SIMPLEGUI CinderRibbons::getSharedSimpleGUI()
