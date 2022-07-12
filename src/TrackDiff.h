#pragma once
#include "ofMain.h"
#include "ofxCv.h"
#include "ofxAutoReloadedShader.h"


class Track{
public:
    void setup();
    glm::vec2 update(ofPixels source);
    void draw();
    
    float threshold;
    ofxCv::ContourFinder contourFinder;
    
    int currentCount;
    int prevCount;
    int newCount;
    int deadCount;
    int totalCount=0;
    string counts;
    vector<glm::vec2> poses;
};



class Diff{
public:
    void setup(float w, float h, ofTexture sampleTex);
    float update(ofTexture tex);
    void draw();
        
    float width, height;
    ofxAutoReloadedShader shader;
    ofPlanePrimitive plane;
    
    
    ofPixels rawpx;
    ofPixels bgPx;
    ofPixels diffPx;
    ofFbo bgFbo;
    ofFbo diffFbo;
    Track track;
    float colorRatio=0;
    glm::vec2 centroid;
    
    
};
