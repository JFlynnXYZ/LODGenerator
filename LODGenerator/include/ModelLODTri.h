#ifndef MODELLODTRI_H_
#define MODELLODTRI_H_
//----------------------------------------------------------------------------------------------------------------------
/// @file ModelLOD.h
/// @brief basic obj loader inherits from AbstractMesh
//----------------------------------------------------------------------------------------------------------------------
// must include types.h first for Real and GLEW if required
#include <ngl/Types.h>
#include <vector>
#include <ngl/Texture.h>
#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <vector>
#include <cmath>
#include <stdlib.h>
#include <list>
#include <utility>
#include <ngl/Vec4.h>
#include <ngl/AbstractMesh.h>
#include <ngl/BBox.h>
#include <ngl/RibExport.h>
#include "TriangleV.h"

//----------------------------------------------------------------------------------------------------------------------
/// @class ModelLODTri "include/ModelLODTri.h"
/// @brief used to store vertex information from an imported model and used as 
///		as a means of creating an LOD model.
/// modified version of the Obj class from the NGL library
/// @author Jonathan Flynn
/// @version 0.1
/// @date 02/03/15 imported code from Obj.h
/// @todo add functionality to store adjacent triangles/vertices and use classes
//----------------------------------------------------------------------------------------------------------------------
struct vtxTriData {
  std::vector<Vertex *> vtxData;
  std::vector<Triangle *> triData;
};

//----------------------------------------------------------------------------------------------------------------------
/// @brief compare two Vertex pointers collapse cost and return the higher one
/// @returns bool of the higher value
//----------------------------------------------------------------------------------------------------------------------
bool compareVertexCost(Vertex*& a, Vertex*& b);

class ModelLODTri : public ngl::AbstractMesh
{

public :

  //----------------------------------------------------------------------------------------------------------------------
  /// @brief default constructor
  //----------------------------------------------------------------------------------------------------------------------
  ModelLODTri() : AbstractMesh(){;}
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief  constructor to load an objfile as a parameter
  /// @param[in]  &_fname the name of the obj file to load
  //----------------------------------------------------------------------------------------------------------------------
  ModelLODTri( const std::string& _fname );
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief constructor to load an objfile as a parameter
  /// @param[in]  &_fname the name of the obj file to load
  /// @param[in]  &_texName the name of the texture file
  //----------------------------------------------------------------------------------------------------------------------
  ModelLODTri( const std::string& _fname,  const std::string& _texName );
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief copy constructor for LOD creation
  //----------------------------------------------------------------------------------------------------------------------
  ModelLODTri(ModelLODTri &_m );
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief  Method to load the file in
  /// @param[in]  _fname the name of the obj file to load
  /// @param[in] _calcBB if we only want to load data and not use GL then set this to false
  //----------------------------------------------------------------------------------------------------------------------
  bool load( const std::string& _fname, bool _calcBB=true );
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief  method to save the obj
  /// @param[in] _fname the name of the file to save
  //----------------------------------------------------------------------------------------------------------------------
  void save( const std::string& _fname  ) const;
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief  method to get a vertex coordinate using a vertex class pointer
  /// @param[in] _v the vertex pointer for the point you want to get the coordinates for
  /// @returns Vec3 of the x,y,z coordinates for the vertex
  //----------------------------------------------------------------------------------------------------------------------
  ngl::Vec3 getVertexAtVtx(Vertex &_v ) const;
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief  method to get a face class using a triangle class pointer
  /// @param[in] _t the triangle pointer for the face you want to get the class for
  /// @returns Face value of triangle class it relates to
  //----------------------------------------------------------------------------------------------------------------------
  ngl::Face getFaceAtTri( Triangle& _t ) const;
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief  method to create a LOD for the current mesh
  /// @param[in] _nFaces the number of faces the LOD mesh will have
  /// @returns ModelLODTri* of the reduced mesh LOD with _nFaces
  //----------------------------------------------------------------------------------------------------------------------
  ModelLODTri* createLOD(const unsigned int _nFaces );
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief stores the Vertex information in my Vertex class and stores the necessary data for creating the last LOD created
  //----------------------------------------------------------------------------------------------------------------------
  std::vector<Vertex *> m_lodVertexOut;
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief stores the face/triangle information in my Triangle class and stores the necessary data for the last LOD created
  //----------------------------------------------------------------------------------------------------------------------
  std::vector<Triangle *> m_lodTriangleOut;
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief stores the Vertex class info in descending order from collapse cost
  //----------------------------------------------------------------------------------------------------------------------
  std::list<Vertex *> m_lodVertexCollapseCost;
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief copy the Triangle information and return the lists
  /// @param[in] _vtxData has to be the exact structure of data from m_lodTriangle or m_lodTriangleOut
  /// @returns std::vector<Triangle *> of all the triangle data cloned
  //----------------------------------------------------------------------------------------------------------------------
  vtxTriData copyVtxTriData(std::vector<Vertex *> _vtxData ,std::vector<Triangle *> _triData);
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief copy Vertex and Triangle information to Out variables
  //----------------------------------------------------------------------------------------------------------------------
  void copyVtxTriNormTexDataToOut();
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief clear Vertex and Triangle information from Out variables
  //----------------------------------------------------------------------------------------------------------------------
  void clearVtxTriDataOut();
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief store Vertex pointers in descending collapse costs in an ordered list of
  //----------------------------------------------------------------------------------------------------------------------
  void storeCollapseCostList();
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief clear collapse cost ordered list
  //----------------------------------------------------------------------------------------------------------------------
  void clearCollapseCostList();
  //----------------------------------------------------------------------------------------------------------------------
  /// @update collapse cost ordered list
  //----------------------------------------------------------------------------------------------------------------------
  void updateCollapseCostList();






protected :
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief parser function to parse the vertex used by boost::spirit parser
  /// @param[in] _begin the start of the string to parse
  //----------------------------------------------------------------------------------------------------------------------
  void parseVertex( const char *_begin );
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief parser function to parse the Norma used by boost::spirit parser
  /// @param[in] _begin the start of the string to parse
  //----------------------------------------------------------------------------------------------------------------------
  void parseNormal( const char *_begin  );
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief parser function to parse the text cord used by boost::spirit parser
  /// @param[in] _begin the start of the string to parse
  //----------------------------------------------------------------------------------------------------------------------
  void parseTextureCoordinate( const char * _begin );
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief parser function to parse the Face data used by boost::spirit parser
  /// @param[in] _begin the start of the string to parse
  //----------------------------------------------------------------------------------------------------------------------
  void parseFace( const char * _begin );
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief  calculate the cost of an edge collapse from two vertices
  /// @param[in] _u vertex pointer, from this vertex collapse cost onto _v
  /// @param[in] _v vertex pointer, collapse cost onto this vertex from _u
  /// @returns float of the collapse cost from _u to _v
  //----------------------------------------------------------------------------------------------------------------------
  float calculateEColCost( Vertex* _u, Vertex* _v);
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief  calculate the cost of all adjacent vertex collapses from selected vertex
  /// @param[in] _v vertex pointer from which all collapse costs will be calculated
  //----------------------------------------------------------------------------------------------------------------------
  void calculateEColCostAtVtx( Vertex* _v);
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief  calculate all edge collapse costs
  //----------------------------------------------------------------------------------------------------------------------
  void calculateAllEColCosts();
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief  collapse the edge between two vertices. Use vertices from m_lodVertexOut!
  /// @param[in] _u vertex pointer, from this vertex collapse onto _v
  /// @param[in] _v vertex pointer, collapse onto this vertex from _u
  //----------------------------------------------------------------------------------------------------------------------
  void collapseEdge( Vertex* _u, Vertex* _v );
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief stores the Vertex information in my Vertex class
  //----------------------------------------------------------------------------------------------------------------------
  std::vector<Vertex *> m_lodVertex;
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief stores the face/triangle information in my Triangle class
  //----------------------------------------------------------------------------------------------------------------------
  std::vector<Triangle *> m_lodTriangle;
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief stores current number of deleted faces
  //----------------------------------------------------------------------------------------------------------------------
  unsigned int m_nDeletedFaces;
};




#endif
//----------------------------------------------------------------------------------------------------------------------

