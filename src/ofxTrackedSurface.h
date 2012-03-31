//
//  ofxTrackedSurface.h
//  mdt-Core
//
//  Created by Patricio González Vivo on 3/31/12.
//  Copyright (c) 2012 PatricioGonzalezVivo.com. All rights reserved.
//

#ifndef OFXTRACKEDSURFACE
#define OFXTRACKEDSURFACE

#include "ofMain.h"

#include "ofxKinect.h"
#include "ofxOpenCv.h"
#include "ofxSurface.h"
#include "ofxBlobTracker.h"
#include "ofxKinectAutoCalibrator.h"

class ofxTrackedSurface {
public:
    ofxTrackedSurface();
    
    void            init();
    
    int             getWidth() const {return  width; };
    int             getHeight() const {return height; };
    ofPolyline      getSurface(){ return autoSurface.getSurface(); };
    
    void            update();
    void            draw(ofTexture &texture);
    void            exit();
    
    ofEvent<int>    calibrationDone;
    
    bool            bHideSettings, bCalibrated;
    
private:
    void            cleanBackground(bool & pressed);
    
    ofxKinect               kinect;
    ofxSurface              surface;
    ofxKinectAutoCalibrator autoSurface;
    
    ofxPanel                gui;
    ofxFloatSlider          minDist;
    float                   maxDist;
    ofxIntSlider            objectsImageThreshold;
    ofxButton               saveBackground;
    
    ofxCvFloatImage         surfaceImage;
    ofxCvGrayscaleImage     objectsImage;
    ofxCvGrayscaleImage     handsImage;
    
    ofxBlobTracker          hands;
    ofxBlobTracker          objects;
    
    int                     width, height, numPixels;
};

#endif
