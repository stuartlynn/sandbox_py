//
//  ARSandbox.h
//  sandbox
//
//  Created by Robby on 1/29/17.
//
//

#ifndef ARSandbox_h
#define ARSandbox_h

#include "ofMain.h"
#include "ofxKinect.h"
#include "ofxOpenCv.h"
#include "ofxCv.h"
#include "ofMath.h"
#include "ofxGui.h"
#include <vector>

typedef enum {
	uncalibrated,
	calibrating,
	calibrated,
} Calibration;

class ARSandbox : public ofBaseApp{
	
public:
	void setup();
	void update();
	void draw();
	void exit();
	
	void toggleCalibration();
private:
	Calibration calibration = uncalibrated;
public:
	
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
	
	// GUI INPUTS
	unsigned int farThresh;
	unsigned int nearThresh;
	unsigned int smoothingFrames;
	float waterLevel;
	float grassLevel;
	float sandLevel;
	float hillLevel;
	float snowLevel;
	bool findCountoursToggle;
	bool landscapeToggle;
//	ofxButton normalizeButton;
//	ofxButton exportGeoJSONButton;
//	ofxButton clearNormalizationButton;
	bool grayscaleToggle;
	bool shaderToggle;
	
	// HOMOGRAPHY, CALIBRATION
	void calibrate();
	void drawChessBoard(ofPoint center, float width, int numSide);
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
	
	// GEOJSON generation
	string generatePolygon(vector <ofPoint> points,float height);
	string generateFeatureCollection( );
	void exportGeoJSONPressed();
	// 3D DEPTH MESH
	ofMesh makeKinectDepthMesh();
	bool perspective;
	double levels[20];
	// more
	ofImage makeDepthRainbow();
	ofxCvColorImage rainbowFromGrayscale(ofxCvGrayscaleImage image);
	ofxCvColorImage convertGrayscaleDataFormat(ofxCvGrayscaleImage image);
	ofxCvColorImage landscapeRampFromGrayscale(ofxCvGrayscaleImage image);
	ofxCvContourFinder contourFinder[20];
	ofxCvBlob countours[20];
	void findBlobs();
	void drawContour(ofxCvBlob blob);
	
	ofShader shader;
	
};

#endif /* ARSandbox_h */
