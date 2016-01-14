#include "YourGenerator.h"
#include "Kore/Math/Random.h"


using namespace Kore;


Kore::Texture* YourGenerator::Generate(int width, int height)
{
	Texture* texture = new Texture(width, height, Kore::Texture::RGBA32, true);
	u8* data = texture->lock();


	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {

			// Writes blue to all pixels - replace with your own code
			u8* values = &data[y * width * 4 + x * 4];
			values[0] = 255;
			values[1] = 0;
			values[2] = 0;
			values[3] = 255;
		}
	}



	texture->unlock();


	return texture;
}

YourGenerator::YourGenerator()
{

}
