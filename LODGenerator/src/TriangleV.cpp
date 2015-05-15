#include "TriangleV.h"
#include <algorithm>
#include <ngl/Types.h>
#include <assert.h>

#ifndef EPSILON
  const static float  EPSILON = 0.001f;
#endif

//----------------------------------------------------------------------------------------------------------------------
/// @file TriangleV.cpp
/// @brief implementation files for TriangleV class
//----------------------------------------------------------------------------------------------------------------------

Vertex::~Vertex()
{
  for (unsigned int i=0; i<m_vertAdj.size(); ++i)
  {
    m_vertAdj[i]->m_vertAdj.erase(std::remove(m_vertAdj[i]->m_vertAdj.begin(), m_vertAdj[i]->m_vertAdj.end(), this),
                                  m_vertAdj[i]->m_vertAdj.end());
  }
}

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
void Vertex::remIfNonNeighbour( Vertex* _v )
{
  if(std::find(m_vertAdj.begin(), m_vertAdj.end(), _v) == m_vertAdj.end()) return;

  for (unsigned int i=0; i<m_faceAdj.size(); ++i)
  {
    if (m_faceAdj[i]->hasVert(_v)) return;

  }
  m_vertAdj.erase(std::remove(m_vertAdj.begin(), m_vertAdj.end(), _v),
                  m_vertAdj.end());
}

//----------------------------------------------------------------------------------------------------------------------
bool Vertex::hasAdjVert( Vertex *_v )
{
  std::vector< Vertex *>::iterator it;
  it = std::find(m_vertAdj.begin(), m_vertAdj.end(), _v);
  return (it != m_vertAdj.end());
}
//----------------------------------------------------------------------------------------------------------------------
std::vector<Triangle *>::iterator Vertex::findAdjFace(Triangle *_t)
{
  std::vector<Triangle *>::iterator it;
  it = std::find(m_faceAdj.begin(), m_faceAdj.end(), _t);
  return it;
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
Triangle::~Triangle()
{
  for (unsigned int i=0; i<3; ++i)
  {
    if (m_vert[i])
    {
      m_vert[i]->remAdjFace(this);
    }
  }
  for (unsigned int i=0; i<3; ++i)
  {
    int i2 = (i+1)%3;
    if(!m_vert[i] || !m_vert[i2]) continue;

    m_vert[i]->remIfNonNeighbour(m_vert[i2]);
    m_vert[i2]->remIfNonNeighbour(m_vert[i]);
  }
}

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
void Triangle::setNormID(int _id, int _pos)
{
  if (m_normID.size()+1 > _pos)
  {
    m_normID.push_back(_id);
  }
  else
  {
    m_normID[_pos] = _id;
  }
}
//----------------------------------------------------------------------------------------------------------------------
void Triangle::setTexID(int _id, unsigned int _pos)
{
  if (m_texID.size()+1 > _pos)
  {
    m_texID.push_back(_id);
  }
  else
  {
    m_texID[_pos] = _id;
  }
}
//----------------------------------------------------------------------------------------------------------------------
void Triangle::calculateNormal()
{
  // create two vectors v and w from the triangle's points
  ngl::Real v[3] = {( m_vert[1]->m_vert.m_x - m_vert[0]->m_vert.m_x ),
                    ( m_vert[1]->m_vert.m_y - m_vert[0]->m_vert.m_y ),
                    ( m_vert[1]->m_vert.m_z - m_vert[0]->m_vert.m_z )};

  ngl::Real w[3] = {( m_vert[2]->m_vert.m_x - m_vert[0]->m_vert.m_x ),
                    ( m_vert[2]->m_vert.m_y - m_vert[0]->m_vert.m_y ),
                    ( m_vert[2]->m_vert.m_z - m_vert[0]->m_vert.m_z )};
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
  assert(_u && _v);
  assert(_u == m_vert[0] || _u == m_vert[1] || _u == m_vert[2]);
  assert(_v != m_vert[0] && _v != m_vert[1] && _v != m_vert[2]);

  std::vector< Vertex *>::iterator it;
  it = std::find(m_vert.begin(), m_vert.end(), _u);
  if (it != m_vert.end())
  {
    m_vert[std::distance(m_vert.begin(), it)] = _v;
  }
  _u->remAdjFace(this);
  assert(!_v->hasAdjFace(this));
  _v->addAdjFace(this);
  for (unsigned int i=0; i<3; i++)
  {
    _u->remIfNonNeighbour(m_vert[i]);
    m_vert[i]->remIfNonNeighbour(_u);
  }

  for (unsigned int i=0; i<3; i++)
  {
    assert(m_vert[i]->hasAdjFace(this)==1);
    for(unsigned int j=0; j<3; j++)
    {
      if (i != j &&
          std::find(m_vert.begin(), m_vert.end(), _v) != m_vert.end() &&
          !m_vert[i]->hasAdjVert(m_vert[j]))
      {
          m_vert[i]->m_vertAdj.push_back(m_vert[j]);
      }
    }
  }
  calculateNormal();
}

