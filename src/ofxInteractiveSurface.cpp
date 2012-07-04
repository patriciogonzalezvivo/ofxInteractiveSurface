//
//  ofxInteractiveSurface.cpp
//  ofxInteractiveSurface
//
//  Created by Patricio Gonzalez Vivo on 5/3/12.
//  Copyright (c) 2012 http://PatricioGonzalezVivo.com . All rights reserved.
//

#include "ofxInteractiveSurface.h"

ofxInteractiveSurface::ofxInteractiveSurface(){
    
    // Initialize the kinect driver
    //
    kinect.init();
    kinect.setRegistration(true);
    kinect.open();
    
    width = 640;
	height = 480;
    numPixels = width * height; // NumPixels store the total amount of pixels to analize
    
    //  Allocate the ofxCvImages that are going to be use to analize the data
    //
    handsImage.allocate(width,height);
	surfaceImage.allocate(width,height);
    objectsImage.allocate(width,height);
	
    //  GUI
    //
    gui.setup("Panel");
    gui.add(maxDist.setup("surface_distance", 1000.0, 500.0, 1200));
    gui.add(minDist.setup("surface_height", 30.0, 10.0, 100.0));
    gui.add(objectsImageThreshold.setup("threshold", 0, 0, 255));
    gui.loadFromFile("settings.xml");
    
    // Events Listeners
    //
    minDist.addListener(this,&ofxInteractiveSurface::_cleanBackground);
    
    ofAddListener(hands.blobAdded, this, &ofxInteractiveSurface::_handAdded);
    ofAddListener(hands.blobMoved, this, &ofxInteractiveSurface::_handMoved);
    ofAddListener(hands.blobDeleted, this, &ofxInteractiveSurface::_handDeleted);
    
    ofAddListener(objects.blobAdded, this, &ofxInteractiveSurface::_objectAdded);
    ofAddListener(objects.blobMoved, this, &ofxInteractiveSurface::_objectMoved);
    ofAddListener(objects.blobDeleted, this, &ofxInteractiveSurface::_objectDeleted);
    
    //  Surface preparation
    //
    view.loadSettings(0,"settings.xml");    // Load the settings of the surface  
    load();                                 // Load previus configuration

    bDebug = false;
    trackMode = TRACK_NONE;
}

//  Load the previus calibration data
//
void ofxInteractiveSurface::load(){
    ofSetFullscreen(true);                  // Fullscreen it´s need if the surface it´s on other screen (on dual monitor)
    surfaceContour = view.getMask();                        // Get a local copy of the surface
    kinect.setDepthClipping((maxDist-minDist),maxDist);     // Do the right clipping
    countDown = 200;                                        // The count down it´s to make a background sustraction
    bCalibrated = true;                                     // Turn the calibration flag on
}

//  Makes a new calibration
//
void ofxInteractiveSurface::calibrate(){
    ofSetFullscreen(true);          // Fullscreen it's need for calibration
    bCalibrated = false;            // Turn the calibration flag to off
    autoCalibrator.init(&kinect);   // Initialice calibration
}

// ------------------------------------------------------------------------- MAIN LOOP
//
void ofxInteractiveSurface::update(){
    
    //  Update Kinects data
    //
    kinect.update();
    
    //  This is a timer for refreshing the background after calibration
    //
    if (countDown > 0){
        countDown--;
    } else if (countDown == 0){
        hands.bUpdateBackground = true;             //  Store the background ...
        objects.bUpdateBackground = true;           //  ... on both trackers
        
        ofNotifyEvent(calibrationDone, surfaceContour); // We are ready to go!!!
        countDown--;                                    // CountDown have to be -1 to be inactive
    }
    
    //  If it have new data
    //
    if(kinect.isFrameNew()) {
        
        if (!bCalibrated){
            
            //  It´s on calibration process ( bCaligrated = false )
            //
            bCalibrated = autoCalibrator.update( view );    // this routine goes for the 6 steps and returns true when finish
            
            //  When finish the 6 steps it´s going enter in this IF just one time.
            //
            if (bCalibrated){
                maxDist = autoCalibrator.getSurfaceDistance(); // Get the surface where he found the surface ...
                load();                                     // ... and load the data. The process of calibration it will end
            }                                               // ... with the countDown for storing a background
            
        } else {
            
            if ((trackMode == TRACK_BOTH) ||
                (trackMode == TRACK_ACTIVE_OBJECT) || 
                (trackMode == TRACK_ACTIVE_HANDS)){
                //  Analice the Kinect data´s, clipp it and split it in two images
                //  once for the surface (surfaceImage) and the other one for the space above the surface (handsImage).
                //
                float *depthRaw = kinect.getDistancePixels();
                unsigned char * handsPixels = handsImage.getPixels();
                float * surfacePixels = surfaceImage.getPixelsAsFloats();
                
                for(int i = 0; i < numPixels; i++, depthRaw++) {
                    if(*depthRaw <= maxDist && *depthRaw >= (maxDist-minDist)){
                        handsPixels[i] = 0;
                        surfacePixels[i] = ofMap(*depthRaw, maxDist, (maxDist-minDist), 0.0f ,1.0f);
                    } else if ( *depthRaw < maxDist ){
                        if ( *depthRaw == 0)
                            handsPixels[i] = 0;
                        else 
                            handsPixels[i] = 255;
                    } else if ( *depthRaw > (maxDist-minDist) ){
                        handsPixels[i] = 0;
                        surfacePixels[i] = 0.0f;
                    } else {
                        handsPixels[i] = 0;
                    }
                }
            } else if (trackMode == TRACK_JUST_OBJECT){
                
                //  JUST update the surface of the table. Width out looking for oclusions from above
                //
                float *depthRaw = kinect.getDistancePixels();
                float *surfacePixels = surfaceImage.getPixelsAsFloats();
                
                for(int i = 0; i < numPixels; i++, depthRaw++) {
                    if( *depthRaw < (maxDist-minDist*0.5) && *depthRaw > (maxDist-minDist)){
                        surfacePixels[i] = ofMap(*depthRaw, maxDist, (maxDist-minDist), 0.0f ,1.0f);
                    } else {
                        surfacePixels[i] = 0.0f;
                    }
                }
                
            } else if (trackMode == TRACK_JUST_HANDS){
                
                //  TODO!
                
                //  JUST update the hands over the table.
                //
                float *depthRaw = kinect.getDistancePixels();
                unsigned char * handsPixels = handsImage.getPixels();
                
                for(int i = 0; i < numPixels; i++, depthRaw++) {
                    if(*depthRaw <= maxDist && *depthRaw >= (maxDist-minDist)){
                        handsPixels[i] = 0;
                    } else if ( *depthRaw < maxDist ){
                        if ( *depthRaw == 0)
                            handsPixels[i] = 0;
                        else 
                            handsPixels[i] = 255;
                    } else if ( *depthRaw > (maxDist-minDist) ){
                        handsPixels[i] = 0;
                    } 
                }
            }
            
            //  The objects over the surfaces and the hands over the table recive a different treatment and it´s
            //  very rare that need to analice both at the same time. That´s depend on the tipe of interaction it´s 
            //  nedd
            //
            if ((trackMode == TRACK_BOTH) ||
                (trackMode == TRACK_JUST_OBJECT) || 
                (trackMode == TRACK_ACTIVE_OBJECT)){
                
                //  The floating point image became a grayscaled image that will be thresholded
                //
                surfaceImage.blur(3);
                objectsImage = surfaceImage;
                objects.update(objectsImage, objectsImageThreshold );
            }
            
            if ((trackMode == TRACK_BOTH) ||
                (trackMode == TRACK_JUST_HANDS) || 
                (trackMode == TRACK_ACTIVE_HANDS)){
                
                //  The hands image it´s already thresholded so it need to bee processed with out any other process.
                //
                handsImage.flagImageChanged();
                hands.update(handsImage,-1, 100 , 340*240, 10, 20, true, true);
            }
        }
    } 
}

void ofxInteractiveSurface::draw(ofTexture &texture ){
    if (!bCalibrated){
        
        //  It´s important to achive a good contrast for the calibration
        //  That´s why it will force a black background and ridd of the surface coorners lines.
        ofBackground(ofColor::black);
        view.bEditMode = false;
        view.bEditMask = false;
        view.bActive = true;
        
        ofSetColor(255,255);
        view.draw( autoCalibrator.getTextureReference() );
        
        /*
        if(bDebug)
            autoSurface.getDebugTextureReference().draw(0,0);
        */
        
    } else {
        //  This is a timer for refreshing the background after calibration
        //
        if (countDown > 0){
            bDebug = true;
        } else if (countDown == 0){
            bDebug = false;
        }
        
        // Passthrou the texture of what it´s need to draw.
        view.draw(texture);
        
        if(bDebug){
            // Debuging the BlobTracker information
            ofSetColor(255,255);
            objectsImage.draw(ofGetWidth()-320,10,320,240);
            ofDrawBitmapString("Layer 0 (Objects)", ofGetWidth()-320+15, 25);
            handsImage.draw(ofGetWidth()-320,260,320,240);
            ofDrawBitmapString("Layer 1 (Hands)", ofGetWidth()-320+15, 275);
            
            //  GUI
            //
            ofSetColor(255,255);
            gui.draw();
            
            //  Debuging app performance
            //
            ofSetColor(255,255);
            ofDrawBitmapString("Fps: " + ofToString( ofGetFrameRate()), 15, 15);
            
            
            //  Draw objects contours
            //
            ofPushMatrix();
            ofSetColor(255,100);
            glMultMatrixf( view.getGlMatrix() );
            ofPushStyle();
            ofNoFill();
            for( int i=0; i<(int)objects.size(); i++ ) {
                if ( surfaceContour.inside( objects[i].centroid) ){
                    
                    ofSetColor(221, 0, 204, 200);
                    objects[i].drawBox(0,0,view.getWidth(),view.getHeight());
                    ofSetColor(0,255,255);
                    objects[i].drawContours(0,0,view.getWidth(),view.getHeight());
                    
                    ofSetColor(255,255);
                    ofDrawBitmapString(ofToString(objects[i].id), objects[i].centroid.x * view.getWidth(), objects[i].centroid.y * view.getHeight() );
                }
            }
            ofPopStyle();
            ofPopMatrix(); 
            
            
            //  Draw Hands contours (violet), palms (blue) and finger tips (red).
            //
            ofPushMatrix();
            ofSetColor(255,200);
            glMultMatrixf( view.getGlMatrix() );
            ofPushStyle();
            for( int i=0; i<(int)hands.size(); i++ ) {
                ofNoFill();
                ofSetColor(0,255,255);
                hands[i].drawContours(0,0,view.getWidth(),view.getHeight());
                
                if (hands[i].gotFingers){
                    ofFill();
                    ofSetColor(0,0,255);
                    ofCircle(hands[i].palm.x * view.getWidth(), hands[i].palm.y * view.getHeight(),10);
                    for (int j = 0; j < hands[i].nFingers ; j++){
                        ofSetColor(255,0,0);
                        ofCircle(hands[i].fingers[j].x * view.getWidth(),hands[i].fingers[j].y * view.getHeight(),4);
                    }
                }
            }
            ofPopStyle();
            ofPopMatrix();
        }
    }
}

// -------------------------------------------------------------------------------- EVENTS

void ofxInteractiveSurface::exit(){
    gui.saveToFile("settings.xml");
    minDist.removeListener(this,&ofxInteractiveSurface::_cleanBackground);
    kinect.close();
}

void ofxInteractiveSurface::_cleanBackground(float &_threshold){
    hands.bUpdateBackground = true;
    objects.bUpdateBackground = true;
}

void  ofxInteractiveSurface::_objectAdded(ofxBlob &_blob){
    if ( (surfaceContour.inside(_blob.centroid)) && 
         (countDown < 0 )){
        _blob.color = kinect.getColorAt(_blob.centroid.x,_blob.centroid.y);
        ofNotifyEvent(objectAdded, _blob);
        //ofLog(OF_LOG_NOTICE,"Object added at: " + ofToString(_blob.centroid));
    }
}

void  ofxInteractiveSurface::_objectMoved(ofxBlob &_blob){
    if ( (surfaceContour.inside(_blob.centroid)) && 
        (countDown < 0 )){
        _blob.color = kinect.getColorAt(_blob.centroid.x,_blob.centroid.y);
        ofNotifyEvent(objectMoved, _blob);
        //ofLog(OF_LOG_NOTICE,"Object moved at: " + ofToString(_blob.centroid));
    }
}

void  ofxInteractiveSurface::_objectDeleted(ofxBlob &_blob){
    if ( (surfaceContour.inside(_blob.centroid)) && 
        (countDown < 0 )){
        ofNotifyEvent(objectDeleted, _blob);
        //ofLog(OF_LOG_NOTICE,"Object deleted at: " + ofToString(_blob.centroid));
    }
}

void  ofxInteractiveSurface::_handAdded(ofxBlob &_blob){
    if ( (countDown < 0 ) ){
        _blob.color.set(0,0,0);
        ofNotifyEvent(handAdded, _blob);
        //ofLog(OF_LOG_NOTICE,"Hand added at: " + ofToString(_blob.centroid));
    } 
}

void  ofxInteractiveSurface::_handMoved(ofxBlob &_blob){
    if ( (countDown < 0 ) ){
        _blob.color.set(0,0,0);
        ofNotifyEvent(handMoved, _blob);
        //ofLog(OF_LOG_NOTICE,"Hand moved at: " + ofToString(_blob.centroid));
    } 
}

void  ofxInteractiveSurface::_handDeleted(ofxBlob &_blob){
    if ( (countDown < 0 ) ){
        _blob.color.set(0,0,0);
        ofNotifyEvent(handDeleted, _blob);
        //ofLog(OF_LOG_NOTICE,"Hand deleted at: " + ofToString(_blob.centroid));
    }
}

