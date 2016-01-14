#pragma once
#include "TextureGenerator.h"


/************************************************************************/
/* Task 1.3 - Implement your own texture generator
/************************************************************************/
class YourGenerator :
	public TextureGenerator
{
	

public:
	virtual Kore::Texture* Generate(int width, int height);

	YourGenerator();
};

