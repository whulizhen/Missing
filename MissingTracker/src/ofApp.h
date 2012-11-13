/*
 * add second kinect
 * tune/calibrate
 */

#pragma once

#include "ofMain.h"
#include "ofxKinect.h"
#include "ofxAutoControlPanel.h"
#include "ofxCv.h"
#include "Conversion.h"
#include "ofxOsc.h"
#include "KinectFilter.h"

class KinectTracker {
protected:
	ofxKinect kinect;
	ofImage valid, background;
	KinectFilter filter;
	bool newFrame, clearBackground, calibrating;
	int backgroundThreshold;
	ofVec3f upVector;
	ofVec2f offset;
	float rotation;
	ofMesh mesh;
	vector<float> meshArea;
	
public:
	
	KinectTracker()
	:newFrame(false)
	,clearBackground(false)
	,calibrating(false)
	,backgroundThreshold(0)
	,rotation(0)
	{
	}
	
	void setup() {
		kinect.init(false, false);
		kinect.setRegistration(false);
		kinect.open();
		ofxCv::imitate(background, kinect, CV_8UC1);
		ofxCv::imitate(valid, kinect, CV_8UC1);
	}
	void setClearBackground() {
		clearBackground = true;
	}
	void setCalibrating(bool calibrating) {
		this->calibrating = calibrating;
	}
	void setBackgroundThreshold(int backgroundThreshold) {
		this->backgroundThreshold = backgroundThreshold;
	}
	void setUpVector(ofVec3f upVector) {
		this->upVector = upVector;
	}
	void setOffset(ofVec2f offset) {
		this->offset = offset;
	}
	void setRotation(float rotation) {
		this->rotation = rotation;
	}
	void update() {
		kinect.update();
		if(kinect.isFrameNew()) {
			newFrame = true;
			filter.update(kinect.getDepthPixelsRef());
			unsigned char* kinectPixels = filter.getMasked().getPixels();
			unsigned char* validPixels = valid.getPixels();
			unsigned char* backgroundPixels = background.getPixels();
			int width = kinect.getWidth(), height = kinect.getHeight();
			int n = width * height;
			if(clearBackground) {
				for(int i = 0; i < n; i++) {
					backgroundPixels[i] = 0;
				}
				background.update();
				clearBackground = false;
			}
			if(calibrating) {
				for(int i = 0; i < n; i++) {
					if(kinectPixels[i] != 0) {
						if(backgroundPixels[i] == 0) {
							backgroundPixels[i] = kinectPixels[i];
						}	else {
							backgroundPixels[i] = (backgroundPixels[i] + kinectPixels[i]) / 2;
						}
					}
				}
				background.update();
			}
			for(int i = 0; i < n; i++) {
				int kinectPixel = kinectPixels[i];
				int backgroundPixel = backgroundPixels[i];
				bool far = abs(kinectPixel - backgroundPixel) > backgroundThreshold;
				if(kinectPixel > 0 && (backgroundPixel == 0 || (backgroundPixel > 0 && far))) {
					validPixels[i] = 255;
				} else {
					validPixels[i] = 0;
				}
			}
			valid.update();
			
			ofQuaternion orientationQuat;
			upVector.normalize();
			orientationQuat.makeRotate(ofVec3f(0, 0, 1), upVector);
			ofMatrix4x4 orientationMat;
			orientationQuat.get(orientationMat);
			
			mesh.clear();
			meshArea.clear();
			const unsigned short* rawDepthPixels = kinect.getRawDepthPixels();
			int i = 0;
			for(int y = 0; y < height; y++) {
				for(int x = 0; x < width; x++) {
					if(x + 1 < width &&	validPixels[i] && validPixels[i + 1]) {
						// only need to do one of these, can cache it for the next loop
						ofVec3f cur = kinect.getWorldCoordinateAt(x, y, rawDepthPixels[i]);
						ofVec3f right = kinect.getWorldCoordinateAt(x + 1, y, rawDepthPixels[i + 1]);
						float curArea = cur.distance(right);
						curArea *= curArea;
						cur = orientationMat * cur;
						cur += offset;
						cur.rotate(rotation, ofVec3f(0, 0, 1));
						mesh.addVertex(cur);
						meshArea.push_back(curArea);
					}
					i++;
				}
			}
		}
	}
	ofMesh& getMesh() {
		return mesh;
	}
	vector<float>& getMeshArea() {
		return meshArea;
	}
	bool isFrameNew() {
		bool cur = newFrame;
		newFrame = false;
		return cur;
	}
	ofxKinect& getKinect() {
		return kinect;
	}
	ofImage& getBackground() {
		return background;
	}
	ofImage& getValid() {
		return valid;
	}
};

class ofApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	
	ofxAutoControlPanel gui;
	
	bool calibrating, clearBackground;
	float calibrationStart;
	
	KinectTracker kinect;
	
	ofMesh foregroundCloud, foregroundFlat;
	ofFloatImage presence;	
	ofxCv::ContourFinder contourFinder;
	ofxOscSender osc;
	
	ofEasyCam cam;
};
