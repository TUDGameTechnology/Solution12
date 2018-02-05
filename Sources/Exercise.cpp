#include <Kore/pch.h>
#include <Kore/IO/FileReader.h>
#include <Kore/Math/Core.h>
#include <Kore/System.h>
#include <Kore/Input/Keyboard.h>
#include <Kore/Input/Mouse.h>
#include <Kore/Audio2/Audio.h>
#include <Kore/Audio1/Audio.h>
#include <Kore/Graphics1/Image.h>
#include <Kore/Graphics4/Graphics.h>
#include <Kore/Graphics4/PipelineState.h>
#include <Kore/Math/Quaternion.h>
#include "ObjLoader.h"

#include <algorithm>
#include <iostream>

using namespace Kore;

class DynamicSound {
public:
	
	Sound sound;
	s16* originalLeft;
	s16* originalRight;

	DynamicSound(const char* filename) : sound(filename) {
		originalLeft = new s16[sound.size];
		originalRight = new s16[sound.size];
		for (int i = 0; i < sound.size; ++i) {
			originalLeft[i] = sound.left[i];
			originalRight[i] = sound.right[i];
		}
	}
	
	void play(vec3 listener, vec3 position) {
		/************************************************************************/
		/* Task 1: Implement the missing code in the function to create positional sounds */
		/************************************************************************/

		// Determine the distance from listener to position

		// Set these values so they reflect the direction to the sound source
		// For directly right, rightVolume = 1.0, leftVolume = 0.0
		// For directly left, leftVolume = 1.0, rightVolume = 0.0
		// For very close to the listener's horizontal position, leftVolume = rightVolume = 0.5
		// Update: The comment originally said to choose left, right = 0.5 at the largest angles,
		// this was not the original intention. If you have already finished the exercised and
		// used the original 0.5, this will also be counted as correct.
		float rightVolume, leftVolume;
		vec3 lisToPos = position - listener;
		float distance = lisToPos.getLength();

		if (distance > 0.0001f) {
			// set higher relative volume where the sound comes from (and proportionally lower on the other side)
			lisToPos.normalize();
			float cos = lisToPos.x();
			rightVolume = (cos + 1) / 2;
			leftVolume = 1 - rightVolume;
		}
		else {
			// if the sound is where the listener is, he hears it evenly from both sides
			rightVolume = 0.5;
			leftVolume = 0.5;
		}


		Audio1::stop(&sound);

		// Modify sound data
		// The arrays contain interleaved stereo data in signed 16 bit integer values
		// Example - only plays on the right channel with half amplitude
		// Modify this code to use the values you computed above
		for (int i = 0; i < sound.size; ++i) {
			sound.left[i] = static_cast<s16>(originalLeft[i] * leftVolume / Kore::exp(distance));
			sound.right[i] = static_cast<s16>(originalRight[i] * rightVolume / Kore::exp(distance));
		}

		Audio1::play(&sound);
	}
};

class MeshObject {
public:
	MeshObject(const char* meshFile, const char* textureFile, const Graphics4::VertexStructure& structure, float scale = 1.0f) {
		mesh = loadObj(meshFile);
		image = new Graphics4::Texture(textureFile, true);
		
		minx = miny = minz = 9999999;
		maxx = maxy = maxz = -9999999;
		
		vertexBuffer = new Graphics4::VertexBuffer(mesh->numVertices, structure, 0);
		float* vertices = vertexBuffer->lock();
		for (int i = 0; i < mesh->numVertices; ++i) {
			vertices[i * 8 + 0] = mesh->vertices[i * 8 + 0] * scale;
			minx = min(vertices[i * 8 + 0], minx);
			maxx = max(vertices[i * 8 + 0], maxx);
			vertices[i * 8 + 1] = mesh->vertices[i * 8 + 1] * scale;
			miny = min(vertices[i * 8 + 1], miny);
			maxy = max(vertices[i * 8 + 1], maxy);
			vertices[i * 8 + 2] = mesh->vertices[i * 8 + 2] * scale;
			minz = min(vertices[i * 8 + 2], minz);
			maxz = max(vertices[i * 8 + 2], maxz);
			vertices[i * 8 + 3] = mesh->vertices[i * 8 + 3];
			vertices[i * 8 + 4] = 1.0f - mesh->vertices[i * 8 + 4];
			vertices[i * 8 + 5] = mesh->vertices[i * 8 + 5];
			vertices[i * 8 + 6] = mesh->vertices[i * 8 + 6];
			vertices[i * 8 + 7] = mesh->vertices[i * 8 + 7];
		}
		vertexBuffer->unlock();
		indexBuffer = new Graphics4::IndexBuffer(mesh->numFaces * 3);
		int* indices = indexBuffer->lock();
		for (int i = 0; i < mesh->numFaces * 3; i++) {
			indices[i] = mesh->indices[i];
		}
		indexBuffer->unlock();
		M = mat4::Identity();
	}
	
	void render(Graphics4::TextureUnit tex) {
		Graphics4::setTexture(tex, image);
		Graphics4::setVertexBuffer(*vertexBuffer);
		Graphics4::setIndexBuffer(*indexBuffer);
		Graphics4::drawIndexedVertices();
	}
	
	virtual void update(float tdif) {
		(void)tdif;	// Do nothing
	}
	
	mat4 M;
protected:
	float minx, miny, minz;
	float maxx, maxy, maxz;
private:
	Graphics4::VertexBuffer* vertexBuffer;
	Graphics4::IndexBuffer* indexBuffer;
	Mesh* mesh;
	Graphics4::Texture* image;
};

class Ball : public MeshObject {
public:
	Ball(float x, float y, float z, const Graphics4::VertexStructure& structure, float scale = 1.0f) : MeshObject("ball.obj", "unshaded.png", structure, scale), x(x), y(y), z(z), dir(0, 0, 0), sound("untitled.wav") {
		rotation = Quaternion(vec3(0, 0, 1), 0);
	}
	
	void update(float tdif) override {
		vec3 dir = this->dir;
		if (dir.getLength() != 0) {
			dir.setLength(dir.getLength() * tdif * 60.0f);
		}
		x += dir.x();
		if (x > 1) {
			x = 1;
		}
		if (x < -1) {
			x = -1;
		}
		y += dir.y();
		if (y < 0) {
			y = 4;
		}
		if (y > 4) {
			y = 0;
		}
		z += dir.z();



		if (dir.getLength() != 0) {
			float Horizontal = dir.dot(vec3(1, 0, 0));
			float Vertical = dir.dot(vec3(0, 1, 0));
			
			rotation = rotation.rotated(Quaternion(vec3(-1, 0, 0), Vertical * 3.0f));
			rotation = rotation.rotated(Quaternion(vec3(0, 1, 0), Horizontal * 3.0f));
		}
		M = mat4::Translation(x, y, z) * rotation.matrix();
	}
	
	void playSound(vec3 listener) {
		sound.play(listener, vec3(x, y, z));
	}
	
	vec3 dir;
	Quaternion rotation;
	float x, y, z;
	DynamicSound sound;
};

namespace {
	void updateBall();
	const int width = 1024;
	const int height = 768;
	double startTime;
	Graphics4::Shader* vertexShader;
	Graphics4::Shader* fragmentShader;
	Graphics4::PipelineState* pipeline;
	MeshObject* objects[] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
	Ball* balls[] = { nullptr, nullptr };
	Graphics4::TextureUnit tex;
	Graphics4::ConstantLocation pvLocation;
	Graphics4::ConstantLocation mLocation;
	mat4 PV;
	float lastTime = 0;
	vec3 position(0, 1.75f, -1.25f);
	float lastSoundTime = 0;
	
	void update() {
		float t = (float)(System::time() - startTime);
		float tdif = t - lastTime;
		lastTime = t;

		updateBall();

		if (t > lastSoundTime) {
			balls[1]->playSound(vec3(balls[0]->x, balls[0]->y, balls[0]->z));
			lastSoundTime += 0.5;
		}

		Graphics4::begin();
		Graphics4::clear(Graphics4::ClearColorFlag | Graphics4::ClearDepthFlag, 0xff9999FF, 1000.0f);
		Graphics4::setPipeline(pipeline);
		PV = mat4::Perspective(90, (float)width / (float)height, 0.1f, 100) * mat4::lookAt(position, vec3(0.0, 1.75f, 0.0), vec3(0, 1, 0));
		Graphics4::setMatrix(pvLocation, PV);

		MeshObject** current = &objects[0];
		while (*current != nullptr) {
			(*current)->update(tdif);
			Graphics4::setMatrix(mLocation, (*current)->M);
			(*current)->render(tex);
			++current;
		}

		Graphics4::end();
		Graphics4::swapBuffers();
	}
	
	bool left = false, right = false, up = false, down = false;
	
	void updateBall() {
		// user controlled ball
		float speed = 0.05f;
		if (left) {
			balls[0]->dir.x() = -speed;
		}
		else if (right) {
			balls[0]->dir.x() = speed;
		}
		else {
			balls[0]->dir.x() = 0;
		}
		if (up) {
			balls[0]->dir.y() = speed;
		}
		else if (down) {
			balls[0]->dir.y() = -speed;
		}
		else {
			balls[0]->dir.y() = 0;
		}
		// NPC ball
		balls[1]->dir.y() = -0.02f;
		if (balls[1]->y == 4) {
			balls[1]->x = ((float)rand() / RAND_MAX)*2-1;
		}
	}
	
	void keyDown(KeyCode code) {
		if (code == KeyLeft) {
			left = true;
		}
		else if (code == KeyRight) {
			right = true;
		}
		else if (code == KeyUp) {
			up = true;
		}
		else if (code == KeyDown) {
			down = true;
		}
	}
	
	void keyUp(KeyCode code) {
		if (code == KeyLeft) {
			left = false;
		}
		else if (code == KeyRight) {
			right = false;
		}
		else if (code == KeyUp) {
			up = false;
		}
		else if (code == KeyDown) {
			down = false;
		}
	}
	
	void mouseMove(int windowId, int x, int y, int movementX, int movementY) {

	}
	
	void mousePress(int windowId, int button, int x, int y) {
	
	}
	
	void mouseRelease(int windowId, int button, int x, int y) {
	
	}
	
	void init() {
		srand(42);

		FileReader vs("shader.vert");
		FileReader fs("shader.frag");
		vertexShader = new Graphics4::Shader(vs.readAll(), vs.size(), Graphics4::VertexShader);
		fragmentShader = new Graphics4::Shader(fs.readAll(), fs.size(), Graphics4::FragmentShader);

		// This defines the structure of your Vertex Buffer
		Graphics4::VertexStructure structure;
		structure.add("pos", Graphics4::Float3VertexData);
		structure.add("tex", Graphics4::Float2VertexData);
		structure.add("nor", Graphics4::Float3VertexData);

		pipeline = new Graphics4::PipelineState;
		pipeline->inputLayout[0] = &structure;
		pipeline->inputLayout[1] = nullptr;
		pipeline->vertexShader = vertexShader;
		pipeline->fragmentShader = fragmentShader;
		pipeline->depthMode = Graphics4::ZCompareLess;
		pipeline->depthWrite = true;
		pipeline->compile();

		tex = pipeline->getTextureUnit("tex");
		pvLocation = pipeline->getConstantLocation("PV");
		mLocation = pipeline->getConstantLocation("M");

		objects[0] = balls[0] = new Ball(0.0f, 1.0f, 0.0f, structure, 3);
		objects[1] = balls[1] = new Ball(((float)rand() / RAND_MAX) * 2 - 1, 4.0f, 0.0f, structure, 3);
		objects[2] = new MeshObject("base.obj", "floor.png", structure);
		objects[2]->M = mat4::RotationX(3.1415f / 2.0f)*mat4::Scale(0.15f, 1, 1);
		objects[3] = new MeshObject("base.obj", "StarMap.png", structure);
		objects[3]->M = mat4::RotationX(3.1415f / 2.0f)*mat4::Scale(1, 1, 1)*mat4::Translation(0, 0, 0.5f);

		Graphics4::setTextureAddressing(tex, Graphics4::U, Graphics4::Repeat);
		Graphics4::setTextureAddressing(tex, Graphics4::V, Graphics4::Repeat);
	}
}

int kore(int argc, char** argv) {
	Kore::System::init("Solution 12", width, height);
	init();
	Kore::System::setCallback(update);

	startTime = System::time();
	Kore::Audio1::init();
	Kore::Audio2::init();

	Keyboard::the()->KeyDown = keyDown;
	Keyboard::the()->KeyUp = keyUp;
	Mouse::the()->Move = mouseMove;
	Mouse::the()->Press = mousePress;
	Mouse::the()->Release = mouseRelease;

	Kore::System::start();

	return 0;
}
