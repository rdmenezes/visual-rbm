// c
#include <stdint.h>
// stdlib
#include <functional>
// stb_image
#include <stb_image.hxx>


// pixel printing functions
typedef void (*printFunc)(float*);

void printPixel1(float* buff)
{
	printf("%f,", buff[0]);
}
void printPixel2(float* buff)
{
	printf("%f,%f,", buff[0], buff[1]);
}
void printPixel3(float* buff)
{
	printf("%f,%f,%f,", buff[0], buff[1], buff[2]);
}

void printPixel4(float* buff)
{
	printf("%f,%f,%f,%f,", buff[0], buff[1], buff[2], buff[3]);
}

void printHelp()
{
	const char* Usage =
		"Convert an image to an array of values (on [0,1]) for saving to CSV.  Prints\n"
		"values to stdout.\n"
		"\n"
		"Usage: image2csv [INPUT]\n"
		"  INPUT     An input image to convert\n"
		"\n"
		"Supported formats: JPEG, PNG, TGA, PSD, GIF, HDR, PIC\n";
	printf(Usage);
}

int main(int argc, char** argv)
{
	const printFunc printPixel[] = {printPixel1, printPixel2, printPixel3, printPixel4};

	if(argc != 2)
	{
		printHelp();
		return -1;
	}

	const char* filename = argv[1];

	int width, height, components;
	float* buff = stbi_loadf(filename, &width, &height, &components, 0);

	if(!buff)
	{
		printf("Failed to load image due to error: %s\n", stbi_failure_reason());
		return -1;
	}

	uint64_t pixel_count = uint64_t(width) * uint64_t(height);
	
	const int func = components - 1;

	for(uint64_t k = 0; k < pixel_count; k++)
	{
		(*printPixel[func])(buff);
		buff += components;
	}
	printf("\n");

	return 0;
}