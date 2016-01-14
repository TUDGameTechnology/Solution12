#pragma once
#include "TextureGenerator.h"

// Carries out the "overlay" combine mode
class OverlayCombiner :
	public TextureGenerator
{
	
	
public:
	virtual Kore::Texture* Generate(int width, int height);


};

