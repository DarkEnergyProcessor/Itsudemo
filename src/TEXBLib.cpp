#include <string>

#include "TEXB.h"
#include "TEXBPixel.h"
#include "TEXB/TEXBLib.h"

#if defined(_MSC_VER)
#	define TEXB_EXPORT __declspec(dllexport)
#elif defined(__GNUC__) || defined(__clang__)
#	define TEXB_EXPORT __attribute__((visibility("default")))
#else
#	define TEXB_EXPORT
#endif

thread_local std::string lastError = LIBTEXB_VERSION;

extern "C"
{

TEXB_EXPORT size_t TEXB_version()
{
	return LIBTEXB_VERSION_NUM;
}

TEXB_EXPORT const char *TEXB_version_string()
{
	return LIBTEXB_VERSION;
}

TEXB_EXPORT const char *TEXB_get_last_error()
{
	return lastError.c_str();
}

TEXB_EXPORT TEXB_TextureBank *TEXB_create(uint32_t width, uint32_t height)
{
	try
	{
		return new TextureBank(width, height);
	}
	catch (std::exception &e)
	{
		lastError = e.what();
		return nullptr;
	}
	catch (...)
	{
		std::terminate();
	}
}

TEXB_EXPORT TEXB_TextureBank *TEXB_open_from_file(const char *path)
{
	try
	{
		return TextureBank::FromFile(path);
	}
	catch (std::exception &e)
	{
		lastError = e.what();
	}
	catch (int e)
	{
		lastError = strerror(e);
	}
	catch (...)
	{
		std::terminate();
	}

	return nullptr;
}

TEXB_EXPORT TEXB_TextureBank *TEXB_open_from_callback(void *handle, size_t(*reader)(void *, size_t, size_t, void *))
{
	try
	{
		return TextureBank::FromCallback(handle, reader);
	}
	catch (std::exception &e)
	{
		lastError = e.what();
	}
	catch (...)
	{
		std::terminate();
	}

	return nullptr;
}

TEXB_EXPORT TEXB_TextureBank *TEXB_open_from_memory(const void *data, size_t size)
{
	try
	{
		return TextureBank::FromMemory((const uint8_t *) data, size);
	}
	catch (std::exception &e)
	{
		lastError = e.what();
	}
	catch (int e)
	{
		lastError = strerror(e);
	}
	catch (...)
	{
		std::terminate();
	}

	return nullptr;
}

TEXB_EXPORT TEXB_TextureBank *TEXB_clone(TEXB_TextureBank *texb)
{
	try
	{
		return texb->Clone();
	}
	catch (std::exception &e)
	{
		lastError = e.what();
	}
	catch (...)
	{
		std::terminate();
	}

	return nullptr;
}

TEXB_EXPORT void TEXB_free(TEXB_TextureBank *t1, TEXB_TextureBank **t2)
{
	delete t1;

	if (t2)
	{
		delete *t2;
		*t2 = nullptr;
	}
}

TEXB_EXPORT const char *TEXB_get_name(TEXB_TextureBank *texb)
{
	return texb->Name.c_str();
}

TEXB_EXPORT int TEXB_set_name(TEXB_TextureBank *texb, const char *newname)
{
	try
	{
		texb->Name = newname;
		return 1;
	}
	catch (std::exception &e)
	{
		lastError = e.what();
	}

	return 0;
}

TEXB_EXPORT uint32_t TEXB_get_atlas_width(TEXB_TextureBank *texb)
{
	return texb->Width;
}

TEXB_EXPORT uint32_t TEXB_get_atlas_height(TEXB_TextureBank *texb)
{
	return texb->Height;
}

TEXB_EXPORT void *TEXB_get_atlas_contents(TEXB_TextureBank *texb)
{
	return texb->RawImage;
}

TEXB_EXPORT uint16_t TEXB_get_flags(TEXB_TextureBank *texb)
{
	return texb->Flags;
}

TEXB_EXPORT size_t TEXB_get_image_count(TEXB_TextureBank *texb)
{
	return texb->GetImageCount();
}

TEXB_EXPORT TEXB_TextureImage *TEXB_get_image(TEXB_TextureBank *texb, size_t index, const char *name)
{
	try
	{
		if (name)
			return texb->FetchImage(name);
		
		return texb->FetchImage(index);
	}
	catch (std::exception &e)
	{
		lastError = e.what();
	}
	catch (int e)
	{
		lastError = strerror(e);
	}
	catch (...)
	{
		std::terminate();
	}

	return nullptr;
}

TEXB_EXPORT TEXB_TextureImage *TEXB_insert_image(
	TEXB_TextureBank *texb,
	TEXB_TextureImage *timg,
	uint32_t top,
	uint32_t left,
	uint32_t bottom,
	uint32_t right,
	size_t *index
)
{
	try
	{
		size_t idx = texb->DefineImage(timg->Name, top, left, bottom, right, timg->Width, timg->Height);
		TEXB_TextureImage *newTimg = texb->FetchImage(idx);
		memcpy(newTimg->RawImage, timg->RawImage, timg->Width * timg->Height * GetBytePerPixel(timg->ChannelKind, timg->PixelFormat));
		
		if (index)
			*index = idx;

		return newTimg;
	}
	catch (std::exception &e)
	{
		lastError = e.what();
	}
	catch (...)
	{
		std::terminate();
	}

	return nullptr;
}

TEXB_EXPORT TEXB_TextureImage *TEXB_create_image_in_atlas(
	TEXB_TextureBank *texb,
	const char *name,
	uint32_t top,
	uint32_t left,
	uint32_t bottom,
	uint32_t right,
	uint32_t width,
	uint32_t height,
	size_t *index
)
{
	try
	{
		size_t idx = texb->DefineImage(name, top, left, bottom, right, width, height);
		TEXB_TextureImage *newTimg = texb->FetchImage(idx);

		if (index)
			*index = idx;

		return newTimg;
	}
	catch (std::exception &e)
	{
		lastError = e.what();
	}
	catch (...)
	{
		std::terminate();
	}

	return nullptr;
}

TEXB_EXPORT TEXB_TextureImage *TEXB_create_new_image(
	const char *name,
	TEXB_CHANNEL_KIND channelkind,
	TEXB_PIXEL_FORMAT pixelformat,
	uint32_t width,
	uint32_t height
)
{
	try
	{
		return new TEXB_TextureImage(width, height, channelkind, pixelformat);
	}
	catch (std::exception &e)
	{
		lastError = e.what();
	}
	catch (...)
	{
		std::terminate();
	}

	return nullptr;
}

}
