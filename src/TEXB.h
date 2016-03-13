/**
* TEXB.h
* Definition about most common function
**/

#ifndef TEXB_H_
#define TEXB_H_

// In case you want to use malloc and free, just add it to your compiler preprocessor
// #define LIBTEXB_ALLOC(type,size) malloc(sizeof(type)*(size))
// #define LIBTEXB_FREE(ptr) free(ptr)

// Defaults to C++ new and delete
#ifndef LIBTEXB_ALLOC
#define LIBTEXB_ALLOC(type,size) new type[size]
#endif

#ifndef LIBTEXB_FREE
#define LIBTEXB_FREE(ptr) delete[] ptr
#endif

#ifdef LIBTEXB_NO_EXCEPTION
#define LIBTEXB_FAILEXIT return NULL
#define LIBTEXB_FAILWITH(errcode) {errno=errcode;return NULL;}
#else
#define LIBTEXB_FAILEXIT throw int(errno)
#define LIBTEXB_FAILWITH(errcode) throw errcode
#endif

#include <string>
#include <vector>
#include <map>

#include <cstdlib>

// To prevent GCC 4.8 or below complaining about C++11 features must be enabled, use stdint.h instead of cstdint
#include <stdint.h>

struct Point
{
	uint32_t X;
	uint32_t Y;
};

struct UVPoint
{
	double U;
	double V;
};

// Class of Image in TEXB
struct TextureImage;

// Class of Playground texture bank
struct TextureBank
{
protected:
	uint32_t RawImageWidth;
	uint32_t RawImageHeight;
	uint16_t _Flags;
	std::vector<TextureImage*> ImageList_Id;
	std::map<std::string,uint32_t> ImageList_Names;
	std::vector<uint32_t*> VertexIndexUVs;
	
	TextureBank():Width(RawImageWidth),Height(RawImageHeight),Flags(_Flags) {}
public:
	~TextureBank();
	// Texture bank name defined in file.
	std::string Name;
	// Texture bank width
	const uint32_t& Width;
	// Texture bank height
	const uint32_t& Height;
	// Texture bank raw image
	uint8_t* RawImage;
	// Texture bank flags when decoded with FromFile or FromMemory
	const uint16_t& Flags;
	// Creates TextureBank from specificed width and height.
	// It's recommended that the width and the height is power of 2, but width and height doesn't need to be equal
	// Example: 1024x512 is acceptable width and height
	TextureBank(uint32_t width,uint32_t height);
	// Loads TEXB from file
	// Throws int or returns NULL on error
	static TextureBank* FromFile(std::string filename);
	// Loads TEXB from memory
	// Throws int or returns NULL on error
	static TextureBank* FromMemory(uint8_t* memory,size_t memsize);
	// Clone this TextureBank to a new instance.
	TextureBank* Clone();
	
	// List all images name in TEXB as std::vector
	// Images name doesn't contain .png.imag suffix
	std::vector<std::string> ListImages();
	// Returns the raw image data from image name in TextureBank in form of RGBA
	// Returns NULL or throws exception on failure
	TextureImage* FetchImage(std::string image_name);
	// Returns the raw image data from index in TextureBank in form of RGBA
	// Returns NULL or throws exception on failure
	TextureImage* FetchImage(uint32_t index);
	// Returns all TextureImage
	std::vector<TextureImage*> FetchAll();
	// Returns raw image of this texture bank
	std::vector<uint8_t> FetchRaw();
	// Same as FetchImage(std::string)
	inline TextureImage* operator[](std::string image_name)
	{
		return FetchImage(image_name);
	}
	// Same as FetchImage(uint32_t)
	inline TextureImage* operator[](uint32_t index)
	{
		return FetchImage(index);
	}
	// Same as FetchImage(std::string) except that it accepts const char*
	inline TextureImage* operator[](const char* image_name)
	{
		return FetchImage(std::string(image_name));
	}
	
	// Replaces image inside texture bank with the specificed TextureImage
	// Returns 0 on success
	int32_t ReplaceImage(TextureImage* image);
	// Replaces image with specificed index inside texture bank with the specificed TextureImage
	// Returns 0 on success
	int32_t ReplaceImage(TextureImage* image,uint32_t index);
	// Define new texture image location
	// Returns 0 and the texture image index on success
	int32_t DefineImage(const Point* texture_vertexes,const UVPoint* texture_uvs,std::string name,uint32_t* index);
	// Defines new texture image location with their UVs automatically generated.
	// Returns 0 and the texture image index on success
	int32_t DefineImage(const Point* texture_where_width_height,std::string name,uint32_t* index);
	// Saves current TextureBank to file
	// Returns 0 on success
	int32_t SaveToFile(std::string filename,uint32_t compression_level=9);
	// Saves current TextureBank to memory. The memory is allocated by the function
	// memory_size can't be NULL
	// Returns 0 on success
	int32_t SaveToMemory(uint8_t*& memory,size_t* memory_size,uint32_t compression_level=9);
	int32_t SaveToMemory(uint8_t** memory,size_t* memory_size,uint32_t compression_level=9);
	// Reflect all changes made in the TextureImage by writing to TEXB raw buffer
	// Warning: can be slow if called repeatedly
	void ReflectChanges();
	
	friend class TextureImage;
};

struct TextureImage
{
protected:
	TextureBank* from_texb;
	// Pre-calculated when constructed.
	//uint8_t* Indexes;	// Most likely to be 6 in size
	//uint32_t* Vertexes;	// Most likely to be 4 in size
	TextureImage() {}
public:
	~TextureImage();
	// Creates TextureImage with specificed Width, Height, and RawImage data.
	// raw image is copied so it's safe to free the raw image memory specificed in raw_image afterwards.
	// if raw image is null, the raw image filled with white.
	TextureImage(uint32_t width,uint32_t height,uint8_t* raw_image=NULL);
	// Copy this TextureImage to a new instance
	TextureImage* Clone();
	// Image width
	uint32_t Width;
	// Image height
	uint32_t Height;
	// Image pixel data in form of RGBA. 4-bytes/pixel
	uint8_t* RawImage;
	// Name of this image without .png.imag
	std::string Name;

	friend class TextureBank;
};

typedef TextureBank TEXB;
typedef TextureImage TIMG;

#endif
