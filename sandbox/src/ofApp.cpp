#include "ofApp.h"
#include <stdlib.h>

#include "Poco/Base64Encoder.h"
#include "Poco/Base64Decoder.h"

using namespace cv;
using namespace ofxCv;

//--------------------------------------------------------------
void ofApp::setup() {
	ofSetLogLevel(OF_LOG_VERBOSE);
	ofSetFrameRate(60);
	ofSetFullscreen(1);
	
	showGUI = true;

	int maxSmoothingFrames = 20;

    gui.setup();
	gui.setPosition(10, 10);
	gui.add(farThreshold.setup("Far Threshold", 150, 0, 255));
	gui.add(nearThreshold.setup("Near Threshold", 161, 0, 255));
    gui.add(normalizeButton.setup("Normalize"));
    
    gui.add(clearNormalizationButton.setup("Clear Normalization"));
    gui.add(smoothingFrames.setup("No Smoothing frames",1,1,maxSmoothingFrames));
    gui.add(findCountoursToggle.setup("Find Countours",false));
	gui.add(grayscaleToggle.setup("Grayscale",true));
    gui.add(landscapeToggle.setup("Landscape",false));
    gui.add(waterLevel.setup("Water Level", 0.1f, 0.0f, 1.0f));
    gui.add(grassLevel.setup("Grass Level",0.5f, 0.0f,1.0f));
    gui.add(hillLevel.setup("Hill Level",0.5f, 0.0f,1.0f));
	gui.add(snowLevel.setup("Snow Level",0.5f, 0.0f,1.0f));
    gui.add(exportGeoJSONButton.setup("Export GeoJSON"));
	gui.add(shaderToggle.setup("Use Shaders",false));

//	exportGeoJSONButton.addListener(this,&ofApp::exportGeoJSONPressed);
	normalizeButton.addListener(this,&ofApp::normalizePressed);
//	clearNormalizationButton.addListener(this,&ofApp::clearNormalization);
	
	farThreshold.addListener(this, &ofApp::guiInts);
	nearThreshold.addListener(this, &ofApp::guiInts);
	smoothingFrames.addListener(this, &ofApp::guiInts);
	findCountoursToggle.addListener(this, &ofApp::guiBools);
	shaderToggle.addListener(this, &ofApp::guiBools);
	grayscaleToggle.addListener(this, &ofApp::guiBools);
	landscapeToggle.addListener(this, &ofApp::guiBools);
	waterLevel.addListener(this, &ofApp::guiFloats);
	grassLevel.addListener(this, &ofApp::guiFloats);
	hillLevel.addListener(this, &ofApp::guiFloats);
	snowLevel.addListener(this, &ofApp::guiFloats);
//	normalizeButton.addListener(this, &ofApp::guiHandler);
//	clearNormalizationButton.addListener(this, &ofApp::guiHandler);
//	exportGeoJSONButton.addListener(this, &ofApp::guiHandler);
	
	sandbox.setup();
	
	
	// SOCKET IO
	isConnected = false;
	address = "http://localhost:3000";
	status = "not connected";
	socketIO.setup(address);
	ofAddListener(socketIO.notifyEvent, this, &ofApp::gotEvent);
	ofAddListener(socketIO.connectionEvent, this, &ofApp::onConnection);

//	downsample.allocate(200, 200, OF_IMAGE_COLOR);
	downsample.allocate(ofGetScreenWidth(), ofGetScreenHeight(), OF_IMAGE_COLOR);
}

void ofApp::guiInts(int &variable){
	updateGUIToSandbox();
}
void ofApp::guiFloats(float &variable){
	updateGUIToSandbox();
}
void ofApp::guiBools(bool &variable){
	updateGUIToSandbox();
}

void ofApp::updateGUIToSandbox(){
	sandbox.farThreshold = farThreshold;
	sandbox.nearThreshold = nearThreshold;
	sandbox.smoothingFrames = smoothingFrames;
	sandbox.findCountoursToggle = findCountoursToggle;
	sandbox.grayscaleToggle = grayscaleToggle;
	sandbox.landscapeToggle = landscapeToggle;
	sandbox.waterLevel = waterLevel;
	sandbox.grassLevel = grassLevel;
	sandbox.hillLevel = hillLevel;
	sandbox.snowLevel = snowLevel;
	sandbox.shaderToggle = shaderToggle;
}

//--------------------------------------------------------------
void ofApp::update() {
	sandbox.update();

	if(isConnected){
		downsample.setFromPixels(sandbox.outputImage.getPixels());
//		downsample.resize(888, 500);
		downsample.resize(100, 100);
		sendImageOverSocket(downsample);
	}
}

//--------------------------------------------------------------
void ofApp::draw() {
	sandbox.draw();
	if(showGUI){
		ofSetColor(255, 255);
		gui.draw();
	}
}

//--------------------------------------------------------------
void ofApp::exit() {
	sandbox.exit();
}

//--------------------------------------------------------------
void ofApp::normalizePressed(){
}

//--------------------------------------------------------------
void ofApp::keyPressed (int key) {
	switch (key) {
			
		case 'p':
			sandbox.perspective = !sandbox.perspective;
			break;
			
		case 'c':
			sandbox.toggleCalibration();
			break;
			
		case ' ':
			showGUI = !showGUI;
			break;

		case OF_KEY_UP:
			break;
			
		case OF_KEY_DOWN:
			break;
	}
}


void ofApp::sendImageOverSocket(ofImage image){
	ofPixels px;
	px.setFromPixels(image.getPixels().getData(), image.getWidth(), image.getHeight(), image.getImageType());

	ofBuffer imageBuffer;
	ofSaveImage(px, imageBuffer);
	stringstream b64Image = encode(imageBuffer);
	std::string eventName = "image";
	std::string imgString = "";
	imgString = "data:image/jpeg;base64," + b64Image.str();
	imgString.erase(std::remove(imgString.begin(), imgString.end(), '\n'), imgString.end());
	imgString.erase(std::remove(imgString.begin(), imgString.end(), '\r'), imgString.end());
	cout << "sending emit: " << eventName << endl;
	socketIO.emit(eventName, imgString);
}

stringstream ofApp::encode(ofBuffer buffer){
	// Convert the binary image data to string using base64 encoding
	stringstream ss;
	Poco::Base64Encoder b64enc(ss);
	//	b64enc.setLineLength(0);
	b64enc << buffer;
	// here's the data
	ss.str();
	return ss;
}

void ofApp::onConnection () {
	isConnected = true;
	bindEvents();
}

void ofApp::bindEvents () {
	std::string serverEventName = "server-event";
	socketIO.bindEvent(serverEvent, serverEventName);
	ofAddListener(serverEvent, this, &ofApp::onServerEvent);

	std::string pingEventName = "ping";
	socketIO.bindEvent(pingEvent, pingEventName);
	ofAddListener(pingEvent, this, &ofApp::onPingEvent);
}

void ofApp::gotEvent(string& name) {
	status = name;
}

void ofApp::onServerEvent (ofxSocketIOData& data) {
	ofLogNotice("ofxSocketIO", data.getStringValue("stringData"));
	ofLogNotice("ofxSocketIO", ofToString(data.getIntValue("intData")));
	ofLogNotice("ofxSocketIO", ofToString(data.getFloatValue("floatData")));
	ofLogNotice("ofxSocketIO", ofToString(data.getBoolValue("boolData")));
}

void ofApp::onPingEvent (ofxSocketIOData& data) {
	ofLogNotice("ofxSocketIO", "ping");
	std::string pong = "pong";
	std::string param = "foo";
	socketIO.emit(pong, param);
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
