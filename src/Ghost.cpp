#include "Ghost.h"

//----------------------------------------------------------------------
/*
        Initialize static members
 */
int Ghost::bufferSize = 70000;
int Ghost::pastBufferSize = 30000;
ofTexture Ghost::sampleFrame = ofTexture();

vector<ofTexture> Ghost::globalFrames = vector<ofTexture>();
vector<double> Ghost::globalColorRate = vector<double>();
vector<glm::vec2> Ghost::globalCentroid = vector<glm::vec2>();
int Ghost::activeFrameCount = 0;
unsigned int Ghost::globalGhostCount=0;
bool Ghost::isBufferFull = false;

vector<ofTexture> Ghost::pastFrames = vector<ofTexture>();
vector<double> Ghost::pastColorRate = vector<double>();
vector<glm::vec2> Ghost::pastCentroid = vector<glm::vec2>();
vector<string> Ghost::pastAlreadyLoaded = vector<string>();
int Ghost::pastLoadedFrames=0;
bool Ghost::isPastBufferFull=false;


//----------------------------------------------------------------------
void Ghost::setup(ofTexture sampleTexture){
    Ghost::sampleFrame.allocate(sampleTexture.getWidth(), sampleTexture.getHeight(), sampleTexture.getTextureData().glInternalFormat);
    Ghost::globalFrames.resize(Ghost::bufferSize, Ghost::sampleFrame);
    Ghost::globalColorRate.resize(Ghost::bufferSize);
    Ghost::globalCentroid.resize(Ghost::bufferSize);
    
    Ghost::pastFrames.resize(Ghost::pastBufferSize, Ghost::sampleFrame);
    Ghost::pastColorRate.resize(Ghost::pastBufferSize);
    Ghost::pastCentroid.resize(Ghost::pastBufferSize);
    
}

//----------------------------------------------------------------------
void Ghost::receiveFrame(ofTexture& tex, double colorRate, glm::vec2 centroid){
    ofPixels px;
    tex.readToPixels(px);
    ofImage img;
    img.setUseTexture(true);
    img.setFromPixels(px);
//    img.getTexture().enableMipmap();
//    img.getTexture().setTextureMinMagFilter(GL_LINEAR, GL_LINEAR);
//    img.getTexture().getTextureData().bFlipTexture = true;
    Ghost::globalFrames[activeFrameCount] = img.getTexture();
    Ghost::globalColorRate[activeFrameCount] = colorRate;
    Ghost::globalCentroid[activeFrameCount] = centroid;

    Ghost::activeFrameCount++;
    if(Ghost::activeFrameCount> Ghost::activeFrameCount%bufferSize) Ghost::isBufferFull=true;
    Ghost::activeFrameCount = Ghost::activeFrameCount%bufferSize;
}

//----------------------------------------------------------------------
void Ghost::draw(){
    
}
//----------------------------------------------------------------------
/*
        end of static members
 */

//----------------------------------------------------------------------
void Ghost::setup(float w, float h, float size, float life){
    
    renderFrame.allocate(w, h, GL_RGBA);
    width = w;
    height= h;
    
    if(size==-1) ghostSize = ofRandom(gSizeMin, gSizeMax);
    else ghostSize = size;
    pos.x = ofRandom(0,(3.f-(1.f/ghostSize))/3.f);
    pos.y = ofRandom(0,(3.f-(1.f/ghostSize))/3.f);
    
    Ghost::globalGhostCount++;
    if(life==-1) lifespan = ofRandom(0, 600) + 360;
    else lifespan = life*60.f;
    
    
        // decide past or current
    if(ofRandomuf()>0.5){ // current
        fromPast = false;
        if(Ghost::isBufferFull){
            startPos = ofRandom(0, Ghost::bufferSize-lifespan);
            currentPos = startPos;
        }else{
            startPos = ofRandom(0, Ghost::activeFrameCount);
            currentPos = startPos;
        }
    }else{ // past
        fromPast = true;
        if(Ghost::isPastBufferFull){
            startPos = ofRandom(0, Ghost::pastBufferSize-lifespan);
            currentPos = startPos;
        }else{
            startPos = ofRandom(0, Ghost::pastLoadedFrames-lifespan);
            currentPos = startPos;
        }
    }
    
    
    debugColor = ofRandom(0,360);
    
}
//----------------------------------------------------------------------
GhostInfo Ghost::update(){
    GhostInfo info;
    if(currentPos > startPos + lifespan) isDead = true;

    if(!isDead){
        if(!fromPast){
            renderFrame = Ghost::globalFrames[currentPos];
            currentPos = (currentPos+1)%Ghost::globalFrames.size();
            lifeNormal = float(currentPos-startPos) / float(lifespan);
            // 1. size 2. lifespan, colorratio
            info.size = 1.f/ghostSize;
            info.lifespan = float(lifespan)/60.f;
            info.colorRate = Ghost::globalColorRate[currentPos];
            info.centroid = Ghost::globalCentroid[currentPos];
        }else{
            renderFrame = Ghost::pastFrames[currentPos];
            currentPos = (currentPos+1)%Ghost::pastFrames.size();
            lifeNormal = float(currentPos-startPos) / float(lifespan);
            // 1. size 2. lifespan, colorratio
            info.size = 1.f/ghostSize;
            info.lifespan = float(lifespan)/60.f;
            info.colorRate = Ghost::pastColorRate[currentPos];
            info.centroid = Ghost::pastCentroid[currentPos];
        }
    }
    
    return info;
}

//----------------------------------------------------------------------
