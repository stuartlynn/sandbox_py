#pragma once

#include "ofMain.h"
#include "ofxKinect.h"
#include "ofxOpenCv.h"
#include "ofxCv.h"
#include "ofMath.h"
#include "ofxGui.h"



class ofApp : public ofBaseApp{
	
public:
	void setup();
	void update();
	void draw();
	void exit();
	
	void drawPointCloud();
	
	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);
    void normalizePressed();
    void clearNormalization();
	
	ofxKinect kinect;
    ofTrueTypeFont myfont;

	
	ofxCvColorImage colorImg;
	
	ofxCvGrayscaleImage depthImage; // grayscale depth image
	ofxCvGrayscaleImage depthThreshNear; // the near thresholded image
	ofxCvGrayscaleImage depthThreshFar; // the far thresholded image
    
    
    ofxFloatSlider farThresh;
    ofxFloatSlider nearThresh;
    ofxFloatSlider smoothingFrames;
    
    ofxToggle findCountoursToggle;
    
    ofxButton normalizeButton;
    ofxButton clearNormalizationButton;

    
    ofxPanel gui;
		
	int nearThreshold;
	int farThreshold;
	
	void drawDepthRainbow(int x, int y, int w, int h);
	void drawCameraImage(int x, int y, int width, int height);
	
	// calibration
	void drawChessBoard(ofPoint center, float width, int numSide);
	bool calibrationMode;
	void calibrate();
	
	cv::Mat homography;
    cv::Mat normalization;
    
	bool homographyReady;
    bool haveNormalization;
	
	ofImage getCameraImage();
	ofImage makeDepthRainbow();
	ofImage cameraFeed;
	ofImage depthFeed;
    
    ofxCvContourFinder contourFinder;
	
	ofImage outputImage;
	
	ofMesh kinectTriangleStripMesh();
	
	bool perspective;

};
