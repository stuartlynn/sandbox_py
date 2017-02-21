#include "ofApp.h"

#include "Poco/Base64Encoder.h"
#include "Poco/Base64Decoder.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetFrameRate(10);

	// terrain
	terrainImage.allocate(75, 75, OF_IMAGE_GRAYSCALE);
	
	// socket io
	isConnected = false;
	address = "http://localhost:3000";
	status = "not connected";
	socketIO.setup(address);
	ofAddListener(socketIO.notifyEvent, this, &ofApp::gotEvent);
	ofAddListener(socketIO.connectionEvent, this, &ofApp::onConnection);
}

//--------------------------------------------------------------
void ofApp::update(){
	// terrain
	updateTerrain();
	
	// socket io
	if(isConnected){
		sendImageOverSocket();
	}
}

//--------------------------------------------------------------
void ofApp::draw(){
	// terrain
	terrainImage.draw(0, 0, ofGetHeight(), ofGetHeight());

}


stringstream ofApp::encode(ofBuffer buffer){
	// Convert the binary image data to string using base64 encoding
	stringstream ss;
	Poco::Base64Encoder b64enc(ss);
	//	b64enc.setLineLength(0);
	b64enc << buffer;
	
	// here's the data
	ss.str();
	//	cout << ss.str() << endl;
	
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


void ofApp::sendImageOverSocket(){
	ofPixels px;
	px.setFromPixels(terrainImage.getPixels().getData(), terrainImage.getWidth(), terrainImage.getHeight(), terrainImage.getImageType());
	
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


void ofApp::updateTerrain(){
	float SCALE = 0.02;
	unsigned char *pixels = (unsigned char*)terrainImage.getPixels().getData();
	for(int i = 0; i < terrainImage.getWidth(); i++){
		for(int j = 0; j < terrainImage.getHeight(); j++){
			float point = ofNoise(i*SCALE, j*SCALE, ofGetElapsedTimeMillis()*0.0005);
			pixels[i+j*((int)terrainImage.getWidth())] = point*255;
		}
	}
	terrainImage.update();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}
//--------------------------------------------------------------
void ofApp::keyReleased(int key){}
//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){}
//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){}
//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){}
//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){}
//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){}
//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){}
//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){}
//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){}
//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){}
