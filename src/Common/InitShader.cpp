/**
 * @file InitShader.cpp
 * @authors Ed Angel, Nick St.Pierre
 * @date 2013-03-13
 * @brief Provides a wrapper utility for quickly
 * linking against glsl programs.
 **/

#include <cstdio>
#include <iostream>
#include "platform.h"
#include "InitShader.hpp"

/**
 * @def GEOMETRY_VERTICES_OUT_EXT
 * GEOMETRY_VERTICES_OUT_EXT is a Magic OpenGL constant.
 * On some systems, we might need to define this manually.
 * It is normally provided by OpenGL directly.
 * FIXME: This seems hacky!
 */
#ifndef GEOMETRY_VERTICES_OUT_EXT
#define GEOMETRY_VERTICES_OUT_EXT 0x8DDA
#endif

/**
 * @def GL_GEOMETRY_SHADER
 * GEOMETRY_VERTICES_OUT_EXT is a Magic OpenGL constant.
 * On some systems, we might need to define this manually.
 * It is normally provided by OpenGL directly.
 * FIXME: This seems hacky!
 */
#ifndef GL_GEOMETRY_SHADER
#define GL_GEOMETRY_SHADER 0x8DD9
#endif

namespace Angel {

  /**
   * Read in a shader file into a NULL-terminated string.
   *
   * @param shaderFile The file to read.
   *
   * @return A pointer to the NULL terminated string.
   */
  static char *readShaderSource( const char* shaderFile ) {
    FILE* fp = fopen( shaderFile, "r" );

    if ( fp == NULL ) {
      return NULL;
    }

    fseek( fp, 0L, SEEK_END );
    long size = ftell( fp );

    fseek( fp, 0L, SEEK_SET );
    char* buf = new char[size + 1];
    
    buf[fread( buf, 1, size, fp )] = '\0';
    fclose( fp );

    return buf;
  }

  /**
   * InitShader takes two shader sourcefiles and compiles them into a
   * shader program.
   *
   * @param vShaderFile the vertex shader source file
   * @param fShaderFile the fragment shader source file
   *
   * @return A handle to the compiled glsl program.
   */
  GLuint InitShader( const char* vShaderFile, const char* fShaderFile ) {
    struct Shader {
      const char* filename;
      GLenum type;
      GLchar* source;
    } shaders[2] = { { vShaderFile, GL_VERTEX_SHADER, NULL }, {
        fShaderFile, GL_FRAGMENT_SHADER, NULL } };

    GLuint program = glCreateProgram();
    
    for ( int i = 0; i < 2; ++i ) {
      Shader& s = shaders[i];
      s.source = readShaderSource( s.filename );
      if ( shaders[i].source == NULL ) {
        std::cerr << "Failed to read " << s.filename << std::endl;
        exit( EXIT_FAILURE );
      }

      GLuint shader = glCreateShader( s.type );
      glShaderSource( shader, 1, (const GLchar**) &s.source, NULL );
      glCompileShader( shader );

      GLint compiled;
      glGetShaderiv( shader, GL_COMPILE_STATUS, &compiled );
      if ( !compiled ) {
        std::cerr << s.filename << " failed to compile:" << std::endl;
        GLint logSize;
        glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &logSize );
        char* logMsg = new char[logSize];
        glGetShaderInfoLog( shader, logSize, NULL, logMsg );
        std::cerr << logMsg << std::endl;
        delete[] logMsg;

        /*exit( EXIT_FAILURE );*/
      }

      delete[] s.source;

      glAttachShader( program, shader );
    }

    /* link  and error check */glLinkProgram( program );

    GLint linked;
    glGetProgramiv( program, GL_LINK_STATUS, &linked );
    if ( !linked ) {
      std::cerr << "Shader program failed to link" << std::endl;
      GLint logSize;
      glGetProgramiv( program, GL_INFO_LOG_LENGTH, &logSize );
      char* logMsg = new char[logSize];
      glGetProgramInfoLog( program, logSize, NULL, logMsg );
      std::cerr << logMsg << std::endl;
      delete[] logMsg;

      exit( EXIT_FAILURE );
    }

    /* use program object */
    //glUseProgram(program);
    return program;
  }

  /**
   * InitShader takes three shader sourcefiles and compiles them into a
   * shader program.
   *
   * @param vShaderFile the vertex shader source file
   * @param gShaderFile the geometry shader source file
   * @param fShaderFile the fragment shader source file
   *
   * @return A handle to the compiled glsl program.
   */
  GLuint InitShader( const char* vShaderFile, const char* gShaderFile,
                     const char* fShaderFile ) {

    struct Shader {
      const char* filename;
      GLenum type;
      GLchar* source;
    } shaders[3] = { { vShaderFile, GL_VERTEX_SHADER, NULL }, {
        gShaderFile, GL_GEOMETRY_SHADER, NULL },
                     { fShaderFile, GL_FRAGMENT_SHADER, NULL } };

    GLuint program = glCreateProgram();

    for ( int i = 0; i < 3; ++i ) {

      Shader& s = shaders[i];
      s.source = readShaderSource( s.filename );
      if ( shaders[i].source == NULL ) {
        std::cerr << "Failed to read " << s.filename << std::endl;
        exit( EXIT_FAILURE );
      }

      GLuint shader = glCreateShader( s.type );
      glShaderSource( shader, 1, (const GLchar**) &s.source, NULL );
      glCompileShader( shader );

      GLint compiled;
      glGetShaderiv( shader, GL_COMPILE_STATUS, &compiled );
      if ( !compiled ) {
        std::cerr << s.filename << " failed to compile:" << std::endl;
        GLint logSize;
        glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &logSize );
        char* logMsg = new char[logSize];
        glGetShaderInfoLog( shader, logSize, NULL, logMsg );
        std::cerr << logMsg << std::endl;
        delete[] logMsg;

        /*exit( EXIT_FAILURE );*/
      }

      delete[] s.source;

      if ( s.type == GL_GEOMETRY_SHADER ) {// Gshader init requires a few extra things

        //GLint n;
        //glGetIntegerv( GL_MAX_GEOMETRY_OUTPUT_COMPONENTS, &n );

        glProgramParameteriEXT( shader, GL_GEOMETRY_INPUT_TYPE_EXT,
                                GL_TRIANGLES );

        glProgramParameteriEXT( shader, GL_GEOMETRY_OUTPUT_TYPE_EXT,
                                GL_TRIANGLE_STRIP );

      }

      glAttachShader( program, shader );
    }
    
    // Gshader gak
    glProgramParameteriEXT( program, GEOMETRY_VERTICES_OUT_EXT, 4 );

    /* link  and error check */glLinkProgram( program );
    /* test */glLinkProgram( program );

    GLint linked;
    glGetProgramiv( program, GL_LINK_STATUS, &linked );
    if ( !linked ) {
      std::cerr << "Shader program failed to link" << std::endl;
      GLint logSize;
      glGetProgramiv( program, GL_INFO_LOG_LENGTH, &logSize );
      char* logMsg = new char[logSize];
      glGetProgramInfoLog( program, logSize, NULL, logMsg );
      std::cerr << logMsg << std::endl;
      delete[] logMsg;

      exit( EXIT_FAILURE );
    }

    /* use program object */
    //glUseProgram(program);
    return program;
  }

}  // Close namespace Angel block
