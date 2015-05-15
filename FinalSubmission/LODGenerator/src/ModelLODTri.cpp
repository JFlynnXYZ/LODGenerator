#include "boost/bind.hpp"

#include "boost/spirit.hpp"
/// @todo re-write this at some stage to use boost::spirit::qi

#include <boost/foreach.hpp>

#include <iostream>
#include <cfloat>
#include <map>

#include "ModelLODTri.h"
#include "TriangleV.h"
//----------------------------------------------------------------------------------------------------------------------
/// @file ModelLODTri.cpp
/// @brief implementation files for ModelLODTri class
//----------------------------------------------------------------------------------------------------------------------

// make a namespace for our parser to save writing boost::spirit:: all the time
namespace spt=boost::spirit;

// syntactic sugar for specifying our grammar
typedef spt::rule<spt::phrase_scanner_t> srule;

//----------------------------------------------------------------------------------------------------------------------
ModelLODTri::~ModelLODTri()
{
  for ( unsigned int i=0; i < m_lodVertex.size(); ++i)
  {
    delete(m_lodVertex[i]);
  }

  for ( unsigned int i=0; i < m_lodTriangle.size(); ++i)
  {
    delete(m_lodTriangle[i]);
  }

  clearVtxTriDataOut();

}

//----------------------------------------------------------------------------------------------------------------------
bool compareVertexCost(Vertex*& a, Vertex*& b)
{
  return (a->getCollapseCost() < b->getCollapseCost());
}

//----------------------------------------------------------------------------------------------------------------------
// parse a vertex
void ModelLODTri::parseVertex( const char *_begin )
{
  std::vector<ngl::Real> values;
  // here is the parse rule to load the data into a vector (above)
  srule vertex = "v" >> spt::real_p[spt::append(values)] >>
                        spt::real_p[spt::append(values)] >>
                        spt::real_p[spt::append(values)];
  // now parse the data
  spt::parse_info<> result = spt::parse(_begin, vertex, spt::space_p);
  // should check this at some stage
  NGL_UNUSED(result);
  // and add it to our vert list in abstact mesh parent
  m_verts.push_back(ngl::Vec3(values[0],values[1],values[2]));
  // add the vertex id and value to my custom Vertex class
  m_lodVertex.push_back(new Vertex(m_verts.size()-1, ngl::Vec3(values[0],values[1],values[2])));
}


//----------------------------------------------------------------------------------------------------------------------
// parse a texture coordinate
void ModelLODTri::parseTextureCoordinate(const char * _begin )
{
  std::vector<ngl::Real> values;
  // generate our parse rule for a tex cord,
  // this can be either a 2 or 3 d text so the *rule looks for an additional one
  srule texcord = "vt" >> spt::real_p[spt::append(values)] >>
                          spt::real_p[spt::append(values)] >>
                          *(spt::real_p[spt::append(values)]);
  spt::parse_info<> result = spt::parse(_begin, texcord, spt::space_p);
  // should check the return values at some stage
  NGL_UNUSED(result);

  // build tex cord
  // if we have a value use it other wise set to 0
  ngl::Real vt3 = values.size() == 3 ? values[2] : 0.0f;
  m_tex.push_back(ngl::Vec3(values[0],values[1],vt3));
}

//----------------------------------------------------------------------------------------------------------------------
// parse a normal
void ModelLODTri::parseNormal( const char *_begin )
{
  std::vector<ngl::Real> values;
  // here is our rule for normals
  srule norm = "vn" >> spt::real_p[spt::append(values)] >>
                       spt::real_p[spt::append(values)] >>
                       spt::real_p[spt::append(values)];
  // parse and push back to the list
  spt::parse_info<> result = spt::parse(_begin, norm, spt::space_p);
  // should check the return values at some stage
  NGL_UNUSED(result);
  m_norm.push_back(ngl::Vec3(values[0],values[1],values[2]));
}

//----------------------------------------------------------------------------------------------------------------------
// parse face
void ModelLODTri::parseFace(const char * _begin   )
{
  int k =0;
  // ok this one is quite complex first create some lists for our face data
  // list to hold the vertex data indices
  std::vector<int> vec;
  // list to hold the tex cord indices
  std::vector<int> tvec;
  // list to hold the normal indices
  std::vector<int> nvec;

  // create the parse rule for a face entry V/T/N
  // so our entry can be always a vert, followed by optional t and norm seperated by /
  // also it is possible to have just a V value with no / so the rule should do all this
  srule entry = spt::int_p[spt::append(vec)] >>
    (
      ("/" >> (spt::int_p[spt::append(tvec)] | spt::epsilon_p) >>
       "/" >> (spt::int_p[spt::append(nvec)] | spt::epsilon_p)
      )
      | spt::epsilon_p
    );
  // a face has at least 3 of the above entries plus many optional ones
  srule face = "f"  >> entry >> entry >> entry >> *(entry);
  // now we've done this we can parse
  spt::parse(_begin, face, spt::space_p);

  unsigned int numVerts=vec.size();
  // so now build a face structure.
  ngl::Face f;
  // create my triangle face structure.
  Triangle* lodTri = new Triangle(m_lodTriangle.size());
  // verts are -1 the size
  f.m_numVerts=numVerts-1;
  f.m_textureCoord=false;
  f.m_normals=false;
  // copy the vertex indices into our face data structure index in obj start from 1
  // so we need to do -1 for our array index
  BOOST_FOREACH(int i, vec)
  {
    f.m_vert.push_back(i-1);
    // store the Vertex class info in the triangle
    lodTri->m_vert.push_back(m_lodVertex[i-1]);
  }

  // copy the Vertex class value into the adjacent vertex for each vertex class
  // and add the adjacent triangles to each vertex.
  for (unsigned int i=0; i<vec.size(); i++)
  {
    for (unsigned int j=0; j<vec.size(); j++)
    {
      if (i!=j)
      {
        m_lodVertex[vec[i]-1]->addAdjVert(m_lodVertex[vec[j]-1]);
      }
    }
    m_lodVertex[vec[i]-1]->addAdjFace(lodTri);
  }

  // merge in texture coordinates and normals, if present
  // OBJ format requires an encoding for faces which uses one of the vertex/texture/normal specifications
  // consistently across the entire face.  eg. we can have all v/vt/vn, or all v//vn, or all v, but not
  // v//vn then v/vt/vn ...
  if(!nvec.empty())
  {
    if(nvec.size() != vec.size())
    {
     std::cerr <<"Something wrong with the face data will continue but may not be correct\n";
    }

    // copy in these references to normal vectors to the mesh's normal vector
    // also, add the value to the triangle as well as the ID
    k =0;
    BOOST_FOREACH(int i, nvec)
    {
      f.m_norm.push_back(i-1);
      // add the value and ID to the Triangle class
      lodTri->m_norm.push_back(m_norm[i-1]);
      lodTri->setNormID(i-1,k);
      k++;
    }
    f.m_normals=true;
    lodTri->m_normals=true;
  }

  //
  // merge in texture coordinates, if present
  //
  if(!tvec.empty())
  {
    if(tvec.size() != vec.size())
    {
     std::cerr <<"Something wrong with the face data will continue but may not be correct\n";
    }
    k = 0;
    // copy in these references to texture vectors to the mesh's texture vector
    // also, add the value to the triangle as well as the ID
    BOOST_FOREACH(int i, tvec)
    {
      f.m_tex.push_back(i-1);
      // add the value and ID to the Triangle class
      lodTri->m_tex.push_back(m_tex[i-1]);
      lodTri->setTexID(i-1, k);
    }

    f.m_textureCoord=true;
    lodTri->m_textureCoord=true;

  }
  // Calculate the triangle face normal
  lodTri->calculateNormal();

  // finally save the face into our face list
  m_face.push_back(f);
  // save the lod triangle to the triangle list
  m_lodTriangle.push_back(lodTri);
}

//----------------------------------------------------------------------------------------------------------------------
bool ModelLODTri::load(const std::string &_fname,bool _calcBB )
{
 // here we build up our ebnf rules for parsing
  // so first we have a comment
  srule comment = spt::comment_p("#");

  // see below for the rest of the obj spec and other good format data
  // http://local.wasp.uwa.edu.au/~pbourke/dataformats/obj/

  // vertices rule v is a parse of 3 reals and we run the parseVertex function
  srule vertex= ("v"  >> spt::real_p >> spt::real_p >> spt::real_p) [bind(&ModelLODTri::parseVertex,boost::ref(*this), _1)];
  // our tex rule and binding of the parse function
  srule tex= ("vt" >> spt::real_p >> spt::real_p) [bind(&ModelLODTri::parseTextureCoordinate, boost::ref(*this), _1)];
  // the normal rule and parsing function
  srule norm= ("vn" >> spt::real_p >> spt::real_p >> spt::real_p) [bind(&ModelLODTri::parseNormal,boost::ref(*this), _1)];

  // our vertex data can be any of the above values
  srule vertex_type = vertex | tex | norm;

  // the rule for the face and parser
  srule  face = (spt::ch_p('f') >> *(spt::anychar_p))[bind(&ModelLODTri::parseFace, boost::ref(*this), _1)];
  // open the file to parse
  std::ifstream in(_fname.c_str());
  if (in.is_open() != true)
  {
    std::cout<<"FILE NOT FOUND !!!! "<<_fname.c_str()<<"\n";
    return false;

  }
  std::string str;
  // loop grabbing a line and then pass it to our parsing framework
  while(getline(in, str))
  {
    spt::parse(str.c_str(), vertex_type  | face | comment, spt::space_p);
  }
  // now we are done close the file
  in.close();

  // grab the sizes used for drawing later
  m_nVerts=m_verts.size();
  m_nNorm=m_norm.size();
  m_nTex=m_tex.size();
  m_nFaces=m_face.size();

  // Calculate the Edge Collapse costs at the start
  calculateAllEColCosts();

  // Calculate the center of the object.
  if(_calcBB == true)
  {
    this->calcDimensions();
  }
  return true;

}

//----------------------------------------------------------------------------------------------------------------------
ModelLODTri::ModelLODTri( const std::string& _fname  ) :AbstractMesh()
{
    m_vbo=false;
    m_ext=0;
    // set default values
    m_nVerts=m_nNorm=m_nTex=m_nFaces=0;
    //set the default extents to 0
    m_maxX=0.0f; m_maxY=0.0f; m_maxZ=0.0f;
    m_minX=0.0f; m_minY=0.0f; m_minZ=0.0f;
    m_nNorm=m_nTex=0;

    // load the file in
    m_loaded=load(_fname);

    m_texture = false;
}

//----------------------------------------------------------------------------------------------------------------------
ModelLODTri::ModelLODTri( const std::string& _fname,const std::string& _texName   ):AbstractMesh()
{
    m_vbo=false;
    m_vao=false;
    m_ext=0;
    // set default values
    m_nVerts=m_nNorm=m_nTex=m_nFaces=0;
    //set the default extents to 0
    m_maxX=0.0f; m_maxY=0.0f; m_maxZ=0.0f;
    m_minX=0.0f; m_minY=0.0f; m_minZ=0.0f;
    m_nNorm=m_nTex=0;
    // load the file in
    m_loaded=load(_fname);

    // load texture
    loadTexture(_texName);
    m_texture = true;
}
//----------------------------------------------------------------------------------------------------------------------
ModelLODTri::ModelLODTri( ModelLODTri& _m )
{
  // clone data from lodVertexOut and lodTriangle out
  vtxTriData mVtxTriOut = _m.copyVtxTriData(_m.m_lodVertexOut, _m.m_lodTriangleOut);

  // store the cloned data into the lodVertex and lodTriangle locations
  m_lodVertex = mVtxTriOut.vtxData;
  m_lodTriangle = mVtxTriOut.triData;

  m_vbo=false;
  m_vao=false;
  m_ext=0;
  // set default values
  m_nVerts=m_nNorm=m_nTex=m_nFaces=0;
  m_loaded = true;
  m_texture = false;
  m_maxX=0.0f; m_maxY=0.0f; m_maxZ=0.0f;
  m_minX=0.0f; m_minY=0.0f; m_minZ=0.0f;

  // resize to make data allocation quicker
  m_face.resize(m_lodTriangle.size());

  // store maps for old and new ids so to renumber them correctly without adding
  // two of the same values to the lists
  std::map <int, int> oldNewIDVtxMatch;
  std::map <int, int> oldNewIDNormMatch;
  std::map <int, int> oldNewIDTexMatch;

  // stores the temp newID
  int newID;

  // renumbers and organises all Verts, Normals and Texture coords
  for (unsigned int i=0; i<m_lodTriangle.size(); ++i)
  {

    // stores all the new face data
    ngl::Face face;

    // Normal renumber: checks if the id has already been used, then adds the
    // coordinate value to m_norm followed by adding the new position ID to the
    // face
    for (unsigned int j=0; j<m_lodTriangle[i]->m_norm.size(); ++j)
    {
      if (oldNewIDNormMatch.find(m_lodTriangle[i]->getNormID(j)) == oldNewIDNormMatch.end())
      {
        // stores the normal value
        m_norm.push_back(m_lodTriangle[i]->m_norm[j]);
        newID = m_norm.size()-1;
        // stores the location of the normal value in m_norm
        face.m_norm.push_back(newID);
        // adds to map to stop multiple ids for the same value
        oldNewIDNormMatch[m_lodTriangle[i]->getNormID(j)]=newID;
      }
      else
      {
        // ID already used, so add to the face normal list the new ID
        face.m_norm.push_back(oldNewIDNormMatch[m_lodTriangle[i]->getNormID(j)]);
      }
    }


    // Texture coord renumber: checks if the id has already been used, then adds the
    // coordinate value to m_tex followed by adding the new position ID to the
    // face
    for (unsigned int j=0; j<m_lodTriangle[i]->m_tex.size(); ++j)
    {
      if (oldNewIDTexMatch.find(m_lodTriangle[i]->getTexID(j)) == oldNewIDTexMatch.end())
      {
        // stores the texture coord value
        m_tex.push_back(m_lodTriangle[i]->m_tex[j]);
        newID = m_tex.size()-1;
        // stores the location of the texture coord value in m_tex
        face.m_tex.push_back(newID);
        // adds to map to stop multiple ids for the same value
        oldNewIDTexMatch[m_lodTriangle[i]->getTexID(j)]=newID;
      }
      else
      {
        // ID already used, so add to the face texture coord the new ID
        face.m_tex.push_back(oldNewIDTexMatch[m_lodTriangle[i]->getTexID(j)]);
      }
    }

    // Verts renumber: checks if the id has already been used, then adds the
    // coordinate value to m_vert followed by adding the new position ID to the
    // face
    for (unsigned int j=0; j<m_lodTriangle[i]->m_vert.size(); ++j)
    {
      if (oldNewIDVtxMatch.find(m_lodTriangle[i]->m_vert[j]->getID()) == oldNewIDVtxMatch.end())
      {
        // stores the vert value
        m_verts.push_back(m_lodTriangle[i]->m_vert[j]->m_vert);
        newID = m_verts.size()-1;
        // stores the location of the texture coord value in m_vert
        // takes the value from the Vertex class ID
        face.m_vert.push_back(newID);
        // adds to map to stop multiple ids for the same value
        oldNewIDVtxMatch[m_lodTriangle[i]->m_vert[j]->getID()]=newID;
      }
      else
      {
        // ID already used, so add to the face vert the new ID
        face.m_vert.push_back(oldNewIDVtxMatch[m_lodTriangle[i]->m_vert[j]->getID()]);
      }
    }
    // stores the booleans and number of verts in face
    face.m_normals = m_lodTriangle[i]->m_normals;
    face.m_numVerts = face.m_vert.size()-1;
    face.m_textureCoord = m_lodTriangle[i]->m_textureCoord;

    // store the face in m_face
    m_face[i] = face;
  }

  // store the size of each list
  m_nVerts=m_verts.size();
  m_nNorm=m_norm.size();
  m_nTex=m_tex.size();
  m_nFaces=m_face.size();

  // copy the vertex and triangle data to the out variable
  copyVtxTriNormTexDataToOut();

  // store the collapse cost of the Vertices into an ordered list
  storeCollapseCostList();

  // calculate bbox and centre
  this->calcDimensions();
  this->createVAO();
}

//----------------------------------------------------------------------------------------------------------------------
void ModelLODTri::save(const std::string& _fname)const
{
  // Open the stream and parse
  std::fstream fileOut;
  fileOut.open(_fname.c_str(),std::ios::out);
  if (!fileOut.is_open())
  {
    std::cout <<"File : "<<_fname<<" Not founds "<<std::endl;
    return;
  }
  // write out some comments
  fileOut<<"# This file was created by ngl Obj exporter "<<_fname.c_str()<<std::endl;
  // was c++ 11  for(Vec3 v : m_norm) for all of these
  // write out the verts
  BOOST_FOREACH(ngl::Vec3 v , m_verts)
  {
    fileOut<<"v "<<v.m_x<<" "<<v.m_y<<" "<<v.m_z<<std::endl;
  }

  // write out the tex cords
  BOOST_FOREACH(ngl::Vec3 v , m_tex)
  {
    fileOut<<"vt "<<v.m_x<<" "<<v.m_y<<std::endl;
  }
  // write out the normals

  BOOST_FOREACH(ngl::Vec3 v , m_norm)
  {
    fileOut<<"vn "<<v.m_x<<" "<<v.m_y<<" "<<v.m_z<<std::endl;
  }

  // finally the faces
  BOOST_FOREACH(ngl::Face f , m_face)
  {
  fileOut<<"f ";
  // we now have V/T/N for each to write out
  for(unsigned int i=0; i<=f.m_numVerts; ++i)
  {
    // don't forget that obj indices start from 1 not 0 (i did originally !)
    fileOut<<f.m_vert[i]+1;
    fileOut<<"/";
    fileOut<<f.m_tex[i]+1;
    fileOut<<"/";

    fileOut<<f.m_norm[i]+1;
    fileOut<<" ";
  }
  fileOut<<std::endl;
  }
}

//----------------------------------------------------------------------------------------------------------------------
float ModelLODTri::calculateEColCost( Vertex* _u, Vertex* _v)
{
  ngl::Real edgeLength = (m_verts[_v->getID()] - m_verts[_u->getID()]).length();
  ngl::Real curvature = 0;

  std::vector<Triangle *> sideFaces;

  // Find what triangles are adjacent to both vertices
  for (unsigned int i=0; i < _u->m_faceAdj.size(); ++i)
  {
    if(_u->m_faceAdj[i]->hasVert(_v))
    {
      sideFaces.push_back(_u->m_faceAdj[i]);
    }
  }

  // use the triangle facing most away from the this side faces
  // to determine the curvature term
  for (unsigned int i=0; i < _u->m_faceAdj.size(); ++i)
  {
    float minCurve=1; // Curve for face i and the closer side to it
    for (unsigned int j=0; j < sideFaces.size(); ++j)
    {
      float dotprod = _u->m_faceAdj[i]->getFaceNormal().dot(sideFaces[j]->getFaceNormal());
      minCurve = fmin(minCurve, (1-dotprod)/2.0f);
    }
    curvature = fmax(curvature, minCurve);
  }
  // the more coplanar the lower the curvature term
  return edgeLength * curvature;
}
//----------------------------------------------------------------------------------------------------------------------
void ModelLODTri::calculateEColCostAtVtx( Vertex* _v)
{
  if (_v->m_vertAdj.size() == 0)
  {
    // v doesn't have any adjacent vertices and so it costs nothing to collapse
    _v->setCollapseVertex(NULL);
    _v->setCollapseCost(FLT_MIN);
    return;
  }

  // set pointer to NULL and to the highest value
  _v->setCollapseVertex(NULL);
  _v->setCollapseCost(FLT_MAX);

  // search all adjacent faces for the least cost edge collapse
  for (unsigned int i=0; i<_v->m_vertAdj.size(); ++i)
  {
    float cost = calculateEColCost(_v, _v->m_vertAdj[i]);
    if (cost < _v->getCollapseCost())
    {
      // set the collapse Vertex and the collapse cost
      _v->setCollapseVertex(_v->m_vertAdj[i]);
      _v->setCollapseCost(cost);
    }
  }

}
//----------------------------------------------------------------------------------------------------------------------
void ModelLODTri::calculateAllEColCosts()
{
  for (unsigned int i=0; i<m_verts.size(); ++i)
  {
    calculateEColCostAtVtx(m_lodVertex[i]);
  }
  // Deep copy all vertex and triangle values to their Out counterparts
  copyVtxTriNormTexDataToOut();
  storeCollapseCostList();
}
//----------------------------------------------------------------------------------------------------------------------
void ModelLODTri::collapseEdge(Vertex *_u, Vertex *_v)
{
  if (!_v)
  {
    // u is a vertex by itself so just delete it
    delete _u;
    return;
  }

  // temp store adjacent verts
  std::vector<Vertex *> vertTmp = _u->m_vertAdj;

  for ( int i =_u->m_faceAdj.size()-1; i >= 0; --i)
  {
    if (_u->m_faceAdj[i]->hasVert(_v))
    {
      // set NULL in triangle out
      m_lodTriangleOut[_u->m_faceAdj[i]->getID()] = NULL;
      delete(_u->m_faceAdj[i]);
      // add to number of deleted faces
      m_nDeletedFaces += 1;
    }
  }
  for ( int i =_u->m_faceAdj.size()-1; i >= 0; --i)
  {
    // update remaining triangles to have v instead of u
    _u->m_faceAdj[i]->replaceVertex(_u,_v);
  }
  // delete the vertex _u
  delete _u;

  // recompute the edge collapse costs for adjacent verts for _v
  for ( unsigned int i=0; i < vertTmp.size(); ++i)
  {
    calculateEColCostAtVtx(vertTmp[i]);
  }

}
//----------------------------------------------------------------------------------------------------------------------
vtxTriData ModelLODTri::copyVtxTriData(std::vector<Vertex *> _vtxData ,std::vector<Triangle *> _triData)
{
  std::vector<Vertex *> newVtxData;
  std::vector<Triangle *> newTriData;
  // resize the vector to the required size if necessary
  newVtxData.resize(_vtxData.size());
  newTriData.resize(_triData.size());

  for ( unsigned int i=0; i < fmax( _vtxData.size(), _triData.size()); ++i )
  {
    // copy the data and create a new pointer for each vertex
    if (i < _vtxData.size())
    {
      newVtxData[i] = _vtxData[i]->clone();
      // Resize the Adjacent Vert and Face vectors
      newVtxData[i]->m_vertAdj.resize(_vtxData[i]->m_vertAdj.size());
      newVtxData[i]->m_faceAdj.resize(_vtxData[i]->m_faceAdj.size());
    }
    if (i < _triData.size())
    {
      newTriData[i] = _triData[i]->clone();
      // Resize the triangle vert Vector
      newTriData[i]->m_vert.resize(_triData[i]->m_vert.size());
    }
  }

  // Storing the new adjacent vertex and faces
  for (unsigned int i=0; i < _vtxData.size(); ++i)
  {
    // copy over the collapsevertex from the new out Vector
    newVtxData[i]->setCollapseVertex(newVtxData[_vtxData[i]->getCollapseVertex()->getID()]);
    // iterate though the adjacent vertices and triangles for the new cloned for out Vector
    for (unsigned int j=0; j< fmax(_vtxData[i]->m_vertAdj.size(), _vtxData[i]->m_faceAdj.size()); ++j)
    {
      if (j < _vtxData[i]->m_vertAdj.size())
      {
        newVtxData[i]->m_vertAdj[j] = newVtxData[_vtxData[i]->m_vertAdj[j]->getID()];
      }
      if (j < _vtxData[i]->m_faceAdj.size())
      {
        newVtxData[i]->m_faceAdj[j] = newTriData[_vtxData[i]->m_faceAdj[j]->getID()];
      }
    }
  }

  // Storing the new triangle vertices
  for (unsigned int i=0; i < _triData.size(); ++i)
  {
    for (unsigned int j=0; j < _triData[i]->m_vert.size(); ++j)
    {
      newTriData[i]->m_vert[j] = newVtxData[_triData[i]->m_vert[j]->getID()];
    }
  }

  // group the data to be returned as one value in a struct
  vtxTriData returnData;

  returnData.vtxData = newVtxData;
  returnData.triData = newTriData;

  return returnData;
}

//----------------------------------------------------------------------------------------------------------------------
void ModelLODTri::copyVtxTriNormTexDataToOut()
{
  // clear all data currently in the out vectors
  clearVtxTriDataOut();

  vtxTriData outData;
  outData = copyVtxTriData(m_lodVertex, m_lodTriangle);

  m_lodVertexOut = outData.vtxData;
  m_lodTriangleOut = outData.triData;

}
//----------------------------------------------------------------------------------------------------------------------
void ModelLODTri::clearVtxTriDataOut()
{
  // iterate through both vertexOut and triangleOut, deleting the values

  for ( unsigned int i=0; i < m_lodVertexOut.size(); ++i)
  {
    delete(m_lodVertexOut[i]);
  }

  for ( unsigned int i=0; i < m_lodTriangleOut.size(); ++i)
  {
    delete(m_lodTriangleOut[i]);
  }
}
//----------------------------------------------------------------------------------------------------------------------
void ModelLODTri::clearCollapseCostList()
{
  m_lodVertexCollapseCost.clear();
}

//----------------------------------------------------------------------------------------------------------------------
void ModelLODTri::storeCollapseCostList()
{
  clearCollapseCostList();
  for (unsigned int i=0; i<m_lodVertex.size(); ++i)
  {
    m_lodVertexCollapseCost.push_back(m_lodVertexOut[i]);
  }
  updateCollapseCostList();
}
//----------------------------------------------------------------------------------------------------------------------
void ModelLODTri::updateCollapseCostList()
{
  m_lodVertexCollapseCost.sort(compareVertexCost);
}
//----------------------------------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------------------------------------
ModelLODTri *ModelLODTri::createLOD(const unsigned int _nFaces)
{
  m_nDeletedFaces = 0;
  while (m_nDeletedFaces < m_nFaces - _nFaces)
  {
    // get the cheapest vertex from the front of the collapse cost list
    Vertex* cheapestVertex = m_lodVertexCollapseCost.front();
    // store the vertexID to set the pointer in m_lodVertexOut to null after
    int vtxID = cheapestVertex->getID();
    // erase the first value from the collapse cost list
    m_lodVertexCollapseCost.pop_front();
    // collapse the edge from the cheapestVertex to its collapseVertex
    collapseEdge(cheapestVertex, cheapestVertex->getCollapseVertex());
    // re-order the collapse cost list for the new collapse values
    updateCollapseCostList();
    // set the lodVertexOut value to NULL to clear them from the list after
    m_lodVertexOut[vtxID] = NULL;
  }
  // removing nulls from m_lodVertexOut
  for (int i=m_lodVertexOut.size()-1; i>= 0; --i)
  {
    if (!m_lodVertexOut[i])
    {
      m_lodVertexOut.erase(m_lodVertexOut.begin()+i);
    }
  }
  // removing nulls from m_lodTriangleOut
  for (int i=m_lodTriangleOut.size()-1; i>= 0; --i)
  {
    if (!m_lodTriangleOut[i])
    {
      m_lodTriangleOut.erase(m_lodTriangleOut.begin()+i);
    }
  }

  // renumber all the triangles
  for (unsigned int i=0; i<m_lodTriangleOut.size(); ++i)
  {
    m_lodTriangleOut[i]->setID(i);
  }

  // renumber all the vertices
  for (unsigned int i=0; i<m_lodVertexOut.size(); ++i)
  {
    m_lodVertexOut[i]->setID(i);
  }

  // copy the data to a new modelLODTri
  ModelLODTri* newLOD = new ModelLODTri(*this);



  return newLOD;
}

