//
//  ofxInteractiveSurface.h
//  ofxInteractiveSurface
//
//  Created by Patricio Gonzalez Vivo on 5/3/12.
//  Copyright (c) 2012 http://PatricioGonzalezVivo.com . All rights reserved.
//

#ifndef OFXTRACKEDSURFACE
#define OFXTRACKEDSURFACE

#include "ofMain.h"

#include "ofxKinect.h"

#include "ofxOpenCv.h"
#include "ofxBlobTracker.h"

#include "ofxInteractiveViewPort.h"
#include "ofxKinectAutoCalibrator.h"

enum ofxTrackMode {
    TRACK_NONE,
    TRACK_JUST_OBJECT,
    TRACK_JUST_HANDS,
    TRACK_ACTIVE_OBJECT,
    TRACK_ACTIVE_HANDS,
    TRACK_BOTH
};

class ofxInteractiveSurface {
public:
    ofxInteractiveSurface();
    
    void            load();         // Load previus calibration setup from "settings.xml"
    void            calibrate();    // Make a new calibration
    
    void            setTrackMode(ofxTrackMode _trackMode){ trackMode = _trackMode;};
    
    int             getWidth() const {return  width; };
    int             getHeight() const {return height; };
    ofPolyline      getSurfaceContour(){ return surfaceContour; };
    
    bool            isCalibrated() const {bool bCalibrated;};
    
    void            update();
    void            draw(ofTexture &texture);
    void            exit();
    
    // Events
    ofEvent<ofPolyline> calibrationDone;
    ofEvent<ofxBlob>    objectAdded;
    ofEvent<ofxBlob>    objectMoved;
    ofEvent<ofxBlob>    objectDeleted;
    ofEvent<ofxBlob>    handAdded;
    ofEvent<ofxBlob>    handMoved;
    ofEvent<ofxBlob>    handDeleted;
    
    bool                bDebug;
    
private:
    void                    _cleanBackground(float &_threshold);
    void                    _objectAdded(ofxBlob &_blob);
    void                    _objectMoved(ofxBlob &_blob);
    void                    _objectDeleted(ofxBlob &_blob);
    void                    _handAdded(ofxBlob &_blob);
    void                    _handMoved(ofxBlob &_blob);
    void                    _handDeleted(ofxBlob &_blob);
    
    ofxKinect               kinect;
    ofxTrackMode            trackMode;
    ofxInteractiveViewPort  view;
    ofxKinectAutoCalibrator autoCalibrator;
    
    ofxPanel                gui;
    ofxFloatSlider          minDist;
    ofxFloatSlider          maxDist;
    ofxIntSlider            objectsImageThreshold;
    
    ofxCvFloatImage         surfaceImage;
    ofxCvGrayscaleImage     objectsImage;
    ofxCvGrayscaleImage     handsImage;
    
    ofPolyline              surfaceContour;
    ofxBlobTracker          hands;
    ofxBlobTracker          objects;
    
    int                     width, height, numPixels, countDown;
    bool                    bCalibrated;
};

#endif
