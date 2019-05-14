#include "ofApp.h"

/*
 * Settings
 */

/* Audio */
#define BUFFER_SIZE 256
#define SAMPLE_RATE ofxVokaturi::SAMPLE_RATE

//--------------------------------------------------------------
void ofApp::setup(){
    ofSetBackgroundAuto(false);
    ofSoundStreamSettings settings;
    
    soundStream.printDeviceList();
    
    auto devices = soundStream.getMatchingDevices("default");
    if(!devices.empty()){
        settings.setOutDevice(devices[0]);
    }
    
    settings.setInListener(this);
    settings.sampleRate = SAMPLE_RATE;
    settings.numOutputChannels = 0;
    settings.numInputChannels = 1;
    settings.bufferSize = BUFFER_SIZE;
    soundStream.setup(settings);
//    

    ofEnableSmoothing();
    ofSetCircleResolution(50);
    //neutral color
    neutralC.r =245;
    neutralC.g =245;
    neutralC.b =256;
    //happy color
    happyC.r = 255;
     happyC.g = 247;
    happyC.b  = 0;
    //sad color
        sadC.r = 0;
        sadC.g = 75;
        sadC.b = 255;
    //anger color
        angerC.r = 255;
       angerC.g = 0;
       angerC.b = 0;
    
    //fear color
       fearC.r= 0;
     fearC.g= 255;
      fearC.b= 0;

    ofBackground(0);
    
    //Kinnecrt setup
    ofSetLogLevel(OF_LOG_VERBOSE);
    
    // enable depth->video image calibration
    kinect.setRegistration(true);
    
    kinect.init();
    //kinect.init(true); // shows infrared instead of RGB video image
    //kinect.init(false, false); // disable video image (faster fps)
    
    kinect.open();        // opens first available kinect
    //kinect.open(1);    // open a kinect by id, starting with 0 (sorted by serial # lexicographically))
    //kinect.open("A00362A08602047A");    // open a kinect using it's unique serial #
    
    // print the intrinsic IR sensor values
    if(kinect.isConnected()) {
        ofLogNotice() << "sensor-emitter dist: " << kinect.getSensorEmitterDistance() << "cm";
        ofLogNotice() << "sensor-camera dist:  " << kinect.getSensorCameraDistance() << "cm";
        ofLogNotice() << "zero plane pixel size: " << kinect.getZeroPlanePixelSize() << "mm";
        ofLogNotice() << "zero plane dist: " << kinect.getZeroPlaneDistance() << "mm";
    }
    
#ifdef USE_TWO_KINECTS
    kinect2.init();
    kinect2.open();
#endif
    
    colorImg.allocate(kinect.width, kinect.height);
    grayImage.allocate(kinect.width, kinect.height);
    grayThreshNear.allocate(kinect.width, kinect.height);
    grayThreshFar.allocate(kinect.width, kinect.height);
    
    //LOOK AT THESE NUMBERS
    nearThreshold = 1000;
    farThreshold = -1200;
    bThreshWithOpenCV = true;
    
    ofSetFrameRate(40);
    
    // zero the tilt on startup
//    angle = 0;
//    kinect.setCameraTiltAngle(angle);
    
    // start from the front
    bDrawPointCloud = true;
}

//--------------------------------------------------------------
void ofApp::update(){
    kinect.update();
    valid = vokaturi.isValid();
    if(valid)
        emotions = vokaturi.getEmotions();
   //map alpha and updat eto color reading
    neutralC.a = ofMap(emotions.neutrality,0,1,0,255);
    happyC.a =ofMap(emotions.happiness,0,1,0,255);
    sadC.a= ofMap(emotions.sadness,0,1,0,255);
    angerC.a= ofMap(emotions.anger, 0,1, 0,255);
    fearC.a = ofMap(emotions.fear, 0,1, 0,255) ;
    
    color.r=  (neutralC.r *emotions.happiness)+(happyC.r * emotions.happiness)+ (sadC.r *emotions.sadness)+ (angerC.r *emotions.anger)+ (fearC.r *emotions.fear);
    
    color.g= (neutralC.g *emotions.happiness)+(happyC.g * emotions.happiness)+ (sadC.g *emotions.sadness)+ (angerC.g *emotions.anger)+ (fearC.g *emotions.fear);
    
   color.b= (neutralC.b *emotions.happiness)+(happyC.b * emotions.happiness)+ (sadC.b *emotions.sadness)+ (angerC.b *emotions.anger)+ (fearC.b *emotions.fear);
   

//    //update kinnect
    // there is a new frame and we are connected
    if(kinect.isFrameNew()) {
        
        // load grayscale depth image from the kinect source
        grayImage.setFromPixels(kinect.getDepthPixels());
        
        // we do two thresholds - one for the far plane and one for the near plane
        // we then do a cvAnd to get the pixels which are a union of the two thresholds
        if(bThreshWithOpenCV) {
            grayThreshNear = grayImage;
            grayThreshFar = grayImage;
            grayThreshNear.threshold(nearThreshold, true);
            grayThreshFar.threshold(farThreshold);
            cvAnd(grayThreshNear.getCvImage(), grayThreshFar.getCvImage(), grayImage.getCvImage(), NULL);
        } else {
            
            // or we do it ourselves - show people how they can work with the pixels
            ofPixels & pix = grayImage.getPixels();
            int numPixels = pix.size();
            for(int i = 0; i < numPixels; i++) {
                if(pix[i] < nearThreshold && pix[i] > farThreshold) {
                    pix[i] = 255;
                } else {
                    pix[i] = 0;
                }
            }
        }
        
        // update the cv images
        grayImage.flagImageChanged();
        
        // find contours which are between the size of 20 pixels and 1/3 the w*h pixels.
        // also, find holes is set to true so we will get interior contours as well....
        contourFinder.findContours(grayImage, 10, (kinect.width*kinect.height)/2, 20, false);
    }
    
#ifdef USE_TWO_KINECTS
    kinect2.update();
#endif
}

//--------------------------------------------------------------

#define DRAW_EMO(__emo, y)                        \
ofDrawBitmapString(#__emo + ofToString(": ")            \
+ ofToString(emotions.__emo, 4), 4, (1 + y) * 18)

void ofApp::draw(){
    ofPushStyle();
    ofSetColor(0, 40);

       ofDrawRectangle(0, 0, 250, 150);
    ofPopStyle();
    ofPushStyle();
    ofSetColor(0, 3);
      ofDrawRectangle(0, 0, ofGetWindowWidth(), ofGetWindowHeight());
        ofPopStyle();
    
    ofDrawBitmapString(ofToString("VOKATURI STATUS: ")
                       + (valid ? "VALID" : "NOT VALID"), 4, 18);
   

 ofSetColor(color);
//    ofDrawCircle(ofGetWindowWidth()/2, ofGetWindowHeight()/2,  ofMap(emotions.neutrality,0,1,0,100));
//    
//
//    ofDrawCircle(ofGetWindowWidth()/2, ofGetWindowHeight()/2,  ofMap(emotions.happiness,0,1,0,100));
//
//    ofDrawCircle(ofGetWindowWidth()/2, ofGetWindowHeight()/2,  ofMap(emotions.sadness,0,1,0,100));
//
//    ofDrawCircle(ofGetWindowWidth()/2, ofGetWindowHeight()/2,  ofMap(emotions.anger,0,1,0,100));
//
//    ofDrawCircle(ofGetWindowWidth()/2, ofGetWindowHeight()/2,ofMap(emotions.fear,0,1,0,100));
    
    ofPushStyle();
    ofSetColor(0);
//    ofDrawCircle(mouseX, mouseY, 10);
    ofPopStyle();
    ofPushStyle();
//    ofSetColor(0,255,255);
    ofSetColor(neutralC);
    DRAW_EMO(neutrality, 1);
    ofSetColor(happyC);
    DRAW_EMO(happiness, 2);
     ofSetColor(sadC);
    DRAW_EMO(sadness, 3);
     ofSetColor(angerC);
    DRAW_EMO(anger, 4);
     ofSetColor(fearC);
    DRAW_EMO(fear, 5);
    ofPopStyle();
    
    
    //kinnect{
    ofSetColor(0, 0, 0);
    
    if(bDrawPointCloud) {
        easyCam.begin();
        drawPointCloud();
        easyCam.end();
    } else {
        // draw from the live kinect
//        kinect.drawDepth(10, 10, 400, 300);
//        kinect.draw(420, 10, 400, 300);
//        
//        grayImage.draw(10, 320, 400, 300);
//       contourFinder.draw(10, 320, 400, 300);
        
#ifdef USE_TWO_KINECTS
        kinect2.draw(420, 320, 400, 300);
#endif
    }
    
    // draw instructions
    ofSetColor(255, 255, 255);
    stringstream reportStream;
    
//    if(kinect.hasAccelControl()) {
//        reportStream << "accel is: " << ofToString(kinect.getMksAccel().x, 2) << " / "
//        << ofToString(kinect.getMksAccel().y, 2) << " / "
//        << ofToString(kinect.getMksAccel().z, 2) << endl;
//    } else {
//        reportStream << "Note: this is a newer Xbox Kinect or Kinect For Windows device," << endl
//        << "motor / led / accel controls are not currently supported" << endl << endl;
//    }
//
//    reportStream << "press p to switch between images and point cloud, rotate the point cloud with the mouse" << endl
//    << "using opencv threshold = " << bThreshWithOpenCV <<" (press spacebar)" << endl
//    << "set near threshold " << nearThreshold << " (press: + -)" << endl
//    << "set far threshold " << farThreshold << " (press: < >) num blobs found " << contourFinder.nBlobs
//    << ", fps: " << ofGetFrameRate() << endl
//    << "press c to close the connection and o to open it again, connection is: " << kinect.isConnected() << endl;
//
//    if(kinect.hasCamTiltControl()) {
//        reportStream << "press UP and DOWN to change the tilt angle: " << angle << " degrees" << endl
//        << "press 1-5 & 0 to change the led mode" << endl;
//    }
    
    ofDrawBitmapString(reportStream.str(), 20, 652);
}

//--------------------------------------------------------------
void ofApp::audioIn(ofSoundBuffer & input)
{
    vokaturi.record(input.getBuffer());
}

void ofApp::drawPointCloud() {
    int w = 640;
    int h = 480;
    ofMesh mesh;
    mesh.setMode(OF_PRIMITIVE_POINTS);
    int step = 2;
    for(int y = 0; y < h; y += step) {
        for(int x = 0; x < w; x += step) {
            if(kinect.getDistanceAt(x, y) > 0) {
//              mesh.addColor(kinect.getColorAt(x,y));
           mesh.addColor(color);
                mesh.addVertex(kinect.getWorldCoordinateAt(x, y));
            }
        }
    }
    glPointSize(5);
    ofPushMatrix();
    // the projected points are 'upside down' and 'backwards'
    ofScale(1, -1, -1);
    ofTranslate(0, 0, -1000); // center the points a bit
    ofEnableDepthTest();
    mesh.drawVertices();
    ofDisableDepthTest();
    ofPopMatrix();
}

//--------------------------------------------------------------
void ofApp::exit() {
    kinect.setCameraTiltAngle(0); // zero the tilt on exit
    kinect.close();
    
#ifdef USE_TWO_KINECTS
    kinect2.close();
#endif
}
