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
    
    areaSum = 0;
    if(contourFinder.size()==1){
        cv::Point2f cp = contourFinder.getCenter(0);
        centroidBuffer.x = cp.x;
        centroidBuffer.y = cp.y;
        areaSum = contourFinder.getContourArea(0);
    }
    else{
        ofPolyline tmp;
        for(int i = 0; i < contourFinder.size(); i++) {
            ofPoint center = toOf(contourFinder.getCenter(i));
            poses.push_back(glm::vec2(center.x, center.y));
            tmp.addVertex(center.x, center.y);
            areaSum+= contourFinder.getContourArea(i);
        }
        centroidBuffer = tmp.getCentroid2D();
    }
    
    centroid.x = ofLerp(centroid.x, centroidBuffer.x, 0.07);
    centroid.y = ofLerp(centroid.y, centroidBuffer.y, 0.07);
    
    return centroid;

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
//        if(label> totalCount) totalCount = label;
        string msg = ofToString(label);
        ofDrawBitmapString(msg, 0, 0);
        ofVec2f velocity = toOf(contourFinder.getVelocity(i));
        ofScale(5, 5);
        ofDrawLine(0, 0, velocity.x, velocity.y);
        ofPopMatrix();
    }
    ofPushStyle();
    ofSetColor(255,0,0);
    ofDrawCircle(centroid, 5);
    ofPopStyle();
    
    
}

//----------------------------------------------------------------------------
//------------------------------------------------------------------------
void Diff::setup(float w, float h, ofTexture sampleTex){
    width = w; height = h;
    
    ofPixels p;
    sampleTex.readToPixels(p);
    imitate(bgPx, p);
    imitate(diffPx, p);
    imitate(rawpx, p);
    
    track.setup();
}

//------------------------------------------------------------------------
float Diff::update(ofTexture tex){

        
    tex.readToPixels(rawpx);
    centroid = track.update(rawpx);
    centroid.x /= rawpx.getWidth();
    centroid.y /= rawpx.getHeight();

//    rawpx.setImageType(OF_IMAGE_GRAYSCALE);
//    absdiff(rawpx, bgPx, diffPx);

    float ratio = track.areaSum / (rawpx.getWidth()*rawpx.getHeight());
    colorRatio = ratio * 100.f;

    return colorRatio;

    
}

//------------------------------------------------------------------------
void Diff::draw(){
    track.draw();
}
