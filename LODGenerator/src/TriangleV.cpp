#include "TriangleV.h"
#include <algorithm>
#include <ngl/Types.h>

#ifndef EPSILON
  const static float  EPSILON = 0.001f;
#endif

//----------------------------------------------------------------------------------------------------------------------
/// @file TriangleV.cpp
/// @brief implementation files for TriangleV class
//----------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------
bool Vertex::operator==( const Vertex &_v )const
{
  return(FCompare(_v.m_id,m_id));
}

//----------------------------------------------------------------------------------------------------------------------
bool Vertex::operator!=( const Vertex &_v )const
{
  return(!FCompare(_v.m_id,m_id));
}

//----------------------------------------------------------------------------------------------------------------------
Vertex& Vertex::operator=( const Vertex &_v )
{
  m_id = _v.m_id;
  return *this;
}

//----------------------------------------------------------------------------------------------------------------------
void Vertex::remAdjFace(Triangle *_t)
{
  m_faceAdj.erase(std::remove(m_faceAdj.begin(), m_faceAdj.end(), _t),
                  m_faceAdj.end());
}

//----------------------------------------------------------------------------------------------------------------------
void Vertex::remAdjVert( Vertex* _v)
{
  m_vertAdj.erase(std::remove(m_vertAdj.begin(), m_vertAdj.end(), _v),
                  m_vertAdj.end());
}

//----------------------------------------------------------------------------------------------------------------------
void Vertex::addAdjFace( Triangle* _t)
{
  std::vector< Triangle *>::iterator it;
  it = std::find(m_faceAdj.begin(), m_faceAdj.end(), _t);
  if (it == m_faceAdj.end())
    m_faceAdj.push_back(_t);
}
//----------------------------------------------------------------------------------------------------------------------
void Vertex::addAdjVert( Vertex* _v)
{
  std::vector< Vertex *>::iterator it;
  it = std::find(m_vertAdj.begin(), m_vertAdj.end(), _v);
  if (it == m_vertAdj.end())
    m_vertAdj.push_back(_v);
}
//----------------------------------------------------------------------------------------------------------------------
bool Vertex::hasAdjVert( Vertex *_v )
{
  std::vector< Vertex *>::iterator it;
  it = std::find(m_vertAdj.begin(), m_vertAdj.end(), _v);
  return (it != m_vertAdj.end());
}
//----------------------------------------------------------------------------------------------------------------------
bool Vertex::hasAdjFace(Triangle *_t)
{
  std::vector< Triangle *>::iterator it;
  it = std::find(m_faceAdj.begin(), m_faceAdj.end(), _t);
  return (it != m_faceAdj.end());
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
bool Triangle::operator==( const Triangle &_t)const
{
  return (_t.m_id == m_id &&
          _t.m_vert[0] == m_vert[0] &&
          _t.m_vert[1] == m_vert[1] &&
          _t.m_vert[2] == m_vert[2]);
}

//----------------------------------------------------------------------------------------------------------------------
bool Triangle::operator!=( const Triangle &_t)const
{
  return (_t.m_id != m_id ||
          _t.m_vert[0] != m_vert[0] ||
          _t.m_vert[1] != m_vert[1] ||
          _t.m_vert[2] != m_vert[2]);
}

//----------------------------------------------------------------------------------------------------------------------
Triangle& Triangle::operator=( Triangle &_t )
{
  m_vert = _t.m_vert;
  m_id = _t.m_id;
  return *this;
}
//----------------------------------------------------------------------------------------------------------------------
void Triangle::calculateNormal( std::vector<ngl::Vec3> _verts )
{
  // create two vectors v and w from the triangle's points
  ngl::Real v[3] = {( _verts[m_vert[1]->getID()].m_x - _verts[m_vert[0]->getID()].m_x ),
                    ( _verts[m_vert[1]->getID()].m_y - _verts[m_vert[0]->getID()].m_y ),
                    ( _verts[m_vert[1]->getID()].m_z - _verts[m_vert[0]->getID()].m_z )};

  ngl::Real w[3] = {( _verts[m_vert[2]->getID()].m_x - _verts[m_vert[0]->getID()].m_x ),
                    ( _verts[m_vert[2]->getID()].m_y - _verts[m_vert[0]->getID()].m_y ),
                    ( _verts[m_vert[2]->getID()].m_z - _verts[m_vert[0]->getID()].m_z )};
  // cross product the two vectors
  m_fNormal.m_x = (v[1]*w[2]) - (v[2]*w[1]);
  m_fNormal.m_y = (v[2]*w[0]) - (v[0]*w[2]);
  m_fNormal.m_z = (v[0]*w[1]) - (v[1]*w[0]);
  // if the length is zero, don't normalize
  if (m_fNormal.length() != 0)
  {
    m_fNormal.normalize();
  }
}
//----------------------------------------------------------------------------------------------------------------------
bool Triangle::hasVert( Vertex *_v )
{
  std::vector< Vertex *>::iterator it;
  it = std::find(m_vert.begin(), m_vert.end(), _v);
  return (it != m_vert.end());
}
//----------------------------------------------------------------------------------------------------------------------
void Triangle::replaceVertex(Vertex *_u, Vertex *_v)
{
  std::vector< Vertex *>::iterator it;
  it = std::find(m_vert.begin(), m_vert.end(), _u);
  if (it != m_vert.end())
  {
    m_vert[std::distance(m_vert.begin(), it)] = _v;
  }
}
