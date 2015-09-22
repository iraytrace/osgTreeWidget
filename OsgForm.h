#ifndef OSGFORM_H
#define OSGFORM_H

#include <QWidget>
#include <QFutureWatcher>
#include <osg/Group>
#include "ViewingCore.h"
namespace Ui {
class OsgForm;
}

class OsgForm : public QWidget
{
    Q_OBJECT

public:
    explicit OsgForm(QWidget *parent = 0);
    ~OsgForm();


public slots:
    void openFile(const QString fileName);
    bool saveFile(const QString fileName);

private slots:
    void readNodesFinished();

private:
    Ui::OsgForm *ui;

    osg::ref_ptr<osg::Group> m_root;
    osg::ref_ptr<ViewingCore> m_viewingCore;
    osg::ref_ptr<osg::Node> readNodes(const QString fileName);

    QFutureWatcher< osg::ref_ptr<osg::Node> >m_watcher;
};

#endif // OSGFORM_H
