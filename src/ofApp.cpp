#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ofSetVerticalSync(false);
    ofSetFrameRate(60);
    ofBackground(0);
    ofSeedRandom();
    ofSetWindowShape(5760,1080);
    ofSetWindowPosition(0, 0);
    ofDisableArbTex();
    width = 5760;
    height = 1080;
    
    video.setUseTexture(true);
    video.setDeviceID(getVideoDeviceByKeyword(video, "brio", "built"));
    video.setDesiredFrameRate(60);
    video.setup(1920,1080, true);
    videoFbo.allocate(1920,1080,GL_RGBA);
    videoFbo.getTexture().enableMipmap();
    videoFbo.getTexture().setTextureMinMagFilter(GL_LINEAR, GL_LINEAR);
    
    smallVideoSize = {960, 540};
    smallVideoFbo.allocate(smallVideoSize.x, smallVideoSize.y, GL_RGBA);
    Ghost::setup(smallVideoFbo.getTexture());
    plane.set(width,height,2,2);
    plane.mapTexCoords(0, 0, width, height);
    plane.setPosition(width/2, height/2, 0);
    ghostShader.load("shaders/ghost");
    ghostFbo.allocate(width, height, GL_RGBA);
    ghostFbo.getTexture().disableMipmap();
    ghostFbo.getTexture().getTextureData().bFlipTexture=true;
    scaleRatio.x = smallVideoSize.x / video.getWidth();
    scaleRatio.y = smallVideoSize.y / video.getHeight();
    receiver.setup(rPORT);
    vReceiver.setup(vPORT);
    sender.setup(ADDRESS, sPORT);
    
    
        // recording stuff
    ofxTextureRecorder::Settings settings(smallVideoFbo.getTexture());
    settings.imageFormat = OF_IMAGE_FORMAT_PNG;
    settings.numThreads = 12;
    settings.maxMemoryUsage = 9000000000;
    settings.folderPath="";
    recorder.setup(settings);
}

//--------------------------------------------------------------
void ofApp::update(){
    
            // receive video stream
    video.update();
    if(video.isFrameNew()){
        videoFps = 1./(ofGetElapsedTimef()-prevT);
        prevT =ofGetElapsedTimef();
        smallVideoFbo.begin();
        ofClear(0);
        ofPushMatrix();
        ofScale(scaleRatio.x, scaleRatio.y);
        video.draw(0,0);
        ofPopMatrix();
        smallVideoFbo.end();
    }
            // receive osc data
    oscReceive();
    if(colorRatio<0.1) bfeedBuffer = false;
    else bfeedBuffer = true;
    bfeedBuffer = true;
    
            // feed into the buffer
    if(bfeedBuffer)
        Ghost::receiveFrame(smallVideoFbo.getTexture(), colorRatio, ghostCentroid);
    
    
            // send osc to sound
    for(int i=0; i<ghosts.size(); ++i){
        GhostInfo ghostInfo = ghosts[i].update();
        sendOsc("/ghostInfo", i, ghostInfo.size, ghostInfo.lifespan, ghostInfo.colorRate, ghostInfo.centroid.x, ghostInfo.centroid.y);
        if(ghosts[i].isDead){
            ghosts.erase(ghosts.begin()+i);
        }
    }
    if(ghosts.size()<3) randomTrigger();
    
    
    
    
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    ghostFbo.begin();
    ofClear(0,255);
    ofSetColor(0,255);
    ofDrawRectangle(0,0,width,height);
    video.getTexture().draw(1920,0);
    if(ghosts.size()==0){
        ghostShader.begin();
        ghostShader.setUniform1f("count", ghosts.size());
        ghostShader.setUniform2f("resolution", width, height);
        ghostShader.setUniformTexture("mainframe", videoFbo.getTexture(),0);
        plane.draw();
        ghostShader.end();
    }
    else{
        for(int i=0; i<ghosts.size(); ++i){
            ghostShader.begin();
            ghostShader.setUniform1f("count", ghosts.size());
            ghostShader.setUniform2f("resolution", width, height);
            ghostShader.setUniform2f("ghostpos", ghosts[i].pos);
            ghostShader.setUniform1f("ghostlife", ghosts[i].lifeNormal);
            ghostShader.setUniform1f("ghostsize", ghosts[i].ghostSize);
            ghostShader.setUniformTexture("mainframe", videoFbo.getTexture(), 0);
            ghostShader.setUniformTexture("ghosttex", ghosts[i].renderFrame, 1);
            plane.draw();
            ghostShader.end();
        }
    }
    ofDisableBlendMode();
    ghostFbo.end();
    
    record();
    loadFromPast();
    
    if(threadedLoadPrep.isReady()){
        currentLoadFrameIdx=0;
        framesToLoad = threadedLoadPrep.getList();
        infoToLoad = threadedLoadPrep.getInfo();
        threadedLoadPrep.stop();
        bload=true;
    }
    
    
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    
    ghostFbo.draw(0,0);
    
    if(testIdx<Ghost::pastLoadedFrames){
        Ghost::pastFrames[testIdx].draw(0,0);
        testIdx++;
        testIdx = testIdx%Ghost::pastLoadedFrames;
    }
    
    
    if(debug){
        ofSetColor(255);
        stringstream ss;
        ss << "fps" << '\t' << ofGetFrameRate() << '\n';
        ss << "vFps" << '\t' << videoFps << '\n';
        ss << "screen" << '\t' << ofGetWidth() << "," << ofGetHeight() << '\n';
        ss << "video" << '\t' << video.getWidth() << video.getHeight() << '\n';
        ss << "cRatio" << '\t' << colorRatio << '\n';
        ss << "count" << '\t' << ghosts.size() << '\n';
        ss << "gFrame" << '\t' << ofGetFrameNum() << '\n';
        ss << "Buf" << '\t' << (Ghost::isBufferFull?Ghost::globalFrames.size():Ghost::activeFrameCount) << '\n';
        ss << "Record" << '\t' << (brecord?"True":"False") << '\n';
        ss << "Load" << '\t' << (bload?"True":"False") << '\n';
        ss << "lpThread" << '\t' <<threadedLoadPrep.isThreadRunning()<< '\n'<< '\n';
        ss << "ID" << '\t' << "life" << '\t' << "posX" << '\t' << "posY" << '\t' << "size" << '\n';
        
        for(int i=0; i<ghosts.size(); ++i){
            ss.precision(3);
            ss << ofToString(i) << '\t' << ghosts[i].lifeNormal << '\t';
            ss.unsetf(ios::fixed);
            ss << ghosts[i].pos.x << '\t' << ghosts[i].pos.y << '\t';
            ss << ghosts[i].ghostSize<< '\n';
            
            ofNoFill();
            ofRectangle r;
            r.set(
                  width*ghosts[i].pos.x,
                  height*ghosts[i].pos.y,
                  1.f/ghosts[i].ghostSize*width/3,
                  1.f/ghosts[i].ghostSize*height
                  );
            ofColor c = ofColor::fromHsb(ghosts[i].debugColor, 255,255);
            ofSetColor(c);
            ofDrawRectangle(r);
            ofSetColor(255);
            ofFill();
        }
        
        
        ofDrawBitmapStringHighlight(ss.str(), 30,30);
        
    }
    
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if(key == 'a'){
        Ghost g;
        g.setup(smallVideoSize.x, smallVideoSize.y);
        ghosts.push_back(g);
    }
    if(key == 'd'){
        debug = !debug;
    }
    if(key == 'R'){
        printf("%s", "Recording Triggered.");
        setRecord(180);
    }
    if(key == 'L'){
        threadedLoadPrep.setup();
        threadedLoadPrep.update();
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
    width = w;
    height = h;
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
//--------------------------------------------------------------
int ofApp::getVideoDeviceByKeyword(ofVideoGrabber vid, string keyword, string second){
    int deviceId=-1;
    vector<ofVideoDevice> devices = video.listDevices();

    for(int i=0; i<devices.size(); ++i){
        if(ofIsStringInString(ofToLower(devices[i].deviceName), keyword)){
            deviceId = i;
            ofLog() << "Video Device: " << devices[i].deviceName;
            break;
        }
        if(i==devices.size()-1 && deviceId==-1){
            for(int j=0; j<devices.size(); ++j){
                if(ofIsStringInString(ofToLower(devices[j].deviceName), second)){
                    deviceId = j;
                    ofLog() << "Video Device: " << devices[j].deviceName;
                    break;
                }
            }
        }
    }
    return deviceId;
}

//--------------------------------------------------------------
void ofApp::randomTrigger(){
    float t = ofGetElapsedTimef();
    if(t-prevTime > delay){
        Ghost g;
        g.setup(smallVideoSize.x, smallVideoSize.y);
        ghosts.push_back(g);
        delay = ofRandom(0.1,0.5 );
        prevTime = t;
    }
}

//--------------------------------------------------------------
void ofApp::oscReceive(){
        // from camera
    while(vReceiver.hasWaitingMessages()){
        ofxOscMessage m;
        vReceiver.getNextMessage(m);
        
        if(m.getAddress()=="/mainData"){
            colorRatio = m.getArgAsFloat(0);
            ghostCentroid.x = m.getArgAsFloat(1);
            ghostCentroid.y = m.getArgAsFloat(2);
        }
    }
        // from sound
    while(receiver.hasWaitingMessages()){
        ofxOscMessage m;
        receiver.getNextMessage(m);
        
        if(m.getAddress()=="/ghostTrigger"){
            float lifespan;
            float size;
            Ghost g;
            g.setup(smallVideoSize.x, smallVideoSize.y, size, lifespan);
            ghosts.push_back(g);
        }
    }
}

void ofApp::sendOsc(string addr, float val){
    ofxOscMessage m;
    m.setAddress(addr);
    m.addFloatArg(val);
    sender.sendMessage(m);
}
void ofApp::sendOsc(string addr, glm::vec2 val){
    ofxOscMessage m;
    m.setAddress(addr);
    m.addFloatArg(val.x);
    m.addFloatArg(val.y);
    sender.sendMessage(m);
}
void ofApp::sendOsc(string addr, float v1, float v2, float v3, float v4, float v5, float v6){
    ofxOscMessage m;
    m.setAddress(addr);
    m.addFloatArg(v1);
    m.addFloatArg(v2);
    m.addFloatArg(v3);
    m.addFloatArg(v4);
    m.addFloatArg(v5);
    m.addFloatArg(v6);
    sender.sendMessage(m);
}
void ofApp::setRecord(int frameCount){
    recordFrameCount = frameCount;
    recorder.folderPath = "past/"+ofGetTimestampString("%Y_%m_%d_%H_%M_%S_%i")+"/";
    if(!Ghost::isBufferFull){
        if(Ghost::activeFrameCount>frameCount){
            recordStartFrame = ofRandom(0, Ghost::activeFrameCount);
            while(Ghost::activeFrameCount>frameCount+recordStartFrame){
                printf("%s", "Recalculating the recording target\n");
                recordStartFrame = ofRandom(0, Ghost::activeFrameCount);
            }
            brecord = true;
            printf("%s", "Recording setup finished\n");
        }else{
            printf("%s", "Insufficient Frames. Not recording\n");
            return;
        }
    }else{
        recordStartFrame = ofRandom(0, Ghost::globalFrames.size()-frameCount);
        brecord = true;
        printf("%s", "Recording setup finished\n");
    }
    dataToSave.clear();
    recordingFrameIdx = recordStartFrame;
    recorder.setFrame(1);
}

void ofApp::record(){
    if(brecord){
        recorder.save(Ghost::globalFrames[recordingFrameIdx]);
        ofJson singleFrameInfo;
        singleFrameInfo["centroid"]["x"] = Ghost::globalCentroid[recordingFrameIdx].x;
        singleFrameInfo["centroid"]["y"] = Ghost::globalCentroid[recordingFrameIdx].y;
        singleFrameInfo["cratio"] = Ghost::globalColorRate[recordingFrameIdx];
        dataToSave.push_back(singleFrameInfo);
        if(recordingFrameIdx<recordStartFrame+recordFrameCount){
            recordingFrameIdx++;
            return;
        }else{
            ofSavePrettyJson(recorder.folderPath+"info.json", dataToSave);
            brecord=false;
            return;
        }
    }
}


void ofApp::loadFromPast(){
    if(bload){
        if(currentLoadFrameIdx<framesToLoad.size()){
            ofImage tmp;
            tmp.setUseTexture(true);
            tmp.load(framesToLoad[currentLoadFrameIdx]);
            tmp.update();
            int modulatedFrameIdx = (Ghost::pastLoadedFrames+framesToLoad.size())%Ghost::pastBufferSize;
            if(Ghost::pastLoadedFrames+framesToLoad.size() > modulatedFrameIdx){
                Ghost::isPastBufferFull=true;
            }
            Ghost::pastFrames[(Ghost::pastLoadedFrames+currentLoadFrameIdx)%Ghost::pastBufferSize] = tmp.getTexture();
            glm::vec2 tmpCentroid;
            tmpCentroid.x = infoToLoad[currentLoadFrameIdx]["centroid"]["x"];
            tmpCentroid.y = infoToLoad[currentLoadFrameIdx]["centroid"]["y"];
            Ghost::pastCentroid[(Ghost::pastLoadedFrames+currentLoadFrameIdx)%Ghost::pastBufferSize] = tmpCentroid;
            Ghost::pastColorRate[(Ghost::pastLoadedFrames+currentLoadFrameIdx)%Ghost::pastBufferSize] = infoToLoad[currentLoadFrameIdx]["cratio"];
            currentLoadFrameIdx++;
        }else{
            Ghost::pastLoadedFrames += currentLoadFrameIdx;
            bload=false;
        }
    }
}
