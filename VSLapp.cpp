#include "VSLapp.h"
#include <QLockFile>
#include <QDir>
#include <QApplication>
#include <QSettings>
#include <QDateTime>
#include <QMessageBox>

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
    QString message = QString("%1\n")
            .arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmss"));
#ifdef __unix
    message.append(getuid());
#endif

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

void VSLapp::showAboutDialog(QWidget *parent)
{
    QMessageBox msgBox(QMessageBox::Information,
                       QString("About %1").arg(qApp->applicationName()),
                       QString("This is <b>%1</b> from <br><b>%2</b>")
                       .arg(qApp->applicationName())
                       .arg(qApp->organizationName()),
                       QMessageBox::Ok,
                       parent,
                       Qt::Dialog);

    msgBox.setInformativeText(QString("Built %1").arg(__DATE__ " " __TIME__));
    msgBox.setDetailedText(QString("Binary:%1").arg(VSLapp::getApplicationDir()));

    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.exec();
}



