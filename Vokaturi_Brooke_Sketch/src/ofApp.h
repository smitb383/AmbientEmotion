#pragma once

#include "ofxVokaturi.h"
#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxKinect.h"

class ofApp : public ofBaseApp{
private:
    ofSoundStream soundStream;
    ofxVokaturi vokaturi;
    ofxVokaturi::Emotions emotions;
    bool valid;
public:
    void setup();
    void update();
    void draw();
    void exit();
    
    void drawPointCloud();

    
    void audioIn(ofSoundBuffer&);
//    void drawPointCloud();
    //colors / alphas
    ofColor neutralC,happyC,sadC, angerC, fearC;
    float neutralA, happyA, sadA, angerA, fearA;
    float r,g,b;
    ofColor color;
    
////    //camera setup
////#ifdef _USE_LIVE_VIDEO
//    ofVideoGrabber   vidGrabber;
//    ofxCvColorImage colorImg;
//    ofxCvGrayscaleImage GreyImg;
//    ofxCvGrayscaleImage     grayImage;    ofxCvGrayscaleImage     grayBg;
//    ofxCvGrayscaleImage     grayDiff;
//    ofxCvContourFinder     contourFinder;
//    int                 threshold;
//    bool                bLearnBakground;
////#else
//    ofVideoPlayer  vidPlayer;
    

    //kinect setup
    ofxKinect kinect;
    
#ifdef USE_TWO_KINECTS
    ofxKinect kinect2;
#endif
    
    ofxCvColorImage colorImg;
    
    ofxCvGrayscaleImage grayImage; // grayscale depth image
    ofxCvGrayscaleImage grayThreshNear; // the near thresholded image
    ofxCvGrayscaleImage grayThreshFar; // the far thresholded image
    
    ofxCvContourFinder contourFinder;
    
    bool bThreshWithOpenCV;
    bool bDrawPointCloud;
    
    int nearThreshold;
    int farThreshold;
    
    int angle;
    
    // used for viewing the point cloud
    ofEasyCam easyCam;
   
};


