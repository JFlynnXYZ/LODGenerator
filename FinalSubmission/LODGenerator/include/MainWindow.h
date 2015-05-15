#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "GLWindow.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

private slots:
  void on_loadB_clicked();

  void on_m_lods_clicked(const QModelIndex &index);

  void on_deleteLODB_clicked();

  void on_createLODB_clicked();

  void on_exportAllB_clicked();

  void on_exportLODB_clicked();

private:
  Ui::MainWindow *m_ui;
  /// @brief our openGL widget

  GLWindow *m_gl;
};

#endif // MAINWINDOW_H
