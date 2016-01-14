#pragma once
#include "TextureGenerator.h"

// Simple combination of the two images. inputs[1] is drawn over inputs[0]
class CombineGenerator :
	public TextureGenerator
{

	Kore::Texture* texture1;
	Kore::Texture* texture2;


public:

	virtual Kore::Texture* Generate(int width, int height);

	CombineGenerator();
	~CombineGenerator(void);
};

