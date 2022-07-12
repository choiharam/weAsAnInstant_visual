#include "TrackDiff.h"

using namespace ofxCv;
using namespace cv;

//----------------------------------------------------------------------------
void Track::setup(){
    
    contourFinder.setMinAreaRadius(1);
    contourFinder.setMaxAreaRadius(20000);
    contourFinder.setThreshold(15);
    contourFinder.getTracker().setPersistence(15);
    contourFinder.getTracker().setMaximumDistance(100); // movement in 1 fps
    
}

//----------------------------------------------------------------------------
glm::vec2 Track::update(ofPixels source){
//    ofPixels px;
//    source.readToPixels(px);
//    blur(px, 15);

    contourFinder.findContours(source);

    poses.clear();
    ofPolyline tmp;
    for(int i = 0; i < contourFinder.size(); i++) {
        ofPoint center = toOf(contourFinder.getCenter(i));
        poses.push_back(glm::vec2(center.x, center.y));
        tmp.addVertex(center.x, center.y);
    }

    return tmp.getCentroid2D();// / glm::vec2(px.getWidth(), px.getHeight());

}


//----------------------------------------------------------------------------
void Track::draw(){
    
    ofSetColor(0,255,0);
    contourFinder.draw();
    ofSetColor(255);
    poses.clear();
    for(int i = 0; i < poses.size(); i++) {
        ofPoint center = toOf(contourFinder.getCenter(i));
        poses.push_back(glm::vec2(center.x, center.y));
        ofPushMatrix();
        ofTranslate(center.x, center.y);
        int label = i;
        if(label> totalCount) totalCount = label;
        string msg = ofToString(label);// + ":" + ofToString(tracker.getAge(label));
        ofDrawBitmapString(msg, 0, 0);
        ofVec2f velocity = toOf(contourFinder.getVelocity(i));
        ofScale(5, 5);
        ofDrawLine(0, 0, velocity.x, velocity.y);
        ofPopMatrix();
    }
    
}

//----------------------------------------------------------------------------
//------------------------------------------------------------------------
void Diff::setup(float w, float h, ofTexture sampleTex){
    width = w; height = h;
    shader.load("shaders/diff");
    
    
    
    ofPixels p;
    sampleTex.readToPixels(p);
    imitate(bgPx, p);
    imitate(diffPx, p);
    imitate(rawpx, p);
    
    bgFbo.allocate(width,height,GL_RGB);
    bgFbo.begin(); ofClear(0); bgFbo.end();
    diffFbo.allocate(width,height,GL_RGB);
    diffFbo.begin(); ofClear(0); diffFbo.end();
    plane.set(width, height, 2,2);
    plane.mapTexCoords(0,0,width,height);
    plane.setPosition(width/2, height/2, 0);
    track.setup();
}

//------------------------------------------------------------------------
float Diff::update(ofTexture tex){

        
    tex.readToPixels(rawpx);
    centroid = track.update(rawpx);

    rawpx.setImageType(OF_IMAGE_GRAYSCALE);
    absdiff(rawpx, bgPx, diffPx);


    float count = 0;
    for(auto p: rawpx){
        if(p>25) count+=1;
    }
    float ratio = count / float(rawpx.getWidth()*rawpx.getHeight());
    colorRatio = ratio;

    return colorRatio;

    
}

//------------------------------------------------------------------------
void Diff::draw(){
    diffFbo.draw(0,0);
    track.draw();
}
