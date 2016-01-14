#include "GaussianBlurFilter.h"


// Exponential function (not implemented in our version of Kore)
float exponential(float x) {
	return Kore::pow(2.71828182845904523536f, x);
}

// Calculate the gaussian with mu = 0 and the specified sigma
float Gaussian(float x, float y, float sigma) {
	/************************************************************************/
	/* Task 1.2: Calculate the value of the Gaussian	                    */
	/************************************************************************/
	float result = 1.0f;
	return result;
}


GaussianBlurFilter::GaussianBlurFilter(int kernelSize, float sigma) {
	this->kernelSize = kernelSize;
	this->sigma = sigma;
}


void GaussianBlurFilter::ApplyKernel(u8* original, u8* pixel, int x, int y, float* kernel, int width, int height) {
	float values[4];
	values[0] = values[1] = values[2] = values[3] = 0.0f;
	
	/************************************************************************/
	/* Task 1.2: Apply the kernel here - Treat the border pixels by extending
	/* them beyond the image borders                                        */
	/************************************************************************/

	// Just copies the image - replace with your own code
	for (int i = 0; i < 3; i++) {
		values[i] = original[y * 4 * width + x * 4 + i];
	}

	// Set the pixel
	for (int i = 0; i < 3; i++) {
		pixel[i] = values[i];
	}
	pixel[3] = 255;
}


Texture* GaussianBlurFilter::Generate(int width, int height) {
	Texture* texture = new Texture(width, height, Kore::Texture::RGBA32, true);
	u8 values[4];
	u8* data = texture->lock();
	
	Texture* input = inputs[0]->Generate(width, height);
	
	u8* inputData = input->lock();

	

	float* kernel = new float[kernelSize*kernelSize];
	int kernelHalfSize = (int) kernelSize / 2;
	float sum = 0.0f;
	for (int x = 0; x < kernelSize; x++) {
		for (int y = 0; y < kernelSize; y++) {
			kernel[x + y * kernelSize] = Gaussian((float) x - kernelHalfSize, (float) y - kernelHalfSize, sigma);
			sum += kernel[x + y * kernelSize];
		}
	}

	for (int x = 0; x < kernelSize; x++) {
		for (int y = 0; y < kernelSize; y++) {
			kernel[x + y * kernelSize] /= sum;
		}
	}



	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			ApplyKernel(inputData, &data[x * 4 + y * 4 * width], x, y, kernel, width, height);
		}
	}


	
	input->unlock();
	texture->unlock();

	return texture;
}

