#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "RecentFiles.h"

namespace Ui {
class MainWindow;
}
class QCloseEvent;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    /// When the user clicks File->Open ... we end up here
    void on_actionFileOpen_triggered();

    ///
    void on_actionFileSave_triggered();
    void on_actionFileSaveAs_triggered();
    /// construct/display a reasonable About dialog
    void on_actionHelpAbout_triggered();

    /// Called to clean up when the window is about to close
    void closeEvent(QCloseEvent *event);
private slots:
    void loadFile(QString filename);

private:
    bool shouldAbortClose();

    Ui::MainWindow *ui;
    RecentFiles m_recentMenu;
};

#endif // MAINWINDOW_H




