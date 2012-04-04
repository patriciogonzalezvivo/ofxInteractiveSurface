//
//  ofxSurface.h
//  ofxSurface
//
//  Created by Patricio González Vivo on 3/5/12.
//  Copyright (c) 2012 http://www.PatricioGonzalezVivo.com All rights reserved.
//

#ifndef OFXSURFACE
#define OFXSURFACE

#include "ofMain.h"
#include "ofxXmlSettings.h"

class ofxSurface {
public:
  
    ofxSurface();
    
    bool        loadSettings(int _nTag, string _configFile = "none");
    bool        saveSettings(string _configFile = "none");
    
    void        setMask(ofPolyline &_polyLine);
    void        setCoorners(ofPoint _coorners[4]);
    
    
    int         getId() const { return nId; };
    ofPoint     getPos() const { return ofPoint(x,y); };
    int         getWidth() const {return  width; };
    int         getHeight() const {return height; };
    ofPolyline& getMask() { return maskCorners; };
    ofPoint     getSurfaceToScreen(ofPoint _pos){ return surfaceToScreenMatrix * _pos; };
    ofPoint     getScreenToSurface(ofPoint _pos){ return screenToSurfaceMatrix * _pos; };
    GLfloat*    getGlMatrix() { return glMatrix; };
    
    void        move(ofPoint _pos);
    void        scale(float _scale);
    void        rotate(float _angle);
    
    void        update();
    void        draw( ofTexture &texture );
    
    bool        isOver(ofPoint _loc) { return textureCorners.inside(_loc); };
    
    bool        bActive, bEditMode, bEditMask;
    
    void        resetMask();
    void        resetFrame();
    
protected:
    void        doSurfaceToScreenMatrix();      // Update the SurfaceToScreen transformation matrix
    void        doScreenToSurfaceMatrix();      // Update the ScreenToSurface transformation matrix
    void        doGaussianElimination(float *input, int n); // This is used making the matrix
    
    // Mouse & Key Events from the testApp
    //
	void        _mouseMoved(ofMouseEventArgs &e);
    void        _mousePressed(ofMouseEventArgs &e);
    void        _mouseDragged(ofMouseEventArgs &e);
    void        _mouseReleased(ofMouseEventArgs &e);
    void        _keyPressed(ofKeyEventArgs &e);
    
    // Mask variables
    //
    ofFbo       maskFbo;
    ofShader    maskShader;
    ofPolyline  maskCorners;
	int         selectedMaskCorner;
    
    // Texture varialbes
    //
    ofPolyline  textureCorners;
    int         selectedTextureCorner;
    int         textureWidth, textureHeight;
    
    ofPoint     src[4];
    ofMatrix4x4 surfaceToScreenMatrix;
    ofMatrix4x4 screenToSurfaceMatrix;
    GLfloat     glMatrix[16];
        
    // General Variables
    //
    ofRectangle box;
    string      configFile;
    float       x, y;
    int         width, height;
    int         nId;
    
    bool        bUpdateMask;
    bool        bUpdateCoord;
};
#endif
