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
    
    // The nId number it´s the name of the instance on the .xml file. This
    // allows saving multiples surface parameters in one single config file
    //
    bool    loadSettings(int _nTag, string _configFile = "none");
    bool    saveSettings(string _configFile = "none");
    
    bool    isOver(ofPoint _loc) { return textureCorners.inside(_loc); };
    int     getId() const { return nId; };
    ofPoint getPos() const { return ofPoint(x,y); };
    
    void    move(ofPoint _pos);
    void    scale(float _scale);
    void    rotate(float _angle);
    
    // Load and Draw ( super simple )
    //
    void    draw( ofTexture &texture );
    
    bool    bAutoActive, bActive, bEditMode, bEditMask;
    
protected:
    virtual void doFrame();
    virtual void doBox();
    
    void    doMask();                       // Update the mask
    void    doSurfaceToScreenMatrix();      // Update the SurfaceToScreen transformation matrix
    void    doScreenToSurfaceMatrix();      // Update the ScreenToSurface transformation matrix
    void    doGaussianElimination(float *input, int n); // This is used making the matrix
    
    // Mouse & Key Events from the testApp
    //
	void    _mouseMoved(ofMouseEventArgs &e);
    void    _mousePressed(ofMouseEventArgs &e);
    void    _mouseDragged(ofMouseEventArgs &e);
    void    _mouseReleased(ofMouseEventArgs &e);
    void    _keyPressed(ofKeyEventArgs &e);
    
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
};
#endif
