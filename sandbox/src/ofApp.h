#pragma once

#include "ofMain.h"
#include "ofMath.h"
#include "ofxGui.h"
#include <vector>

#include "ARSandbox.h"

// SOCKET IO
#include "ofEvents.h"
#include "ofxSocketIO.h"
#include "ofxSocketIOData.h"


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
	
	ARSandbox sandbox;

    // GUI
	bool showGUI;
	ofxPanel gui;
	ofxIntSlider nearThreshold;
	ofxIntSlider farThreshold;
    ofxIntSlider smoothingFrames;
    ofxFloatSlider waterLevel;
    ofxFloatSlider grassLevel;
    ofxFloatSlider sandLevel;
    ofxFloatSlider hillLevel;
    ofxFloatSlider snowLevel;
	ofxToggle findCountoursToggle;
    ofxToggle landscapeToggle;
    ofxButton normalizeButton;
    ofxButton exportGeoJSONButton;
    ofxButton clearNormalizationButton;
	ofxToggle grayscaleToggle;
	ofxToggle shaderToggle;
	
	void normalizePressed();
	
	void guiInts(int &variable);
	void guiFloats(float &variable);
	void guiBools(bool &variable);
	void updateGUIToSandbox();
	
	
	// SOCKET IO
	void sendImageOverSocket(ofImage image);
	ofImage downsample;

	ofxSocketIO socketIO;
	void gotEvent(std::string& name);
	void onServerEvent(ofxSocketIOData& data);
	void onPingEvent(ofxSocketIOData& data);
	bool isConnected;
	void onConnection();
	void bindEvents();
	ofEvent<ofxSocketIOData&> serverEvent;
	ofEvent<ofxSocketIOData&> pingEvent;
	std::string address;
	std::string status;
	stringstream encode(ofBuffer buffer);
};
