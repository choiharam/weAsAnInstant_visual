#pragma once

#include "ofMain.h"
//#include "ofxPingPong.h"
#include "ofxAutoReloadedShader.h"

class Feedback{
public:
    void setup(float w, float h);
    void update(ofTexture & vid);
    void draw();
    
    float width, height;
    ofPlanePrimitive plane;
    ofxAutoReloadedShader ppShader;
    float refreshSpeed=0.044;//=0.066f;
    float vergangen= 0.001;
    
    ofFbo src, dst;
    
};
