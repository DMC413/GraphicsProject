#include <string>
#include <map>
#include <list>
#include <stdexcept>

#include "Scene.hpp"
#include "Object.hpp"
#include "ParticleSystem.hpp"

typedef std::pair< std::string, Object * > mapping;

Scene::Scene() {
  currentObj = list.end();
  gShader = 0;
}

Scene::~Scene() {

  /* Traverse the list and free all Objects. */  
  std::list< Object* >::reverse_iterator it;
  for (it = list.rbegin(); it != list.rend(); ++it)
    delete *it;

}

void Scene::InsertObject( Object *obj ) {
  list.push_back( obj );
  map.insert( mapping( obj->Name(), obj ) );
}

Object *Scene::AddObject( const std::string &objName,
			  GLuint shader, ObjectType oType ) {

  Object* obj;

  // Note that 'shader' defaults to 0.
  if ((!shader) && (!gShader))
    throw std::invalid_argument( "A call to AddObject() was made without "
				 "specifying either the object-specific shader,\n"
				 "\tor informing the parent Scene of a default shader to use." );
  if( oType == OBJECT ) {
    obj = new Object( objName, ((shader) ? shader : gShader) );
  } else {
    obj = new ParticleSystem( 1, objName, ((shader) ? shader : gShader) );
  }
  InsertObject( obj );
  return obj;
}

/**
   Sets the Default shader for the scene.
   In the context of inheritance by objects,
   This sets the shader to use to render the physical object.
   
   @param gShader The GLuint handle to the shader to use.
   
   @return void.
**/

void Scene::SetShader( GLuint gShader ) {
  this->gShader = gShader;
}

/**
   Retrieves the handle for the default shader for the scene.
   In the context of inheritance by objects,
   This retrieves the shader handle to use to draw the object.

   @return A GLuint handle to the shader program.
**/

GLuint Scene::GetShader( void ) {
  return gShader;
}

/**
   DeleteObject is the actual implementation function that will
   remove an Object from the Scene list and Scene map,
   then free the object.
   @param obj The pointer to the object to free.
**/
void Scene::DeleteObject( Object *obj ) {

  if (obj == Active()) Prev();

  list.remove( obj );
  map.erase( obj->Name() );
  delete obj;

}

void Scene::DelObject( const std::string &objName ) {
  Object *obj = map[ objName ];
  DeleteObject( obj );
}

void Scene::DelObject( void ) {
  DeleteObject(*(list.begin()));
}


/**
   Completely remove this object and all his children.
**/
void Scene::DestroyObject( void ) {
  /*  std::list< Object* >::iterator it;
  for (it = list.begin(); it != list.end(); ++it) {
    (*it)->DestroyObject();
    DeleteObject( *it );
  }
  */
}
  

void Scene::PopObject( void ) {
  DeleteObject(*(--list.end()));
}

Object *Scene::Next( void ) {

  // If the list is empty, we can't cycle.
  if (list.size() == 0)
    throw std::logic_error( "Next() called, but there are no Objects"
			    " in this list." );

  // Move to the next one. Cycle back if needed.
  if (++currentObj == list.end())
    currentObj = list.begin();

  return *currentObj;

}

Object *Scene::Prev( void ) {
  
  if (list.size() == 0)
    throw std::logic_error( "Prev() called, but there are no objects"
			    " in this list." );
  
  if (currentObj == list.begin())
    currentObj = --list.end();
  else
    --currentObj;
  
  return *currentObj;

}


Object *Scene::Active( void ) {
  
  if (list.size() == 0) 
    throw std::logic_error( "Active() called, but the object list is empty." );
  else if (currentObj == list.end())
    throw std::logic_error( "Active() called, but the active object is out-of-bounds." );
  else
    return *currentObj;

}

void Scene::Draw( void ) {
  std::list< Object* >::iterator it;
  for (it = list.begin(); it != list.end(); ++it) {
    (*it)->Draw();
  }
}

Object *Scene::operator[]( std::string const &objname ) {
  
  std::map< std::string, Object* >::iterator ret;
  ret = map.find( objname );
  
  if (ret == map.end()) return NULL;
  //if (ret == map.end()) throw std::out_of_range("Requested scene object \"" + objname + "\" not in scene");
  return ret->second;

}

Scene &Scene::operator=( const Scene &copy ) {

  this->gShader = copy.gShader;
  this->map.clear();
  this->list.clear();
  this->currentObj = list.end();
  return *this;

}

Scene::Scene( const Scene &copy ) {
  (*this) = copy;
}
