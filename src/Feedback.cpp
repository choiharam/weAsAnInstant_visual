#include "Feedback.h"
//----------------------------------------------------------------------
void Feedback::setup(float w, float h){
    width = w;
    height = h;
 
        // pingpong
    ppShader.load("shaders/feedback");
    vector<float> d(width*height*3);
    for(size_t i=0; i<width*height; i++){
        d[i*3+0] = 0;
        d[i*3+1] = 0;
        d[i*3+2] = 0;
    }
    
    src.allocate(width, height, GL_RGB32F);
    dst.allocate(width, height, GL_RGB32F);
    src.getTexture().setTextureMinMagFilter(GL_LINEAR, GL_LINEAR);
    src.getTexture().setTextureMinMagFilter(GL_LINEAR, GL_LINEAR);
    src.getTexture().loadData(d.data(), width, height, GL_RGB);
    dst.getTexture().loadData(d.data(), width, height, GL_RGB);
    
    
}

//----------------------------------------------------------------------
void Feedback::update(ofTexture & vid){
    
    
    
    dst.begin();
    ppShader.begin();
    ppShader.setUniform1f("time", ofGetElapsedTimef());
    ppShader.setUniform2f("resolution", width, height);
    ppShader.setUniformTexture("backbuffer", src.getTexture(), 0);
    ppShader.setUniformTexture("video", vid, 1);
    ppShader.setUniform1f("refreshSpeed", refreshSpeed);
    ppShader.setUniform1f("vergangen", vergangen);
    src.draw(0,0);
    ppShader.end();
    dst.end();
    std::swap(src,dst);
}

//----------------------------------------------------------------------
void Feedback::draw(){
    dst.draw(0,0);
    
    
}
//----------------------------------------------------------------------
