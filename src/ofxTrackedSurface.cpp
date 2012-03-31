//
//  ofxTrackedSurface.cpp
//  mdt-Core
//
//  Created by Patricio González Vivo on 3/31/12.
//  Copyright (c) 2012 PatricioGonzalezVivo.com. All rights reserved.
//

#include "ofxTrackedSurface.h"

ofxTrackedSurface::ofxTrackedSurface(){
}

void ofxTrackedSurface::init(){
    kinect.init();
    kinect.setRegistration(true);
    kinect.open();
    
    width = 640;
	height = 480;
    numPixels = width * height;
    
    handsImage.allocate(width,height);
	surfaceImage.allocate(width,height);
    objectsImage.allocate(width,height);
	
    gui.setup("Panel");
    gui.add(minDist.setup("Alt de Obj", 30.0, 10.0, 100.0));
    gui.add(objectsImageThreshold.setup("humbral", 80, 0, 255));
    gui.add(saveBackground.setup("clean background") );
    gui.loadFromFile("settings.xml");
    
    saveBackground.addListener(this,&ofxTrackedSurface::cleanBackground);
    
    surface.loadSettings(0,"configSurface.xml");
    
    bHideSettings = false;
    bCalibrated = false;
    autoSurface.init(&kinect);
}

void ofxTrackedSurface::update(){
    kinect.update();
    
    if(kinect.isFrameNew()) {
        if (!bCalibrated){
            
            bCalibrated = autoSurface.update( surface );
            
            if (bCalibrated){
                minDist = 30;
                maxDist = autoSurface.getSurfaceDistance();
                hands.bUpdateBackground = true;
                objects.bUpdateBackground = true;
                kinect.setDepthClipping((maxDist-minDist),maxDist);
                
            }
            
        } else {
            kinect.setDepthClipping(maxDist-minDist,maxDist);
            
            float *depthRaw = kinect.getDistancePixels();
            
            unsigned char * blobPixels = handsImage.getPixels();
            float * surfacePixels = surfaceImage.getPixelsAsFloats();
            
            for(int i = 0; i < numPixels; i++, depthRaw++) {
                if(*depthRaw <= maxDist && *depthRaw >= (maxDist-minDist)){
                    blobPixels[i] = 0;
                    surfacePixels[i] = ofMap(*depthRaw, maxDist, (maxDist-minDist), 0.0f ,1.0f);
                } else if ( *depthRaw < maxDist ){
                    if ( *depthRaw == 0)
                        blobPixels[i] = 0;
                    else 
                        blobPixels[i] = 255;
                } else if ( *depthRaw > (maxDist-minDist) ){
                    blobPixels[i] = 0;
                    surfacePixels[i] = 0;
                } else {
                    blobPixels[i] = 0;
                }
            }
            
            // Table SURFACE -> OBJECTS over it
            surfaceImage.flagImageChanged();
            surfaceImage.updateTexture();
            surfaceImage.blur(3);
            objectsImage = surfaceImage;
            
            objects.update(objectsImage, objectsImageThreshold );
            objectsImage.updateTexture();
            
            // HANDS
            handsImage.flagImageChanged();
            hands.update(handsImage);
            handsImage.updateTexture();
        }
    } 
}

void ofxTrackedSurface::draw(ofTexture &texture ){
    if (!bCalibrated){
        ofBackground(ofColor::black);
        
        ofSetColor(255,255);
        surface.draw( autoSurface.getTextureReference() );
        
        autoSurface.debugFbo.draw(0, 0);
        
    } else {
        ofSetColor(255, 255);
        
        surface.draw(texture);
        
        if(!bHideSettings){
            
            ofSetColor(255,255);
            gui.draw();
            
            ofPushMatrix();
            ofSetColor(255,100);
            glMultMatrixf( surface.getGlMatrix() );
            objects.draw(0,0, width, height);
            ofPopMatrix();
            
            ofPushMatrix();
            ofSetColor(255,200);
            glMultMatrixf( surface.getGlMatrix() );
            hands.draw(0,0, width, height);
            ofPopMatrix();
        }
    }
}

void ofxTrackedSurface::exit(){
    gui.saveToFile("settings.xml");
    saveBackground.removeListener(this,&ofxTrackedSurface::cleanBackground);
    kinect.close();
}

void ofxTrackedSurface::cleanBackground(bool & pressed){
    hands.bUpdateBackground = true;
    objects.bUpdateBackground = true;
}