#pragma once
#include "ofMain.h"
#include "ofxCv.h"
#include "ofxAutoReloadedShader.h"


class Track{
public:
    void setup();
    glm::vec2 update(ofPixels source);
    void draw();
    
    ofxCv::ContourFinder contourFinder;
    vector<glm::vec2> poses;
    float areaSum;
    
};



class Diff{
public:
    void setup(float w, float h, ofTexture sampleTex);
    float update(ofTexture tex);
    void draw();
        
    float width, height;
    
    
    ofPixels rawpx;
    ofPixels bgPx;
    ofPixels diffPx;
    Track track;
    float colorRatio=0;
    glm::vec2 centroid;
    
    
};
