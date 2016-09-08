#include "ofApp.h"
#include <stdlib.h>

using namespace cv;
using namespace ofxCv;

//--------------------------------------------------------------
void ofApp::setup() {
	ofSetLogLevel(OF_LOG_VERBOSE);
	ofSetFrameRate(60);
	ofSetFullscreen(1);
	
	calibrationMode = false;
	homographyReady = false;
	perspective = false;
	
    int maxSmoothingFrames = 20;
    
    gui.setup();
    gui.add(farThresh.setup("farThresh", 10, -1000,4000));
    gui.add(nearThresh.setup("nearThresh", -10, -1000, 1000));
    gui.add(normalizeButton.setup("Normalize"));
    gui.add(clearNormalizationButton.setup("Clear Normalization"));
    gui.add(smoothingFrames.setup("No Smoothing frames",1,1,maxSmoothingFrames));
            
    gui.add(findCountoursToggle.setup("Find Countours",false));
    
    frameNo=0;
    
    for(int i=0; i< maxSmoothingFrames; i++){
        depthFrames.push_back( Mat::zeros(kinect.width, kinect.height, CV_64F));
    }
    
    mostRecentDepthField = Mat::zeros(kinect.width,kinect.height, CV_64F);

    
    normalizeButton.addListener(this,&ofApp::normalizePressed);
    clearNormalizationButton.addListener(this,&ofApp::clearNormalization);
    
    haveNormalization = false;
	// INITIALIZE KINECT
	
	kinect.setRegistration(true); // enable depth->video image calibration
	kinect.init();
    
	if(kinect.open()){		// opens first available kinect
		printf("Opening Kinect with %d width and %d height\n",kinect.width, kinect.height);
		
		colorImg.allocate(kinect.width, kinect.height);
		depthImage.allocate(kinect.width, kinect.height);
		depthThreshNear.allocate(kinect.width, kinect.height);
		depthThreshFar.allocate(kinect.width, kinect.height);
		
		depthFeed.allocate(kinect.width, kinect.height, OF_IMAGE_COLOR);
		outputImage.allocate(ofGetScreenWidth(), ofGetScreenHeight(), OF_IMAGE_COLOR);
//		outputImage.allocate(kinect.width, kinect.height, OF_IMAGE_COLOR);
		
        normalization = Mat::zeros(kinect.width, kinect.height, CV_64F);

		depthMesh.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);

		// zero the tilt on startup
//		kinect.setCameraTiltAngle(0);
		
	}
	else{
		printf("No Kinect\n");
		ofExit();
	}
}
//--------------------------------------------------------------

void ofApp::clearNormalization(){
    haveNormalization = false;
}

//--------------------------------------------------------------
void ofApp::normalizePressed(){
    cout << "button clicked!!!!!!" <<endl;
    for (int i = 0; i < kinect.width; i++) {
        for (int j = 0; j < kinect.height; j++) {
            normalization.at<double>(i,j) =  kinect.getWorldCoordinateAt(i, j).z;
        }
    }
    haveNormalization = true;

}

//--------------------------------------------------------------
void ofApp::update() {
	
	// gray background until calibrated
	if(!homographyReady || calibrationMode){
		ofBackground(128);
	}else {
		ofBackground(0);
	}
	
	kinect.update();
	if(kinect.isFrameNew()) {
		
		cameraFeed.setFromPixels(kinect.getPixels());
		depthFeed.setFromPixels(kinect.getDepthPixels());

		if(1) {
			depthThreshNear = depthImage;
			depthThreshFar = depthImage;
			depthThreshNear.threshold(nearThreshold, true);
			depthThreshFar.threshold(farThreshold);
			cvAnd(depthThreshNear.getCvImage(), depthThreshFar.getCvImage(), depthImage.getCvImage(), NULL);
		} else {
			ofPixels & pix = depthImage.getPixels();
			int numPixels = pix.size();
			for(int i = 0; i < numPixels; i++) {
				if(pix[i] < nearThreshold && pix[i] > farThreshold) {
					pix[i] = 255;
				} else {
					pix[i] = 0;
				}
			}
		}
		depthImage.flagImageChanged();
	}
}
//--------------------------------------------------------------

void ofApp::bufferFrames(){
    depthFrames.pop_back();
    cv::Mat frame = Mat::zeros(kinect.width, kinect.height, CV_64F);
    for (int i = 0; i < kinect.width; i++) {
        for (int j = 0; j < kinect.height; j++) {
            frame.at<double>(i,j) =  kinect.getWorldCoordinateAt(i, j).z;
        }
    }

    depthFrames.insert(depthFrames.begin(),frame);
}

//--------------------------------------------------------------
void ofApp::draw() {
    
    frameNo++;
    ofApp::bufferFrames();
    
	ofSetColor(255, 255);
	
	if(calibrationMode){
		// CALIBRATION LOOP
		calibrate();
	}
	else if(homographyReady){
		// MAIN LOOP
		ofImage depthRainbow = makeDepthRainbow();
		warpPerspective(toCv(depthRainbow), toCv(outputImage), homography, cvSize(ofGetScreenWidth(), ofGetScreenHeight()));
		outputImage.update();
		outputImage.draw(0, 0, ofGetWidth(), ofGetHeight());
        if(findCountoursToggle){
            cout <<"looin for countours" <<endl;
        }
//        gui.draw();
		

	}
	else{
		// PRE-CALIBRATION LOOP
		ofPoint center = ofPoint(ofGetScreenWidth() * 0.5, ofGetScreenHeight() * 0.5);
		float vidHeight = 400;
		float vidWidth = vidHeight * 1.3333;
		// draw camera / depth
		kinect.draw(center.x - vidWidth, center.y - vidHeight * 0.5, vidWidth, vidHeight);
		kinect.drawDepth(center.x, center.y - vidHeight * 0.5, vidWidth, vidHeight);
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
		kinectTriangleStripMesh();
		depthMesh.draw();
		ofPopMatrix();
		ofDisableDepthTest();
		cam.end();
	}

}

void ofApp::drawChessBoard(ofPoint center, float width, int numSide){
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

void ofApp::calibrate(){
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
		homographyReady = true;
		cout << homography << endl;
		
		// break calibration loop
		calibrationMode = false;
	}
}
//-------------------------------------------------------------

void ofApp::averageFrames(){
    mostRecentDepthField = Mat::zeros(kinect.width,kinect.height, CV_64F);
    for(int i=0; i< smoothingFrames; i++){
        mostRecentDepthField  +=  depthFrames[i]/(float)smoothingFrames;
    }
}

// relies on data inside of depthFeed
ofImage ofApp::makeDepthRainbow(){
	
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

void ofApp::kinectTriangleStripMesh(){
	depthMesh.clear();

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
				//         (4)   (2)
				//       /  |  /  |
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
}

//--------------------------------------------------------------
void ofApp::exit() {
//    kinect.setCameraTiltAngle(0); // zero the tilt on exit
	kinect.close();
}

//--------------------------------------------------------------
void ofApp::keyPressed (int key) {
	switch (key) {
			
		case '>':
		case '.':
			farThreshold ++;
			if (farThreshold > 255) farThreshold = 255;
			break;
			
		case '<':
		case ',':
			farThreshold --;
			if (farThreshold < 0) farThreshold = 0;
			break;
			
		case '+':
		case '=':
			nearThreshold +=100;
			if (nearThreshold > 255) nearThreshold = 255;
//			break;
			
		case '-':
			nearThreshold -=100;
//			if (nearThreshold < 0) nearThreshold = 0;
			break;
			
		case 'p':
			perspective = !perspective;
			break;
//        case 'w':
//            kinect.enableDepthNearValueWhite(!kinect.isDepthNearValueWhite());
//            break;
			
		case 'q':
//            kinect.setCameraTiltAngle(0); // zero the tilt
			kinect.close();
			break;
			
		case 'c':
//			kinect.setCameraTiltAngle(0); // zero the tilt
			calibrationMode = !calibrationMode;
			break;
			
			
		case OF_KEY_UP:
//            angle++;
//            if(angle>30) angle=30;
//            kinect.setCameraTiltAngle(angle);
			break;
			
		case OF_KEY_DOWN:
//            angle--;
//            if(angle<-30) angle=-30;
//            kinect.setCameraTiltAngle(angle);
			break;
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){ }
//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){ }
//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){ }
//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){ }
//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){ }
//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){ }
//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){ }
//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){ }
//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){ }
//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ }
