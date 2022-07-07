#pragma once
#include "ofMain.h"
#include <atomic>

class ThreadedPrepare: public ofThread{
public:

    ~ThreadedPrepare(){
        stop();
        waitForThread(false);
    }

    void setup(){
        isPrepared=false;
        start();
    }
    void start(){
        startThread();
    }

    void stop(){
        isPrepared=false;
        std::unique_lock<std::mutex> lck(mutex);
        stopThread();
        condition.notify_all();
    }
    void threadedFunction(){
        while(isThreadRunning()){
            
            std::unique_lock<std::mutex> lock(mutex);

            // locked since now
            
            
            // loading image list
            ofDirectory dir;
            framesToLoad.clear();
            infoToLoad.clear();
            vector<string> notusedPast;
            string pastroot = "past/";
            dir.listDir(pastroot);
            dir.sort();
            for(auto f:dir.getFiles()){
                if(f.isDirectory())
                    notusedPast.push_back(f.getFileName());
            }
            for(int i=0; i<Ghost::pastAlreadyLoaded.size(); ++i){
                for(int j=0; j<notusedPast.size(); ++j){
                    if(Ghost::pastAlreadyLoaded[i] == notusedPast[j]){
                        notusedPast.erase(notusedPast.begin()+j);
                    }
                }
            }
            
            string choice = notusedPast[int(ofRandom(notusedPast.size()))];
            Ghost::pastAlreadyLoaded.push_back(choice);
            dir.listDir(pastroot+choice);
            cout << pastroot+choice << endl;
            dir.sort();
            for(int i=0; i<dir.size(); ++i){
                if(dir.getFile(i).getExtension() == "png"){
        //            printf("%s", dir.getFile(i).getAbsolutePath().c_str());
                    framesToLoad.push_back(dir.getFile(i).getAbsolutePath());
                }
            }
            
            // loading json file
            ofFile file(pastroot+choice+"/info.json");
            if(file.exists()){
                file >> infoToLoad;
//                for(auto & frame: infoToLoad){
//                    if(!frame.empty()){
//                        cout << frame["centroid"]["x"] << '\t' <<frame["centroid"]["y"] << endl;
//                        cout << frame["crate"] << endl;
//                    }
//                }
            }
            
            isPrepared = true;


            // wait for the main thread since now
            condition.wait(lock);
        }
    }

    void update(){
        // locked
        std::unique_lock<std::mutex> lock(mutex);
        condition.notify_all();
    }
    
    bool isReady(){
        return isPrepared;
    }
    
    vector<string> getList(){
        return framesToLoad;
    }
    ofJson getInfo(){
        return infoToLoad;
    }
    


protected:
    std::condition_variable condition;
    bool isPrepared=false;
    vector<string> framesToLoad;
    ofJson infoToLoad;
    
};
