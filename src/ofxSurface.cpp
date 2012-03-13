//
//  ofxSurface.cpp
//  emptyExample
//
//  Created by Patricio González Vivo on 3/5/12.
//  Copyright (c) 2012 http://www.PatricioGonzalezVivo.com All rights reserved.
//

#include "ofxSurface.h"

ofxSurface::ofxSurface(){
    
    // Link the core events in order to catch mouse and keys
    //
    ofAddListener(ofEvents().mouseMoved, this, &ofxSurface::_mouseMoved);
	ofAddListener(ofEvents().mousePressed, this, &ofxSurface::_mousePressed);
	ofAddListener(ofEvents().mouseDragged, this, &ofxSurface::_mouseDragged);
	ofAddListener(ofEvents().mouseReleased, this, &ofxSurface::_mouseReleased);
	ofAddListener(ofEvents().keyPressed, this, &ofxSurface::_keyPressed);
    
    // This two files handle the selected points of the mask or texture corners
    // if it´s -1 means nothing is selected, other wise the number it´s the position
    // on the array
    //
    selectedMaskCorner = -1;
    selectedTextureCorner = -1;
    
    // "e" to enable/disable EditMode where you can transform, and modify the texture
    // In editMode press "m" to access to the mask points, or "M" to stay in that mode 
    // until "m" it´s pressed again.
    //
    bEditMode = true;
    bEditMask = false;
    bVisible = true;
    bActive = false;
    bAutoActive = true;
    
    // In order to simplify some things I´m using ofPolyline, specialy for making easy
    // to check if the mouse it´s over the surface.
    // So each textureCorner have an absolute value. It could be improve using normalized
    // and centerBased coordenates in order to make propper scalations and rotations.
    //
    textureCorners.addVertex(0.0,0.0);
    textureCorners.addVertex(1.0,0.0);
    textureCorners.addVertex(1.0,1.0);
    textureCorners.addVertex(0.0,1.0);
    
    // MaskPointer are normalized because they depend on the textureCorners. To deal with
    // them it´s necesary to make some transformations using the ScreenToSurface matrix
    //
    ofPoint newPoint = ofPoint(0.0,0.0,0.0);
    maskCorners.addVertex(newPoint);
    newPoint.set(1.0,0.0,0.0);
    maskCorners.addVertex(newPoint);
    newPoint.set(1.0,1.0,0.0);
    maskCorners.addVertex(newPoint);
    newPoint.set(0.0,1.0,0.0);
    maskCorners.addVertex(newPoint);
    
    // This shader it´s basicaly a alphaMask shader for quick and clean masking
    // also I added some opacity variables in order to chech and see the masked-texture
    // wile you are editing it.
    //
    string shaderProgram = "#version 120\n\
#extension GL_ARB_texture_rectangle : enable\n\
\n\
uniform sampler2DRect tex0;\n\
uniform sampler2DRect maskTex;\n\
uniform float texOpacity;\n\
uniform float maskOpacity;\n\
\n\
void main (void){\n\
    vec2 pos = gl_TexCoord[0].st;\n\
    \n\
    vec4 src = texture2DRect(tex0, pos);\n\
    float mask = texture2DRect(maskTex, pos).r;\n\
    \n\
    gl_FragColor = vec4( src.rgb * texOpacity , clamp( min(src.a,mask) , maskOpacity, 1.0));\n\
}\n\
\n";
    maskShader.setupShaderFromSource(GL_FRAGMENT_SHADER, shaderProgram);    // Load ...
    maskShader.linkProgram();                                               // ... and compile the shader
    
    configFile = "config.xml";
    title = NULL;
    nId = -1;
    doTitleBar();
}

// ------------------------------------------------------------- SETUP
void ofxSurface::loadSettings( int _nTag, string _configFile){
    ofxXmlSettings XML;
    
    if (_configFile != "none")
        configFile = _configFile;
    
    // Open the xml file
    //
    if (XML.loadFile(configFile)){
        if (XML.pushTag("surface", _nTag)){
            
            // Load the type and do what it have to 
            //
            nId = XML.getValue("id", 0);
                
            // The 4 texture coordenates are absolute and from them it makes
            // two matrix transformation one from The screen to the workd and biceversa 
            //
            if (XML.pushTag("texture")){
                for(int i = 0; i < 4; i++){
                    XML.pushTag("point",i);
                    textureCorners[i].set(XML.getValue("x", 0.0),XML.getValue("y", 0.0));
                    XML.popTag();
                }
                doScreenToSurfaceMatrix();
                doSurfaceToScreenMatrix();
                doBox();
                XML.popTag();
            }
            
            // Then it�s the mask path, made from normalized points that then will fix to
            // the texture proportions
            //
            if(XML.pushTag("mask")){
                
                int totalMaskCorners = XML.getNumTags("point");
                
                // Clean the maskCorners before loading new-onces
                //
                if (totalMaskCorners > 0){
                    maskCorners.clear();
                }
                
                for(int i = 0; i < totalMaskCorners; i++){
                    XML.pushTag("point",i);
                    maskCorners.addVertex(XML.getValue("x", 0.0),XML.getValue("y", 0.0));
                    XML.popTag();
                }
                XML.popTag();
                maskFbo.allocate(width, height);
                doMask();
            }
            
            // loaded or not it will decent one level to the root
            //
            XML.popTag();
        }
    } else
        cout << "ERROR: ofxSurface::loadSettings couldn't load surface " << nId << " on " << configFile << endl;
}

void ofxSurface::saveSettings(string _configFile){
    ofxXmlSettings XML;
    
    if (_configFile != "none")
        configFile = _configFile;
    
    if (XML.loadFile(configFile)){
        // Get the total number of surfaces...
        //
        int totalSurfaces = XML.getNumTags("surface");
        
        // ... and search for the right id for loading
        //
        for (int i = 0; i < totalSurfaces; i++){
            if (XML.pushTag("surface", i)){
                
                // Once it found the right surface that match the id
                // load the data
                //
                if ( XML.getValue("id", 0) == nId){
                    if (XML.pushTag("texture")){
                        for(int i = 0; i < 4; i++){
                            XML.setValue("point:x",textureCorners[i].x, i);
                            XML.setValue("point:y",textureCorners[i].y, i);
                        }
                        XML.popTag();
                    }
                    
                    if (XML.pushTag("mask")){
                        int totalSavedPoints = XML.getNumTags("point");
                        
                        for(int j = 0; j < maskCorners.size(); j++){
                            int tagNum = j;
                            
                            if (i >= totalSavedPoints)
                                tagNum = XML.addTag("point");
                            
                            XML.setValue("point:x",maskCorners[j].x, tagNum);
                            XML.setValue("point:y",maskCorners[j].y, tagNum);
                        }
                        
                        int totalCorners = maskCorners.size();
                        totalSavedPoints = XML.getNumTags("point");
                        
                        if ( totalCorners < totalSavedPoints){
                            for(int j = totalSavedPoints; j > totalCorners; j--){
                                XML.removeTag("point",j-1);
                            }
                        }
                        
                        doMask();
                        XML.popTag();
                    }
                }
                
                XML.popTag();
                XML.saveFile();
            }
        }
    } else
        cout << "ERROR: ofxSurface::saveSettings couldn't save " << nId << " surface on " << configFile << endl;
    
    
}

// ------------------------------------------------------ LOOPS
//
void ofxSurface::draw( ofTexture &texture ){
    if ( bEditMode || bVisible ){
        // If the texture change or it´s new it will update some parameters
        // like the size of the FBO, de mask and the matrix
        //
        if ((width != texture.getWidth()) ||
            (height != texture.getHeight()) ){
            width = texture.getWidth();
            height = texture.getHeight();
            
            maskFbo.allocate(width,height);
            doMask();
            doSurfaceToScreenMatrix();
        }
        
        //  Here is where the magic happends
        //  Make the matrix multiplication and mask the texture
        ofPushMatrix();
        float texOpacity = 0.0;
        float maskOpacity = 0.0;
        
        if ( textureCorners.inside(ofGetMouseX(), ofGetMouseY()) && (bEditMode)){
            texOpacity = 1.0;
            maskOpacity = 0.2;
        } else if (!bEditMode){
            texOpacity = 1.0;
            maskOpacity = 0.0;
        } else {
            texOpacity = 0.8;
            maskOpacity = 0.0;
        }
        
        // Matrix multiplication: rotates, translate and resize to get the right perspective
        //
        glMultMatrixf(glMatrix);
        
        // Active the alpha-masking shader
        //
        maskShader.begin();
        maskShader.setUniformTexture("maskTex", maskFbo.getTextureReference(), 1 );
        maskShader.setUniform1f("texOpacity", texOpacity);
        maskShader.setUniform1f("maskOpacity", maskOpacity);
        
        // Draw the texture (image, video, Fbo, etc)
        //
        texture.draw(0,0);
        
        // Close and re-set everything like nothing happend
        //
        maskShader.end();
        ofPopMatrix();
    }

    // This just draw the circles and lines
    //
    if (bEditMode){
        ofPushStyle();
        if (title != NULL)
            title->draw();
        
        if ( !bEditMask ){
            ofFill();
            // Draw dragables texture corners
            //
            for(int i = 0; i < 4; i++){
                if ( ( selectedTextureCorner == i) || ( ofDist(ofGetMouseX(), ofGetMouseY(), textureCorners[i].x, textureCorners[i].y) <= 4 ) ) ofSetColor(200,255);
                else ofSetColor(200,100);
                    
                ofRect(textureCorners[i].x-4,textureCorners[i].y-4, 8,8);
                
                // Draw contour Line
                //
                ofLine(textureCorners[i].x, textureCorners[i].y, textureCorners[(i+1)%4].x, textureCorners[(i+1)%4].y);
            }
        } else {
            // Draw dragables mask corners
            //
            for(int i = 0; i < maskCorners.size(); i++){
                ofVec3f pos = ofVec3f( maskCorners[i].x * width, maskCorners[i].y * height, 0.0);
                pos = surfaceToScreenMatrix * pos;
                
                if ( (selectedMaskCorner == i) || ( ofDist(ofGetMouseX(), ofGetMouseY(), pos.x, pos.y) <= 4 ) ) {
                    ofSetColor(255,255);
                    ofCircle( pos, 4);
                    ofSetColor(255,100);
                    ofFill();
                } else {
                    ofNoFill();
                    ofSetColor(255,100);
                }
                    
                ofCircle( pos, 4);
                
                // Draw contour mask line
                //
                ofSetColor(255,200);
                ofVec3f nextPos = ofVec3f(maskCorners[(i+1)%maskCorners.size()].x*width, 
                                          maskCorners[(i+1)%maskCorners.size()].y*height, 0.0);
                nextPos = surfaceToScreenMatrix * nextPos;
                ofLine(pos.x,pos.y,nextPos.x,nextPos.y);
            }
        }
        ofPopStyle();
    }
}

// -------------------------------------------------------- ACTIONS
//
void ofxSurface::move(ofPoint _pos){
    ofVec2f diff = _pos - getPos();
    
    for(int i = 0; i < 4; i++){
        textureCorners[i] += diff;
    }
    
    doScreenToSurfaceMatrix();
}

void ofxSurface::scale(float _scale){
    for(int i = 0; i < 4; i++){
        ofVec2f center = getPos();
        ofVec2f fromCenterToCorner = textureCorners[i] - center;
        
        float radio = fromCenterToCorner.length();
        float angle = -1.0*atan2f(fromCenterToCorner.x,fromCenterToCorner.y)+(PI/2);
        
        radio *= _scale;
        
        textureCorners[i] = center + ofPoint(radio * cos(angle),
                                             radio * sin(angle),
                                             0.0);
    }
    doSurfaceToScreenMatrix();
}

void ofxSurface::rotate(float _rotAngle){
    for(int i = 0; i < 4; i++){
        ofVec2f center = getPos();
        ofVec2f fromCenterToCorner = textureCorners[i] - center;
        
        float radio = fromCenterToCorner.length();
        float angle = -1.0*atan2f(fromCenterToCorner.x,fromCenterToCorner.y)+(PI/2);
        
        angle += _rotAngle;
        
        textureCorners[i] = center + ofPoint(radio * cos(angle),
                                             radio * sin(angle),
                                             0.0);
    }
    doSurfaceToScreenMatrix();
}

void ofxSurface::doTitleBar(){
    if ( title != NULL)
        delete title;
    
    title = new ofxTitleBar(&box, &nId);
    title->addButton('m', &bEditMask, TOGGLE_BUTTON);
    title->addButton('v', &bVisible, TOGGLE_BUTTON);
    ofAddListener( title->reset , this, &ofxSurface::_resetSurface);
}

void ofxSurface::doBox(){
    box = textureCorners.getBoundingBox();
}

void ofxSurface::doFrame(){
    textureCorners[0].set(0, 0);
    textureCorners[1].set(width, 0);
    textureCorners[2].set(width, height);
    textureCorners[3].set(0, height);
}

void ofxSurface::doMask(){
    // Generate masking contour
    maskFbo.begin();
    ofClear(0,255);
    ofBeginShape();
    ofSetColor(255, 255, 255);
    for(int i = 0; i < maskCorners.size(); i++ ){
        ofVertex(maskCorners[i].x*width,maskCorners[i].y*height);
    }
    ofEndShape(true);
    maskFbo.end();
}

void ofxSurface::doSurfaceToScreenMatrix(){
    ofPoint src[4];
    
    src[0].set(0, 0);
    src[1].set(width,0.0);
    src[2].set(width,height);
    src[3].set(0, height);
    
    ofPoint dst[4];
    for(int i = 0; i < 4; i++){
        dst[i] = textureCorners[i];
    }
    
    x = textureCorners.getCentroid2D().x;
    y = textureCorners.getCentroid2D().y;
    
    // create the equation system to be solved
    //
    // from: Multiple View Geometry in Computer Vision 2ed
    //       Hartley R. and Zisserman A.
    //
    // x' = xH
    // where H is the homography: a 3 by 3 matrix
    // that transformed to inhomogeneous coordinates for each point
    // gives the following equations for each point:
    //
    // x' * (h31*x + h32*y + h33) = h11*x + h12*y + h13
    // y' * (h31*x + h32*y + h33) = h21*x + h22*y + h23
    //
    // as the homography is scale independent we can let h33 be 1 (indeed any of the terms)
    // so for 4 points we have 8 equations for 8 terms to solve: h11 - h32
    // after ordering the terms it gives the following matrix
    // that can be solved with gaussian elimination:
    
    float P[8][9]=
    {
        {-src[0].x, -src[0].y, -1,   0,   0,  0, src[0].x*dst[0].x, src[0].y*dst[0].x, -dst[0].x }, // h11
        {  0,   0,  0, -src[0].x, -src[0].y, -1, src[0].x*dst[0].y, src[0].y*dst[0].y, -dst[0].y }, // h12
        
        {-src[1].x, -src[1].y, -1,   0,   0,  0, src[1].x*dst[1].x, src[1].y*dst[1].x, -dst[1].x }, // h13
        {  0,   0,  0, -src[1].x, -src[1].y, -1, src[1].x*dst[1].y, src[1].y*dst[1].y, -dst[1].y }, // h21
        
        {-src[2].x, -src[2].y, -1,   0,   0,  0, src[2].x*dst[2].x, src[2].y*dst[2].x, -dst[2].x }, // h22
        {  0,   0,  0, -src[2].x, -src[2].y, -1, src[2].x*dst[2].y, src[2].y*dst[2].y, -dst[2].y }, // h23
        
        {-src[3].x, -src[3].y, -1,   0,   0,  0, src[3].x*dst[3].x, src[3].y*dst[3].x, -dst[3].x }, // h31
        {  0,   0,  0, -src[3].x, -src[3].y, -1, src[3].x*dst[3].y, src[3].y*dst[3].y, -dst[3].y }, // h32
    };
    
    doGaussianElimination(&P[0][0],9);
    
    // gaussian elimination gives the results of the equation system
    // in the last column of the original matrix.
    // opengl needs the transposed 4x4 matrix:
    float aux_H[]= {P[0][8],P[3][8],0,P[6][8], // h11  h21 0 h31
        P[1][8],P[4][8],0,P[7][8], // h12  h22 0 h32
        0      ,      0,0,0,       // 0    0   0 0
        P[2][8],P[5][8],0,1        // h13  h23 0 h33
    };                          
    
    for(int i=0; i<16; i++) 
        glMatrix[i] = aux_H[i];
    
    surfaceToScreenMatrix(0,0)=P[0][8];
    surfaceToScreenMatrix(0,1)=P[1][8];
    surfaceToScreenMatrix(0,2)=0;
    surfaceToScreenMatrix(0,3)=P[2][8];
    
    surfaceToScreenMatrix(1,0)=P[3][8];
    surfaceToScreenMatrix(1,1)=P[4][8];
    surfaceToScreenMatrix(1,2)=0;
    surfaceToScreenMatrix(1,3)=P[5][8];
    
    surfaceToScreenMatrix(2,0)=0;
    surfaceToScreenMatrix(2,1)=0;
    surfaceToScreenMatrix(2,2)=0;
    surfaceToScreenMatrix(2,3)=0;
    
    surfaceToScreenMatrix(3,0)=P[6][8];
    surfaceToScreenMatrix(3,1)=P[7][8];
    surfaceToScreenMatrix(3,2)=0;
    surfaceToScreenMatrix(3,3)=1;
}

void ofxSurface::doScreenToSurfaceMatrix(){
    ofPoint dst[4];
    
    dst[0].set(0, 0);
    dst[1].set(width,0.0);
    dst[2].set(width,height);
    dst[3].set(0, height);
    
    ofPoint src[4];
    for(int i = 0; i < 4; i++){
        src[i] = textureCorners[i];
    }
    
    // create the equation system to be solved
    //
    // from: Multiple View Geometry in Computer Vision 2ed
    //       Hartley R. and Zisserman A.
    //
    // x' = xH
    // where H is the homography: a 3 by 3 matrix
    // that transformed to inhomogeneous coordinates for each point
    // gives the following equations for each point:
    //
    // x' * (h31*x + h32*y + h33) = h11*x + h12*y + h13
    // y' * (h31*x + h32*y + h33) = h21*x + h22*y + h23
    //
    // as the homography is scale independent we can let h33 be 1 (indeed any of the terms)
    // so for 4 points we have 8 equations for 8 terms to solve: h11 - h32
    // after ordering the terms it gives the following matrix
    // that can be solved with gaussian elimination:
    
    float P[8][9]=
    {
        {-src[0].x, -src[0].y, -1,   0,   0,  0, src[0].x*dst[0].x, src[0].y*dst[0].x, -dst[0].x }, // h11
        {  0,   0,  0, -src[0].x, -src[0].y, -1, src[0].x*dst[0].y, src[0].y*dst[0].y, -dst[0].y }, // h12
        
        {-src[1].x, -src[1].y, -1,   0,   0,  0, src[1].x*dst[1].x, src[1].y*dst[1].x, -dst[1].x }, // h13
        {  0,   0,  0, -src[1].x, -src[1].y, -1, src[1].x*dst[1].y, src[1].y*dst[1].y, -dst[1].y }, // h21
        
        {-src[2].x, -src[2].y, -1,   0,   0,  0, src[2].x*dst[2].x, src[2].y*dst[2].x, -dst[2].x }, // h22
        {  0,   0,  0, -src[2].x, -src[2].y, -1, src[2].x*dst[2].y, src[2].y*dst[2].y, -dst[2].y }, // h23
        
        {-src[3].x, -src[3].y, -1,   0,   0,  0, src[3].x*dst[3].x, src[3].y*dst[3].x, -dst[3].x }, // h31
        {  0,   0,  0, -src[3].x, -src[3].y, -1, src[3].x*dst[3].y, src[3].y*dst[3].y, -dst[3].y }, // h32
    };
    
    doGaussianElimination(&P[0][0],9);
    
    screenToSurfaceMatrix(0,0)=P[0][8];
	screenToSurfaceMatrix(0,1)=P[1][8];
	screenToSurfaceMatrix(0,2)=0;
	screenToSurfaceMatrix(0,3)=P[2][8];
	
	screenToSurfaceMatrix(1,0)=P[3][8];
	screenToSurfaceMatrix(1,1)=P[4][8];
	screenToSurfaceMatrix(1,2)=0;
	screenToSurfaceMatrix(1,3)=P[5][8];
	
	screenToSurfaceMatrix(2,0)=0;
	screenToSurfaceMatrix(2,1)=0;
	screenToSurfaceMatrix(2,2)=0;
	screenToSurfaceMatrix(2,3)=0;
	
	screenToSurfaceMatrix(3,0)=P[6][8];
	screenToSurfaceMatrix(3,1)=P[7][8];
	screenToSurfaceMatrix(3,2)=0;
	screenToSurfaceMatrix(3,3)=1;
    
}

void ofxSurface::doGaussianElimination(float *input, int n){
    // ported to c from pseudocode in
    // http://en.wikipedia.org/wiki/Gaussian_elimination
    
    float * A = input;
    int i = 0;
    int j = 0;
    int m = n-1;
    while (i < m && j < n)
    {
        // Find pivot in column j, starting in row i:
        int maxi = i;
        for(int k = i+1; k<m; k++)
        {
            if(fabs(A[k*n+j]) > fabs(A[maxi*n+j]))
            {
                maxi = k;
            }
        }
        if (A[maxi*n+j] != 0)
        {
            //swap rows i and maxi, but do not change the value of i
            if(i!=maxi)
                for(int k=0; k<n; k++)
                {
                    float aux = A[i*n+k];
                    A[i*n+k]=A[maxi*n+k];
                    A[maxi*n+k]=aux;
                }
            //Now A[i,j] will contain the old value of A[maxi,j].
            //divide each entry in row i by A[i,j]
            float A_ij=A[i*n+j];
            for(int k=0; k<n; k++)
            {
                A[i*n+k]/=A_ij;
            }
            //Now A[i,j] will have the value 1.
            for(int u = i+1; u< m; u++)
            {
                //subtract A[u,j] * row i from row u
                float A_uj = A[u*n+j];
                for(int k=0; k<n; k++)
                {
                    A[u*n+k]-=A_uj*A[i*n+k];
                }
                //Now A[u,j] will be 0, since A[u,j] - A[i,j] * A[u,j] = A[u,j] - 1 * A[u,j] = 0.
            }
            
            i++;
        }
        j++;
    }
    
    //back substitution
    for(int i=m-2; i>=0; i--)
    {
        for(int j=i+1; j<n-1; j++)
        {
            A[i*n+m]-=A[i*n+j]*A[j*n+m];
            //A[i*n+j]=0;
        }
    }
}

// -------------------------------------------------------- Mouse Events
void ofxSurface::_resetSurface( int &_nId ){
    doFrame();
    doSurfaceToScreenMatrix();
    doMask();
    saveSettings(configFile);
    doBox();
}

void ofxSurface::_mouseMoved(ofMouseEventArgs &e){
    ofVec2f mouse = ofVec2f(e.x, e.y);
    
    if (bAutoActive)
        bActive = textureCorners.inside(mouse);
}

void ofxSurface::_mousePressed(ofMouseEventArgs &e){
    ofVec3f mouse = ofVec3f(e.x, e.y, 0.0);

    if (bEditMode){
        if (!bEditMask){
            // Editing the texture corners
            //
            for(int i = 0; i < 4; i++){
                if ( ofDist(e.x, e.y, textureCorners[i].x, textureCorners[i].y) <= 10 )
                    selectedTextureCorner = i;
            }
        } else {
            // Editing the mask corners
            //
            bool overDot = false;
            for(int i = 0; i < maskCorners.size(); i++){
                ofVec3f pos = ofVec3f( maskCorners[i].x * width, maskCorners[i].y * height, 0.0);
                pos = surfaceToScreenMatrix * pos;
                
                if ( ofDist(e.x, e.y, pos.x, pos.y) <= 10 ){
                    selectedMaskCorner = i;
                    overDot = true;
                }
            }
            
            // Add new Dot if it´s over the line
            //
            if (!overDot && bActive ){
                
                doScreenToSurfaceMatrix();
                mouse = screenToSurfaceMatrix * mouse;
                mouse.x = mouse.x / width;
                mouse.y = mouse.y / height;
                
                int addNew = -1;
                
                // Search for the right placer to incert the point in the array
                //
                for (int i = 0; i < maskCorners.size(); i++){
                    int next = (i+1)%maskCorners.size();
                    
                    ofVec2f AtoM = mouse - maskCorners[i];
                    ofVec2f AtoB = maskCorners[next] - maskCorners[i];
                    
                    float a = atan2f(AtoM.x, AtoM.y);
                    float b = atan2f(AtoB.x, AtoB.y);
                    
                    if ( abs(a - b) < 0.05){
                        addNew = next;
                    }
                }
                
                if (addNew >= 0 ){
                    maskCorners.getVertices().insert( maskCorners.getVertices().begin()+addNew, mouse);
                    selectedMaskCorner = addNew;
                }
                
            }
        }
    }
}

void ofxSurface::_mouseDragged(ofMouseEventArgs &e){
    ofVec3f mouse = ofVec3f(e.x, e.y,0);
    ofVec3f mouseLast = ofVec3f(ofGetPreviousMouseX(),ofGetPreviousMouseY(),0);
    
    if (bEditMode){
        if (!bEditMask){
            // Drag texture corners
            //
            if (( selectedTextureCorner >= 0) && ( selectedTextureCorner < 4) ){
                
                if (e.button == 2){
                    // Deformation
                    //
                    textureCorners[selectedTextureCorner].x = ofGetMouseX();
                    textureCorners[selectedTextureCorner].y = ofGetMouseY();
                    
                    doSurfaceToScreenMatrix();
                    saveSettings(configFile);
                } else if ( ofGetKeyPressed() ){
                    // Rotation
                    //
                    ofVec2f center = getPos();
                    
                    ofVec2f fromCenterTo = mouseLast - center;
                    float prevAngle = -1.0*atan2f(fromCenterTo.x,fromCenterTo.y)+(PI/2);
                    
                    fromCenterTo = mouse - center;
                    float actualAngle = -1.0*atan2f(fromCenterTo.x,fromCenterTo.y)+(PI/2);
                    
                    float dif = actualAngle-prevAngle;
                    
                    rotate(dif);
                } else if ( e.button == 1 ){
                    // Centered Scale
                    //
                    float prevDist = mouseLast.distance(getPos());
                    float actualDist = mouse.distance(getPos());
                    
                    float dif = actualDist/prevDist;
                    
                    scale(dif);            
                } else {
                    // Corner Scale
                    //
                    ofVec2f center = getPos();
                    
                    int  opositCorner = (selectedTextureCorner - 2 < 0)? (4+selectedTextureCorner-2) : (selectedTextureCorner-2);
                    ofVec2f toOpositCorner = center - textureCorners[opositCorner];  
                    
                    float prevDist = mouseLast.distance( textureCorners[opositCorner] );
                    float actualDist = mouse.distance( textureCorners[opositCorner] );
                    
                    float dif = actualDist/prevDist;
                    
                    move( textureCorners[opositCorner] + toOpositCorner * dif );
                    scale(dif); 
                } 
                doBox();
                
            // Drag all the surface
            //
            } else if ( bActive ){
                for (int i = 0; i < 4; i++){
                    textureCorners[i] += mouse-mouseLast;
                }
                
                doSurfaceToScreenMatrix();
                doBox();
                saveSettings(configFile);
                mouseLast = mouse;
            }
        } else {
            
            // Drag mask points
            //
            for(int i = 0; i < maskCorners.size(); i++){
                ofVec3f pos = ofVec3f( maskCorners[i].x * width, maskCorners[i].y * height, 0.0);
                pos = surfaceToScreenMatrix * pos;
                
                if ((selectedMaskCorner >= 0) && (selectedMaskCorner < maskCorners.size() )){
                    ofVec3f newPos = ofVec3f(mouse.x, mouse.y, 0.0);
                    doScreenToSurfaceMatrix();
                    newPos = screenToSurfaceMatrix * mouse;
                    newPos.x = ofClamp(newPos.x / width, 0.0, 1.0);
                    newPos.y = ofClamp(newPos.y / height, 0.0, 1.0);
                    
                    maskCorners[selectedMaskCorner] = newPos;
                }
            }
            doMask();
            saveSettings(configFile);
        }
    }
    mouseLast = ofVec2f(e.x, e.y);
}

void ofxSurface::_mouseReleased(ofMouseEventArgs &e){
    if (bEditMode){
        if (!bEditMask){
            if (( selectedTextureCorner >= 0) && ( selectedTextureCorner < 4) ){
                doSurfaceToScreenMatrix();
                saveSettings(configFile);
                selectedTextureCorner = -1;
            }
        } else {
            saveSettings(configFile);
            selectedMaskCorner = -1;
        }
    }
}

//Key Events
void ofxSurface::_keyPressed(ofKeyEventArgs &e){
        
    switch (e.key) {
        case OF_KEY_F3:
            bEditMode = !bEditMode;
            break;
    }
    
    if (bActive && bEditMode & bEditMask) {

        // Delete the selected mask point
        //
        if ( (e.key == 'd') && 
            (selectedMaskCorner >= 0) && 
            (selectedMaskCorner < maskCorners.size() ) ){
            maskCorners.getVertices().erase(maskCorners.getVertices().begin()+ selectedMaskCorner );
            selectedMaskCorner = -1;
            doMask();
            saveSettings(configFile);
        }
        
        // Reset all the mask or the texture
        //
        if ( (e.key == 'c') ){
            maskCorners.clear();
            selectedMaskCorner = -1;
            ofPoint newPoint = ofPoint(0.0,0.0,0.0);
            maskCorners.addVertex(newPoint);
            newPoint.set(1.0,0.0,0.0);
            maskCorners.addVertex(newPoint);
            newPoint.set(1.0,1.0,0.0);
            maskCorners.addVertex(newPoint);
            newPoint.set(0.0,1.0,0.0);
            maskCorners.addVertex(newPoint);
            doMask();
            saveSettings(configFile);
        }
    }   
}