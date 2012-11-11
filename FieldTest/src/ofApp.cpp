/*
 z axis is up/down, everything is on the x/y plane
 +z is up, -z is down
 everything on screen is done in millimeters, though data is diverse

 need to test with actual serial device
 */

#include "ofApp.h"

#include "Conversion.h"

float stageSize = feetInchesToMillimeters(15, 8);
float stageHeight = feetToMillimeters(10);
float eyeLevel = feetInchesToMillimeters(5, 8);

void ofApp::setup() {
	ofSetVerticalSync(true);
	ofEnableAlphaBlending();
	
	driver.setup("tty", 9600);
	driver.setDeadZone(10);
	
	font.loadFont("uni05_53.ttf", 6, false);
	
	Speaker::setupMesh();
	
	wiresCloud.setMode(OF_PRIMITIVE_POINTS);
	wiresCloud.load("wires.ply");
	scale(wiresCloud, inchesToMillimeters(1));
	
	centersCloud.setMode(OF_PRIMITIVE_POINTS);
	centersCloud.load("centers.ply");
	scale(centersCloud, inchesToMillimeters(1));
	
	buildWires();
	buildSpeakers();
	
	autorun = false;
}

void ofApp::buildWires() {
	wires.setMode(OF_PRIMITIVE_LINES);
	for(int i = 0; i < wiresCloud.getNumVertices(); i++) {
		ofVec3f wireCenter = wiresCloud.getVertex(i);
		wires.addVertex(wireCenter);
		wires.addVertex(wireCenter + ofVec3f(0, 0, stageHeight));
	}
}

void ofApp::buildSpeakers() {
	ofSeedRandom(0);
	for(int i = 0; i < centersCloud.getNumVertices(); i++) {
		Speaker speaker;
		ofVec3f position = centersCloud.getVertex(i);
		position.z = feetToMillimeters(ofRandom(2, 6));
		speaker.setup(position, wiresCloud);
		speakers.push_back(speaker);
	}
}

void ofApp::update() {
	listeners.clear();
	if(autorun) {
		ofVec2f planet;
		float t = .05 * ofGetElapsedTimef();
		planet.x = ofMap(ofNoise(t, 0), 0, 1, -stageSize, stageSize) / 2; 
		planet.y = ofMap(ofNoise(0, t), 0, 1, -stageSize, stageSize) / 2;
		listeners.push_back(planet);
	}
	if(ofGetKeyPressed('m')) {
		listeners.push_back(ofVec2f(0, 0));
	}
	if(ofGetKeyPressed(' ')) {
		listeners.push_back(ofVec2f(
			ofMap(mouseX, 0, ofGetWidth(), -stageSize / 2, stageSize / 2),
			ofMap(mouseY, 0, ofGetHeight(), stageSize / 2, -stageSize / 2)));
	}
	
	for(int i = 0; i < speakers.size(); i++) {
		speakers[i].update(listeners);
	}
	
	vector<float> angles;
	for(int i = 0; i < speakers.size(); i++) {
		angles.push_back(speakers[i].getAngle());
	}
	driver.update(angles);
}

void ofApp::drawScene(bool showLabels) {
	ofTranslate(0, 0, -eyeLevel);
	ofPushMatrix();
	ofNoFill();
	ofRect(-stageSize / 2, -stageSize / 2, stageSize, stageSize);
	ofTranslate(0, 0, stageHeight);
	ofRect(-stageSize / 2, -stageSize / 2, stageSize, stageSize);
	ofPopMatrix();
	
	for(int i = 0; i < speakers.size(); i++) {
		speakers[i].draw(showLabels);
	}
	
	for(int i = 0; i < listeners.size(); i++) {
		ofCircle(listeners[i], feetToMillimeters(1));
	}
	
	wiresCloud.draw();
	centersCloud.draw();
	wires.draw();
}

void ofApp::drawPlan(float x, float y, float side) {
	ofPushView();
	ofViewport(x, y, side, side);
	ofPushStyle();
	ofFill();
	ofSetColor(0);
	ofRect(0, 0, side, side);
	ofPopStyle();
	ofSetupScreenOrtho(side, side, OF_ORIENTATION_DEFAULT, false, -stageHeight, stageHeight);
	ofTranslate(side / 2, side / 2);
	float scale = side / stageSize;
	ofScale(scale, scale, scale);
	drawScene(false);
	ofPopView();
}

void ofApp::drawSection(float x, float y, float side) {
	ofPushView();
	ofViewport(x, y, side, side);
	ofPushStyle();
	ofFill();
	ofSetColor(0);
	ofRect(0, 0, side, side);
	ofPopStyle();
	ofSetupScreenOrtho(side, side, OF_ORIENTATION_DEFAULT, false, -stageHeight, stageHeight);
	ofTranslate(side / 2, side / 2);
	ofRotateX(-90);
	float scale = side / stageSize;
	ofScale(scale, scale, scale);
	drawScene(false);
	ofPopView();
}

void ofApp::drawPerspective() {
	cam.begin();
	ofScale(.2, .2, .2);
	drawScene(true);
	cam.end();
}

void ofApp::draw() {
	ofBackground(0);
	ofSetColor(255, 128);
	
	drawPerspective();
	drawPlan(ofGetWidth() - 256, 0, 256);
	drawSection(ofGetWidth() - 256, 256, 256);
	
	vector<unsigned char>& packet = driver.getPacket();
	string msg;
	for(int i = 0; i < packet.size(); i++) {
		msg += ofToString(i, 2, '0') + " 0x" + ofToHex(packet[i]) + "\n";
	}
	font.drawString(msg, 10, 10);
}

void ofApp::keyPressed(int key) {
	if(key == 'a') {
		autorun = !autorun;
	}
}