//
//  Created by Patricio Gonzalez Vivo on 5/3/12.
//  Copyright (c) 2012 http://PatricioGonzalezVivo.com . All rights reserved.
//

#include "testApp.h"

string helpScreen = "\n \
    - F1:   Fullscreen ON/OFF\n \
    - F2:   Surface Edit-Mode on/off\n \
    - F3:   Masking-Mode ON/OFF (need Edit-Mode ) \n \
    \n \
            On mask mode on:\n \
                            - x: delete mask path point\n \
                            - r: reset mask path\n \
    \n \
    - F4:   Reset surface coorners\n \
    \n \
    - m/M:  show/hide mouse\n \
    - d/D:  debug mode ON/OFF\n \
    - h/H:  turn ON/OFF this help screen\n \
    \n \
    - l/L:  load/reload previus calibration setup\n \
    - c/C:  calibrate\n";

//-------------------------------------------------------------- SETING
void testApp::setup(){
    ofEnableAlphaBlending();
    ofEnableSmoothing();
    
    ofAddListener(iSurface.calibrationDone, this, &testApp::calibrationDone);
    ofAddListener(iSurface.handAdded, this, &testApp::handAdded);
    ofAddListener(iSurface.handMoved, this, &testApp::handMoved);
    ofAddListener(iSurface.handDeleted, this, &testApp::handDeleted);
    ofAddListener(iSurface.objectAdded, this, &testApp::objectAdded);
    ofAddListener(iSurface.objectMoved, this, &testApp::objectMoved);
    ofAddListener(iSurface.objectDeleted, this, &testApp::objectDeleted);
    
    //  Set everthing OFF for default
    //
    bHelp = false;
    bMouse = false;
    iSurface.bDebug = false;
    
    ofHideCursor();
}

//-------------------------------------------------------------- LOOP
void testApp::update(){
    
    //  Updatign tSurface mantein the mapping between the texture and 
    //  the tracking objects/hands. Also if it's call do the calibration
    //
    iSurface.update();
}

void testApp::draw(){
    if (iSurface.bDebug)
        ofBackgroundGradient(ofColor::gray, ofColor::black);
    else
        ofBackground(ofColor::black);
    
    
    //  Pass the texture of what you want to map
    //iSurface.draw( something.getTextureReference() );

    if (bHelp){
        ofSetColor(0,200);
        ofRect(0,0,ofGetWidth(),ofGetHeight());
        ofSetColor(255,255);
        ofDrawBitmapString(helpScreen, 150,250);
    }
}

//-------------------------------------------------------------- EVENTS

void testApp::calibrationDone(ofPolyline &_surface){
    ofLog(OF_LOG_NOTICE, "Calibration done");
}

void testApp::handAdded(ofxBlob &_blob){
    ofLog(OF_LOG_NOTICE, "Hand added");   
}

void testApp::handMoved(ofxBlob &_blob){
    ofLog(OF_LOG_NOTICE, "Hand moved");    
}

void testApp::handDeleted(ofxBlob &_blob){
    ofLog(OF_LOG_NOTICE, "Hand deleted");
}

void testApp::objectAdded(ofxBlob &_blob){
    ofLog(OF_LOG_NOTICE, "Object added");
}

void testApp::objectMoved(ofxBlob &_blob){
    ofLog(OF_LOG_NOTICE, "Object moved");   
}
void testApp::objectDeleted(ofxBlob &_blob){
    ofLog(OF_LOG_NOTICE, "Object deleted");
}

void testApp::keyPressed(int key){
    switch (key) {
        case OF_KEY_F1:
            ofToggleFullscreen();
            break;
        case 'd':
        case 'D':
            iSurface.bDebug = !iSurface.bDebug;
            break;
        case 'c':
        case 'C':
            iSurface.calibrate();
            break;
        case 'l':
        case 'L':
            iSurface.load();
            break;
        case 'h':
        case 'H':
            bHelp = !bHelp;
            break; 
        case 'm':
        case 'M':
            bMouse = !bMouse;
            break; 
    }
    
    if (bMouse){
        ofShowCursor();
    } else {
        ofHideCursor();
    }
}

void testApp::keyReleased(int key){
}

void testApp::mouseMoved(int x, int y ){
}

void testApp::mousePressed(int x, int y, int button){
   
}

void testApp::mouseDragged(int x, int y, int button){
    
}

void testApp::mouseReleased(int x, int y, int button){
    
}

void testApp::windowResized(int w, int h){
}

void testApp::gotMessage(ofMessage msg){
}

void testApp::dragEvent(ofDragInfo dragInfo){
}

void testApp::exit(){
    iSurface.exit();
}