#pragma once
#include "ofMain.h"
#define gSizeMin 3.0
#define gSizeMax 0.5

class GhostInfo{
public:
    float size;
    float lifespan;
    float colorRate;
    glm::vec2 centroid;
};

class Ghost{
public:
    
//global
    static void setup(ofTexture sampleTexture);
    static void draw();
    static void receiveFrame(ofTexture& tex, double colorRate, glm::vec2 centroid);
    
    
    static int bufferSize;
    static ofTexture sampleFrame;
    static vector<ofTexture> globalFrames;
    static vector<double> globalColorRate;
    static vector<glm::vec2> globalCentroid;
    static int pastBufferSize;
    static vector<ofTexture> pastFrames;
    static vector<double> pastColorRate;
    static vector<glm::vec2> pastCentroid;
    static int pastLoadedFrames;
    static vector<string> pastAlreadyLoaded;
    static bool isPastBufferFull;
    static int activeFrameCount;
    static unsigned int globalGhostCount;
    static bool isBufferFull;
    
    
    
//instance
    void setup(float w, float h, float size=-1, float life=-1);
    GhostInfo update();
    
    float width,height;
    glm::vec2 pos;
    int lifespan; // unit: frame number
    int startPos;
    int currentPos;
    float ghostSize;
    float lifeNormal;
    bool isDead=false;
    ofTexture renderFrame;
    float debugColor;
    bool fromPast;
    
};
