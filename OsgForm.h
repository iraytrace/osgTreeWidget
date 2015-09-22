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

signals:
    void showMessage(QString);
public slots:
    void openFile(const QString fileName);
    bool saveFile(const QString fileName);
    void addNode(osg::ref_ptr<osg::Node> n);

protected slots:
    void wrieNodesFinished();
private slots:
    void readNodesFinished();

private:
    Ui::OsgForm *ui;

    osg::ref_ptr<osg::Group> m_root;
    osg::ref_ptr<ViewingCore> m_viewingCore;
    osg::ref_ptr<osg::Node> readNodes(const QString fileName);

    QFutureWatcher< osg::ref_ptr<osg::Node> >m_loadWatcher;
    QFutureWatcher< bool >m_saveWatcher;
    void setProgressBarState(bool turnOn);
    bool saveThread(osg::ref_ptr<osg::Node> node, const QString fileName);
    QCursor m_stashedCursor;

};

#endif // OSGFORM_H
