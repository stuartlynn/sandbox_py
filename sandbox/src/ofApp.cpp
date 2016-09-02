#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {
    ofSetLogLevel(OF_LOG_VERBOSE);
	ofSetFrameRate(60);
	
    // enable depth->video image calibration
    kinect.setRegistration(true);
    kinect.init();
    //kinect.init(true); // shows infrared instead of RGB video image
    //kinect.init(false, false); // disable video image (faster fps)
    kinect.open();		// opens first available kinect
	
	printf("Opening Kinect with %d width and %d height\n",kinect.width, kinect.height);
	
	colorImg.allocate(kinect.width, kinect.height);
    depthImage.allocate(kinect.width, kinect.height);
    depthThreshNear.allocate(kinect.width, kinect.height);
    depthThreshFar.allocate(kinect.width, kinect.height);
    
    nearThreshold = 230;
    farThreshold = 70;
    bThreshWithOpenCV = true;
    
    // zero the tilt on startup
    angle = 0;
    kinect.setCameraTiltAngle(angle);
    
    // start from the front
    bDrawPointCloud = false;
}

//--------------------------------------------------------------
void ofApp::update() {
    
    ofBackground(0);
    
    kinect.update();
    
    if(kinect.isFrameNew()) {
        depthImage.setFromPixels(kinect.getDepthPixels());
        
        // we do two thresholds - one for the far plane and one for the near plane
        // we then do a cvAnd to get the pixels which are a union of the two thresholds
        if(bThreshWithOpenCV) {
            depthThreshNear = depthImage;
            depthThreshFar = depthImage;
            depthThreshNear.threshold(nearThreshold, true);
            depthThreshFar.threshold(farThreshold);
            cvAnd(depthThreshNear.getCvImage(), depthThreshFar.getCvImage(), depthImage.getCvImage(), NULL);
        } else {
            // or we do it ourselves - show people how they can work with the pixels
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
        
        // update the cv images
        depthImage.flagImageChanged();
    }
}

//--------------------------------------------------------------
void ofApp::draw() {
    
    ofSetColor(255, 255);
    
//	kinect.drawDepth(10, 10, 400, 300);
//	kinect.draw(420, 10, 400, 300);
//	
//	depthImage.draw(10, 320, 400, 300);

	drawDepthRainbow(0, 0, 640, 480);
	drawCameraImage(640, 0, 640, 480);
}

static float minZ = 0.0;
static float maxZ = 100.0;

void ofApp::drawCheckerboard(ofPoint center, float width){
	
}

void ofApp::drawDepthRainbow(int x, int y, int width, int height){
	ofImage img;

	int w = 640;
	int h = 480;
	
	float updateMinZ = 100;
	float updateMaxZ = 100;
	
	img.allocate(w, h, OF_IMAGE_COLOR);
	img.setColor(ofColor::white);
	for (int i = 0; i < w; i++) {
		for (int j = 0; j < h; j++) {
			ofVec3f depth = kinect.getWorldCoordinateAt(i, j);
//			ofColor color= ofColor(((int)depth.x)%255, ((int)depth.y)%255, ((int)depth.z)%255);
			ofColor color= ofColor(((int)depth.z)%255, ((int)depth.z)%255, ((int)depth.z)%255);
			
//			color.setHue(200);
//			color.setBrightness(200);
//			color.setSaturation(200);

			int hue = ofMap(depth.z, minZ, maxZ, 0, 255);
			
			color.setHsb(hue, 200, 200);
			
			img.setColor(i % w, j % h, color);

			if(depth.z < updateMinZ) updateMinZ = depth.z;
			if(depth.z > updateMaxZ) updateMaxZ = depth.z;
		}
	}
	img.update();
	img.draw(x, y, width, height);
//	ofScale(1, -1, -1);

	ofDrawBitmapString("MAX:", 0, 500);
	ofDrawBitmapString(ofToString(maxZ), 30, 500);
	ofDrawBitmapString("MIN:", 0, 520);
	ofDrawBitmapString(ofToString(minZ), 30, 520);
	
	maxZ = updateMaxZ;
	minZ = updateMinZ;
	
//	if(kinect.getDistanceAt(x, y) > 0) {
//		kinect.getColorAt(x,y);
//		kinect.getWorldCoordinateAt(x, y);
//	}

}

void ofApp::drawCameraImage(int x, int y, int width, int height){
	ofImage img;
	int w = 640;
	int h = 480;
	img.allocate(w, h, OF_IMAGE_COLOR);
	img.setColor(ofColor::white);
	for (int i = 0; i < w; i++) {
		for (int j = 0; j < h; j++) {
			ofColor color = kinect.getColorAt(i,j);
			img.setColor(i % w, j % h, color);
		}
	}
	img.update();
	img.draw(x, y, width, height);
}


void ofApp::drawPointCloud() {
    int w = 640;
    int h = 480;
    ofMesh mesh;
    mesh.setMode(OF_PRIMITIVE_POINTS);
    int step = 2;
    for(int y = 0; y < h; y += step) {
        for(int x = 0; x < w; x += step) {
            if(kinect.getDistanceAt(x, y) > 0) {
                mesh.addColor(kinect.getColorAt(x,y));
                mesh.addVertex(kinect.getWorldCoordinateAt(x, y));
            }
        }
    }
    glPointSize(3);
    ofPushMatrix();
    // the projected points are 'upside down' and 'backwards'
    ofScale(1, -1, -1);
    ofTranslate(0, 0, -1000); // center the points a bit
    ofEnableDepthTest();
    mesh.drawVertices();
    ofDisableDepthTest();
    ofPopMatrix();
}

//--------------------------------------------------------------
void ofApp::exit() {
    kinect.setCameraTiltAngle(0); // zero the tilt on exit
    kinect.close();    
}

//--------------------------------------------------------------
void ofApp::keyPressed (int key) {
    switch (key) {
        case ' ':
            bThreshWithOpenCV = !bThreshWithOpenCV;
            break;
            
        case'p':
            bDrawPointCloud = !bDrawPointCloud;
            break;
            
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
            nearThreshold ++;
            if (nearThreshold > 255) nearThreshold = 255;
            break;
            
        case '-':
            nearThreshold --;
            if (nearThreshold < 0) nearThreshold = 0;
            break;
            
        case 'w':
            kinect.enableDepthNearValueWhite(!kinect.isDepthNearValueWhite());
            break;
            
        case 'c':
            kinect.setCameraTiltAngle(0); // zero the tilt
            kinect.close();
            break;
            
			
        case OF_KEY_UP:
            angle++;
            if(angle>30) angle=30;
            kinect.setCameraTiltAngle(angle);
            break;
            
        case OF_KEY_DOWN:
            angle--;
            if(angle<-30) angle=-30;
            kinect.setCameraTiltAngle(angle);
            break;
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
