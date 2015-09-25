#ifndef OSGTREEWIDGET_H
#define OSGTREEWIDGET_H

#include <QTreeWidget>
#include <osg/Node>
#include <QFutureWatcher>
#include <QMenu>

class OsgTreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    OsgTreeWidget(QWidget * parent = 0);

    void addObject(osg::Object *object);
    bool itemMatchesObject(QTreeWidgetItem *i,
                           osg::ref_ptr<osg::Object> obj);
    void lookForMatch(std::vector<QTreeWidgetItem *> &matchingItems,
                      osg::ref_ptr<osg::Object> obj,
                        QTreeWidgetItem *item = 0);

    QTreeWidgetItem *childThatMatches(QTreeWidgetItem *top,
            osg::ref_ptr<osg::Object> obj);

signals:
    void currentObject(osg::ref_ptr<osg::Object> object);

public slots:
    void resizeAllColumns();
    void customMenuRequested(const QPoint &pos);

private slots:
    void currentItemWasChanged(QTreeWidgetItem * current, QTreeWidgetItem *);
    void addObjectFinished();
    void insertMatrixAboveCurrentItem();

private:
    void buildPopupMenu();
    void ensureObjectHasName(osg::Object *object, std::string parentName);
    QTreeWidgetItem *createItemForObject(osg::Object *object, int parentNumberInChild);
    unsigned getIndexOfParentInChild(osg::Object *object, osg::Object *parentObject);
    bool addChildrenOfGroup(osg::Object *object, QTreeWidgetItem *groupItem);
    bool addChildrenOfGeode(osg::Object *object, QTreeWidgetItem *geodeItem);

    QFutureWatcher< QTreeWidgetItem * >m_watcher;
    QTreeWidgetItem *addObjectItem(osg::Object *object,
                                   osg::Object *parentObject);
    QCursor m_stashedCursor;
    QMenu m_popupMenu;
};

#endif // OSGTREEWIDGET_H
