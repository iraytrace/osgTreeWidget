#include "Osg3dView.h"

#include <QMenu>
#include <QTime>

#include <osg/LightModel>
#include <osgViewer/Renderer>
#include <osg/ValueObject>
#include <osg/StateSet>
#include <osgUtil/IntersectionVisitor>
#include <osgUtil/LineSegmentIntersector>

#include <QKeySequence>
#include <QToolBar>
static const bool debugView = false;
#define vDebug if (debugView) qDebug

Osg3dView::Osg3dView(QWidget *parent)
    : QOpenGLWidget(parent)
    , m_viewingCore(new ViewingCore)
    , m_mouseMode(MM_ORBIT)
    , m_mouseIsPressed(false)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(customMenuRequested(QPoint)));
#if 0   // This would be interesting to have  Perhaps it belongs in OsgForm
        // Alas, it would be hard to design in qtdesigner
        QToolBar *tb = new QToolBar(this);
        tb->addAction("Hello");
        tb->addAction("goodbye");
#endif
    buildPopupMenu();

    // Construct the embedded graphics window
    m_osgGraphicsWindow = new osgViewer::GraphicsWindowEmbedded(0,0,width(),height());
    getCamera()->setGraphicsContext(m_osgGraphicsWindow);

    // Set up the camera
    getCamera()->setViewport(new osg::Viewport(0,0,width(),height()));
    getCamera()->setProjectionMatrixAsPerspective(30.0f,
            static_cast<double>(width())/static_cast<double>(height()),
            1.0f,
            10000.0f);
    // By default draw everthing that has even 1 bit set in the nodemask
    getCamera()->setCullMask( (unsigned)~0 );
    getCamera()->setDataVariance(osg::Object::DYNAMIC);

    // As of July 2010 there wasn't really a good way to multi-thread OSG
    // under Qt so just set the threading model to be SingleThreaded
    setThreadingModel(osgViewer::Viewer::SingleThreaded);

    // draw both sides of polygons
    setLightingTwoSided();

    // this will probably be overwritten but avoids warnings
    setScene(new osg::Node);
    update();
}

void Osg3dView::setScene(osg::Node *root)
{
    this->setSceneData(root);
    m_viewingCore->setSceneData(root);
}

void Osg3dView::paintGL()
{
    vDebug("paintGL");

    // Update the camera
    osg::Camera *cam = this->getCamera();
    const osg::Viewport* vp = cam->getViewport();

    m_viewingCore->setAspect(vp->width() / vp->height());

    cam->setViewMatrix(m_viewingCore->getInverseMatrix());
    cam->setProjectionMatrix(m_viewingCore->computeProjection());

    // if m_timeToDrawLastFrame > threshold &&  m_mouseIsPressed
    //   draw simplified
    // else
    //   draw the full polygon mesh

    QTime frameTimer = QTime::currentTime();
    // Invoke the OSG traversal pipeline
    frame();
    m_timeToDrawLastFrame = frameTimer.elapsed();

    emit updated();
}

void Osg3dView::resizeGL(int w, int h)
{
    vDebug("resizeGL");

    m_osgGraphicsWindow->getEventQueue()->windowResize(9, 0, w, h);
    m_osgGraphicsWindow->resized(0,0,w,h);
}

void Osg3dView::hello()
{
    qDebug("hello");
}
QString Osg3dView::mouseModeDescription(Osg3dView::MouseMode mouseMode)
{
    switch (mouseMode) {
    case MM_ORBIT: return QString("Orbit"); break;
    case MM_PAN: return QString("Pan"); break;
    case MM_ZOOM: return QString("Zoom"); break;
    case MM_ROTATE: return QString("Rotate"); break;
    case MM_PICK_CENTER: return QString("CenterPick"); break;
    case MM_SELECTOBJECT: return QString("Select"); break;
    case MM_PICKPOINT: return QString("Pick Point"); break;
    default:
        break;
    }
    return QString("Invalid");
}
void Osg3dView::setMouseMode(Osg3dView::MouseMode mode)
{
    MouseMode oldMode = m_mouseMode;
    m_mouseMode = mode;

    foreach (QAction *a, m_mouseModeActions) {
        if (a->data().toUInt() == oldMode)
            a->setChecked(false);
        if (a->data().toUInt() == mode)
            a->setChecked(true);
    }

    if(mode == MM_ROTATE)
        m_viewingCore->setViewingCoreMode( ViewingCore::FIRST_PERSON );
    else {
        m_viewingCore->setViewingCoreMode( ViewingCore::THIRD_PERSON );
    }

    emit mouseModeChanged(oldMode, mode);
}

osg::Vec2d Osg3dView::getNormalized(const int ix, const int iy)
{
    int x=0, y=0, width=0, height=0;
    osg::Vec2d ndc;

    // we don't really need the x, y values but the width/height are important
    m_osgGraphicsWindow->getWindowRectangle(x, y, width, height);
    int center = width/2;
    ndc[0] = ((double)ix - (double)center) / (double)center;
    if (ndc[0] > 1.0) ndc[0] = 1.0;

    center = height/2;
    int invertedY = height - iy;
    ndc[1] = ((double)invertedY - (double)center) / (double)center;
    if (ndc[1] > 1.0) ndc[1] = 1.0;

    return ndc;
}

void Osg3dView::findObjectsUnderMouseEvent()
{
    osg::ref_ptr<osgUtil::IntersectionVisitor> theIntersectionVisitor;
    osg::Vec3d startPoint;
    osg::Vec3d farPoint = m_viewingCore->getFarPoint(m_savedEventNDCoords.x(),
                                                     m_savedEventNDCoords.y());

    m_viewingCore->getStartPoint(startPoint,
                                 farPoint,
                                 m_savedEventNDCoords.x(),
                                 m_savedEventNDCoords.y());

    m_clickIntersector =
            new osgUtil::LineSegmentIntersector( startPoint, farPoint );

    theIntersectionVisitor = new osgUtil::IntersectionVisitor( m_clickIntersector, NULL );
    this->getSceneData()->accept( *theIntersectionVisitor );
}

osg::NodePath Osg3dView::getFirstItemClicked()
{
    osg::NodePath nothing;

    if (!m_clickIntersector->containsIntersections()) {
        return nothing;
    }

    osgUtil::LineSegmentIntersector::Intersections & intersections =
            m_clickIntersector->getIntersections();

    const osgUtil::LineSegmentIntersector::Intersection &oneIntersection =
            *intersections.begin();

    return oneIntersection.nodePath;
}

bool Osg3dView::firstItemClickedWasLoaded()
{
    if (!m_clickIntersector->containsIntersections())
        return false;

    osgUtil::LineSegmentIntersector::Intersections & intersections =
            m_clickIntersector->getIntersections();

    const osgUtil::LineSegmentIntersector::Intersection &oneIntersection =
            *intersections.begin();

    osg::Group *root = this->getSceneData()->asGroup();

    if (oneIntersection.nodePath.at(0) == root &&
        oneIntersection.nodePath.at(1) == root->getChild(0))
        return true;

    return false;
}

unsigned Osg3dView::numberOfIntersections()
{
    osgUtil::LineSegmentIntersector::Intersections & intersections =
            m_clickIntersector->getIntersections();

    return intersections.size();
}

osg::NodePath Osg3dView::getFirstLoadedItemClicked()
{
    osg::NodePath nothing;

    if (!m_clickIntersector->containsIntersections())
        return nothing;

    osgUtil::LineSegmentIntersector::Intersections & intersections =
            m_clickIntersector->getIntersections();

    for (osgUtil::LineSegmentIntersector::Intersections::iterator itr =
         intersections.begin();
         itr != intersections.end() ;
         itr++) {
        const osgUtil::LineSegmentIntersector::Intersection &oneIntersection =
                *itr;

        osg::Group *root = this->getSceneData()->asGroup();

        // if this is a child of loaded model we are done, return the NodePath
        if (oneIntersection.nodePath.at(0) == root &&
            oneIntersection.nodePath.at(1) == root->getChild(0)) {
            return oneIntersection.nodePath;
        }
    }

    return nothing;
}

void Osg3dView::pickPointOnObject()
{
    if (!m_clickIntersector->containsIntersections())
        return;

    osgUtil::LineSegmentIntersector::Intersections & intersections =
            m_clickIntersector->getIntersections();

    for (osgUtil::LineSegmentIntersector::Intersections::iterator itr =
         intersections.begin();
         itr != intersections.end() ;
         itr++) {
        const osgUtil::LineSegmentIntersector::Intersection &oneIntersection =
                *itr;

        osg::Group *root = this->getSceneData()->asGroup();

        // if this is a child of loaded model we are done, return the NodePath
        if (oneIntersection.nodePath.at(0) == root &&
            oneIntersection.nodePath.at(1) == root->getChild(0)) {
            osg::Vec3d pt = oneIntersection.getWorldIntersectPoint();
            emit pointPicked(pt);
            return;
        }
    }
}

#include <osg/LineWidth>
void Osg3dView::pickAnObjectFromView()
{
    if (numberOfIntersections() == 0)
        return;


    osgUtil::LineSegmentIntersector::Intersections & intersections =
            m_clickIntersector->getIntersections();

    const osgUtil::LineSegmentIntersector::Intersection &oneIntersection =
            *intersections.begin();

    osg::Drawable *d = oneIntersection.drawable;
    osg::Geometry *geom = (osg::Geometry *)d->asGeometry()->clone(osg::CopyOp(osg::CopyOp::SHALLOW_COPY));
    if (!geom) return;

    osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array(1);
    (*colors)[0].set( 1.0f, 1.0f, 1.0f, 1.0f);
    geom->setColorArray(colors.get());
    geom->setColorBinding(osg::Geometry::BIND_OVERALL);

    osg::Geode *geode = new osg::Geode;
    geode->addDrawable(geom);
    osg::StateSet *ss = geode->getOrCreateStateSet();
    osg::ref_ptr<osg::PolygonMode> pm =
               dynamic_cast<osg::PolygonMode *>
               (ss->getAttribute(osg::StateAttribute::POLYGONMODE));
    if(!pm) {
        pm = new osg::PolygonMode;
        ss->setAttribute(pm.get());
    }
    pm->setMode(osg::PolygonMode::FRONT_AND_BACK,
                osg::PolygonMode::LINE);
    ss->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    ss->setAttribute(new osg::LineWidth(5.0f));

    this->getSceneData()->asGroup()->addChild(geode);
    update();

    osg::NodePath np = getFirstLoadedItemClicked();

    // turn pointers into ref_ptrs because we don't know
    // where this will end up
    QVector< osg::ref_ptr<osg::Node> > myNodePath;
    for (unsigned i=0 ; i < np.size() ; i++) {
        osg::ref_ptr<osg::Node> saveMe(np.at(i));
        myNodePath.push_back(saveMe);
    }

    emit pickObject(myNodePath);
}


void Osg3dView::mousePressEvent(QMouseEvent *event)
{
    vDebug("mousePressEvent");

    if (event->button() == Qt::LeftButton) {

        m_savedEventNDCoords = getNormalized(event->x(), event->y());

        // always identify the item under the mouse in case
        // it is a drag object
        findObjectsUnderMouseEvent();

        // if np.at(1) != loadedModel we are dragging/clicking some control
        // in that case we want to stash the current mouse mode, do the drag
        // and restore the mouse mode when we are done.

        // Do the job asked
        if (m_mouseMode & (MM_PAN|MM_ROTATE|MM_ORBIT|MM_ZOOM) )
            m_viewingCore->setPanStart( m_savedEventNDCoords.x(),
                                        m_savedEventNDCoords.y());
        else if (m_mouseMode & MM_PICK_CENTER) {
            m_viewingCore->pickCenter(m_savedEventNDCoords.x(),
                                      m_savedEventNDCoords.y() );
            update();
        } else if (m_mouseMode & MM_SELECTOBJECT) {
            // In this case we probably want to skip any intersections with
            // objects other than those under loadedModel;
            pickAnObjectFromView();
        } else if (m_mouseMode & MM_PICKPOINT) {
            pickPointOnObject();
        }

        m_mouseIsPressed = true;
    }
}

void Osg3dView::mouseMoveEvent(QMouseEvent *event)
{
    vDebug("mouseMoveEvent");
    osg::Vec2d currentNDC = getNormalized(event->x(), event->y());
    osg::Vec2d delta = currentNDC - m_savedEventNDCoords;

    switch (m_mouseMode) {
    case MM_ORBIT:
        m_viewingCore->rotate( m_savedEventNDCoords, delta);
        break;
    case MM_PAN:
        m_viewingCore->pan(delta.x(), delta.y());
        break;
    case MM_ZOOM: {
        double tempScale = m_viewingCore->getFovyScale();
        m_viewingCore->setFovyScale(1.03);

        if(delta.y() > 0)
            m_viewingCore->fovyScaleDown();

        if(delta.y() < 0)
            m_viewingCore->fovyScaleUp();

        m_viewingCore->setFovyScale(tempScale);
        break;
    }
    case MM_ROTATE:
        m_viewingCore->rotate(  m_savedEventNDCoords, delta );
        break;
    default:
        break;
    }

    m_savedEventNDCoords = currentNDC;
    update();
}

void Osg3dView::mouseReleaseEvent(QMouseEvent *event)
{
    vDebug("mouseReleaseEvent");
    m_savedEventNDCoords = getNormalized(event->x(), event->y());

    if (event->button() == Qt::LeftButton)
        m_mouseIsPressed = false;
}


void Osg3dView::wheelEvent(QWheelEvent *event)
{
    if(event->delta() > 0)
        m_viewingCore->dolly(0.5);
    else
        m_viewingCore->dolly(-0.5);
    update();
}

void Osg3dView::buildPopupMenu()
{
    QAction *a;
    m_popupMenu.setTitle("3dView");
    QMenu *sub = m_popupMenu.addMenu("MouseMode...");
    sub->setObjectName("MouseMode...");

    a = sub->addAction("Orbit", this, SLOT(setMouseMode()), QKeySequence(Qt::Key_O));
    a->setData(QVariant(MM_ORBIT));
    a->setCheckable(true);
    a->setChecked(true);
    m_mouseModeActions.push_back(a);
    a = sub->addAction("Pan", this, SLOT(setMouseMode()), QKeySequence(Qt::Key_P));
    a->setData(QVariant(MM_PAN));
    a->setCheckable(true);
    a->setChecked(false);
    m_mouseModeActions.push_back(a);
    a = sub->addAction("Rotate", this, SLOT(setMouseMode()), QKeySequence(Qt::Key_R));
    a->setData(QVariant(MM_ROTATE));
    a->setCheckable(true);
    a->setChecked(false);
    m_mouseModeActions.push_back(a);
    a = sub->addAction("Zoom", this, SLOT(setMouseMode()), QKeySequence(Qt::Key_Z));
    a->setData(QVariant(MM_ZOOM));
    a->setCheckable(true);
    a->setChecked(false);
    m_mouseModeActions.push_back(a);
    a = sub->addAction("Pick Center", this, SLOT(setMouseMode()), QKeySequence(Qt::Key_C));
    a->setData(QVariant(MM_PICK_CENTER));
    a->setCheckable(true);
    a->setChecked(false);
    m_mouseModeActions.push_back(a);
    a = sub->addAction("Select Object", this, SLOT(setMouseMode()), QKeySequence(Qt::Key_S));
    a->setData(QVariant(MM_SELECTOBJECT));
    a->setCheckable(true);
    a->setChecked(false);
    a = sub->addAction("Pick Point", this, SLOT(setMouseMode()), QKeySequence(Qt::CTRL+ Qt::Key_P));
    m_mouseModeActions.push_back(a);
    a->setData(QVariant(MM_PICKPOINT ));
    a->setCheckable(true);
    a->setChecked(false);
    m_mouseModeActions.push_back(a);

    sub = m_popupMenu.addMenu("Std View...");
    a = sub->addAction("Top", this, SLOT(setStandardView()), QKeySequence(Qt::SHIFT + Qt::Key_T));
    a->setData(V_TOP);
    a = sub->addAction("Underside", this, SLOT(setStandardView()), QKeySequence(Qt::SHIFT + Qt::Key_U));
    a->setData(V_BOTTOM);
    a = sub->addAction("Front", this, SLOT(setStandardView()), QKeySequence(Qt::SHIFT + Qt::Key_F));
    a->setData(V_FRONT);
    a = sub->addAction("Back", this, SLOT(setStandardView()), QKeySequence(Qt::SHIFT + Qt::Key_B));
    a->setData(V_BACK);
    a = sub->addAction("Right", this, SLOT(setStandardView()), QKeySequence(Qt::SHIFT + Qt::Key_R));
    a->setData(V_RIGHT);
    a = sub->addAction("Left", this, SLOT(setStandardView()), QKeySequence(Qt::SHIFT + Qt::Key_L));
    a->setData(V_LEFT);

    sub = m_popupMenu.addMenu("Projection...");
    a = sub->addAction("Orthographic", this, SLOT(setProjection()));
    a->setData(P_ORTHO);
    a = sub->addAction("Perspective", this, SLOT(setProjection()));
    a->setData(P_PERSP);

    sub = m_popupMenu.addMenu("DrawMode...");
    a = sub->addAction("Facets", this, SLOT(setDrawMode()), QKeySequence(Qt::Key_F));
    a->setData(osg::PolygonMode::FILL);
    a = sub->addAction("Wireframe", this, SLOT(setDrawMode()), QKeySequence(Qt::Key_W));
    a->setData(osg::PolygonMode::LINE);
    a = sub->addAction("Verticies", this, SLOT(setDrawMode()), QKeySequence(Qt::Key_V));
    a->setData(osg::PolygonMode::POINT);

    sub = m_popupMenu.addMenu("Toggle...");
    a = sub->addAction("MenuBar", this, SIGNAL(toggleMenuBar()), QKeySequence(Qt::CTRL+ Qt::Key_M));
    a = sub->addAction("ToolBar", this, SIGNAL(toggleToolBar()), QKeySequence(Qt::CTRL+ Qt::Key_T));
    a = sub->addAction("PointDisplay", this, SIGNAL(togglePickedPoint()), QKeySequence(Qt::CTRL+ Qt::SHIFT + Qt::Key_P));
}

void Osg3dView::customMenuRequested(const QPoint &pos)
{
    vDebug("customMenu %d %d", pos.x(), pos.y());

    m_popupMenu.popup(this->mapToGlobal(pos));
}

void Osg3dView::setLightingTwoSided()
{
    osg::ref_ptr<osg::LightModel> lm = new osg::LightModel;
    lm->setTwoSided(true);
    lm->setAmbientIntensity(osg::Vec4(0.1f,0.1f,0.1f,1.0f));

    osg::StateSet *ss;

    for (int i=0 ; i < 2 ; i++ ) {
        ss = ((osgViewer::Renderer *)getCamera()
              ->getRenderer())->getSceneView(i)->getGlobalStateSet();

        ss->setAttributeAndModes(lm, osg::StateAttribute::ON);
    }
}


void Osg3dView::setMouseMode()
{
    QAction *a = dynamic_cast<QAction *>(sender());
    if (!a)
        return;
    MouseMode mode = static_cast<MouseMode>(a->data().toUInt());
    setMouseMode(mode);
}

void Osg3dView::setStandardView()
{
    QAction *a = dynamic_cast<QAction *>(sender());
    if (!a)
        return;

    StandardView view = static_cast<StandardView>(a->data().toUInt());
    switch (view) {
    case V_TOP: m_viewingCore->viewTop(); break;
    case V_BOTTOM: m_viewingCore->viewBottom(); break;
    case V_FRONT: m_viewingCore->viewFront(); break;
    case V_BACK: m_viewingCore->viewBack(); break;
    case V_RIGHT: m_viewingCore->viewRight(); break;
    case V_LEFT: m_viewingCore->viewLeft(); break;
    }
    update();
}

void Osg3dView::setDrawMode(osg::PolygonMode::Mode drawMode)
{
    if (drawMode == 0) {
        QAction *a = dynamic_cast<QAction *>(sender());
        if (!a)
            return;

        drawMode = static_cast<osg::PolygonMode::Mode>(a->data().toUInt());
    }

    osg::ref_ptr<osg::StateSet> ss = this->getSceneData()->getOrCreateStateSet();
    osg::ref_ptr<osg::PolygonMode> pm =
               dynamic_cast<osg::PolygonMode *>
               (ss->getAttribute(osg::StateAttribute::POLYGONMODE));

    if(!pm) {
        pm = new osg::PolygonMode;
        ss->setAttribute(pm.get());
    }
    pm->setMode(osg::PolygonMode::FRONT_AND_BACK,
                drawMode);

    switch (drawMode) {
    case osg::PolygonMode::LINE:
    case osg::PolygonMode::POINT:
        ss->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
        break;
    default:
        ss->setMode(GL_LIGHTING, osg::StateAttribute::ON);
        break;
    }

    update();
}

void Osg3dView::setProjection()
{
    QAction *a = dynamic_cast<QAction *>(sender());
    if (!a)
        return;

    Projection projType = static_cast<Projection>(a->data().toUInt());
    switch (projType) {
    case P_ORTHO:
        m_viewingCore->setOrtho(true);
        break;
    case P_PERSP:
        m_viewingCore->setOrtho(false);
        break;
    }
    update();
}
#include <QApplication>
void Osg3dView::enterEvent(QEvent *event)
{
    QWidget *w = qApp->focusWidget();
    this->setFocus();
    if ( w && w != this) {
        m_lastFocused = w;
    } else {
        m_lastFocused = (QWidget *)0;
    }
}

void Osg3dView::leaveEvent(QEvent *event)
{
    this->clearFocus();

    if (m_lastFocused && m_lastFocused != this) {
        m_lastFocused->setFocus();
        m_lastFocused = (QWidget *)0;
    }
}

void Osg3dView::keyPressEvent(QKeyEvent *event)
{
    int key = event->key();

    bool isShifted = event->modifiers() & Qt::ShiftModifier;

    switch (key) {
    case Qt::Key_M:
        if (event->modifiers() & Qt::ControlModifier)
            emit toggleMenuBar();
        break;
    case Qt::Key_W: setDrawMode(osg::PolygonMode::LINE); update(); break;
    case Qt::Key_V: setDrawMode(osg::PolygonMode::POINT); update(); break;
    case Qt::Key_T:
        if (isShifted) {
            m_viewingCore->viewTop();
            update();
        } else if (event->modifiers() & Qt::ControlModifier) {
            emit toggleToolBar();
        }
            break;
    case Qt::Key_L: if (isShifted) m_viewingCore->viewLeft(); update(); break;
    case Qt::Key_B: if (isShifted) m_viewingCore->viewBack(); update(); break;
    case Qt::Key_F:
        if (isShifted) {
            m_viewingCore->viewFront();
        } else {
            setDrawMode(osg::PolygonMode::FILL);
        }
        update();
        break;
    case Qt::Key_U: if (isShifted) m_viewingCore->viewBottom(); update(); break;
    case Qt::Key_O: setMouseMode(MM_ORBIT); break;
    case Qt::Key_P:
        if (event->modifiers() & Qt::ControlModifier)
            if (event->modifiers() & Qt::ShiftModifier)
                emit togglePickedPoint();
            else
                setMouseMode(MM_PICKPOINT);
        else
            setMouseMode(MM_PAN);
        break;
    case Qt::Key_Z: setMouseMode(MM_ZOOM); break;
    case Qt::Key_R:
        if (isShifted) {
            m_viewingCore->viewRight();
            update();
        }else
            setMouseMode(MM_ROTATE);
        break;
    case Qt::Key_C: setMouseMode(MM_PICK_CENTER); break;
    case Qt::Key_S: setMouseMode(MM_SELECTOBJECT); break;
    }
}
