#include <assert.h>
#include <iostream>

#include "Canvas.h"
#include "Settings.h"
#include "Stroke.h"

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Drawable>
#include <osg/BoundingBox>
#include <osg/LineWidth>
#include <osg/StateSet>
#include <osg/Plane>
#include <osg/BlendFunc>
#include <osg/BoundingSphere>

entity::Canvas::Canvas()
    : osg::Group()
    , m_mR(osg::Matrix::rotate(0, dureu::NORMAL))
    , m_mT(osg::Matrix::translate(0,0,0))
    , m_transform(new osg::MatrixTransform(m_mR * m_mT))
    , m_switch(new osg::Switch)
    , m_geodeData(new osg::Geode)
    , m_frame(new osg::Geometry)
    , m_pickable(new osg::Geometry)
    , m_axis(new osg::Geometry)
    , m_norm(new osg::Geometry)
    , m_intersection(new osg::Geometry)

    , m_strokeCurrent(0)
    , m_strokesSelected(0)
    , m_strokeSelected(0)
    , m_photoCurrent(0)

    , m_center(osg::Vec3f(0.f,0.f,0.f)) // moves only when strokes are introduced so that to define it as centroid
    , m_normal(dureu::NORMAL)
    , m_color(dureu::CANVAS_CLR_REST) // frame and pickable color
    , m_edit(false)
    , m_rotaxis(osg::Vec3f(0.f, 1.f, 0.f))
{
    osg::StateSet* stateset = new osg::StateSet;
    osg::LineWidth* linewidth = new osg::LineWidth();
    linewidth->setWidth(dureu::CANVAS_LINE_WIDTH);
    osg::BlendFunc* blendfunc = new osg::BlendFunc();
    //blendfunc->setFunction(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::ANTIALIAS);
    stateset->setAttributeAndModes(linewidth,osg::StateAttribute::ON);
    stateset->setAttributeAndModes(blendfunc, osg::StateAttribute::ON);
    stateset->setMode(GL_LINE_SMOOTH, osg::StateAttribute::ON);
    stateset->setMode(GL_BLEND, osg::StateAttribute::ON);
    stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
    this->setStateSet(stateset);

    m_transform->setName("Transform");
    m_switch->setName("Switch");
    m_pickable->setName("Pickable");

    outLogMsg("New Canvas ctor complete");
}

entity::Canvas::Canvas(const entity::Canvas& cnv, const osg::CopyOp& copyop)
    : osg::Group(cnv, copyop)
    , m_mR(cnv.m_mR)
    , m_mT(cnv.m_mT)
    , m_transform(cnv.m_transform)
    , m_switch(cnv.m_switch)
    , m_geodeData(cnv.m_geodeData)
    , m_frame(cnv.m_frame)
    , m_pickable(cnv.m_pickable)
    , m_axis(cnv.m_axis)
    , m_norm(cnv.m_norm)
    , m_intersection(new osg::Geometry)

    , m_strokeCurrent(0)
    , m_strokesSelected(0)
    , m_strokeSelected(0)
    , m_photoCurrent(0)

    , m_center(cnv.m_center)
    , m_normal(cnv.m_normal)
    , m_color(cnv.m_color)
    , m_edit(cnv.m_edit)
    , m_rotaxis(osg::Vec3f(0.f, 1.f, 0.f))
{
}

/* Method to initialize canvases' scene graph structure
 * must be called from AddCanvasCommand right after the canvas
 * allocated.
*/
void entity::Canvas::initializeSG()
{
    osg::Geode* geodeFrame = new osg::Geode;
    osg::Geode* geodeNormal = new osg::Geode;
    osg::Geode* geodeAxis = new osg::Geode;
    osg::Geode* geodeIntersection = new osg::Geode;
    this->addChild(m_transform.get());
    m_transform->setName("Transform");
    m_transform->addChild(m_switch.get());
    m_switch->setName("Switch");
    m_switch->addChild(geodeFrame, true); // child #0
    m_switch->addChild(geodeAxis, true); // #1
    m_switch->addChild(geodeNormal, false); // #2
    m_switch->addChild(geodeIntersection, false); // #3
    m_switch->addChild(m_geodeData.get(), true); // _geodeData is  empty, it is for user input: strokes

    geodeFrame->setName("GeodeFrame");
    geodeFrame->addDrawable(m_frame.get());
    geodeFrame->addDrawable(m_pickable.get());
    geodeNormal->setName("GeodeNormal");
    geodeNormal->addDrawable(m_norm.get());
    geodeAxis->setName("GeodeAxis");
    geodeAxis->addDrawable(m_axis.get());
    geodeIntersection->setName("GeodeIntersection");
    geodeIntersection->addDrawable(m_intersection.get());

    osg::Vec3Array* vFrame = new osg::Vec3Array(4);
    m_frame->setVertexArray(vFrame);
    m_frame->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_LOOP,0,4));

    osg::Vec3Array* vPick = new osg::Vec3Array(4);
    m_pickable->setVertexArray(vPick);
    m_pickable->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,4));

    osg::Vec3Array* vAxis = new osg::Vec3Array(4);
    m_axis->setVertexArray(vAxis);
    m_axis->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES,0,4));
    osg::Vec4Array* colorAxis = new osg::Vec4Array;
    colorAxis->push_back(solarized::orange);
    colorAxis->push_back(solarized::orange);
    colorAxis->push_back(solarized::yellow);
    colorAxis->push_back(solarized::yellow);
    m_axis->setColorArray(colorAxis, osg::Array::BIND_PER_VERTEX);

    osg::Vec3Array* vNormal = new osg::Vec3Array(2);
    m_norm->setVertexArray(vNormal);
    m_norm->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES,0,2));
    osg::Vec4Array* colorNormal = new osg::Vec4Array;
    colorNormal->push_back(dureu::CANVAS_CLR_EDIT);
    m_norm->setColorArray(colorNormal, osg::Array::BIND_OVERALL);

    osg::Vec3Array* vIntersection = new osg::Vec3Array(2);
    m_intersection->setVertexArray(vIntersection);
    m_intersection->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES, 0, 2));
    osg::Vec4Array* colorIntersection = new osg::Vec4Array;
    colorIntersection->push_back(m_color);
    m_intersection->setColorArray(colorIntersection, osg::Array::BIND_OVERALL);

    osg::Vec4Array* colorFrame = new osg::Vec4Array;
    colorFrame->push_back(m_color);
    m_frame->setColorArray(colorFrame, osg::Array::BIND_OVERALL);

    osg::Vec4Array* colorPick = new osg::Vec4Array;
    colorPick->push_back(m_color);
    m_pickable->setColorArray(colorPick, osg::Array::BIND_OVERALL);

    this->updateTransforms();
    this->setColor(m_color);
    this->setVertices(m_center, dureu::CANVAS_MINW, dureu::CANVAS_MINH, dureu::CANVAS_CORNER, dureu::CANVAS_AXIS);
}

void entity::Canvas::setMatrixRotation(const osg::Matrix& R)
{
    m_mR = R;
    this->updateTransforms();
}

const osg::Matrix&entity::Canvas::getMatrixRotation() const
{
    return m_mR;
}

void entity::Canvas::setMatrixTranslation(const osg::Matrix& T)
{
    m_mT = T;
    this->updateTransforms();
}

const osg::Matrix&entity::Canvas::getMatrixTranslation() const
{
    return m_mT;
}

/* This method should never be called directly.
 * It is here only to comply with serializer interface.
*/
void entity::Canvas::setTransform(osg::MatrixTransform* t)
{
    m_transform = t;
}

const osg::MatrixTransform*entity::Canvas::getTransform() const
{
    return m_transform.get();
}

osg::MatrixTransform* entity::Canvas::getTransform()
{
    return m_transform.get();
}

void entity::Canvas::setSwitch(osg::Switch* sw)
{
    m_switch = sw;
}

const osg::Switch* entity::Canvas::getSwitch() const
{
    return m_switch.get();
}

void entity::Canvas::setFrame(osg::Geometry* geom)
{
    m_frame = geom;
}

const osg::Geometry* entity::Canvas::getFrame() const
{
    return m_frame.get();
}

void entity::Canvas::setPickable(osg::Geometry* geom)
{
    m_pickable = geom;
}

const osg::Geometry* entity::Canvas::getPickable() const
{
    return m_pickable.get();
}

void entity::Canvas::setAxis(osg::Geometry* geom)
{
    m_axis = geom;
}

const osg::Geometry* entity::Canvas::getAxis() const
{
    return m_axis.get();
}

void entity::Canvas::setNorm(osg::Geometry* n)
{
    m_norm = n;
}

const osg::Geometry* entity::Canvas::getNorm() const
{
    return m_norm.get();
}

void entity::Canvas::setIntersection(osg::Geometry *i)
{
    m_intersection = i;
}

const osg::Geometry *entity::Canvas::getIntersection() const
{
    return m_intersection.get();
}

void entity::Canvas::setGeodeData(osg::Geode* geode)
{
    m_geodeData = geode;
}

const osg::Geode* entity::Canvas::getGeodeData() const
{
    return m_geodeData.get();
}

osg::Geode*entity::Canvas::getGeodeData()
{
    return m_geodeData.get();
}

void entity::Canvas::setCenter(const osg::Vec3f &center)
{
    m_center = center;
}

const osg::Vec3f& entity::Canvas::getCenter() const
{
    return m_center;
}

void entity::Canvas::setNormal(const osg::Vec3f &normal)
{
    m_normal = normal;
}

const osg::Vec3f& entity::Canvas::getNormal() const
{
    return m_normal;
}

void entity::Canvas::setColor(const osg::Vec4f &color)
{
    if (color == dureu::CANVAS_CLR_CURRENT) // hide the axis for "rest" canvases
        m_switch->setChildValue(m_switch->getChild(1), true);
    else
        m_switch->setChildValue(m_switch->getChild(1), false);

    m_color = color;
    osg::Vec4Array* colorFrame = static_cast<osg::Vec4Array*>(m_frame->getColorArray());
    (*colorFrame)[0] = m_color;
    m_frame->dirtyDisplayList();
    m_frame->dirtyBound();

    osg::Vec4Array* colorPick = static_cast<osg::Vec4Array*>(m_pickable->getColorArray());
    (*colorPick)[0] = m_color;
    m_pickable->dirtyDisplayList();
    m_pickable->dirtyBound();
}

const osg::Vec4f& entity::Canvas::getColor() const
{
    return m_color;
}

void entity::Canvas::setRotationAxis(const osg::Vec3f &axis)
{
    outLogVec("canvas setting rotation axis to", axis.x(), axis.y(), axis.z());
    m_rotaxis = axis;
}

const osg::Vec3f &entity::Canvas::getRotationAxis() const
{
    return m_rotaxis;
}

void entity::Canvas::setVisibility(bool vis)
{
    m_switch->setChildValue(m_switch->getChild(0), vis);
}

bool entity::Canvas::getVisibility() const{
    return m_switch->getChildValue(m_switch->getChild(0));
}

void entity::Canvas::setVisibilityLocalAxis(bool vis)
{
    m_switch->setChildValue(m_switch->getChild(1), vis);
}

bool entity::Canvas::getVisibilityLocalAxis() const
{
    return m_switch->getChildValue(m_switch->getChild(1));
}

// translates the current params on mt matrix
void entity::Canvas::translate(const osg::Matrix& mt)
{
    if (mt.isNaN()){
        outErrMsg("canvas-translate: translate matrix is NAN");
        return;
    }
    m_mT = m_mT * mt;
    this->updateTransforms();
}

// rotates the current params on mr matrix
void entity::Canvas::rotate(const osg::Matrix& mr)
{
    if (mr.isNaN()){
        outErrMsg("canvas-rotate: matrix is NAN");
        return;
    }
    m_mR = m_mR * mr;
    this->updateTransforms();
}

void entity::Canvas::unselectAll()
{
    this->setPhotoCurrent(false);
    this->setStrokeCurrent(false);
    this->unselectStrokes();
}

void entity::Canvas::unselectStrokes()
{
    this->resetStrokesSelected();
}

void entity::Canvas::selectAllStrokes()
{
    this->resetStrokesSelected();
    for (unsigned int i = 0; i < m_geodeData->getNumChildren(); ++i){
        entity::Stroke* stroke = dynamic_cast<entity::Stroke*>(m_geodeData->getChild(i));
        if (!stroke)
            continue;
        this->addStrokesSelected(stroke);
    }
}

void entity::Canvas::setStrokeCurrent(entity::Stroke *stroke)
{
    if (m_strokeCurrent.get() == stroke)
        return;
    if (m_strokeCurrent.get() !=0 )
        this->setStrokeCurrent(false);
    m_strokeCurrent = stroke;
}

void entity::Canvas::setStrokeCurrent(bool current)
{
    if (!current)
        m_strokeCurrent = NULL;
}

entity::Stroke *entity::Canvas::getStrokeCurrent() const
{
    return m_strokeCurrent.get();
}

void entity::Canvas::addStrokesSelectedAll()
{
    this->resetStrokesSelected();
    for (unsigned int i = 0; i < m_geodeData->getNumChildren(); i++){
        entity::Entity2D* entity = dynamic_cast<entity::Entity2D*>(m_geodeData->getChild(i));
        if (!entity){
            outErrMsg("addStrokesSelectedAll: could not dynamic_cast to Entity2D*");
            continue;
        }
        if (entity->getEntityType() == dureu::ENTITY_STROKE){
            entity::Stroke* stroke = dynamic_cast<entity::Stroke*>(entity);
            if (!stroke){
                outErrMsg("addStrokesSelectedAll: could not dynamic_cast to Stroke*");
                continue;
            }
            this->addStrokesSelected(stroke);
        }
    }
}

void entity::Canvas::addStrokesSelected(entity::Stroke* stroke)
{
    if (!stroke){
        outErrMsg("addStrokesSelected: stroke ptr is NULL");
        return;
    }
    if (!m_geodeData->containsDrawable(stroke)){
        outErrMsg("The stroke does not belong to Canvas, selection is impossible");
        return;
    }
    if (!this->isStrokeSelected(stroke)){
        outLogMsg("stroke is unselected, do select");
        this->setStrokeSelected(stroke);
        m_strokesSelected.push_back(stroke);
    }
    else{
        outLogMsg("stroke is already selected, do unselect");
        this->resetStrokeSelected(stroke);
    }
}

void entity::Canvas::resetStrokesSelected()
{
    for (unsigned int i = 0; i < m_strokesSelected.size(); ++i){
        entity::Stroke* stroke = m_strokesSelected.at(i);
        this->setStrokeSelected(stroke);
        this->setStrokeSelected(false);
    }
    m_strokesSelected.clear();
}

void entity::Canvas::resetStrokeSelected(entity::Stroke *stroke)
{
    unsigned int i;
    for (i = 0; i < m_strokesSelected.size(); ++i){
        entity::Stroke* s = m_strokesSelected.at(i);
        if (stroke == s)
            break;
    }
    this->setStrokeSelected(stroke);
    this->setStrokeSelected(false);
    m_strokesSelected.erase(m_strokesSelected.begin()+i);
}

const std::vector<entity::Stroke* >& entity::Canvas::getStrokesSelected() const
{
    return m_strokesSelected;
}

int entity::Canvas::getStrokesSelectedSize() const
{
    return m_strokesSelected.size();
}

osg::Vec3f entity::Canvas::getStrokesSelectedCenter() const
{
    double mu = 0, mv = 0;
    for (unsigned int i=0; i<m_strokesSelected.size(); ++i)
    {
        entity::Stroke* stroke = m_strokesSelected.at(i);
        if (!stroke){
            outErrMsg("getStrokesSelecterCenter: one of strokes ptr is NULL");
            break;
        }
        osg::BoundingSphere bs = stroke->getBound();
        osg::BoundingBox bb = stroke->getBoundingBox();
        mu += bb.center().x();
        mv += bb.center().y();
    }
    if (m_strokesSelected.size() > 0){
        mu /= m_strokesSelected.size();
        mv /= m_strokesSelected.size();
    }
    return osg::Vec3f(mu, mv, 0);
}

void entity::Canvas::moveStrokes(std::vector<entity::Stroke *>& strokes, double du, double dv)
{
    for (unsigned int i=0; i<strokes.size(); ++i){
        entity::Stroke* stroke = strokes.at(i);
        if (!stroke){
            outErrMsg("moveStrokes: one of strokes ptr is NULL");
            break;
        }
        stroke->moveDelta(du, dv);
    }
}

void entity::Canvas::moveStrokesSelected(double du, double dv)
{
    this->moveStrokes(m_strokesSelected, du, dv);
}

void entity::Canvas::scaleStrokes(std::vector<entity::Stroke *> &strokes, double s, osg::Vec3f center)
{
    for (unsigned int i=0; i<strokes.size(); ++i){
        entity::Stroke* stroke = strokes.at(i);
        if (!stroke){
            outErrMsg("moveStrokes: one of strokes ptr is NULL");
            break;
        }
        stroke->scale(s, center);
    }
}

void entity::Canvas::scaleStrokesSelected(double s, osg::Vec3f center)
{
    this->scaleStrokes(m_strokesSelected, s, center);
}

void entity::Canvas::rotateStrokes(std::vector<entity::Stroke *> strokes, double theta, osg::Vec3f center)
{
    for (unsigned int i=0; i<strokes.size(); ++i){
        entity::Stroke* stroke = strokes.at(i);
        if (!stroke){
            outErrMsg("moveStrokes: one of strokes ptr is NULL");
            break;
        }
        stroke->rotate(theta, center);
    }
}

void entity::Canvas::rotateStrokesSelected(double theta, osg::Vec3f center)
{
    this->rotateStrokes(m_strokesSelected, theta, center);
}

void entity::Canvas::setStrokeSelected(entity::Stroke *stroke)
{
    if (m_strokeSelected.get() == stroke){
        return;
    }
    //if (m_strokeSelected.get() != 0)
    //    this->setStrokeSelected(false);
    m_strokeSelected = stroke;
    this->setStrokeSelected(true);
}

void entity::Canvas::setStrokeSelected(bool selected)
{
    if (!m_strokeSelected.get())
        return;
    if (!selected){
        m_strokeSelected->setColor(dureu::STROKE_CLR_NORMAL);
        m_strokeSelected = 0;
        outLogMsg("normal color is set");
    }
    else{
        m_strokeSelected->setColor(dureu::STROKE_CLR_SELECTED);
        outLogMsg("selected color is set");
    }
}

bool entity::Canvas::isStrokeSelected(entity::Stroke *stroke) const
{
    for (unsigned int i = 0; i < m_strokesSelected.size(); ++i){
        if (stroke == m_strokesSelected.at(i))
            return true;
    }
    return false;
}

entity::Stroke *entity::Canvas::getStrokeSelected() const
{
    return m_strokeSelected.get();
}

bool entity::Canvas::setPhotoCurrent(entity::Photo *photo)
{
    if (m_photoCurrent.get() == photo)
        return true;
    if (!this->getGeodeData()->containsNode(photo))
        return false;
    if (m_photoCurrent.get()!=0)
        this->setPhotoCurrent(false);
    m_photoCurrent = photo;
    this->setPhotoCurrent(true);
    return true;
}

// changes photo frame color based on bool varialbe
void entity::Canvas::setPhotoCurrent(bool current)
{
    if (!m_photoCurrent)
        return;
    if (!current){
        m_photoCurrent->setFrameColor(dureu::PHOTO_CLR_REST);
        m_photoCurrent = 0;
    }
    else
        m_photoCurrent->setFrameColor(dureu::PHOTO_CLR_SELECTED);
}

void entity::Canvas::updateFrame()
{
    if (m_geodeData->getNumChildren() > 0){
        /* local bounding box */
        osg::BoundingBox bb = m_geodeData->getBoundingBox();
        assert(bb.valid());

        /* 2d <--> 2d transform matrices */
        osg::Matrix M = m_transform->getMatrix();
        osg::Matrix invM;
        if (!invM.invert(M)){
            outErrMsg("updateFrame: impossible to invert the transform matrix");
            return;
        }

        /* old 2d local canvas center */
        osg::Vec3f c2d_old = m_center * invM;
        if (std::fabs(c2d_old.z()) > dureu::EPSILON){
            outErrMsg("Warning: updateFrame(): local central point z-coord is not close to zero");
            return;
        }

        /* new 2d local center */
        osg::Vec3f c2d_new = bb.center();
        if (std::fabs(c2d_new.z()) > dureu::EPSILON){
            outErrMsg("Warning: updateFrame(): local central point z-coord is not close to zero");
            return;
        }

        /* move every child back in local delta translation (diff between old and new centers) */
        osg::Vec3f delta2d = c2d_old - c2d_new;
        for (unsigned int i=0; i<m_geodeData->getNumChildren(); ++i){
            entity::Entity2D* entity = dynamic_cast<entity::Entity2D*>(m_geodeData->getChild(i));
            if (!entity){
                outErrMsg("updateFrame: could not dynamic_cast to Entity2D*");
                return;
            }
            entity->moveDelta(delta2d.x(), delta2d.y());
        }

        /* adjust the size of the canvas drawables in old location */
        float dx = 0.5*(bb.xMax()-bb.xMin());
        float dy = 0.5*(bb.yMax()-bb.yMin());
        float szX = std::max(dx, dureu::CANVAS_MINW);
        float szY = std::max(dy, dureu::CANVAS_MINW);
        this->setVertices(c2d_old, szX, szY, dureu::CANVAS_CORNER, dureu::CANVAS_AXIS);

        /* new global center coordinate and delta translate in 3D */
        osg::Vec3f c3d_new = c2d_new * M;
        osg::Vec3f delta3d = c3d_new - m_center;

        /* move whole canvas and its drawables to be positioned at new global center */
        this->translate(osg::Matrix::translate(delta3d.x(), delta3d.y(), delta3d.z()));
    }
}

void entity::Canvas::setModeEdit(bool on)
{
    if (on){
        std::cout << "setModeEdit(): ON - " << on << std::endl;
        this->setColor(dureu::CANVAS_CLR_EDIT);
        this->setVisibilityLocalAxis(false);
    }
    else{
        std::cout << "setModeEdit(): OFF - " << on << std::endl;
        this->setColor(dureu::CANVAS_CLR_CURRENT);
        this->setVisibilityLocalAxis(true); // could be bug here, when originally local axis is off by user
    }
    m_switch->setChildValue(m_switch->getChild(2), on);
    m_edit = on;
}

bool entity::Canvas::getModeEdit() const
{
    return m_edit;
}

osg::Plane entity::Canvas::getPlane() const
{
    osg::Plane plane(m_normal, m_center);
    return plane;
}

osg::MatrixTransform* entity::Canvas::getMatrixTransform() const
{
    return m_transform.get();
}

entity::Canvas *entity::Canvas::clone() const
{
    osg::ref_ptr<entity::Canvas> clone = new Canvas;
    if (!clone.get()) return NULL;
    clone->initializeSG();
    clone->setMatrixRotation(this->getMatrixRotation());
    clone->setMatrixTranslation(this->getMatrixTranslation());
    clone->setName(this->getName());

    for (unsigned int i=0; i<m_geodeData->getNumChildren(); ++i){
        entity::Entity2D* entcopy = dynamic_cast<entity::Entity2D*>(m_geodeData->getChild(i));
        switch(entcopy->getEntityType()){
        case dureu::ENTITY_STROKE:
        {
            entity::Stroke* stroke = dynamic_cast<entity::Stroke*>(m_geodeData->getChild(i));
            if (stroke){
                entity::Stroke* si = new entity::Stroke(*stroke, osg::CopyOp::DEEP_COPY_ALL);
                if (si){
                    if (!clone->getGeodeData()->addDrawable(si)) outErrMsg("canvas clone: could not add stroke as drawable");
                }
                else outErrMsg("canvas clone: coult not clone the stroke");
            }
            else outErrMsg("canvas clone: could not dynamic_cast<Stroke>");
            break;
        }
        case dureu::ENTITY_PHOTO:
            break;
        default:
            break;
        }
    }
    clone->updateFrame();

    return clone.release();
}

entity::Photo* entity::Canvas::getPhotoCurrent() const
{
    return m_photoCurrent.get();
}

entity::Canvas::~Canvas()
{
}

// updates internals from m_r and m_t
void entity::Canvas::updateTransforms()
{
    /* update how the drawables look */
    osg::Matrix M = m_mR * m_mT;
    m_transform->setMatrix(M);

    /* update plane parameters */
    m_normal = dureu::NORMAL;
    m_center = osg::Vec3(0,0,0);
    osg::Plane plane(m_normal, m_center);
    m_center = m_center * M;
    plane.transform(M);
    m_normal = plane.getNormal();
    if (!plane.valid()){
        outErrMsg("updateTransforms: Error while transforming internal canvas data. Virtual plane is not valid."
                  "Resetting plane parameters.");
        outLogVec("normal", plane.getNormal().x(), plane.getNormal().y(), plane.getNormal().z());
        this->resetTransforms();
    }
}

void entity::Canvas::resetTransforms()
{
    /* reset transform params */
    m_mR = osg::Matrix::rotate(0, dureu::NORMAL);
    m_mT = osg::Matrix::translate(0,0,0);
    m_transform->setMatrix(m_mR * m_mT);

    /* reset plane params */
    m_normal = dureu::NORMAL;
    m_center = osg::Vec3(0,0,0);
    osg::Plane plane(m_normal, m_center);
    m_center = m_center * m_mR * m_mT;
    plane.transform(m_mR * m_mT);
    m_normal = plane.getNormal();
    if (!plane.valid()){
        outErrMsg("resetTrandforms: failed. Exiting application");
        exit(1);
    }
}

void entity::Canvas::setVertices(const osg::Vec3f &center, float szX, float szY, float szCr, float szAx)
{
    assert(szX>=dureu::CANVAS_MINW && szY>=dureu::CANVAS_MINH);

    osg::Vec3Array* vFrame = static_cast<osg::Vec3Array*>(m_frame->getVertexArray());
    assert(vFrame->size() == 4);
    (*vFrame)[0] = center + osg::Vec3(szX,szY,0.f);
    (*vFrame)[1] = center + osg::Vec3(-szX,szY,0.f);
    (*vFrame)[2] = center + osg::Vec3(-szX,-szY,0.f);
    (*vFrame)[3] = center + osg::Vec3(szX,-szY,0.f);
    m_frame->dirtyDisplayList();
    m_frame->dirtyBound();

    assert(szCr >= dureu::CANVAS_CORNER);
    osg::Vec3 p0 = (*vFrame)[0];
    osg::Vec3Array* vPick = static_cast<osg::Vec3Array*>(m_pickable->getVertexArray());
    assert(vPick->size() == 4);
    (*vPick)[0] = p0;
    (*vPick)[1] = p0 + osg::Vec3(-szCr, 0.f, 0.f);
    (*vPick)[2] = p0 + osg::Vec3(-szCr, -szCr, 0.f);
    (*vPick)[3] = p0 + osg::Vec3(.0f, -szCr, 0.f);
    m_pickable->dirtyDisplayList();
    m_pickable->dirtyBound();

    assert(szAx>=dureu::CANVAS_AXIS);
    osg::Vec3Array* vAxis = static_cast<osg::Vec3Array*>(m_axis->getVertexArray());
    assert(vAxis->size() == 4);
    (*vAxis)[0] = center;
    (*vAxis)[1] = center + osg::Vec3(szAx,0.f,0.f);
    (*vAxis)[2] = center;
    (*vAxis)[3] = center + osg::Vec3(0.f, szAx, 0.f);
    m_axis->dirtyDisplayList();
    m_axis->dirtyBound();

    float szN = std::max(szX,szY);
    osg::Vec3Array* vNormal = static_cast<osg::Vec3Array*>(m_norm->getVertexArray());
    assert(vNormal->size() == 2);
    (*vNormal)[0] = center;
    (*vNormal)[1] = center + osg::Vec3f(0.f,0.f, szN);
}

REGISTER_OBJECT_WRAPPER(Canvas_Wrapper
                        , new entity::Canvas
                        , entity::Canvas
                        , "osg::Object osg::Group entity::Canvas")
{
    ADD_MATRIX_SERIALIZER(MatrixRotation, osg::Matrix());
    ADD_MATRIX_SERIALIZER(MatrixTranslation, osg::Matrix());

    ADD_OBJECT_SERIALIZER(Transform, osg::MatrixTransform, NULL);
    ADD_OBJECT_SERIALIZER(Switch, osg::Switch, NULL);
    ADD_OBJECT_SERIALIZER(GeodeData, osg::Geode, NULL);
    ADD_OBJECT_SERIALIZER(Frame, osg::Geometry, NULL);
    ADD_OBJECT_SERIALIZER(Pickable, osg::Geometry, NULL);
    ADD_OBJECT_SERIALIZER(Axis, osg::Geometry, NULL);
    ADD_OBJECT_SERIALIZER(Norm, osg::Geometry, NULL);
    ADD_OBJECT_SERIALIZER(Intersection, osg::Geometry, NULL);

    ADD_VEC3F_SERIALIZER(Center, osg::Vec3f());
    ADD_VEC3F_SERIALIZER(Normal, osg::Vec3f());
    ADD_VEC4F_SERIALIZER(Color, osg::Vec4f());
}
