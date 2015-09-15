#ifndef OSGTREEWIDGET_H
#define OSGTREEWIDGET_H

#include <QTreeWidget>
#include <osg/Node>

class OsgTreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    OsgTreeWidget(QWidget * parent = 0);

    void addObject(osg::Object *object, QTreeWidgetItem *parentItem = (QTreeWidgetItem *)0);

signals:
    void currentObject(osg::ref_ptr<osg::Object> object);

public slots:
    void resizeAllColumns();
private slots:
    void currentItemWasChanged(QTreeWidgetItem * current, QTreeWidgetItem *);
private:

};

#endif // OSGTREEWIDGET_H
