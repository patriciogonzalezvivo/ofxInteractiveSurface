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
    
    void            load();         // Load previus calibration setup
    void            calibrate();    // Make a new calibration
    
    int             getWidth() const {return  width; };
    int             getHeight() const {return height; };
    ofxSurface&     getSurface() { return surface; };
    ofPolyline      getSurfaceContour(){ return surfaceContour; };
    
    bool            isCalibrated() const {bool bCalibrated;};
    
    void            update(bool _updateSurface = true, bool _updateHands = true );
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
    ofxSurface              surface, sObjects, sHands;
    ofxKinectAutoCalibrator autoSurface;
    
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
