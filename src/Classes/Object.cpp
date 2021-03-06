#include <string>
#include <vector>
#include <SOIL.h>
#include <stdexcept>

#include "platform.h"
#include "Engine.hpp"
#include "vec.hpp"
#include "mat.hpp"
#include "Object.hpp"
#include "globals.h"
#include "Timer.hpp"

using Angel::vec4;
using Angel::mat4;

Object::Object( const std::string &name, GLuint gShader )
{

  /* The constructor is going to initialize the VAO and a series of VBOs.
     The VAO is our general handle to this collection of VBOs.

     Each VBO contains some component data for how to render the vertex:
     Position, Color, Direction (Normal), Texture and Draw Order. */


  // Create room for our GLUniform handles
  if (DEBUG)
    fprintf( stderr, "\nCreating %d handles for uniforms\n", Object::END );
  handles.resize( Object::END );

  // Associate this Object with the Shader.
  Shader( gShader );

  // Set our name from the constructor...
  this->name = name;

  /* Initialize our draw mode to GL_TRIANGLES until informed otherwise. */
  draw_mode = GL_TRIANGLES;

  // Get Uniform handles for the following shader variables
  Link( Object::IsTextured, "fIsTextured" );
  Link( Object::ObjectCTM, "OTM" );
  Link( Object::MorphPercentage, "morphPercentage" );

  //Default to "Not Textured"
  this->isTextured = false;

  // Linear Interpolation Demo: Morph Percentage
  this->morphPercentage = 1.0;
  // Pointer to an Object to Morph to.
  this->morphTarget = NULL ;

  /* Create our VAO, which is our handle to all 
     the rest of the following information. */
  glGenVertexArrays( 1, &vao );
  glBindVertexArray( vao );
  GLuint glsl_uniform;

  /* Create (Eight) VBOs: One each for Positions, Colors, Normals, 
     Textures, Draw Order; Three for Morphing (Position,Colors,Normals.) */
  glGenBuffers( NUM_BUFFERS, buffer );

  /* Create the Vertex buffer and link it with the shader. */
  glBindBuffer( GL_ARRAY_BUFFER, buffer[VERTICES] );
  glsl_uniform = glGetAttribLocation( gShader, "vPosition" );
  glEnableVertexAttribArray( glsl_uniform );
  glVertexAttribPointer( glsl_uniform, 4, GL_FLOAT, GL_FALSE, 0, 0 );

  /* Create the MORPH Vertex buffer and link it with the shader. */
  glBindBuffer( GL_ARRAY_BUFFER, buffer[VERTICES_MORPH] );
  glsl_uniform = glGetAttribLocation( gShader, "vPositionMorph" );
  glEnableVertexAttribArray( glsl_uniform );
  glVertexAttribPointer( glsl_uniform, 4, GL_FLOAT, GL_FALSE, 0, 0 );

  /* Create the Normal buffer and link it with the shader. */
  glBindBuffer( GL_ARRAY_BUFFER, buffer[NORMALS] );
  glsl_uniform = glGetAttribLocation( gShader, "vNormal" );
  glEnableVertexAttribArray( glsl_uniform );
  glVertexAttribPointer( glsl_uniform, 3, GL_FLOAT, GL_FALSE, 0, 0 );

  /* Create the Normal MORPH buffer and link it with the shader. */
  glBindBuffer( GL_ARRAY_BUFFER, buffer[NORMALS_MORPH] );
  glsl_uniform = glGetAttribLocation( gShader, "vNormalMorph" );
  glEnableVertexAttribArray( glsl_uniform );
  glVertexAttribPointer( glsl_uniform, 3, GL_FLOAT, GL_FALSE, 0, 0 );

  /* Create the Color buffer and link it with the shader. */
  glBindBuffer( GL_ARRAY_BUFFER, buffer[COLORS] );
  glEnable( GL_BLEND );
  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  glsl_uniform = glGetAttribLocation( gShader, "vColor" );
  glEnableVertexAttribArray( glsl_uniform );
  glVertexAttribPointer( glsl_uniform, 4, GL_FLOAT, GL_FALSE, 0, 0 );

  /* Create the Color Morph buffer and link it with the shader. */
  glBindBuffer( GL_ARRAY_BUFFER, buffer[COLORS_MORPH] );
  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  glsl_uniform = glGetAttribLocation( gShader, "vColorMorph" );
  glEnableVertexAttribArray( glsl_uniform );
  glVertexAttribPointer( glsl_uniform, 4, GL_FLOAT, GL_FALSE, 0, 0 );

  /* Create the Texture Coordinate buffer and link it with the shader. */
  glBindBuffer( GL_ARRAY_BUFFER, buffer[TEXCOORDS] );
  glsl_uniform = glGetAttribLocation( gShader, "vTex" );
  glEnableVertexAttribArray( glsl_uniform );
  glVertexAttribPointer( glsl_uniform, 2, GL_FLOAT, GL_FALSE, 0, 0 );

  if (DEBUG) 
    fprintf( stderr,
	     "buffhandles: %u %u %u %u %u %u %u %u\n",
	     buffer[VERTICES], buffer[NORMALS],
	     buffer[COLORS], buffer[TEXCOORDS],
	     buffer[INDICES], buffer[VERTICES_MORPH],
	     buffer[NORMALS_MORPH], buffer[COLORS_MORPH] );

  /* Create the Drawing Order buffer, but we don't need to link it 
     with any uniform,
     because we won't be accessing this data directly. (I.e, the numbers here
     are not important once we are in the Vertex Shader. */
  glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, buffer[INDICES] );

  /* Unset the VAO context. */
  glBindVertexArray( 0 );
}


void Object::destroyMorphTarget(){

  if ( this->morphTarget != NULL ) {
    delete this->morphTarget ;
    this->morphTarget = NULL   ;
  }

}


Object::~Object( void ) {

  destroyMorphTarget() ;

}



void Object::BufferMorphOnly( void ) {

  glBindVertexArray( vao );

  glBindBuffer( GL_ARRAY_BUFFER, buffer[VERTICES_MORPH] );
  glBufferData( GL_ARRAY_BUFFER, sizeof(Angel::vec4) * this->getMorphTargetPtr()->points.size(),
                &(this->getMorphTargetPtr()->points[0]), GL_STATIC_DRAW );

  glBindBuffer( GL_ARRAY_BUFFER, buffer[NORMALS_MORPH] );
  glBufferData( GL_ARRAY_BUFFER, sizeof(Angel::vec3) * this->getMorphTargetPtr()->normals.size(),
                &(this->getMorphTargetPtr()->normals[0]), GL_STATIC_DRAW );

  glBindBuffer( GL_ARRAY_BUFFER, buffer[COLORS_MORPH] );
  glBufferData( GL_ARRAY_BUFFER, sizeof(Angel::vec4) * this->getMorphTargetPtr()->colors.size(),
                &(this->getMorphTargetPtr()->colors[0]), GL_STATIC_DRAW );


  // #MORPH
  // TODO: MORPH TEXTURES AND INDICIES

  glBindVertexArray( 0 );


}


void Object::Buffer( void ) {


  glBindVertexArray( vao );
  
  glBindBuffer( GL_ARRAY_BUFFER, buffer[VERTICES] );
  glBufferData( GL_ARRAY_BUFFER, sizeof(Angel::vec4) * points.size(),
		&(points[0]), GL_STATIC_DRAW );

  glBindBuffer( GL_ARRAY_BUFFER, buffer[NORMALS] );
  glBufferData( GL_ARRAY_BUFFER, sizeof(Angel::vec3) * normals.size(),
		&(normals[0]), GL_STATIC_DRAW );

  glBindBuffer( GL_ARRAY_BUFFER, buffer[COLORS] );
  glBufferData( GL_ARRAY_BUFFER, sizeof(Angel::vec4) * colors.size(),
		&(colors[0]), GL_STATIC_DRAW );
  

  /* Without the following workaround code,
     Mac OSX will segfault attempting to access
     the texcoordinate buffers on nontextured objects. */
  if (texcoords.size() == 0 && isTextured == false) {
    texcoords.push_back(Angel::vec2( -1, -1 ));
  } else if (texcoords.size() > 1) {
    /* Yes, this workaround prevents us from having
       textured objects with only one point.
       Oops. */
    isTextured = true;
  }

  glBindBuffer( GL_ARRAY_BUFFER, buffer[TEXCOORDS] );
  glBufferData( GL_ARRAY_BUFFER, sizeof(Angel::vec2) * texcoords.size(),
		(texcoords.size() ? &(texcoords[0]) : NULL), GL_STATIC_DRAW );
  
  glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, buffer[INDICES] );
  glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(),
		&(indices[0]), GL_STATIC_DRAW );

  glBindVertexArray( 0 );

}


void Object::Link( UniformEnum which, const std::string &name ) {

  if (which >= handles.size()) {
    fprintf( stderr, "WARNING: Ignoring request to link a uniform (#%u) beyond our handles array [%lu].\n",
	     which, handles.size() );
    return;
  }

  // Save the link between the Uniform and the Variable Name.
  _uniformMap[ which ] = name;
  
  fprintf( stderr, "Linking enum[%u] with %s for object %s\n",
	   which, name.c_str(), this->name.c_str() );

  handles[which] = glGetUniformLocation( Shader(), name.c_str() );
  if (DEBUG)
    fprintf( stderr, "Linking handles[%d] to %s; got %d.\n",
	     which, name.c_str(), handles[which] );

}

/**
   Sets the shader to be used by this object.
   Triggers a query of the shader program,
   for the locations of the Uniform locations
   that the object needs.

   @param newShader a GLuint handle to the shader program to use.
   
   @return None.
**/
void Object::Shader( GLuint newShader ) {
  
  // Cache the shader handle for later.
  Scene::Shader( newShader );

  // We have to use the program to query the glUniform locations.
  glUseProgram( newShader );

  // Re-Link our Uniforms to this shader.
  UniformMap::iterator it;
  for (it = _uniformMap.begin(); it != _uniformMap.end(); ++it) {
    Link( it->first, it->second );
  }

}

/**
   Returns the Object's current Shader.
   Defined because C++ will not let you overload an overrided function,
   without re-overloading it in the derived class.

   @return a GLuint handle to the shader program used by this Object.
**/
GLuint Object::Shader( void ) {

  return Scene::Shader();

}

void Object::Texture( const char** filename ) {

  Tick.Tock();
  glBindVertexArray( vao );

  GLuint tex2ddirt = SOIL_load_OGL_texture( filename[0],
					    SOIL_LOAD_AUTO,
					    SOIL_CREATE_NEW_ID,
					    SOIL_FLAG_MIPMAPS | 
					    SOIL_FLAG_INVERT_Y | 
					    SOIL_FLAG_NTSC_SAFE_RGB | 
					    SOIL_FLAG_COMPRESS_TO_DXT );

  GLuint tex2dsand = SOIL_load_OGL_texture( filename[1],
					    SOIL_LOAD_AUTO,
					    SOIL_CREATE_NEW_ID,
					    SOIL_FLAG_MIPMAPS | 
					    SOIL_FLAG_INVERT_Y | 
					    SOIL_FLAG_NTSC_SAFE_RGB | 
					    SOIL_FLAG_COMPRESS_TO_DXT );

  GLuint tex2dgrass = SOIL_load_OGL_texture( filename[2],
					     SOIL_LOAD_AUTO,
					     SOIL_CREATE_NEW_ID,
					     SOIL_FLAG_MIPMAPS | 
					     SOIL_FLAG_INVERT_Y | 
					     SOIL_FLAG_NTSC_SAFE_RGB | 
					     SOIL_FLAG_COMPRESS_TO_DXT );

  GLuint tex2drock = SOIL_load_OGL_texture( filename[3],
					    SOIL_LOAD_AUTO,
					    SOIL_CREATE_NEW_ID,
					    SOIL_FLAG_MIPMAPS | 
					    SOIL_FLAG_INVERT_Y | 
					    SOIL_FLAG_NTSC_SAFE_RGB | 
					    SOIL_FLAG_COMPRESS_TO_DXT );

  GLuint tex2dsnow = SOIL_load_OGL_texture( filename[4],
					    SOIL_LOAD_AUTO,
					    SOIL_CREATE_NEW_ID,
					    SOIL_FLAG_MIPMAPS | 
					    SOIL_FLAG_INVERT_Y | 
					    SOIL_FLAG_NTSC_SAFE_RGB | 
					    SOIL_FLAG_COMPRESS_TO_DXT );
  Tick.Tock();
  fprintf( stderr, "took %lu usec to load textures.\n", Tick.Delta() );

  
  GLuint gSampler0 = glGetUniformLocation( Shader(), "gSampler0" );
  glUniform1i( gSampler0, 0 );
  GLuint gSampler1 = glGetUniformLocation( Shader(), "gSampler1" );
  glUniform1i( gSampler1, 1 );
  GLuint gSampler2 = glGetUniformLocation( Shader(), "gSampler2" );
  glUniform1i( gSampler2, 2 );
  GLuint gSampler3 = glGetUniformLocation( Shader(), "gSampler3" );
  glUniform1i( gSampler3, 3 );
  GLuint gSampler4 = glGetUniformLocation( Shader(), "gSampler4" );
  glUniform1i( gSampler4, 4 );

  glActiveTexture( GL_TEXTURE0 );
  glBindTexture( GL_TEXTURE_2D, tex2ddirt );
  glEnable( GL_TEXTURE_2D );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

  glActiveTexture( GL_TEXTURE1 );
  glBindTexture( GL_TEXTURE_2D, tex2dsand );
  glEnable( GL_TEXTURE_2D );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

  glActiveTexture( GL_TEXTURE2 );
  glBindTexture( GL_TEXTURE_2D, tex2dgrass );
  glEnable( GL_TEXTURE_2D );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

  glActiveTexture( GL_TEXTURE3 );
  glBindTexture( GL_TEXTURE_2D, tex2drock );
  glEnable( GL_TEXTURE_2D );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

  glActiveTexture( GL_TEXTURE4 );
  glBindTexture( GL_TEXTURE_2D, tex2dsnow );
  glEnable( GL_TEXTURE_2D );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  
  glBindVertexArray( 0 );

  Tick.Tock();
  fprintf( stderr, "took %lu usec to finalize textures.\n", Tick.Delta() );
}


void Object::send( Object::UniformEnum which ) {
  switch (which) {
    
  case Object::IsTextured:
    glUniform1i( handles[Object::IsTextured],
		 (isTextured) ? 1 : 0 );
    break;
    
  case Object::ObjectCTM:
    glUniformMatrix4fv( handles[Object::ObjectCTM], 1, GL_TRUE,
			this->trans.OTM() );
    break;
    

  case Object::MorphPercentage:
    glUniform1f( handles[Object::MorphPercentage],
		 this->getMorphPercentage() );

    break;


  default:
    throw std::invalid_argument( "Unknown Uniform Handle Enumeration." );
  }
}

void Object::Draw( void ) {

  glBindVertexArray( vao );

  // Check to see if the correct shader program is engaged.
  GLint currShader;
  glGetIntegerv(GL_CURRENT_PROGRAM, &currShader);
  if ((GLuint)currShader != Shader()) {

    Camera *activeCamera = Engine::instance()->cams()->active();
    
    //if (DEBUG) std::cerr << "Switching shading context.\n";

    // Set OpenGL to use this object's shader.
    glUseProgram( Shader() );

    // Set the Active Camera's shader to the Object's Shader.
    activeCamera->Shader( Shader() );

    // send the Camera's info to the new shader.
    activeCamera->view();
  }  

  send( Object::IsTextured ) ;
  send( Object::ObjectCTM  ) ;
  send( Object::MorphPercentage );

  //  this->getMorphPercentage() == -1.0 ? ; : send( Object::MorphPercentage );

  /* Are we using a draw order? */
  if (indices.size() > 1)
    glDrawElements( draw_mode, indices.size(), GL_UNSIGNED_INT, 0 );
  else
    glDrawArrays( draw_mode, 0, points.size() );

  glBindVertexArray( 0 );

  // Draw all of our Children.
  // (With clothes on, pervert.)
  Scene::Draw();

}

void Object::Mode( GLenum new_mode ) {

  this->draw_mode = new_mode;

}

const std::string &Object::Name( void ) const {

  return name;

}

void Object::Animation(void (*anim_func)( TransCache &arg )) {
  anim_func( this->trans );
  Object::Propagate();
}

void Object::Propagate( void ) {

  //fprintf( stderr, "\n" );
  //fprintf( stderr, "Propagate called on %s\n", name.c_str() );

  std::list<Object*>::iterator it;
  
  //std::cerr << "Calling CALCCTM:\n";
  //Update my Object's CTM...
  this->trans.CalcCTM();

  //send my OTM as the PTM to all of my children.
  for ( it = _list.begin(); it != _list.end(); ++it ) {
    (*it)->trans.PTM( this->trans.OTM() );
    //Tell that child to update his CTM and propegate.
    (*it)->Propagate();
  }

  //std::cerr << "{" << name << "::OTM:" << this->trans.OTM() << "}\n";

}


/**

   returns the position of the object
   this makes the lighting implementation much easier... for this semester.

 */

vec4 Object::GetPosition() const {

  mat4 theOTM = this->trans.OTM() ;

  return vec4( theOTM[0][3],
	       theOTM[1][3],
	       theOTM[2][3],
	       1.0);
}


Object* Object::getMorphTargetPtr() const {

  return this->morphTarget ;

}


void Object::setMorphPercentage(const float _morphPercentage){

  morphPercentage = _morphPercentage ;
}

float Object::getMorphPercentage() const {

  return this->morphPercentage ;
}


Object* Object::genMorphTarget(GLuint gShader) {

  Object *obj = new Object( this->name + "_morph", gShader );
  this->morphTarget = obj ;
  return obj ;

}

int Object::getNumberPoints(){
	return points.size();
}
