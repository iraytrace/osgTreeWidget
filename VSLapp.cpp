#include "VSLapp.h"
#include <QLockFile>
#include <QDir>
#include <QApplication>
#include <QSettings>
#include <QDateTime>
#include <QMessageBox>
#include <QMainWindow>
#include <QTextStream>
#ifdef __unix
#include <unistd.h>
#include <sys/types.h>
#endif

void VSLapp::applicationSetup(const char *organizationName)
{
    // set up application name
    QFileInfo applicationFile(QApplication::applicationFilePath());

    // These allow us to simply construct a "QSettings" object without arguments
    qApp->setOrganizationDomain("mil.army.arl");
    qApp->setApplicationName(applicationFile.baseName());
    qApp->setOrganizationName(organizationName);
    qApp->setApplicationVersion(__DATE__ __TIME__);

    // Look up the last directory where the user opened files.
    // set it if it hasn't been set.
    QSettings settings;
    if (!settings.allKeys().contains("app/currentDirectory"))
        settings.setValue("app/currentDirectory", applicationFile.absoluteDir().absolutePath());

    // log this launch of the program to track usage.
    logApplicationLaunch(applicationFile); // comes after settingsRead so we can set a preference
}

void VSLapp::logApplicationLaunch(QFileInfo appFile)
{
    QSettings settings;

    settings.value("logUsage", QVariant(true));

    qApp->applicationDirPath();

    // obtain the string we will log to the file
    QString message =
            QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
#ifdef __unix
    message.append(getuid());
#endif
    message.append("\n");

    QString nameOfLogFile = QString("%1%2").arg(appFile.absoluteFilePath()).arg(".log");

    QString nameOfLockFile = QString("%1%2").arg(nameOfLogFile).arg(".lck");
    QLockFile lockFile(nameOfLockFile);

    if (lockFile.tryLock(1000)) {
        // got the lock write the data
        QFile usageLogFile(nameOfLogFile);
        if (usageLogFile.open(QIODevice::Append)) {
            usageLogFile.write(message.toStdString().c_str());
            usageLogFile.close();
        }
        lockFile.unlock();
    }
}

QString VSLapp::getApplicationDir()
{
    QDir applicationDir(qApp->applicationDirPath());

#if defined(Q_OS_WIN)
    if (applicationDir.dirName().toLower() == "debug" ||
            applicationDir.dirName().toLower() == "release"
        #  ifdef CMAKE_INTDIR
            || applicationDir.dirName() == CMAKE_INTDIR
        #  endif
            )
        applicationDir.cdUp();
#elif defined(Q_OS_MAC)
    if (applicationDir.dirName() == "MacOS") {
        applicationDir.cdUp();
        applicationDir.cdUp();
        applicationDir.cdUp();
    }
#endif

    return applicationDir.absolutePath();
}

#include <QTextEdit>
class QMessageBoxResize: public QMessageBox
{
public:
    QMessageBoxResize(QWidget *parent = 0) : QMessageBox(parent) {
        setMouseTracking(true);
        setSizeGripEnabled(true);
    }
private:
    virtual bool event(QEvent *e) {
        bool res = QMessageBox::event(e);
        switch (e->type()) {
        case QEvent::MouseMove:
        case QEvent::MouseButtonPress:
            setSizeGripEnabled(true);
            setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
            if (QWidget *textEdit = findChild<QTextEdit *>()) {
                textEdit->setMaximumHeight(QWIDGETSIZE_MAX);
            }
        default:
            break;
        }
        return res;
    }
};
void VSLapp::showAboutDialog(QWidget *parent)
{

    QMessageBoxResize msgBox(parent);
    QString message;
    QTextStream stream(&message);
    stream <<  "Built "  __DATE__ " " __TIME__ << endl
            << "By " BUILT_BY_USER " on " BUILT_ON_MACHINE << endl;

    msgBox.setInformativeText(message);    msgBox.setDetailedText(QString("Binary:%1").arg(VSLapp::getApplicationDir()));
    msgBox.setWindowTitle(QString("About %1").arg(qApp->applicationName()));
    msgBox.setText(QString("This is <b>%1</b> from <br><b>%2</b>")
                   .arg(qApp->applicationName())
                   .arg(qApp->organizationName()));
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.exec();

}

#define MainWindowGeometry "MainWindow/geometry"
#define MainWindowDoRestore "MainWindow/restore"
#define UI_VERSION 1
void VSLapp::mainWindowSetup(QMainWindow *mw)
{
    mw->setWindowTitle(qApp->applicationName());

    // set the geometry
    QSettings settings;
    if (settings.allKeys().contains(MainWindowGeometry)) {
        mw->setGeometry(settings.value(MainWindowGeometry).toRect());
    }
}

void VSLapp::mainWindowSave(QMainWindow *mw)
{
    // stash things that we will want on startup.
    QSettings settings;
    settings.setValue(MainWindowGeometry, mw->geometry());
}





