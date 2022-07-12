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
    mainFeedback.setup(1920, 1080);
    
    smallVideoSize = {960, 540};
    smallVideoFbo.allocate(smallVideoSize.x, smallVideoSize.y, GL_RGBA);
    smallVideoFbo.begin(); ofClear(0,0,0,0); smallVideoFbo.end();
    Ghost::setup(smallVideoFbo.getTexture());


    scaleRatio.x = smallVideoSize.x / video.getWidth();
    scaleRatio.y = smallVideoSize.y / video.getHeight();
    receiver.setup(rPORT);
    sender.setup(ADDRESS, sPORT);
    
        // diff analysis
    verySmallVideoFbo.allocate(480, 270, GL_RGBA);
    verySmallRatio.x = verySmallVideoFbo.getWidth() / video.getWidth();
    verySmallRatio.y = verySmallVideoFbo.getHeight() / video.getHeight();
    diff.setup(verySmallVideoFbo.getWidth(), verySmallVideoFbo.getHeight(), verySmallVideoFbo.getTexture());
    
        // recording stuff
    ofxTextureRecorder::Settings settings(smallVideoFbo.getTexture());
    settings.imageFormat = OF_IMAGE_FORMAT_PNG;
    settings.numThreads = 12;
    settings.maxMemoryUsage = 9000000000;
    settings.folderPath="";
    recorder.setup(settings);
    
    
            // testing stuff
    testInput.allocate(1920, 1080, GL_RGBA);
    testInput.begin(); ofClear(0); testInput.end();
    
}

//--------------------------------------------------------------
void ofApp::update(){
//    testInput.begin();
//    ofClear(0);
//    ofPushMatrix();
//    ofTranslate(mouseX, mouseY);
//    ofRotateZRad(ofGetElapsedTimef());
//    ofSetColor(255);
//    ofSetRectMode(OF_RECTMODE_CENTER);
//    ofDrawRectangle(0, 0, 400, 400);
//    ofSetRectMode(OF_RECTMODE_CORNER);
//    ofPopMatrix();
//    testInput.end();
    
            // receive video stream
    video.update();
    if(video.isFrameNew()){
                // vFps
        videoFps = 1./(ofGetElapsedTimef()-prevT);
        prevT =ofGetElapsedTimef();
                // Feedback main
        mainFeedback.update(video.getTexture());
                // Small fbo for ghosts
        smallVideoFbo.begin();
        ofClear(0);
        ofPushMatrix();
        ofScale(scaleRatio.x, scaleRatio.y);
        mainFeedback.dst.getTexture().draw(0,0);
        ofPopMatrix();
        smallVideoFbo.end();
                // Very small fbo for cving
        verySmallVideoFbo.begin();
        ofClear(0);
        ofPushMatrix();
        ofScale(verySmallRatio.x, verySmallRatio.y);
        mainFeedback.dst.getTexture().draw(0,0);
        ofPopMatrix();
        verySmallVideoFbo.end();
        
                // Cving
        float ratio = diff.update(verySmallVideoFbo.getTexture());
        colorRatio = ratio*100.f;
        ghostCentroid = diff.centroid;
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
    if(ghosts.size()<6) randomTrigger();
    
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
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    ofClear(0,255);
    mainFeedback.dst.draw(1920,0);
    for(int i=0; i<ghosts.size(); ++i){
        ofPushMatrix();
        ofTranslate(ghosts[i].pos.x*glm::vec2(width,height));
        ofScale(1.f/ghosts[i].ghostSize);
        ofSetColor(255*sin(ghosts[i].lifeNormal*PI));
        ghosts[i].renderFrame.draw(0,0);
        ofPopMatrix();
    }
    ofDisableBlendMode();
    
    
    
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
        ss << "Loaded" << '\t' << Ghost::pastLoadedFrames << '\n';
        ss << "lpThread" << '\t' <<threadedLoadPrep.isThreadRunning()<< '\n'<< '\n';
        ss << "ID" << '\t' << "life" << '\t' << "posX" << '\t' << "posY" << '\t' << "size" << '\n';
        
        for(int i=0; i<ghosts.size(); ++i){
            ss.precision(3);
            ss << ofToString(i) << '\t' << ghosts[i].lifeNormal << '\t';
            ss.unsetf(ios::fixed);
            ss << ghosts[i].pos.x << '\t' << ghosts[i].pos.y << '\t';
            ss << ghosts[i].ghostSize<< '\n';
            
            if(ghosts[i].fromPast){
                ofNoFill();
                ofRectangle r;
                r.set(
                      0,
                      0,
                      smallVideoSize.x,
                      smallVideoSize.y
                      );
                ofColor c = ofColor::fromHsb(ghosts[i].debugColor, 255,255);
                ofSetColor(c);
                ofPushMatrix();
                ofTranslate(ghosts[i].pos.x*glm::vec2(width,height));
                ofScale(1.f/ghosts[i].ghostSize);
                ofDrawRectangle(r);
                ofPopMatrix();
                ofSetColor(255);
                ofFill();
            }
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
void ofApp::windowResized(int w, int h){
    width = w;
    height = h;
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

//--------------------------------------------------------------
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

//--------------------------------------------------------------
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

//--------------------------------------------------------------
void ofApp::loadFromPast(){
    if(bload){
        if(currentLoadFrameIdx<framesToLoad.size()){
            if(!thLoader.isThreadRunning()){
                thLoader.loadFromDisk(loadingImg, framesToLoad[currentLoadFrameIdx]);
                return;
            }else{
                int modulatedFrameIdx = (Ghost::pastLoadedFrames+framesToLoad.size())%Ghost::pastBufferSize;
                if(Ghost::pastLoadedFrames+framesToLoad.size() > modulatedFrameIdx){
                    Ghost::isPastBufferFull=true;
                }
                Ghost::pastFrames[(Ghost::pastLoadedFrames+currentLoadFrameIdx)%Ghost::pastBufferSize] = loadingImg.getTexture();
                glm::vec2 tmpCentroid;
                tmpCentroid.x = infoToLoad[currentLoadFrameIdx]["centroid"]["x"];
                tmpCentroid.y = infoToLoad[currentLoadFrameIdx]["centroid"]["y"];
                Ghost::pastCentroid[(Ghost::pastLoadedFrames+currentLoadFrameIdx)%Ghost::pastBufferSize] = tmpCentroid;
                Ghost::pastColorRate[(Ghost::pastLoadedFrames+currentLoadFrameIdx)%Ghost::pastBufferSize] = infoToLoad[currentLoadFrameIdx]["cratio"];
                currentLoadFrameIdx++;
            }
        }else{
            Ghost::pastLoadedFrames += currentLoadFrameIdx;
            bload=false;
        }
    }
}
