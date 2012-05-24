//
//  ofxKinectAutoSurface.h
//  mdt-Core
//
//  Created by Patricio González Vivo on 3/28/12.
//  Copyright (c) 2012 PatricioGonzalezVivo.com. All rights reserved.
//

#ifndef OFXKINECTAUTOCALIBRATOR
#define OFXKINECTAUTOCALIBRATOR

#include "ofMain.h"

#include "ofxGui.h"

#include "ofxOpenCv.h"
#include "ofxKinect.h"
#include "ofxSurface.h"

class ofxKinectAutoCalibrator {
public:
    
    ofxKinectAutoCalibrator();
    
    void        init(ofxKinect *_kinect, int _aproxSurfaceArea = (640*480)*0.2);
    bool        update(ofxSurface &_surface);
    
    int         getCurrentStep() const { return nStep; };
    float       getSurfaceDistance() const { return surfaceDistance; };
    float       getCleanDistance() const { return surfaceMinDistance; };
    
    ofPolyline& getSurface() { return surfaceContour; };
    ofTexture&  getTextureReference() { return fbo.getTextureReference(); };
    ofTexture&  getDebugTextureReference() { return debugFbo.getTextureReference(); }; 
    ofPoint     getkinectToScreen(ofPoint _pos){ return kinectToScreenMatrix * _pos; };

private:
    bool        doStep0();  // Search for an empty space
    bool        doStep1();  // Search for the distance to the surface (it looks for a big solid blob)
    bool        doStep2();  // Calculate a visible position for the dots inside the surface
    bool        doStep3();  // Search for the right threshold here it can see the dots
    bool        doStep4();  // Track each dot position in order to make the matrix
    
    bool        isClean(ofxCvGrayscaleImage &img, float _normTolerance = 0);
    bool        isBlobSolid(ofxCvGrayscaleImage &img, ofxCvBlob &blob, float _normTolerance);
    bool        isBlobCircular(ofxCvBlob &blob);
    
    float       getAverage(ofxCvGrayscaleImage &img, ofPolyline &_polyline);
    ofMatrix4x4 getHomographyMatrix(ofPoint src[4], ofPoint dst[4]);
    void        getGaussianElimination(float *input, int n);
    
    ofxKinect   *kinect;
    
    ofxCvGrayscaleImage grayImage;
    ofxCvColorImage     colorImage;
    ofxCvContourFinder  contourFinder;
    
    ofFbo       fbo;
    ofFbo       debugFbo;
    
    ofPoint     realDots[4];
    ofPoint     screenDots[4];
    ofPoint     trackedDots[4];
    
    ofMatrix4x4 kinectToScreenMatrix;
    
    int         nStep;                      // current step
    
    //          Step 0 & 1
    float       surfaceDistance;            // current distance
    int         surfaceMinArea;             // Minium area to search for blobs that match the surface
    int         surfaceMinDistance;         // Starting scanning distance
    int         surfaceMaxDistance;         // Max scanning distance
    float       scanningSliceHeight;        // Height of each slice of depth data
    float       scanningStepingDistance;    // Distance between each search 
    
    //          Step 2
    ofPolyline  surfaceContour;
    float       scaleAreaFactor;
    
    //          Step 3
    int         countDown;
    int         redThreshold;
    float       minDotArea;                 // Min area for finding blobs on red thresholded image
    float       maxDotArea; 
    
    //          Step 4
    int         scannedDot;
    
    bool        bDone;
};

#endif
