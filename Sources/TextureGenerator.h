#pragma once

#include "pch.h"
#include <Kore/Graphics/Graphics.h>


using namespace Kore;

// Base class for texture generators
class TextureGenerator
{
protected: 

	// Null-terminated list of input generators
	TextureGenerator** inputs;

public:
	
	// Add an input
	void AddInput(TextureGenerator* input);

	// Clear the inputs
	void ClearInputs();


	virtual Kore::Texture* Generate(int width, int height) = 0;

	TextureGenerator(void);
	virtual ~TextureGenerator(void);
};

// Just copies the input image (for now, the dimensions have to match the requested texture size)
class ImageGenerator: public TextureGenerator {
	Kore::Texture* input;

public:
	
	
	virtual Kore::Texture* Generate(int width, int height) {
		return input;
	}

	ImageGenerator(Kore::Texture* input) {
		this->input = input;
	}
	
	virtual ~ImageGenerator(void) {

	}

};