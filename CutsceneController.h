#ifndef _CUTSCENECONTROLLER_H_
#define _CUTSCENECONTROLLER_H_

#include "Support/Scene/Canyon.h"
#include "Support/Vector3.h"
#include "Support/Scene/TransformGroup.h"
#include "Support/Scene/Airplane.h"
#include "Support/Scene/Canyon.h"
#include "Support/Scene/Skybox.h"
#include "Controller.h"
#include "Util.h"

#define STEP_SIZE .4

extern void togglePaused();
extern Canyon *canyon;
extern Skybox *skybox;

class CutsceneController : public Controller {
public:
  virtual void initialize() {
    track = new BezierTrack();
    airplane = new Airplane();
    display = new TransformGroup(Matrix4::TranslationMatrix(0, 0, 0), 1);
    
    track->addPoint(450, 200, -200);
    track->addPoint(280, 200, -200);
    track->addPoint(210, 300, -340);
    track->addPoint(150, 300, -380); //
    track->addPoint(90, 300, -420);
    track->addPoint(0, 50, -260);
    track->addPoint(0, 50, 0);
    
    track->addChild(*airplane);
    
    display->addChild(*track);
  }
  
  virtual void step(double elapsed) {
    track->step(elapsed * STEP_SIZE);
    //track->step(.01);
  }
  
  virtual void draw() {
    Matrix4 identity;
    
    int view = (track->getT() < 1 && false) ? 1 : 2;
    Vector3 position = airplane->getPosition();
    Vector3 camera = Vector3::MakeVector(250, 250, -320);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    if (view == 1) {
      display->setCamera(identity);
    }
    else if (view == 2) {
      gluLookAt(camera[X], camera[Y], camera[Z], position[X], position[Y], position[Z], 0, 1, 0);
    }
    
    glPushMatrix();
    display->draw(identity);
    glPopMatrix();
    
    canyon->draw();
    
    glLoadIdentity();
    
    if (view == 1) {
      skybox->draw(airplane->getDirection());
    }
    else if (view == 2) {
      skybox->draw(camera - position);
    }
  }
  
  virtual void keyDownHandler(int key) {}
  virtual void keyUpHandler(int key) {}
  
  bool isDone() {
    return track->getT() >= 2 - 1e-9;
  }
private:
  Airplane *airplane;
  BezierTrack *track;
  TransformGroup* display;
};

#endif