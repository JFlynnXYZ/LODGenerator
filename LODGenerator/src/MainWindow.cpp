#include "include/MainWindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <iostream>

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

    QString file = fileName.split("/",QString::SkipEmptyParts).back();

    m_ui->pathLE->setText(fileName);
    m_ui->fileNameL->setText(file);
    m_gl->setModelLOD(fileName.toLocal8Bit().constData());
    //m_ui->
  }
}

void MainWindow::on_m_lods_clicked(const QModelIndex &index)
{
  int val = index.column();
  m_gl->m_selectedModel = val-1;

}
