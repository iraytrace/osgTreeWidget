#ifndef VSLAPP_H
#define VSLAPP_H

#include <QString>
class QWidget;
class QFileInfo;

class VSLapp
{

public:
    static void applicationSetup(const char *organizationName);
    static QString getApplicationDir();
    static void showAboutDialog(QWidget *parent);
private:
    VSLapp() {}
    static void logApplicationLaunch(QFileInfo appFile);
};

#endif // VSLAPP_H

