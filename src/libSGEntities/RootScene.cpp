#include "iostream"
#include <sstream>
#include <stdlib.h>
#include <assert.h>

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/Registry>

#include "RootScene.h"
#include "axes.h"
#include "settings.h"

RootScene::RootScene(QUndoStack *undoStack)
    : osg::Group()
    , m_userScene(new entity::UserScene)
    , m_axisGlo(new Axes)
    , m_observer(new ObserveSceneCallback)
    , m_hud(new HUDCamera(dureu::HUD_LEFT, dureu::HUD_RIGHT, dureu::HUD_BOTTOM, dureu::HUD_TOP))
    , m_undoStack(undoStack)
{
    // child #0
    if (!this->addChild(m_userScene.get())){
        outErrMsg("RootScene(): could not add user scene as a child");
        fatalMsg("RootScene(): could not add user scene as a child");
    }

    // child #1
    if (!this->addChild(m_axisGlo.get())){
        outErrMsg("RootScene(): could not add axes as a child");
        fatalMsg("RootScene(): could not add axes as a child");
    }

    m_observer->setScenePointer(m_userScene.get());
    m_userScene->addUpdateCallback(m_observer.get());

    // child #2
    if (!this->addChild(m_hud.get())){
        outErrMsg("RootScene(): could not add hud as a child");
        fatalMsg("RootScene(): could not add hud as a child");
    }

    if (!m_hud->setText(m_observer->getTextGeode())){
        outErrMsg("RootScene(): could not set text to hud camera");
        fatalMsg("RootScene(): could not set text to hud camera");
    }

    this->setName("RootScene");
}

entity::UserScene*RootScene::getUserScene() const
{
    return m_userScene.get();
}

void RootScene::setFilePath(const std::string& name)
{
    m_userScene->setFilePath(name);
}

bool RootScene::isSetFilePath() const
{
    return m_userScene->isSetFilePath();
}

void RootScene::setAxesVisibility(bool vis) {
    m_axisGlo->setVisibility(vis);
}

bool RootScene::getAxesVisibility() const{
    return m_axisGlo->getVisibility();
}

const Axes* RootScene::getAxes() const{
    return m_axisGlo.get();
}

const ObserveSceneCallback *RootScene::getSceneObserver() const{
    return m_observer.get();
}

const HUDCamera *RootScene::getHudCamera() const{
    return m_hud.get();
}

void RootScene::setHudCameraVisibility(bool vis){
    m_hud->setVisibility(vis);
}

bool RootScene::getHudCameraVisibility() const{
    return m_hud->getVisibility();
}

bool RootScene::writeScenetoFile()
{
    if (m_userScene->getFilePath() == "")
        return false;
    return osgDB::writeNodeFile(*(m_userScene.get()), m_userScene->getFilePath());

}

bool RootScene::loadSceneFromFile()
{
    if (!m_undoStack){
        fatalMsg("loadSceneFromFile(): undo stack is NULL. "
                 "Restart the program to ensure undo stack initialization.");
        return false;
    }
    osg::ref_ptr<entity::UserScene> newscene = dynamic_cast<entity::UserScene*>(osgDB::readNodeFile(m_userScene->getFilePath()));
    if (!newscene.get()){
        outErrMsg("loadSceneFromFile: could not load from file, or could not perform the dynamic_cast<osg::Group*>");
        return false;
    }

    /* replace the original */
    if (!this->replaceChild(m_userScene.get(), newscene.get())){
        outErrMsg("loadSceneFromFile: could not replace the original child");
        return false;
    }

    /* update pointer */
    m_userScene = newscene.get();

    /* reset observer */
    m_observer->setScenePointer(m_userScene.get());
    m_userScene->addUpdateCallback(m_observer.get());

    /* update current/previous canvases */
    for (unsigned int i=0; i<m_userScene->getNumChildren(); ++i){
        entity::Canvas* cnv = m_userScene->getCanvas(i);
        if (!cnv){
            outErrMsg("loadSceneFromFile: could not extract a canvas from loaded scene");
            return false;
        }
        cnv->setColor(dureu::CANVAS_CLR_REST);
        m_userScene->setCanvasCurrent(cnv);
    }
    newscene = 0;
    this->printScene();
    return true;
}

int RootScene::getCanvasLevel() const
{
    return m_userScene->getCanvasLevel();
}

int RootScene::getPhotoLevel() const
{
    return m_userScene->getPhotoLevel();
}

void RootScene::addCanvas(const osg::Matrix& R, const osg::Matrix& T)
{
    m_userScene->addCanvas(m_undoStack, R, T);
}

void RootScene::addCanvas(const osg::Matrix& R, const osg::Matrix& T, const std::string& name)
{
    m_userScene->addCanvas(m_undoStack, R, T, name);
}

void RootScene::addStroke(float u, float v, dureu::EVENT event)
{
    m_userScene->addStroke(m_undoStack, u, v, event);
}

void RootScene::addPhoto(const std::string& fname)
{
    m_userScene->addPhoto(m_undoStack, fname);
}

bool RootScene::setCanvasCurrent(entity::Canvas* cnv)
{
    return m_userScene->setCanvasCurrent(cnv);
}

bool RootScene::setCanvasPrevious(entity::Canvas* cnv)
{
    return m_userScene->setCanvasPrevious(cnv);
}

entity::Canvas* RootScene::getCanvasCurrent() const
{
    return m_userScene->getCanvasCurrent();
}

entity::Canvas* RootScene::getCanvasPrevious() const
{
    return m_userScene->getCanvasPrevious();
}

void RootScene::setTransformOffset(const osg::Vec3f& translate, const int mouse)
{
    m_userScene->setTransformOffset(translate, mouse);
}

void RootScene::setTransformRotate(const osg::Vec3f& normal, const int mouse)
{
    m_userScene->setTransformRotate(normal, mouse);
}

RootScene::~RootScene()
{

}

void RootScene::printScene()
{
    outLogVal("RootScene #children", this->getNumChildren());
    for (unsigned int i=0; i<this->getNumChildren(); ++i){
        outLogVal("Child", this->getChild(i)->getName());
    }
    outLogMsg("For each canvas");
    m_userScene->printScene();
}



