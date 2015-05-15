/****************************************************************************
basic OpenGL demo modified from http://qt-project.org/doc/qt-5.0/qtgui/openglwindow.html
****************************************************************************/
#include <QApplication>
#include <iostream>
#include "MainWindow.h"



int main(int argc, char **argv)
{
  QApplication app(argc, argv);
  // now we are going to create our scene window
  MainWindow window;

  // and finally show
  window.show();


  return app.exec();
}



