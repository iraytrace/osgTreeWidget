#ifndef VSLAPP_H
#define VSLAPP_H

#include <QString>
class QWidget;
class QFileInfo;
class QMainWindow;

class VSLapp
{

public:
    static void applicationSetup(const char *organizationName);
    static QString getApplicationDir();
    static void showAboutDialog(QWidget *parent);
    static void mainWindowSetup(QMainWindow *mw);
    static void mainWindowSave(QMainWindow *mw);
private:
    VSLapp() {}
    static void logApplicationLaunch(QFileInfo appFile);
};

#endif // VSLAPP_H


