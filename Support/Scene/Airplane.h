#ifndef _AIRPLANE_H_
#define _AIRPLANE_H_

#include "TransformGroup.h"
#include "Rect.h"
#include "Camera.h"
#include "BezierTrack.h"
#include "AirplaneObject.h"
#include <math.h>

#define LEFT -1
#define RIGHT 1
#define DOWN -1
#define UP 1
#define LR_MAX_TURN 70
#define UD_MAX_TURN 2
#define LR_TURN_MAG 40
#define UD_TURN_MAG 1
#define ROTATIONAL_MAG 2
#define SPEED 160

//Object3d planeObject("Objects/f22.obj");

class Airplane : public TransformGroup {
  public:
    Airplane(double r, double g, double b) {
      allocate(2);
      
      dead = false;
      
      airplane = new TransformGroup(Matrix4::MakeMatrix(), 1);
      camera = new Camera(Vector3::MakeVector(0, 10, -18), Vector3::MakeVector(0, 5.0, 1), Vector3::MakeVector(0, 1, 0));
      planeObject = new AirplaneObject("Objects/f22.obj");
      planeObject->scale(20);
      
      airplane->addChild(*planeObject);
      
      addChild(*camera);
      addChild(*airplane);
      
      lrTurnAccel = 0;
      lrTurnVel = 0;
      udTurnAccel = 0;
      udTurnVel = 0;
      direction = Vector3::MakeVector(0, 0, 1);
      position = Vector3::MakeVector(0, 50, 0);
      
      this->r = r;
      this->b = b;
      this->g = g;
    }
    
    ~Airplane() {
      delete airplane;
      delete camera;
      delete planeObject;
    }
    
    void step(double elapsed) {
      if (dead) {
        timeSinceDeath += elapsed;
        planeObject->step(elapsed);
        return;
      }
      
      /**** LEFT/RIGHT TURNING ****/
      double lrTurnMagnitude = LR_TURN_MAG * elapsed;
      
      if (lrTurnAccel != 0) {
        lrTurnVel += lrTurnAccel * lrTurnMagnitude;
      }
      else {
        if (fabs(lrTurnVel) < lrTurnMagnitude) {
          lrTurnVel = 0;
        }
      }
      
      if (lrTurnVel > 0 && lrTurnAccel != RIGHT) {
        lrTurnVel -= lrTurnMagnitude;
      }
      if (lrTurnVel < 0 && lrTurnAccel != LEFT) {
        lrTurnVel += lrTurnMagnitude;
      }
      
      if (lrTurnVel > LR_MAX_TURN) {
        lrTurnVel = LR_MAX_TURN;
      }
      else if (lrTurnVel < -LR_MAX_TURN) {
        lrTurnVel = -LR_MAX_TURN;
      }
      
      /**** UP/DOWN TURNING ****/
      double udTurnMagnitude = UD_TURN_MAG * elapsed;
      
      if (udTurnAccel != 0) {
        udTurnVel += udTurnAccel * udTurnMagnitude;
      }
      else {
        if (fabs(udTurnVel) < udTurnMagnitude) {
          udTurnVel = 0;
        }
      }
      
      if (udTurnVel > 0 && udTurnAccel != UP) {
        udTurnVel -= udTurnMagnitude;
      }
      if (udTurnVel < 0 && udTurnAccel != DOWN) {
        udTurnVel += udTurnMagnitude;
      }
      
      if (udTurnVel > UD_MAX_TURN) {
        udTurnVel = UD_MAX_TURN;
      }
      else if (udTurnVel < -UD_MAX_TURN) {
        udTurnVel = -UD_MAX_TURN;
      }
      
      direction = Matrix4::RotationYMatrix(-lrTurnVel * ROTATIONAL_MAG * elapsed).multiply(direction);
      Vector3 movement = direction + Vector3::MakeVector(0, udTurnVel, 0);
      position = position + movement.scale(SPEED * elapsed);
      double angle = atan2(-direction[Z], direction[X]);
      
      airplane->getMatrix() = Matrix4::RotationZMatrix(lrTurnVel).multiply(Matrix4::RotationXMatrix(-udTurnVel * 20));
      
      getMatrix() = Matrix4::TranslationMatrix(position).multiply(
                    Matrix4::RotationYMatrix(angle * 180 / M_PI + 90));
    }
    
    virtual void orient(Vector3& position, Vector3& velocity, Vector3& acceleration) {
      direction = velocity;
      this->position = position;
      
      double angleXZ = atan2(-velocity[Z], velocity[X]) * 180 / M_PI + 90;
      double angleY = asin(velocity.normalize()[Y]) * 180 / M_PI;
      Vector3 orthoVel = Vector3::MakeVector(-velocity[Z], 0, velocity[X]);
      double turnAngle = acos(orthoVel.dot(acceleration) / (orthoVel.length() * acceleration.length())) * 180 / M_PI - 90;
      
      airplane->getMatrix() = Matrix4::RotationZMatrix(-turnAngle).multiply(Matrix4::RotationXMatrix(-angleY));
      
      getMatrix() = Matrix4::TranslationMatrix(position).multiply(Matrix4::RotationYMatrix(angleXZ));
    }
    
    void turnLeft() {
      lrTurnAccel = LEFT;
    }
    
    void turnRight() {
      lrTurnAccel = RIGHT;
    }

    void turnUp() {
      udTurnAccel = UP;
    }

    void turnDown() {
      udTurnAccel = DOWN;
    }
    
    void udStopTurn() {
      udTurnAccel = 0;
    }
    
    void lrStopTurn() {
      lrTurnAccel = 0;
    }
    
    void setPosition(Vector3 p) {
      position = p;
    }
    
    Vector3 getPosition() {
      return position;
    }
    
    Vector3 getDirection() {
      return direction;
    }
    
    double getAngle() {
      return atan2(direction[Z], direction[X]);
    }

    Vector3 getWingTip(bool right=true) {
      return getMatrix().multiply(airplane->getMatrix()).multiply(Vector3::MakeVector(right ? -6.5 : 6.5, -1, -4.5));
    }
    
    Vector3 getNose() {
      return getMatrix().multiply(airplane->getMatrix()).multiply(Vector3::MakeVector(0, 0, 7.5));
    }
    
    void kill() {
      timeSinceDeath = 0;
      planeObject->explode();
      airplane->getMatrix() = Matrix4::MakeMatrix();
      dead = true;
    }
    bool isDead() {
      return dead;
    }
    
    bool doneExploding() {
      return timeSinceDeath > 3;
    }
    
    virtual void drawObject(Matrix4& mat) {
      const float explosionRadius = 9.5;
      glColor3f(r, g, b);
      
      if (dead) {
        glMultMatrixd(this->getMatrix().getPointer());
        airplane->drawObject(mat);
        
        glPushAttrib(GL_ENABLE_BIT);
        glDisable(GL_CULL_FACE);
        glEnable (GL_BLEND);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        glColor4f(1, .4, 0, pow(1 - timeSinceDeath / 2, 2));
        if (true || timeSinceDeath < .5) {
          glutSolidSphere(timeSinceDeath * explosionRadius * 2, 12, 12);
        }
        else if (timeSinceDeath < 1.5) {
          glutSolidSphere(explosionRadius * 1.5 - timeSinceDeath * explosionRadius * .5 * 2, 12, 12);
        }
        
        glPopAttrib();
      }
      else {
        TransformGroup::drawObject(mat);
      }
    }
  private:
    TransformGroup *airplane;
    AirplaneObject *planeObject;
    Camera *camera;
    double lrTurnAccel, lrTurnVel, udTurnAccel, udTurnVel;
    Vector3 direction;
    Vector3 position;
    double r, g, b;
    double timeSinceDeath;
    bool dead;
};


#endif
