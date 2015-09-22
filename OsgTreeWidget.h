#ifndef OSGTREEWIDGET_H
#define OSGTREEWIDGET_H

#include <QTreeWidget>
#include <osg/Node>
#include <QFutureWatcher>

class OsgTreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    OsgTreeWidget(QWidget * parent = 0);

    void addObject(osg::Object *object);

signals:
    void currentObject(osg::ref_ptr<osg::Object> object);

public slots:
    void resizeAllColumns();
private slots:
    void currentItemWasChanged(QTreeWidgetItem * current, QTreeWidgetItem *);
    void addObjectFinished();

private:
    QFutureWatcher< QTreeWidgetItem * >m_watcher;
    QTreeWidgetItem *addObjectItem(osg::Object *object, const QString parentName);
    QCursor m_stashedCursor;
};

#endif // OSGTREEWIDGET_H
