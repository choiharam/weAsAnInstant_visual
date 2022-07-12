#pragma once

#include "ofMain.h"
#include "ofxOsc.h"
#include "ofxThreadedImageLoader.h"
#include "ofxAutoReloadedShader.h"
#include "ofxTextureRecorder.h"
#include "Ghost.h"
#include "TrackDiff.h"
#include "Feedback.h"
#include "ThreadedLoadPrepare.h"
#define rPORT 12345
#define sPORT 54321
#define ADDRESS "localhost"


class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void windowResized(int w, int h);
        void randomTrigger();
        void oscReceive();
        int getVideoDeviceByKeyword(ofVideoGrabber vid, string keyword, string second="-1");
        void sendOsc(string addr, float val);
        void sendOsc(string addr, glm::vec2 val);
        void sendOsc(string addr, float v1, float v2, float v3, float v4, float v5, float v6);
        void setRecord(int frameCount);
        void record();
        void loadFromPast();
        
    
        float width, height;
        ofVideoGrabber video;
        ofFbo smallVideoFbo;
        ofFbo videoFbo;
        ofPlanePrimitive plane;
        ofxAutoReloadedShader ghostShader;
        Feedback mainFeedback;
    
        ofFbo ghostFbo;
        glm::vec2 scaleRatio;
        float colorRatio;
        glm::vec2 ghostCentroid;

        vector<Ghost> ghosts;
        bool debug=true;
        bool bfeedBuffer=false;
        int ccount;
        glm::vec2 smallVideoSize;

        float prevTime, delay=1;
     
        ofxOscReceiver receiver;
        ofxOscSender sender;
    
            // Diff analysis
        Diff diff;
    ofFbo verySmallVideoFbo;
    glm::vec2 verySmallRatio;

        

            // video fps measure
        float prevT=0;
        float videoFps=0;
        
            // recording stuff
        ofxTextureRecorder recorder;
        bool brecord=false;
        int recordFrameCount=0;
        int recordStartFrame=0;
        int recordingFrameIdx=0;
        ofJson dataToSave;
        
            // loadingStuff
        bool bload=false;
        vector<string> framesToLoad;
        ofJson infoToLoad;
        int currentLoadFrameIdx=0;
        ThreadedPrepare threadedLoadPrep;
        ofxThreadedImageLoader thLoader;
        ofImage loadingImg;
    
    
            // Testing vars
    ofFbo testInput;
    

};
