/*
  Copyright (C) 2014 Jonathan Flynn

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef ModelLOD_H_
#define ModelLOD_H_
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
#include <ngl/Vec4.h>
#include <ngl/AbstractMesh.h>
#include <ngl/BBox.h>
#include <ngl/RibExport.h>
#include <cmath>

namespace ngl
{
//----------------------------------------------------------------------------------------------------------------------
/// @class ModelLOD "include/ModelLOD.h"
/// @brief used to store vertex information from an imported model and used as 
///		as a means of creating an LOD model.
/// modified version of the Obj class from the NGL library
/// @author Jonathan Flynn
/// @version 0.1
/// @date 02/03/15 imported code from Obj.h
//----------------------------------------------------------------------------------------------------------------------

class NGL_DLLEXPORT ModelLOD : public AbstractMesh
{

public :

  //----------------------------------------------------------------------------------------------------------------------
  /// @brief default constructor
  //----------------------------------------------------------------------------------------------------------------------
  ModelLOD() : AbstractMesh(){;}
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief  constructor to load an objfile as a parameter
  /// @param[in]  &_fname the name of the obj file to load
  //----------------------------------------------------------------------------------------------------------------------
  ModelLOD( const std::string& _fname );
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief constructor to load an objfile as a parameter
  /// @param[in]  &_fname the name of the obj file to load
  /// @param[in]  &_texName the name of the texture file
  //----------------------------------------------------------------------------------------------------------------------
  ModelLOD( const std::string& _fname,  const std::string& _texName );
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief  Method to load the file in
  /// @param[in]  _fname the name of the obj file to load
  /// @param[in] _calcBB if we only want to load data and not use GL then set this to false
  //----------------------------------------------------------------------------------------------------------------------
  bool load(const std::string& _fname, bool _calcBB=true );
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief  method to save the obj
  /// @param[in] _fname the name of the file to save
  //----------------------------------------------------------------------------------------------------------------------
  void save( const std::string& _fname  ) const;

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

};

}


#endif
//----------------------------------------------------------------------------------------------------------------------

