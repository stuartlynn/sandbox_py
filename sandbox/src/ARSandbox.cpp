//
//  ARSandbox.cpp
//  sandbox
//
//  Created by Robby on 1/29/17.
//
//

#include "ARSandbox.h"
#include <stdlib.h>

using namespace cv;
using namespace ofxCv;

//--------------------------------------------------------------
void ARSandbox::setup() {
	
	farThreshold = 150;
	nearThreshold = 161;
	smoothingFrames = 1;
	waterLevel = 0.1f;
	grassLevel = 0.5f;
	hillLevel = 0.5f;
	snowLevel = 0.5f;
	findCountoursToggle = false;
	grayscaleToggle = true;
	landscapeToggle = false;
	shaderToggle = false;

	perspective = false;
	haveNormalization = false;
	
	int maxSmoothingFrames = 20;
		
	frameNo=0;
	for(int i=0; i< maxSmoothingFrames; i++){
		depthFrames.push_back( Mat::zeros(kinect.width, kinect.height, CV_64F));
	}
	mostRecentDepthField = Mat::zeros(kinect.width,kinect.height, CV_64F);
	mostRecentDepthFieldImage.allocate(kinect.width,kinect.height);
	
	normalization = Mat::zeros(kinect.width, kinect.height, CV_64F);
	
	shader.load("simpleShader/shader");
	
	
	outputImage.allocate(ofGetScreenWidth(), ofGetScreenHeight(), OF_IMAGE_COLOR);
	
	// INITIALIZE KINECT
	
	kinect.setRegistration(true); // enable depth->video image calibration
	kinect.init();
	
	
	if(kinect.open()){		// opens first available kinect
		printf("Opening Kinect with %d width and %d height\n",kinect.width, kinect.height);
		
		depthGrayscaleImage.allocate(kinect.width, kinect.height);
//		depthThreshNear.allocate(kinect.width, kinect.height);
//		depthThreshFar.allocate(kinect.width, kinect.height);
		
		depthFeed.allocate(kinect.width, kinect.height, OF_IMAGE_COLOR);
//		outputImage.allocate(kinect.width, kinect.height, OF_IMAGE_COLOR);
		
// zero the tilt on startup
//		kinect.setCameraTiltAngle(0);
	}
	else{
		printf("No Kinect\n");
		ofExit();
	}
}

void ARSandbox::toggleCalibration(){
	if(calibration == calibrating)
		calibration = uncalibrated;
	if(calibration == uncalibrated)
		calibration = calibrating;
}
//--------------------------------------------------------------

void ARSandbox::clearNormalization(){
	haveNormalization = false;
}

//--------------------------------------------------------------
void ARSandbox::normalizePressed(){
	cout << "button clicked!!!!!!" <<endl;
	for (int i = 0; i < kinect.width; i++) {
		for (int j = 0; j < kinect.height; j++) {
			normalization.at<double>(i,j) =  kinect.getWorldCoordinateAt(i, j).z;
		}
	}
	haveNormalization = true;
}

//--------------------------------------------------------------
void ARSandbox::update() {
	
	
	// gray background until calibrated
	if(calibration == calibrated){
		ofBackground(0);
	} else{
		ofBackground(128);
	}
	
	kinect.update();
	if(kinect.isFrameNew()) {
		
		cameraFeed.setFromPixels(kinect.getPixels());
		depthFeed.setFromPixels(kinect.getDepthPixels());
		
		if(0) {
// there's perhaps a smart way to do things, instead of iterating over every pixel...
//			depthThreshNear = depthImage;
//			depthThreshFar = depthImage;
//			depthThreshNear.threshold(nearThreshold, true);
//			depthThreshFar.threshold(farThreshold);
//			cvAnd(depthThreshNear.getCvImage(), depthThreshFar.getCvImage(), depthImage.getCvImage(), NULL);
		} else {
			depthGrayscaleImage.setFromPixels(kinect.getDepthPixels());
			ofPixels & pix = depthGrayscaleImage.getPixels();
			int numPixels = pix.size();
			for(int i = 0; i < numPixels; i++) {
				if(pix[i] > nearThreshold || pix[i] < farThreshold) {
					pix[i] = 0;
				} else {
					pix[i] = ofMap(pix[i], farThreshold, nearThreshold, 0, 255);
				}
			}
			depthGrayscaleImage.flagImageChanged();
		}
	}
	ARSandbox::bufferFrames();
	ARSandbox::averageFrames();
}
//--------------------------------------------------------------

void ARSandbox::bufferFrames(){
	depthFrames.pop_back();
	cv::Mat frame = Mat::zeros(kinect.width, kinect.height, CV_64F);
	ofPixels & pix  = depthGrayscaleImage.getPixels();
	
	for (int i = 0; i < kinect.width; i++) {
		for (int j = 0; j < kinect.height; j++) {
			frame.at<double>(i,j) =  pix[i+ j*kinect.width];
		}
	}
	
	depthFrames.insert(depthFrames.begin(),frame);
}

void ARSandbox::findBlobs(){
	ofxCvColorImage color;
	color.setFromPixels(outputImage.getPixels());
	ofxCvGrayscaleImage half;
	
	
	for(int i=0; i < 20; i++){
		levels[i] = (i+1)*(255/21.0);
	}
	
	
	for(int i =0; i < 20; i++) {
		half = color;
		half.threshold(levels[i]);
		contourFinder[i].findContours(half, 5, (half.width*half.height)/4, 4, false, true);
	}
}

//--------------------------------------------------------------
void ARSandbox::draw() {
	
	frameNo++;
	
	ofSetColor(255, 255);
	
	switch (calibration){
		case uncalibrated: {
			// PRE-CALIBRATION LOOP
			ofPoint center = ofPoint(ofGetScreenWidth() * 0.5, ofGetScreenHeight() * 0.5);
			float vidHeight = 400;
			float vidWidth = vidHeight * 1.3333;
			// draw camera
			kinect.draw(center.x - vidWidth, center.y - vidHeight * 0.5, vidWidth, vidHeight);
			// draw depth
			if(grayscaleToggle){
				depthGrayscaleImage.draw(center.x, center.y - vidHeight * 0.5, vidWidth, vidHeight);
			} else{
				ofxCvColorImage rainbow = rainbowFromGrayscale(depthGrayscaleImage);
				rainbow.draw(center.x, center.y - vidHeight * 0.5, vidWidth, vidHeight);
			}
		} break;
		case calibrating: {
			// CALIBRATION LOOP
			calibrate();
		} break;
		case calibrated: {
			// MAIN LOOP
//			ofImage depthRainbow = makeDepthRainbow();
//			warpPerspective(toCv(depthRainbow), toCv(outputImage), homography, cvSize(ofGetScreenWidth(), ofGetScreenHeight()));
			
			if(landscapeToggle){
				ofxCvColorImage landscape = landscapeRampFromGrayscale(mostRecentDepthFieldImage);
				warpPerspective(toCv(landscape), toCv(outputImage), homography, cvSize(ofGetScreenWidth(), ofGetScreenHeight()));
			}
			else if(grayscaleToggle){
				ofxCvColorImage grayColorData = convertGrayscaleDataFormat(mostRecentDepthFieldImage);
				warpPerspective(toCv(grayColorData), toCv(outputImage), homography, cvSize(ofGetScreenWidth(), ofGetScreenHeight()));
			} else{
				ofxCvColorImage rainbow = rainbowFromGrayscale(mostRecentDepthFieldImage);
				warpPerspective(toCv(rainbow), toCv(outputImage), homography, cvSize(ofGetScreenWidth(), ofGetScreenHeight()));
			}
			outputImage.update();
			if(shaderToggle){
				ofPlanePrimitive plane;
				plane.set(ofGetWidth(), ofGetHeight(), 10, 10);
				plane.mapTexCoords(0, 0, ofGetWidth(), ofGetHeight());
				
				outputImage.getTextureReference().bind();
				shader.begin();
				float mousePosition = ofMap(mouseX, 0, ofGetWidth(), 1.0, -1.0, true);
				mousePosition *= ofGetWidth();
				shader.setUniform1f("mouseX", mousePosition);
				shader.setUniform1f("waterLevel", waterLevel);
				shader.setUniform1f("u_time", ofGetElapsedTimef());
				//			float res[2] = {(float)ofGetWidth(), (float)ofGetHeight()};
				shader.setUniform2f("u_resolution", (float)ofGetWidth()*.2, (float)ofGetHeight()*.2);
				ofPushMatrix();
				ofTranslate(ofGetWidth()/2, ofGetHeight()/2);
				plane.draw();
				ofPopMatrix();
				shader.end();
				outputImage.getTextureReference().unbind();
			} else{
				outputImage.draw(0, 0, ofGetWidth(), ofGetHeight());
			}
			
			if(findCountoursToggle){
				cout <<"looking for countours" <<endl;
			}
			
			findBlobs();
			
			for(int c = 0; c < 20; c++){
				
				for(int i = 0; i < contourFinder[c].nBlobs; i++) {
					ofRectangle r = contourFinder[c].blobs.at(i).boundingRect;
					ofxCvBlob blob = contourFinder[c].blobs.at(i);
					ofNoFill();
					ofSetColor(255, 255, 255);
					drawContour(blob);
//					ofSetColor(92, 160, 255);
//					ofDrawRectangle(r);
					ofFill();
				}
			}
		} break;
	}
	
	if(perspective){
		ofEasyCam cam;
		cam.enableMouseInput();
		cam.setPosition(0, -500, 600);
		cam.lookAt(ofVec3f(0, 0, 0));
		cam.begin();
		ofEnableDepthTest();
		ofPushMatrix();
		ofRotate(ofGetElapsedTimef()*30, 0, 0, 1);
		ofMesh depthMesh = makeKinectDepthMesh();
		depthMesh.draw();
		ofPopMatrix();
		ofDisableDepthTest();
		cam.end();
	}
	
}

void ARSandbox::drawContour(ofxCvBlob blob){
	//    blob.draw();
	ofNoFill();
	ofBeginShape();
	for (int i = 0; i < blob.nPts; i++){
		ofVertex(blob.pts[i].x, blob.pts[i].y);
	}
	ofEndShape(true);
}

void ARSandbox::drawChessBoard(ofPoint center, float width, int numSide){
	float halfWidth = width * 0.5;
	float squareWidth = width / (float)numSide;
	for(int i = 0; i < numSide; i++){
		for(int j = 0; j < numSide; j++){
			ofSetColor(((i+j)%2) * 255);
			ofDrawRectangle(center.x + (i-numSide*0.5) * squareWidth, center.y + (j-numSide*0.5) * squareWidth, squareWidth, squareWidth);
		}
	}
	ofSetColor(255);
}

void ARSandbox::calibrate(){
	ofPoint center = ofPoint(ofGetWidth()*0.5, ofGetHeight()*0.5);
	
	float chessBoardWidth = ofGetHeight() * 0.66;
	drawChessBoard(center, chessBoardWidth, 6);
	
	vector<Point2f> srcPoints;
	for(int i = 1; i < 6; i++){
		for(int j = 1; j < 6; j++){
			float squareSize = chessBoardWidth / 6.0;
			Point2f point = Point2f(center.x - chessBoardWidth * 0.5 + (j)*squareSize,
									center.y - chessBoardWidth * 0.5 + (i)*squareSize);
			srcPoints.push_back( point );
		}
	}
	
	vector<Point2f> pointBuf;
	
	Mat img = toCv(cameraFeed);
	
	int chessFlags = CV_CALIB_CB_ADAPTIVE_THRESH;// | CV_CALIB_CB_NORMALIZE_IMAGE;
	bool found = findChessboardCorners(img, cv::Size(5, 5), pointBuf, chessFlags);
	
	cameraFeed.draw(0, 0, 100, 100);
	
	ofImage backAgain;
	toOf(img, backAgain.getPixels());
	backAgain.update();
	backAgain.draw(0, 100, 100, 100);
	
	if(found){
		homography = findHomography(Mat(pointBuf), Mat(srcPoints));
		calibration = calibrated;
	}
}
//-------------------------------------------------------------

void ARSandbox::averageFrames(){
	mostRecentDepthField = Mat::zeros(kinect.width,kinect.height, CV_64F);
	ofPixels & depthPixels = mostRecentDepthFieldImage.getPixels();
	
	int noPixels =depthPixels.size();
	//smoothingFrames=10;
	
	for(int i=0; i< smoothingFrames; i++){
		mostRecentDepthField += depthFrames[i]/(float)smoothingFrames;
	}
	
	for(int i=0 ; i < kinect.width; i++){
		for(int j=0; j<kinect.height; j++){
			depthPixels[i+kinect.width*j]=mostRecentDepthField.at<double>(i,j);
		}
	}
	mostRecentDepthFieldImage.flagImageChanged();
}

ofxCvColorImage ARSandbox::landscapeRampFromGrayscale(ofxCvGrayscaleImage image){
	ofxCvColorImage landscape;
	landscape.allocate(image.width, image.height);
	ofPixels & landscapePix  = landscape.getPixels();
	ofPixels & grayPix  = image.getPixels();
	
	
	int numGrayPixels = grayPix.size();
	for(int i = 0; i < numGrayPixels; i++ ){
		ofColor color;
		if (grayPix[i] < waterLevel){
			color.r = 95;
			color.g = 145;
			color.b = 226;
		}
		else if(grayPix[i] > waterLevel && grayPix[i] < waterLevel+grassLevel){
			color.r = 77;
			color.g = 198;
			color.b = 47;
		}
		else if((grayPix[i] > waterLevel + grassLevel)  && (grayPix[i] < waterLevel + grassLevel + hillLevel ) ){
			color.r = 127;
			color.g = 93;
			color.b = 15;
		}
		else if( grayPix[i] > waterLevel + grassLevel + hillLevel){
			color.r = 247;
			color.g = 245;
			color.b =  239;
		}
		landscapePix[i*3+0] = color[0];
		landscapePix[i*3+1] = color[1];
		landscapePix[i*3+2] = color[2];
	}
	landscape.flagImageChanged();
	return landscape;
}

ofxCvColorImage ARSandbox::rainbowFromGrayscale(ofxCvGrayscaleImage image){
	ofxCvColorImage rainbow;
	rainbow.allocate(image.width, image.height);
	ofPixels & colorPix = rainbow.getPixels();
	ofPixels & grayPix = image.getPixels();
	int numGrayPixels = grayPix.size();
	for(int i = 0; i < numGrayPixels; i++) {
		ofColor color;
		color.setHsb(grayPix[i], 200, 200);
		colorPix[i*3+0] = color[0];
		colorPix[i*3+1] = color[1];
		colorPix[i*3+2] = color[2];
	}
	rainbow.flagImageChanged();
	return rainbow;
}

ofxCvColorImage ARSandbox::convertGrayscaleDataFormat(ofxCvGrayscaleImage image){
	// this merely converts an ofxCvGrayscaleImage into an ofxCvColorImage format
	//  but maintains that the image appears grayscale
	ofxCvColorImage grayscale;
	grayscale.allocate(image.width, image.height);
	ofPixels & colorPix = grayscale.getPixels();
	ofPixels & grayPix = image.getPixels();
	int numGrayPixels = grayPix.size();
	for(int i = 0; i < numGrayPixels; i++) {
		colorPix[i*3+0] = grayPix[i];
		colorPix[i*3+1] = grayPix[i];
		colorPix[i*3+2] = grayPix[i];
	}
	grayscale.flagImageChanged();
	return grayscale;
}



// relies on data inside of depthFeed
ofImage ARSandbox::makeDepthRainbow(){
	
	ofImage img;
	
	int w = kinect.width;
	int h = kinect.height;
	float depthMax = 0;
	float depthMin = 100000;
	
	img.allocate(w, h, OF_IMAGE_COLOR);
	img.setColor(ofColor::white);
	
	
	for (int i = 0; i < w; i++) {
		for (int j = 0; j < h; j++) {
			
			
			float d = mostRecentDepthField.at<float>(i);
			
			if (d > depthMax){
				depthMax= d;
			}
			if (d < depthMin){
				depthMin= d;
			}
			
			if(haveNormalization){
				d -= normalization.at<double>(i,j);
			}
			float scaledD = ofMap(d, nearThresh,farThresh, 0, 255);
			
			if (scaledD >= 255 || scaledD<0) {
				img.setColor(i, j , ofColor(0,0,0));
			}
			else{
				ofColor color= ofColor::fromHsb((int)scaledD,255,255);
				img.setColor(i,j,color);
			}
			
			
//			ofColor color= ofColor(((int)depth.z), ((int)depth.z), ((int)depth.z));
//			ofColor color= ofColor(((int)depth.x)%255, ((int)depth.y)%255, ((int)depth.z)%255);

//			int hue = ofMap(depth.z, minZ, maxZ, 0, 255);
//			color.setHsb(hue, 200, 200);
			
			
		}
	}
	cout << depthMin << ' ' << depthMax<<endl;
	img.update();
	return img;
}

ofMesh ARSandbox::makeKinectDepthMesh(){
	ofMesh depthMesh;
	depthMesh.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);
	
	int width = kinect.width;
	int height = kinect.height;
	
	// lower resolution for speed
	int STRIDE = 4;
	
	ofPixels depthPixels = depthFeed.getPixels();
	int rowCounter = 0;
	for(int p = 0; p < height-STRIDE; p+=STRIDE){
		for(int q = 0; q < width; q+=STRIDE){
			int w, h1, h2;
			// triangle order goes like this:
			if(rowCounter%2 == 0){
				//  (1)   (3)
				//   |  /  |  /
				//   | /   | /  ...
				//  (2)   (4)
				w = q;
				h1 = p;
				h2 = p+STRIDE;
			} else{
				//         (4)   (2)   <--this row of points lies exactly on top of
				//       /  |  /  |       the previous strip's bottom row of points
				// ...  /   | /   |
				//         (3)   (1)
				w = width-STRIDE-q;
				h1 = p+STRIDE;
				h2 = p;
			}
			depthMesh.addVertex(ofVec3f(w - width*.5,
										h1 - height*.5,
										depthPixels[h1*width+w]));
			depthMesh.addColor(kinect.getColorAt(w, h1));
			depthMesh.addVertex(ofVec3f(w - width*.5,
										h2 - height*.5,
										depthPixels[h2*width+w]));
			depthMesh.addColor(kinect.getColorAt(w, h2));
		}
		rowCounter += 1;
	}
	return depthMesh;
}

//--------------------------------------------------------------
void ARSandbox::exit() {
//	kinect.setCameraTiltAngle(0); // zero the tilt on exit
	kinect.close();
}

//--------------------------------------------------------------
//Code to generate geojson from the detected contours.

void ARSandbox::exportGeoJSONPressed(){
	printf("EXPORING GEO JSON");
	ofFile output;
	output.open(ofToDataPath("test.geojson"), ofFile::WriteOnly);
	string result =  generateFeatureCollection();
	cout << result << endl;
	output << result << endl;
	output.close();
}

string ARSandbox::generateFeatureCollection(){
	std::stringstream result;
	vector<string> features;
	
	result <<"{\"type\": \"FeatureCollection\",\"features\": [ ";
	for (int j=0; j< 20; j++){
		for(int i=0; i < contourFinder[j].blobs.size(); i++){
			features.push_back(generatePolygon(contourFinder[j].blobs[i].pts, levels[j] ));
		}
	}
	
	for(int i=0 ; i< features.size(); i++){
		result << features[i];
		if (i<features.size()-1){
			result<<",";
		}
	}
	
	result <<"]}";
	return result.str();
	
}


string ARSandbox::generatePolygon(vector <ofPoint> points,float height){
	std::stringstream ss;
	
	ss << "{\"type\" : \"Feature\",";
	ss << " \"properties\" : {\"height\":" << height<<" },";
	ss << " \"geometry\" : { ";
	ss << " \"type\" : \"Polygon\", ";
	ss << "\"coordinates\": [[";
	
	for (int i =0; i< points.size(); i++){
		ss << "[";
		ss << points[i].x/kinect.width - 0.5<<"," << points[i].y/kinect.height - 0.5;
		ss << "]";
		ss << ",";
	}
	ss <<"[" <<points[0].x/kinect.width - 0.5<<"," << points[0].y/kinect.height - 0.5 << "]" ;
	ss<< "]]}}";
	return ss.str();
}
