#pragma once

#include "ofMain.h"
#include "ofxOsc.h"
#include "ofxAutoReloadedShader.h"
#include "ofxTextureRecorder.h"
#include "Ghost.h"
#include "ThreadedLoadPrepare.h"
#define rPORT 12345
#define sPORT 54321
#define vPORT 9999
#define ADDRESS "192.168.29.157"


class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
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
        ofxOscReceiver vReceiver;
        ofxOscSender sender;

        

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
    
    
    int testIdx=0;
};
