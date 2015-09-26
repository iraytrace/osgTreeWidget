#ifndef OSGFORM_H
#define OSGFORM_H

#include <QWidget>
#include <QFutureWatcher>
#include <QToolBar>
#include <QMenuBar>


#include <osg/Group>
#include <osg/MatrixTransform>
#include "ViewingCore.h"
#include "Osg3dView.h"
#include "OriginAxis.h"
#include "pointIndicator.h"

class QTableWidgetItem;
class QTreeWidgetItem;
class QCheckBox;
class QPushButton;


namespace Ui {
class OsgForm;
}

class OsgForm : public QWidget
{
    Q_OBJECT

public:
    explicit OsgForm(QWidget *parent = 0);
    ~OsgForm();
    static QString getFileExtensions();
signals:
    void showMessage(QString);
public slots:
    void removeAllNodes();
    void openFile(const QString fileName);
    bool saveFile(const QString fileName);
    void addNode(osg::ref_ptr<osg::Node> n);
    void setCameraMask(osg::Node::NodeMask mask);
    void itemWasChangedInTable(QTableWidgetItem *tabwi);
    void itemWasChangedInTree(QTreeWidgetItem *treewi, int col);
    void setCameraMaskFromLineEdit();
    void handlePick(QVector<osg::ref_ptr<osg::Node> > nodePath);
    void showAxis(bool isShown);
protected slots:
    void toggle3dMenu();
    void toggle3dTools();
    void togglePickPoint();

private slots:
    void wrieNodesFinished();
    void readNodesFinished();
    void tweakCameraMaskBit(int state);
    void setNodeMask(osg::ref_ptr<osg::Node> n, unsigned mask);
    void mouseModeHasChanged(Osg3dView::MouseMode oldMouseMode, Osg3dView::MouseMode newMouseMode);
    void updateCameraDisplay();
    void pointPicked(osg::Vec3d pt);
private:
    void setupUserInterface();
    void buildLayerBox();
    void setProgressBarState(bool turnOn);
    bool saveThread(osg::ref_ptr<osg::Node> node, const QString fileName);
    osg::ref_ptr<osg::Node> readNodes(const QString fileName);
    void buildTheTools();


    Ui::OsgForm *ui;

    osg::ref_ptr<osg::Group> m_root;

    // By convention this is always the 0'th child of m_root;
    osg::ref_ptr<osg::MatrixTransform> m_loadedModel;
    osg::ref_ptr<ViewingCore> m_viewingCore;

    QFutureWatcher< osg::ref_ptr<osg::Node> >m_loadWatcher;
    QFutureWatcher< bool >m_saveWatcher;
    QCursor m_stashedCursor;
    QVector<QCheckBox *> m_checkBoxes;
    QToolBar *m_viewToolBar;
    QMenuBar *m_viewMenuBar;
    OriginAxis m_axis;
    PointIndicator m_pointIndicator;
};

#endif // OSGFORM_H
