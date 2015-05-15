#include "GLWindow.h"
#include <iostream>
#include <ngl/Vec3.h>
#include <ngl/Light.h>
#include <ngl/NGLInit.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/Material.h>
#include <ngl/ShaderLib.h>
#include <QColorDialog>
#include <QFileDialog>
#include "include/MainWindow.h"
#include "ui_mainwindow.h"

#include <sstream>

#define SSTR( x ) dynamic_cast< std::ostringstream & >( \
        ( std::ostringstream() << std::dec << x ) ).str()

//----------------------------------------------------------------------------------------------------------------------
/// @brief the increment for x/y translation with mouse movement
//----------------------------------------------------------------------------------------------------------------------
const static float INCREMENT=0.01f;
//----------------------------------------------------------------------------------------------------------------------
/// @brief the increment for the wheel zoom
//----------------------------------------------------------------------------------------------------------------------
const static float ZOOM=0.1f;


//----------------------------------------------------------------------------------------------------------------------
GLWindow::GLWindow(const QGLFormat _format, QWidget *_parent ) : QGLWidget( _format, _parent )
{

  // set this widget to have the initial keyboard focus
  setFocus();
  // re-size the widget to that of the parent (in this case the GLFrame passed in on construction)
  this->resize(_parent->size());
	m_wireframe=false;
	m_rotation=0.0;
	m_scale=1.0;
	m_position=0.0;

	m_selectedObject=0;


  m_spinXFace=0.0f;
  m_spinYFace=0.0f;
}

// This virtual function is called once before the first call to paintGL() or resizeGL(),
//and then once whenever the widget has been assigned a new QGLContext.
// This function should set up any required OpenGL context rendering flags, defining display lists, etc.

//----------------------------------------------------------------------------------------------------------------------
void GLWindow::initializeGL()
{

  ngl::NGLInit::instance();
  glClearColor(0.4f, 0.4f, 0.4f, 1.0f);			   // Grey Background
  // enable depth testing for drawing
  glEnable(GL_DEPTH_TEST);
  // enable multisampling for smoother drawing
  glEnable(GL_MULTISAMPLE);
  /// create our camera
  ngl::Vec3 eye(0,4,4);
  ngl::Vec3 look(0,0,0);
  ngl::Vec3 up(0,1,0);

  m_camera = new ngl::Camera(eye,look,up);
  m_camera->setShape(45,float(1024/720),0.1,300);
  // now to load the shader and set the values
  // grab an instance of shader manager
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  // we are creating a shader called Phong
  shader->createShaderProgram("Phong");
  // now we are going to create empty shaders for Frag and Vert
  shader->attachShader("PhongVertex",ngl::VERTEX);
  shader->attachShader("PhongFragment",ngl::FRAGMENT);
  // attach the source
  shader->loadShaderSource("PhongVertex","shaders/PhongVertex.glsl");
  shader->loadShaderSource("PhongFragment","shaders/PhongFragment.glsl");
  // compile the shaders
  shader->compileShader("PhongVertex");
  shader->compileShader("PhongFragment");
  // add them to the program
  shader->attachShaderToProgram("Phong","PhongVertex");
  shader->attachShaderToProgram("Phong","PhongFragment");
  // now bind the shader attributes for most NGL primitives we use the following
  // layout attribute 0 is the vertex data (x,y,z)
  shader->bindAttribute("Phong",0,"inVert");
  // attribute 1 is the UV data u,v (if present)
  shader->bindAttribute("Phong",1,"inUV");
  // attribute 2 are the normals x,y,z
  shader->bindAttribute("Phong",2,"inNormal");

  // now we have associated this data we can link the shader
  shader->linkProgramObject("Phong");
  // and make it active ready to load values
  (*shader)["Phong"]->use();
  shader->setShaderParam1i("Normalize",1);
  shader->setShaderParam3f("viewerPos",m_camera->getEye().m_x,m_camera->getEye().m_y,m_camera->getEye().m_z);
  // now pass the modelView and projection values to the shader
  // the shader will use the currently active material and light0 so set them
  ngl::Material m(ngl::POLISHEDSILVER);
  m.loadToShader("material");
  // we need to set a base colour as the material isn't being used for all the params
  shader->setShaderParam4f("Colour",0.23125f,0.23125f,0.23125f,1);

  ngl::Light light(ngl::Vec3(2,2,2),ngl::Colour(1,1,1,1),ngl::Colour(1,1,1,1),ngl::POINTLIGHT);
  // now create our light this is done after the camera so we can pass the
  // transpose of the projection matrix to the light to do correct eye space
  // transformations
  ngl::Mat4 iv=m_camera->getViewMatrix();
  iv.transpose();
  light.setTransform(iv);
  light.setAttenuation(1,0,0);
  light.enable();
  // load these values to the shader as well
  light.loadToShader("light");

  ngl::VAOPrimitives *prim=ngl::VAOPrimitives::instance();
  prim->createSphere("sphere",1.0,40);
  m_text = new  ngl::Text(QFont("Arial",18));
  m_text->setScreenSize(this->size().width(),this->size().height());

  m_modelLOD = NULL;
  m_selectedModel = 0;

  glViewport(0,0,width()*devicePixelRatio(),height()*devicePixelRatio());
}

//----------------------------------------------------------------------------------------------------------------------
//This virtual function is called whenever the widget has been resized.
// The new size is passed in width and height.
void GLWindow::resizeGL( int _w, int _h )
{
  glViewport(0,0,_w,_h);
  m_camera->setShape(45,float(_w/_h),0.1,300);
  m_text->setScreenSize(this->size().width(),this->size().height());

}


void GLWindow::loadMatricesToShader()
{
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();

  ngl::Mat4 MV;
  ngl::Mat4 MVP;
  ngl::Mat3 normalMatrix;
  ngl::Mat4 M;
  M=m_mouseGlobalTX;
  MV=M*m_camera->getViewMatrix();
  MVP=M*m_camera->getVPMatrix() ;
  normalMatrix=MV;
  normalMatrix.inverse();
  shader->setShaderParamFromMat4("MV",MV);
  shader->setShaderParamFromMat4("MVP",MVP);
  shader->setShaderParamFromMat3("normalMatrix",normalMatrix);
  shader->setShaderParamFromMat4("M",M);
}

void GLWindow::exportLOD(int _id)
{
  if (_id == 0)
    return;
  QFileDialog dialog;
  dialog.setFileMode(QFileDialog::AnyFile);
  QStringList filters;
  filters << "Obj (*.obj)";
  dialog.setNameFilters(filters);
  dialog.setViewMode(QFileDialog::Detail);
  QStringList fileNames;
  if(dialog.exec())
  {
    fileNames = dialog.selectedFiles();
  }
  QString fileName = fileNames.at(0);
  m_lods[_id-1]->save(fileName.toLocal8Bit().constData());
}

//----------------------------------------------------------------------------------------------------------------------
//This virtual function is called whenever the widget needs to be painted.
// this is our main drawing routine
void GLWindow::paintGL()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  ngl::VAOPrimitives *prim=ngl::VAOPrimitives::instance();
	if(m_wireframe == true)
	{
		glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	}
	else
	{
		glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	}
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  (*shader)["Phong"]->use();

	m_transform.setPosition(m_position);
	m_transform.setScale(m_scale);
	m_transform.setRotation(m_rotation);


  // Rotation based on the mouse position for our global transform
  ngl::Mat4 rotX;
  ngl::Mat4 rotY;
  // create the rotation matrices
  rotX.rotateX(m_spinXFace);
  rotY.rotateY(m_spinYFace);
  // multiply the rotations
  m_mouseGlobalTX=rotY*rotX;
  // add the translations
  m_mouseGlobalTX.m_m[3][0] = m_modelPos.m_x;
  m_mouseGlobalTX.m_m[3][1] = m_modelPos.m_y;
  m_mouseGlobalTX.m_m[3][2] = m_modelPos.m_z;
  loadMatricesToShader();

  if ( allModels.size() != 0 && allModels[m_selectedModel] != NULL )
    allModels[m_selectedModel]->draw();

  m_text->renderText(10,10,"LODGenerator");
}


void GLWindow::updateAllLODs()
{
  allModels.clear();
  allModels.push_back(m_modelLOD);
  for (int i=0; i<m_lods.size(); ++i)
    allModels.push_back(m_lods[i]);
}


//----------------------------------------------------------------------------------------------------------------------
void GLWindow::mouseMoveEvent ( QMouseEvent * _event )
{
  // note the method buttons() is the button state when event was called
  // this is different from button() which is used to check which button was
  // pressed when the mousePress/Release event is generated
  if(m_rotate && _event->buttons() == Qt::LeftButton)
  {
    int diffx=_event->x()-m_origX;
    int diffy=_event->y()-m_origY;
    m_spinXFace += (float) 0.5f * diffy;
    m_spinYFace += (float) 0.5f * diffx;
    m_origX = _event->x();
    m_origY = _event->y();
    updateGL();

  }
        // right mouse translate code
  else if(m_translate && _event->buttons() == Qt::RightButton)
  {
    int diffX = (int)(_event->x() - m_origXPos);
    int diffY = (int)(_event->y() - m_origYPos);
    m_origXPos=_event->x();
    m_origYPos=_event->y();
    m_modelPos.m_x += INCREMENT * diffX;
    m_modelPos.m_y -= INCREMENT * diffY;
    updateGL();

   }
}


//----------------------------------------------------------------------------------------------------------------------
void GLWindow::mousePressEvent (QMouseEvent * _event  )
{
  // this method is called when the mouse button is pressed in this case we
  // store the value where the maouse was clicked (x,y) and set the Rotate flag to true
  if(_event->button() == Qt::LeftButton)
  {
    m_origX = _event->x();
    m_origY = _event->y();
    m_rotate =true;
  }
  // right mouse translate mode
  else if(_event->button() == Qt::RightButton)
  {
    m_origXPos = _event->x();
    m_origYPos = _event->y();
    m_translate=true;
  }


}

//----------------------------------------------------------------------------------------------------------------------
void GLWindow::mouseReleaseEvent (  QMouseEvent * _event  )
{
  // this event is called when the mouse button is released
  // we then set Rotate to false
  if (_event->button() == Qt::LeftButton)
  {
    m_rotate=false;
  }
        // right mouse translate mode
  if (_event->button() == Qt::RightButton)
  {
    m_translate=false;
  }

}
//----------------------------------------------------------------------------------------------------------------------
void GLWindow::wheelEvent(QWheelEvent *_event)
{

  // check the diff of the wheel position (0 means no change)
  if(_event->delta() > 0)
  {
    m_modelPos.m_z+=ZOOM;
  }
  else if(_event->delta() <0 )
  {
    m_modelPos.m_z-=ZOOM;
  }
  updateGL();
}


GLWindow::~GLWindow()
{
}

void GLWindow::toggleWireframe(bool _mode	 )
{
	m_wireframe=_mode;
	updateGL();
}

void GLWindow::setXRotation( double _x		)
{
	m_rotation.m_x=_x;
	updateGL();
}

void GLWindow::setYRotation( double _y	)
{
	m_rotation.m_y=_y;
	updateGL();
}
void GLWindow::setZRotation( double _z )
{
	m_rotation.m_z=_z;
	updateGL();
}

void GLWindow::setXScale( double _x	)
{
	m_scale.m_x=_x;
	updateGL();
}

void GLWindow::setYScale(	 double _y)
{
	m_scale.m_y=_y;
	updateGL();
}
void GLWindow::setZScale( double _z )
{
	m_scale.m_z=_z;
	updateGL();
}

void GLWindow::setXPosition( double _x	)
{
	m_position.m_x=_x;
	updateGL();
}

void GLWindow::setYPosition( double _y	)
{
	m_position.m_y=_y;
	updateGL();
}
void GLWindow::setZPosition( double _z	 )
{
	m_position.m_z=_z;
	updateGL();
}

void GLWindow::setObjectMode(	int _i)
{
	m_selectedObject=_i;
	updateGL();
}
void GLWindow::setColour()
{
	QColor colour = QColorDialog::getColor();
	if( colour.isValid())
	{
    ngl::ShaderLib *shader=ngl::ShaderLib::instance();
    (*shader)["Phong"]->use();
    shader->setShaderParam4f("Colour",colour.redF(),colour.greenF(),colour.blueF(),1.0);
		updateGL();
	}
}

void GLWindow::setModelLOD(const std::string _fname)
{
//  if (m_modelLOD!=NULL )
//  {
//    delete(m_modelLOD);
//  }
  m_modelLOD = new ModelLODTri(_fname);
  m_modelLOD->createVAO();
  updateGL();
}

void GLWindow::createLOD(unsigned int _nFaces)
{
  m_lods.push_back(m_modelLOD->createLOD(_nFaces));
}

void GLWindow::exportAllLOD()
{
  for (int i=0; i<m_lods.size(); ++i)
  {
    std::string name = file;
    name.append("_");
    name.append(SSTR(i));
    name.append(".obj");
    std::string path = filepath;
    path.append("/"+name+"_LODS/");
    path.append(name);
    m_lods[i]->save(path);
  }
}
