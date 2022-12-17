#include "ofApp.h"	

//--------------------------------------------------------------
Actor::Actor(vector<glm::vec3>& location_list, vector<vector<int>>& next_index_list, vector<int>& destination_list) {

	this->select_index = ofRandom(location_list.size());
	while (true) {

		auto itr = find(destination_list.begin(), destination_list.end(), this->select_index);
		if (itr == destination_list.end()) {

			destination_list.push_back(this->select_index);
			break;
		}

		this->select_index = (this->select_index + 1) % location_list.size();
	}

	this->next_index = this->select_index;
}

//--------------------------------------------------------------
void Actor::update(const int& frame_span, vector<glm::vec3>& location_list, vector<vector<int>>& next_index_list, vector<int>& destination_list) {

	if (ofGetFrameNum() % frame_span == 0) {

		auto tmp_index = this->select_index;
		this->select_index = this->next_index;
		int retry = next_index_list[this->select_index].size();
		this->next_index = next_index_list[this->select_index][(int)ofRandom(next_index_list[this->select_index].size())];
		while (--retry > 0) {

			auto destination_itr = find(destination_list.begin(), destination_list.end(), this->next_index);
			if (destination_itr == destination_list.end()) {

				if (tmp_index != this->next_index) {

					destination_list.push_back(this->next_index);
					break;
				}
			}

			this->next_index = next_index_list[this->select_index][(this->next_index + 1) % next_index_list[this->select_index].size()];
		}
		if (retry <= 0) {

			destination_list.push_back(this->select_index);
			this->next_index = this->select_index;
		}
	}

	auto param = ofGetFrameNum() % frame_span;
	auto distance = location_list[this->next_index] - location_list[this->select_index];
	this->location = location_list[this->select_index] + distance / frame_span * param;

	this->log.push_front(this->location);
	while (this->log.size() > 50) { this->log.pop_back(); }
}

//--------------------------------------------------------------
glm::vec3 Actor::getLocation() {

	return this->location;
}

//--------------------------------------------------------------
deque<glm::vec3> Actor::getLog() {

	return this->log;
}

//--------------------------------------------------------------
void ofApp::setup() {

	ofSetFrameRate(30);
	ofSetWindowTitle("openFrameworks");

	ofBackground(255);
	ofSetRectMode(ofRectMode::OF_RECTMODE_CENTER);
	ofSetCircleResolution(60);

	this->font.loadFont("fonts/Kazesawa-Bold.ttf", 400, true, true, true);
}

//--------------------------------------------------------------
void ofApp::update() {

	if (ofGetFrameNum() % 60 == 0) {

		this->location_list.clear();
		this->next_index_list.clear();
		this->destination_list.clear();
		this->actor_list.clear();

		ofFbo fbo;
		fbo.allocate(ofGetWidth(), ofGetHeight());
		fbo.begin();
		ofTranslate(ofGetWidth() * 0.5, ofGetHeight() * 0.5);
		ofClear(0);
		ofSetColor(0);

		string word = std::to_string(ofGetFrameNum() / 60);
		font.drawString(word, font.stringWidth(word) * -0.5, font.stringHeight(word) - 320);

		fbo.end();

		auto span = 10;
		ofPixels pixels;
		fbo.readToPixels(pixels);
		for (int x = 60; x < 700; x += span) {

			for (int y = 0; y < 720; y += span) {

				ofColor color = pixels.getColor(x, y);
				if (color != ofColor(0, 0)) {

					this->location_list.push_back(glm::vec3(x - ofGetWidth() * 0.5, ofGetHeight() - y - ofGetHeight() * 0.65, 0));
				}
			}
		}

		for (auto& location : this->location_list) {

			vector<int> next_index = vector<int>();
			int index = -1;
			for (auto& other : this->location_list) {

				index++;
				if (location == other) { continue; }

				float distance = glm::distance(location, other);
				if (distance <= span) {

					next_index.push_back(index);
				}
			}

			this->next_index_list.push_back(next_index);
		}

		for (int i = 0; i < 300; i++) {

			this->actor_list.push_back(make_unique<Actor>(this->location_list, this->next_index_list, this->destination_list));
		}
	}


	int frame_span = 10;
	int prev_index_size = 0;

	if (ofGetFrameNum() % frame_span == 0) {

		prev_index_size = this->destination_list.size();
	}

	for (auto& actor : this->actor_list) {

		actor->update(frame_span, this->location_list, this->next_index_list, this->destination_list);
	}

	if (prev_index_size != 0) {

		this->destination_list.erase(this->destination_list.begin(), this->destination_list.begin() + prev_index_size);
	}
}

//--------------------------------------------------------------
void ofApp::draw() {

	this->cam.begin();
	ofTranslate(-15, 0);

	ofSetColor(0);
	for (auto& actor : this->actor_list) {

		for (auto& l : actor->getLog()) {

			ofDrawRectangle(l, 6, 6);
		}
	}

	ofSetColor(255);
	for (auto& actor : this->actor_list) {

		for (auto& l : actor->getLog()) {

			ofDrawRectangle(l, 3, 3);
		}
	}

	this->cam.end();
}


//--------------------------------------------------------------
int main() {

	ofSetupOpenGL(720, 720, OF_WINDOW);
	ofRunApp(new ofApp());
}