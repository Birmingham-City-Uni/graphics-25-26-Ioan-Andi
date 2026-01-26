#include <iostream>
#include <lodepng.h>

int Yinput1 = 0;
int Yinput2 = 0;
int Xinput1 = 0;
int Xinput2 = 0;

int Rvalue = 0;
int Gvalue = 0;
int Bvalue = 0;
int Avalue = 0;

//void SetPixles() {


//	/*std::cout << "Please enter the starting range for Y \n";
//	std::cin >> Yinput1;
//	std::cout << "Y starting range is " << Yinput1 << "\n";
//
//	std::cout << "Please enter the end range for Y \n";
//	std::cin >> Yinput2;
//	std::cout << "Y end range is " << Yinput2 << "\n";
//
//	std::cout << "Please enter the starting range for X \n";
//	std::cin >> Xinput1;
//	std::cout << "X starting range is " << Xinput1 << "\n";
//
//	std::cout << "Please enter the end range for X \n";
//	std::cin >> Xinput2;
//	std::cout << "X end range is " << Xinput2 <<"\n";
//
//	std::cout << "Please enter a value for Red (0-255) \n";
//	std::cin >> Rvalue;
//	std::cout << "Red value is " << Rvalue << "\n";
//
//	std::cout << "Please enter a value for Green (0-255) \n";
//	std::cin >> Gvalue;
//	std::cout << "Green value is " << Gvalue << "\n";
//
//	std::cout << "Please enter a value for Blue (0-255) \n";
//	std::cin >> Bvalue;
//	std::cout << "Blue value is " << Bvalue << "\n";
//
//	std::cout << "Please enter a value for Opacity (0-255) \n";
//	std::cin >> Avalue;
//	std::cout << "Opacity value is " << Avalue << "\n";
//}*/


void SetPixel(std::vector<uint8_t>& buffer, int x, int y, int r, int g, int b, int a) {

	const int nChannels = 4;

	int pixelIdx = x + y;
	int base = pixelIdx * nChannels;

	buffer[base + 0] = r;
	buffer[base + 1] = g;
	buffer[base + 2] = b;
	buffer[base + 3] = a;

	

}

int main()
{
	std::string outputFilename = "output.png";

	//SetPixles();

	const int width = 1920, height = 1080;
	const int nChannels = 4;

	// Setting up an image buffer
	// This std::vector has one 8-bit value for each pixel in each row and column of the image, and
	// for each of the 4 channels (red, green, blue and alpha).
	// Remember 8-bit unsigned values can range from 0 to 255.
	std::vector<uint8_t> imageBuffer(height*width*nChannels);

	// This for loop sets all the pixels of the image to a cyan colour. 
	for (int y = 0; y < height; ++y)
		for (int x = 0; x < width; ++x) {
			int pixelIdx = x + y * width;
			imageBuffer[pixelIdx * nChannels + 0] = 0; // Set red pixel values to 0
			imageBuffer[pixelIdx * nChannels + 1] = 255; // Set green pixel values to 255 (full brightness)
			imageBuffer[pixelIdx * nChannels + 2] = 255; // Set blue pixel values to 255 (full brightness)
			imageBuffer[pixelIdx * nChannels + 3] = 255; // Set alpha (transparency) pixel values to 255 (fully opaque)
		}

	SetPixel(imageBuffer, 5, 5, 255, 0, 0, 255);

	for (int y = 0; y < height; ++y) 
		for (int x = 0; x < width; ++x) {
			int pixelIdx = x + y * width;
			imageBuffer[pixelIdx * nChannels + 0] = 0; // Set red pixel values to 0
			imageBuffer[pixelIdx * nChannels + 1] = 255; // Set green pixel values to 255 (full brightness)
			imageBuffer[pixelIdx * nChannels + 2] = 255; // Set blue pixel values to 255 (full brightness)
			imageBuffer[pixelIdx * nChannels + 3] = 255; // Set alpha (transparency) pixel values to 255 (fully opaque)
		}

	/// *** Lab Tasks ***
	// * Task 1: Try adapting the code above to set the lower half of the image to be a green colour.
	// * Task 2: Doing the maths above to work out indices is a bit annoying! Write your own setPixel function.
	//           This should take x and y coordinates as input, and red, green, blue and alpha values.
	//           Remember to pass in your imageBuffer. Should it be passed in by reference or by value? Should
	//           the reference be const?
	//           We will use this setPixel function to build our rasteriser in the upcoming labs.
	//			 Test your setPixel function by setting pixels in your image to different colours.
	// * Optional Task 3: Use your setPixel function to draw a circle in the centre of the image. Remember a point is
	//           in a circle if sqrt((x - x_0)^2 + (y - y_0)^2) < radius (here x_0, y_0 are the coordinates at the middle of 
	//           the circle). 
	//           Hint - use a similar for loop to the one above, and add an if statement to check if the current
	//           pixel lies in the circle.
	//           Try modifying the order you draw each component in. If you draw the circle before setting the lower 
	//           part of the image to be green, how does this modify the image?
	// * Optional Task 4: Work out how good the compression ratio of the saved PNG image is. PNG images
	//           use *lossless* compression, where all the pixel values of the original image are preserved.
	//           To work out the compression ratio, compare the size of the saved image to the memory
	//           occupied by the image buffer (this is based on the width, height and number of channels).
	//           Try setting the pixels to random values (use rand() and the % operator). What is the 
	//           compression ratio now, and why do you think this is?


	// *** Encoding image data ***
	// PNG files are compressed to save storage space. 
	// The lodepng::encode function applies this compression to the image buffer and saves the result 
	// to the filename given.
	int errorCode;
	errorCode = lodepng::encode(outputFilename, imageBuffer, width, height);
	if (errorCode) { // check the error code, in case an error occurred.
		std::cout << "lodepng error encoding image: " << lodepng_error_text(errorCode) << std::endl;
		return errorCode;
	}

	return 0;
}
