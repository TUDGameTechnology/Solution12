#include <Kore/pch.h>
#include <Kore/Application.h>
#include <Kore/IO/FileReader.h>
#include <Kore/Math/Core.h>
#include <Kore/System.h>
#include <Kore/Input/Keyboard.h>
#include <Kore/Input/Mouse.h>
#include <Kore/Audio/Mixer.h>
#include <Kore/Graphics/Image.h>
#include <Kore/Graphics/Graphics.h>
#include <Kore/Math/Quaternion.h>
#include "ObjLoader.h"

#include <algorithm>
#include <iostream>

using namespace Kore;

class DynamicSound {
public:
	
	Sound sound;
	u8* original;

	DynamicSound(const char* filename) : sound(filename) {
		original = new u8[sound.size];
		for (int i = 0; i < sound.size; ++i) {
			original[i] = sound.data[i];
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


		Mixer::stop(&sound);

		// Modify sound data
		// The arrays contain interleaved stereo data in signed 16 bit integer values
		// Example - only plays on the right channel with half amplitude
		// Modify this code to use the values you computed above
		s16* source = (s16*)original;
		s16* destination = (s16*)sound.data;
		for (int i = 0; i < sound.size / 2; ++i) {
			if (i % 2 == 0) { // test for left channel
				destination[i] = static_cast<s16>(source[i] * leftVolume / exp(distance));
			}
			else {
				destination[i] = static_cast<s16>(source[i] * rightVolume / exp(distance));
			}
		}

		Mixer::play(&sound);
	}
};

class MeshObject {
public:
	MeshObject(const char* meshFile, const char* textureFile, const VertexStructure& structure, float scale = 1.0f) {
		mesh = loadObj(meshFile);
		image = new Texture(textureFile, true);
		
		minx = miny = minz = 9999999;
		maxx = maxy = maxz = -9999999;
		
		vertexBuffer = new VertexBuffer(mesh->numVertices, structure, 0);
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
		indexBuffer = new IndexBuffer(mesh->numFaces * 3);
		int* indices = indexBuffer->lock();
		for (int i = 0; i < mesh->numFaces * 3; i++) {
			indices[i] = mesh->indices[i];
		}
		indexBuffer->unlock();
		M = mat4::Identity();
	}
	
	void render(TextureUnit tex) {
		Graphics::setTexture(tex, image);
		Graphics::setVertexBuffer(*vertexBuffer);
		Graphics::setIndexBuffer(*indexBuffer);
		Graphics::drawIndexedVertices();
	}
	
	virtual void update(float tdif) {
		(void)tdif;	// Do nothing
	}
	
	mat4 M;
protected:
	float minx, miny, minz;
	float maxx, maxy, maxz;
private:
	VertexBuffer* vertexBuffer;
	IndexBuffer* indexBuffer;
	Mesh* mesh;
	Texture* image;
};

class Ball : public MeshObject {
public:
	Ball(float x, float y, float z, const VertexStructure& structure, float scale = 1.0f) : MeshObject("ball.obj", "unshaded.png", structure, scale), x(x), y(y), z(z), dir(0, 0, 0), sound("untitled.wav") {
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
	Shader* vertexShader;
	Shader* fragmentShader;
	Program* program;
	MeshObject* objects[] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
	Ball* balls[] = { nullptr, nullptr };
	TextureUnit tex;
	ConstantLocation pvLocation;
	ConstantLocation mLocation;
	mat4 PV;
	float lastTime = 0;
	vec3 position(0, 2, -3);
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
		
		Kore::Audio::update();
		Graphics::begin();
		Graphics::clear(Graphics::ClearColorFlag | Graphics::ClearDepthFlag, 0xff9999FF, 1000.0f);
		program->set();
		PV = mat4::Perspective(60, (float)width / (float)height, 0.1f, 100) * mat4::lookAt(position, vec3(position.x(), position.y(), position.z() + 10.0f), vec3(0, 1, 0));
		Graphics::setMatrix(pvLocation, PV);
		
		MeshObject** current = &objects[0];
		while (*current != nullptr) {
			(*current)->update(tdif);
			Graphics::setMatrix(mLocation, (*current)->M);
			(*current)->render(tex);
			++current;
		}
		
		Graphics::end();
		Graphics::swapBuffers();
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
	
	void keyDown(KeyCode code, wchar_t character) {
		if (code == Key_Left) {
			left = true;
		}
		else if (code == Key_Right) {
			right = true;
		}
		else if (code == Key_Up) {
			up = true;
		}
		else if (code == Key_Down) {
			down = true;
		}
	}
	
	void keyUp(KeyCode code, wchar_t character) {
		if (code == Key_Left) {
			left = false;
		}
		else if (code == Key_Right) {
			right = false;
		}
		else if (code == Key_Up) {
			up = false;
		}
		else if (code == Key_Down) {
			down = false;
		}
	}
	
	void mouseMove(int x, int y, int movementX, int movementY) {

	}
	
	void mousePress(int button, int x, int y) {
	
	}
	
	void mouseRelease(int button, int x, int y) {
	
	}
	
	void init() {
		srand(42);
		
		FileReader vs("shader.vert");
		FileReader fs("shader.frag");
		vertexShader = new Shader(vs.readAll(), vs.size(), VertexShader);
		fragmentShader = new Shader(fs.readAll(), fs.size(), FragmentShader);
		
		// This defines the structure of your Vertex Buffer
		VertexStructure structure;
		structure.add("pos", Float3VertexData);
		structure.add("tex", Float2VertexData);
		structure.add("nor", Float3VertexData);
		
		program = new Program;
		program->setVertexShader(vertexShader);
		program->setFragmentShader(fragmentShader);
		program->link(structure);
		
		tex = program->getTextureUnit("tex");
		pvLocation = program->getConstantLocation("PV");
		mLocation = program->getConstantLocation("M");
		
		objects[0] = balls[0] = new Ball(0.0f, 1.0f, 0.0f, structure, 3);
		objects[1] = balls[1] = new Ball(((float)rand() / RAND_MAX)*2-1, 4.0f, 0.0f, structure, 3);
		objects[2] = new MeshObject("base.obj", "floor.png", structure);
		objects[2]->M = mat4::RotationX(3.1415f / 2.0f)*mat4::Scale(0.15f, 1, 1);
		objects[3] = new MeshObject("base.obj", "StarMap.png", structure);
		objects[3]->M = mat4::RotationX(3.1415f / 2.0f)*mat4::Scale(1, 1, 1)*mat4::Translation(0, 0, 0.5f);

		
		Graphics::setRenderState(DepthTest, true);
		Graphics::setRenderState(DepthTestCompare, ZCompareLess);
		Graphics::setTextureAddressing(tex, U, Repeat);
		Graphics::setTextureAddressing(tex, V, Repeat);
	}
}

int kore(int argc, char** argv) {
	Application* app = new Application(argc, argv, width, height, 0, false, "Exercise12");
	init();
	app->setCallback(update);
	
	startTime = System::time();
	Kore::Mixer::init();
	Kore::Audio::init();
	
	Keyboard::the()->KeyDown = keyDown;
	Keyboard::the()->KeyUp = keyUp;
	Mouse::the()->Move = mouseMove;
	Mouse::the()->Press = mousePress;
	Mouse::the()->Release = mouseRelease;
	
	app->start();
	
	delete app;
	return 0;
}
