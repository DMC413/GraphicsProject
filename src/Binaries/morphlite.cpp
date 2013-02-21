/**
   @file morph.cpp
   @author Nicholas St.Pierre
   @authors John Huston, Nicholas VerVoort, Chris Compton
   @date 2012-12-06
   @brief This is a derivative of our main project file, fly.cpp.
   @details This is a tech demo for morphing two objects back and forth.
   This is mostly based on ed angel's code from his book.
**/

#include "globals.h"
/* System Headers */
#include <cmath>
#include <cstdio>
#include <sstream>
#include <cstdlib>
#include <time.h>
/* Multi-platform support and OpenGL headers. */
#include "platform.h"
/* Ed Angel's Math Classes */
#include "vec.hpp"
#include "mat.hpp"
/* Utilities and Classes */
#include "model.hpp"
#include "Camera.hpp"
#include "InitShader.hpp"
#include "Cameras.hpp"
#include "Screen.hpp"
#include "Object.hpp"
#include "Timer.hpp"
#include "Scene.hpp"

#include "LightSource.hpp"
#include "Lights.hpp"


// Type Aliases
using Angel::vec3;
using Angel::vec4;
typedef Angel::vec4 color4;
typedef Angel::vec4 point4;

// Global objects for magical camera success
Screen myScreen( 800, 600 );
Scene theScene;
GLuint gShader;
bool fixed_yaw = true;

// Initialization: load and compile shaders, initialize camera(s), load models.
void init() {
  
  // Load shaders and use the resulting shader program. 
  gShader = Angel::InitShader( "shaders/vmorph.glsl", "shaders/fmorph.glsl" );

  // Let the other objects know which shader to use.
  theScene.SetShader( gShader );
  myScreen.camList.SetShader( gShader );

  // We start with no cameras, by default. Add one and set it "active" by using Next().
  myScreen.camList.AddCamera( "Camera1" );
  myScreen.camList.Next();

  // Create an object and add it to the scene with the name "bottle".
  Object *bottle = theScene.AddObject( "bottle" );

  // Use the object loader to actually fill out the vertices and-so-on of the bottle.
  loadModelFromFile( bottle, "../models/bottle-a.obj" );

  // Scale the bottle down!
  bottle->trans.scale.Set( 0.01 );

  // Buffer the object onto the GPU. This does not happen by default,
  // To allow you to make many changes and buffer only once,
  // or to buffer changes selectively.
  bottle->Buffer();

  // Object class has-a pointer to an object which is the morph target.
  // they are created and buffered as follows:

  // this makes a new object and links it to the source object. it returns the addr of the new obj..
  bottle->genMorphTarget( gShader ) ; 

  // we can get the addr of the morph object like this, also.
  Object *bottleMorphTarget = bottle->getMorphTargetPtr() ; 

  // with this model, we can use all the preexisting Object class functionality
  loadModelFromFile( bottleMorphTarget, "../models/bottle-b.obj" ); 
  bottleMorphTarget->trans.scale.Set( 0.01 );

  // YES THIS IS THE REAL OBJECT, NOT THE TARGET. 
  // IT SENDS THE MORPH VERTICES TO THE SHADER, NOT TO THE DRAW LIST TO BE DRAWN!
  bottle->BufferMorphOnly(); 

  // Generic OpenGL setup: Enable the depth buffer and set a nice background color.
  glEnable( GL_DEPTH_TEST );
  glClearColor( 0.3, 0.5, 0.9, 1.0 );

}

void cleanup( void ) {

  theScene.DestroyObject();

}

//--------------------------------------------------------------------

/** A function that takes no arguments.
    Is responsible for drawing a SINGLE VIEWPORT. **/
void displayViewport( void ) {  

  // Draw free-floating objects. I.e, everything in the scene.
  theScene.Draw();

  // Draw objects that are attached to the cameras.
  myScreen.camList.Draw();

}

// GLUT display callback. Effectively calls displayViewport per-each Camera.
void display( void ) {

  // Clear the buffer.
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  // Tell camList to draw using our displayViewport rendering function.
  myScreen.camList.View( displayViewport );

  // Swap to the next buffer.
  glutSwapBuffers();

}

// Sorry about this, but it's better if you don't know.
// For the sake of this simple framework example, the
// GLUT keybinding code only clutters the application.
#include "morphlite_keybindings.c"


// This is called on window resizes, which ultimately
// computes new viewport dimensions and aspect ratios.
void resizeEvent( int width, int height ) {

  /* Handles resizing the child cameras as well. */
  myScreen.Size( width, height );
  glutWarpPointer( myScreen.MidpointX(), myScreen.MidpointY() );

}


void idle( void ) {

  // Compute the time since last idle().
  // This is a global, stateful operation.
  Tick.Tock();

  // Animation variables.
  static double timer = 0.0 ;
  if ( (timer += 0.005 ) > 360.0 ) timer = 0.0 ;
  float percent = ( sin(timer) + 1 ) / 2 ;

  // Update the morph percentage.
  theScene["bottle"]->setMorphPercentage(percent);

  if (DEBUG_MOTION) 
    fprintf( stderr, "Time since last idle: %lu\n", Tick.Delta() );

  // Move all cameras: Apply velocity and acceleration adjustments.
  // If no cameras are currently moving, this will do nothing ;)
  myScreen.camList.IdleMotion();

  // Inform GLUT we'd like to render a new frame.
  glutPostRedisplay();
  
}

int main( int argc, char **argv ) {

  // OS X suppresses events after mouse warp.  This resets the suppression 
  // interval to 0 so that events will not be suppressed. This also found
  // at http://stackoverflow.com/questions/728049/
  // glutpassivemotionfunc-and-glutwarpmousepointer
#ifdef __APPLE__
  CGSetLocalEventsSuppressionInterval( 0.0 );
#endif

  glutInit( &argc, argv );
  glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
  glutInitWindowSize( myScreen.Width(), myScreen.Height() );
  glutCreateWindow( "Linear Interpolation Morphing Demo" );
  glutFullScreen();
  glutSetCursor( GLUT_CURSOR_NONE );

  GLEW_INIT();
  init();

  /* Register our Callbacks */
  glutDisplayFunc( display );
  glutKeyboardFunc( keyboard );
  glutKeyboardUpFunc( keylift );
  glutSpecialFunc( keyboard_ctrl );
  glutMouseFunc( mouse );
  glutMotionFunc( mouseroll );
  glutPassiveMotionFunc( mouselook );
  glutIdleFunc( idle );
  glutReshapeFunc( resizeEvent );

  /* PULL THE TRIGGER */
  glutMainLoop();
  return EXIT_SUCCESS;

}