#pragma once

#include "ofMain.h"
#include "ofxKinect.h"
#include "ofxOpenCv.h"
#include "ofxCv.h"
#include "ofMath.h"
#include "ofxGui.h"
#include <vector>

class ofApp : public ofBaseApp{
	
public:
	void setup();
	void update();
	void draw();
	void exit();
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
	
	// KINECT, VIDEO MEMORY
	ofxKinect kinect;
	// 2 raw feeds
	ofImage cameraFeed;
	ofImage depthFeed;
	// processed images
	ofxCvGrayscaleImage depthGrayscaleImage; // depth grayscale, clipped and remapped with thresholds
//	ofxCvGrayscaleImage depthThreshNear; // the near thresholded image
//	ofxCvGrayscaleImage depthThreshFar; // the far thresholded image
	ofImage outputImage; // the full-screen final projected image

    // GUI
	bool showGUI;
	ofxPanel gui;
    ofxFloatSlider farThresh;
    ofxFloatSlider nearThresh;
    ofxIntSlider smoothingFrames;
    ofxFloatSlider waterLevel;
    ofxFloatSlider grassLevel;
    ofxFloatSlider sandLevel;
    ofxFloatSlider hillLevel;
    ofxFloatSlider snowLevel;
	ofxToggle findCountoursToggle;
    ofxToggle landscapeToggle;
    ofxButton normalizeButton;
    ofxButton clearNormalizationButton;
	ofxToggle grayscaleToggle;
	ofxToggle shaderToggle;

	// HOMOGRAPHY, CALIBRATION
	void calibrate();
	void drawChessBoard(ofPoint center, float width, int numSide);
	bool homographyReady;
	bool calibrationMode;
	cv::Mat homography;

	// NORMALIZATION, AVERAGING
	void normalizePressed();
	void clearNormalization();
	void bufferFrames();
	void averageFrames();
	cv::Mat normalization;
	bool haveNormalization;
	std::vector<cv::Mat> depthFrames;
	cv::Mat mostRecentDepthField;
    ofxCvGrayscaleImage mostRecentDepthFieldImage;
	ofxIntSlider nearThreshold;
	ofxIntSlider farThreshold;
    int frameNo;
	
	// 3D DEPTH MESH
	ofMesh makeKinectDepthMesh();
	bool perspective;
	
	// more
	ofImage makeDepthRainbow();
	ofxCvColorImage rainbowFromGrayscale(ofxCvGrayscaleImage image);
	ofxCvColorImage convertGrayscaleDataFormat(ofxCvGrayscaleImage image);
    ofxCvColorImage landscapeRampFromGrayscale(ofxCvGrayscaleImage image);
    ofxCvContourFinder contourFinder[8];
	void findBlobs();
    void drawContour(ofxCvBlob blob);
	
	ofShader shader;

};
