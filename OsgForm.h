#ifndef OSGFORM_H
#define OSGFORM_H

#include <QWidget>
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

private:
    Ui::OsgForm *ui;

    osg::ref_ptr<osg::Group> m_root;
    osg::ref_ptr<ViewingCore> m_viewingCore;
};

#endif // OSGFORM_H
