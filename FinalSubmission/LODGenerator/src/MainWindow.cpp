#include "include/MainWindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <iostream>

#include <sstream>

#define SSTR( x ) dynamic_cast< std::ostringstream & >( \
        ( std::ostringstream() << std::dec << x ) ).str()

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  m_ui(new Ui::MainWindow)
{
  m_ui->setupUi(this);

  QGLFormat format;
  // set the number of samples for multisampling
  // will need to enable glEnable(GL_MULTISAMPLE); once we have a context
  format.setSamples(4);
  #if defined( DARWIN)
    // at present mac osx Mountain Lion only supports GL3.2
    // the new mavericks will have GL 4.x so can change
    format.setVersion(3,2);
  #else
    // with luck we have the latest GL version so set to this
    format.setVersion(4,5);
  #endif
  // now we are going to set to CoreProfile OpenGL so we can't use and old Immediate mode GL
  format.setProfile(QGLFormat::CoreProfile);
  // now set the depth buffer to 24 bits
  format.setDepthBufferSize(24);

  m_gl=new GLWindow(format, this);
  m_ui->s_mainWindowGridLayout->addWidget(m_gl,0,0,2,1);

  connect(m_ui->m_wireframe,SIGNAL(toggled(bool)),m_gl,SLOT(toggleWireframe(bool)));

}

MainWindow::~MainWindow()
{
  delete m_ui;
}

void MainWindow::on_loadB_clicked()
{
  QFileDialog dialog(this);
  dialog.setFileMode(QFileDialog::ExistingFile);
  QStringList filters;
  filters << tr("Obj (*.obj)") << tr("All Files (*)");
  dialog.setNameFilters(filters);
  dialog.setViewMode(QFileDialog::Detail);
  QStringList fileNames;
  if(dialog.exec())
  {
    fileNames = dialog.selectedFiles();
  }
  if (fileNames.size() > 0)
  {

    QString fileName = fileNames.at(0);
    std::cout << fileName.toLocal8Bit().constData() <<std::endl;
    std::cout << fileName.size()-fileName.lastIndexOf("/") << std::endl;

    QString file = fileName.split("/",QString::SkipEmptyParts).back().split(".",QString::SkipEmptyParts).front();
    QStringList fileNameFull = fileName.split("/",QString::SkipEmptyParts);

    QString path = "/";

    for (int i = 0; i < fileNameFull.size()-1; ++i)
    {
      path.append(fileNameFull.at(i));
      path.append("/");
    }

    std::cout<<path.toLocal8Bit().constData()<<std::endl;
    m_gl->m_lods.clear();
    m_ui->pathLE->setText(fileName);
    m_ui->fileNameL->setText(file);
    m_gl->filepath = path.toLocal8Bit().constData();
    m_gl->file = file.toLocal8Bit().constData();
    m_gl->setModelLOD(fileName.toLocal8Bit().constData());
    m_ui->nFaces->setMaximum(m_gl->getModelLODTri()->getNumFaces()-1);
    m_ui->nFaces->setValue(m_gl->getModelLODTri()->getNumFaces()-1);
    m_ui->m_lods->setCurrentRow(m_gl->m_selectedModel);
    m_gl->updateAllLODs();

  }
}

void MainWindow::on_m_lods_clicked(const QModelIndex &index)
{
  std::string str = index.data().toString().toLocal8Bit().constData();
  if (str == "BaseMesh")
  {
    if (m_gl->getModelLODTri() != NULL)
    {
      m_ui->nFaces->setMaximum(m_gl->getModelLODTri()->getNumFaces()-1);
      m_ui->nFaces->setValue(m_gl->getModelLODTri()->getNumFaces()-1);
    }
  }
  else
    {

    m_ui->nFaces->setMaximum(m_gl->getLODs()[m_gl->m_selectedModel]->getNumFaces()-1);
    m_ui->nFaces->setValue(m_gl->getLODs()[m_gl->m_selectedModel]->getNumFaces()-1);
    }
  m_gl->m_selectedModel = m_ui->m_lods->currentRow();
  m_gl->extUpdateGL();

}

void MainWindow::on_deleteLODB_clicked()
{
  if (m_ui->m_lods->currentRow() != 0)
  {
    //delete(m_gl->m_lods[m_gl->m_selectedModel]);
    m_ui->m_lods->takeItem(m_ui->m_lods->currentRow());
    m_gl->m_selectedModel = 0;
    m_ui->m_lods->setCurrentRow(0);
    m_gl->updateAllLODs();
    m_gl->extUpdateGL();
  }
}

void MainWindow::on_createLODB_clicked()
{
  m_gl->createLOD(m_ui->nFaces->value());
  QString id = SSTR(m_gl->getLODs().size()).c_str();
  m_gl->updateAllLODs();
  m_ui->m_lods->addItem(id);
  m_ui->m_lods->setCurrentRow(m_gl->allModels.size()-1);
  m_gl->extUpdateGL();
}

void MainWindow::on_exportAllB_clicked()
{
    m_gl->exportAllLOD();
}

void MainWindow::on_exportLODB_clicked()
{
    m_gl->exportLOD(m_ui->m_lods->currentRow());
}
