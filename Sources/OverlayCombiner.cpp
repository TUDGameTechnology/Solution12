#include "OverlayCombiner.h"

using namespace Kore;

Texture* OverlayCombiner::Generate(int width, int height) {
	Texture* texture = new Texture(width, height, Kore::Texture::RGBA32, true);
	u8* values;
	u8* valuesBottom;
	u8* valuesTop;
	u8* data = texture->lock();


	TextureGenerator* bottomGenerator = inputs[0];
	TextureGenerator* topGenerator = inputs[1];

	Texture* bottom = bottomGenerator->Generate(width, height);
	Texture* top = topGenerator->Generate(width, height);

	u8* dataBottom = bottom->lock();
	u8* dataTop = top->lock();

	

	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			int index = x * 4 + y * 4 * width;
			values = &data[index];
			valuesBottom = &dataBottom[index];
			valuesTop = &dataTop[index];
			for (int i = 0; i < 3; i++) {
				float a = valuesBottom[i] / 256.0f;
				float b = valuesTop[i] / 256.0f;
				float result;
				if (a < 0.5f) {
					result = 2 * a * b;
				}
				else {
					result = 1.0f - 2.0f*(1.0f - a)*(1.0f - b);
				}
				values[i] = result * 256.0f;
			}
			values[3] = valuesBottom[3];
		}
	}


	
	texture->unlock();
	bottom->unlock();
	top->unlock();


	return texture;
}

