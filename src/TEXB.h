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
// Deprecated
#define LIBTEXB_ALLOC(type,size) new type[size]
#endif

#ifndef LIBTEXB_FREE
// Deprecated
#define LIBTEXB_FREE(ptr) delete[] ptr
#endif

// Deprecated
#ifdef LIBTEXB_NO_EXCEPTION
#define LIBTEXB_FAILEXIT return NULL
#define LIBTEXB_FAILWITH(errcode) {errno=errcode;return NULL;}
#else
#define LIBTEXB_FAILEXIT throw int(errno)
#define LIBTEXB_FAILWITH(errcode) throw errcode
#endif

#include <array>
#include <functional>
#include <string>
#include <vector>
#include <map>

#include <cstdlib>
#include <cstdint>

#include "TEXB/TEXBLib.h"

#if __cplusplus >= 201402L
#define TEXB_DEPRECATED [[deprecated]]
#elif defined(_MSC_VER)
#define TEXB_DEPRECATED __declspec(deprecated)
#elif defined(__GNUC__) || defined(__clang__)
#define TEXB_DEPRECATED __attribute__((deprecated))
#else
#define TEXB_DEPRECATED
#endif

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

struct VertexFormat
{
	uint32_t X, Y, U, V;
};

#pragma pack(push, 1)
struct FixedVertexIndexFormat
{
	std::array<VertexFormat, 4> Vertices;
	std::array<uint8_t, 6> Indices;
};
#pragma pack(pop)

static_assert(sizeof(FixedVertexIndexFormat) == 70, "invalid fixed vertex index format size");

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
	std::map<std::string,size_t> ImageList_Names;
	std::vector<FixedVertexIndexFormat*> VertexIndexUVs;
	
	TextureBank():Width(RawImageWidth),Height(RawImageHeight),Flags(_Flags) {}
public:
	~TextureBank();
	// Texture bank name defined in file.
	std::string Name;
	// Texture bank width
	const uint32_t& Width;
	// Texture bank height
	const uint32_t& Height;
	// Texture bank raw image in RGBA
	uint8_t* RawImage;
	// Texture bank flags when decoded with FromFile or FromMemory
	const uint16_t& Flags;
	// Creates TextureBank from specificed width and height.
	// It's recommended that the width and the height is power of 2, but width and height doesn't need to be equal
	// Example: 1024x512 is acceptable width and height
	TextureBank(uint32_t width,uint32_t height);
	// Loads TEXB from file
	// Throws int or returns NULL on error
	static TextureBank* FromFile(const std::string &filename);
	// Loads TEXB from memory
	// Throws int or returns NULL on error
	static TextureBank* FromMemory(const uint8_t* memory,size_t memsize);
	// Loads TEXB from callback
	// Throws std::exception on error
	static TextureBank* FromCallback(void *handle, size_t(*reader)(void*, size_t, size_t, void*));
	// Clone this TextureBank to a new instance.
	TextureBank* Clone();
	
	size_t GetImageCount() const noexcept;
	// List all images name in TEXB as std::vector
	// Images name doesn't contain .png.imag suffix
	std::vector<std::string> ListImages();
	// Returns the raw image data from image name in TextureBank in form of RGBA
	// Returns NULL or throws exception on failure
	TextureImage* FetchImage(const std::string &image_name);
	// Returns the raw image data from index in TextureBank in form of RGBA
	// Returns NULL or throws exception on failure
	TextureImage* FetchImage(size_t index);
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
	TEXB_DEPRECATED
	int32_t ReplaceImage(TextureImage* image);
	// Replaces image with specificed index inside texture bank with the specificed TextureImage
	// Returns 0 on success
	TEXB_DEPRECATED
	int32_t ReplaceImage(TextureImage* image,uint32_t index);
	// Replaces image with specified index inside texture bank with the speificed TextureImage
	// Throws exception on failure
	// dummy can be anything
	void ReplaceImage(TextureImage *image, size_t index, bool dummy);
	// Define new texture image location
	// Returns 0 and the texture image index on success
	TEXB_DEPRECATED
	int32_t DefineImage(const Point* texture_vertexes,const UVPoint* texture_uvs,std::string name,uint32_t* index);
	// Defines new texture image location with their UVs automatically generated.
	// Returns 0 and the texture image index on success
	TEXB_DEPRECATED
	int32_t DefineImage(const Point* texture_where_width_height,std::string name,uint32_t* index);
	// Define new texture image location
	// Returns the texture image index on success
	size_t DefineImage(const std::string &name, uint32_t top, uint32_t left, uint32_t bottom, uint32_t right, uint32_t width, uint32_t height);
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
	// raw image is assumed to have format of RGBA.
	// raw image is copied so it's safe to free the raw image memory specificed in raw_image afterwards.
	// if raw image is null, the raw image filled with white.
	TextureImage(uint32_t width, uint32_t height, uint8_t* raw_image = nullptr);
	// Creates TextureImage with specified Width, Height, Channel Kind, Pixel Format, and optionally Pixel Data.
	// Pixel Data has format of channelkind and pixfmt.
	// Pixel Data is copied so it's safe to free the Pixel Data afterwards.
	// If Pixel Data is null, the image is filled with white.
	TextureImage(uint32_t width, uint32_t height, TEXB_CHANNEL_KIND channelkind, TEXB_PIXEL_FORMAT pixfmt, uint8_t *pixels = nullptr);
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
	// Channel kind
	TEXB_CHANNEL_KIND ChannelKind;
	// Pixel format
	TEXB_PIXEL_FORMAT PixelFormat;

	friend class TextureBank;
};

typedef TextureBank TEXB;
typedef TextureImage TIMG;

#endif
