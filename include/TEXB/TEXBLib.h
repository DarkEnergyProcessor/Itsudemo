#ifndef _TEXBLIB_H_
#define _TEXBLIB_H_

#include <stdint.h>

#include "TEXB/TEXBVersion.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef LIBTEXB_DURING_COMPILATION
typedef struct TextureBank TEXB_TextureBank;
typedef struct TextureImage TEXB_TextureImage;
#else
typedef struct TEXB_TextureBank TEXB_TextureBank;
typedef struct TEXB_TextureImage TEXB_TextureImage;
#endif

typedef enum
{
	TEXB_FLAG_COMPRESSED = 8,
	TEXB_FLAG_MIPMAP = 16,
	TEXB_FLAG_DOUBLEBUFFERED = 32,
} TEXB_FLAGS;

typedef enum
{
	TEXB_CHANNEL_KIND_ALPHA = 0,
	TEXB_CHANNEL_KIND_LUMINANCE = 1,
	TEXB_CHANNEL_KIND_LUMINANCE_ALPHA = 2,
	TEXB_CHANNEL_KIND_RGB = 3,
	TEXB_CHANNEL_KIND_RGBA = 4,
	TEXB_CHANNEL_KIND_MAX_ENUM
} TEXB_CHANNEL_KIND;
#define TEXB_GET_CHANNEL_KIND(flags) ((TEXB_CHANNEL_KIND) (flags & 7))

typedef enum
{
	TEXB_PIXEL_FORMAT_RGB565 = 0,
	TEXB_PIXEL_FORMAT_RGBA5551 = 1,
	TEXB_PIXEL_FORMAT_RGBA4444 = 2,
	TEXB_PIXEL_FORMAT_BYTE = 3
} TEXB_PIXEL_FORMAT;
#define TEXB_GET_PIXEL_FORMAT(flags) ((TEXB_PIXEL_FORMAT) ((flags >> 6) & 3))

/**
 * @brief Retrieve the libTEXB runtime version. This may be newer than `LIBTEXB_VERSION_NUM`.
 * @return libTEXB version runtime version.
 * @note This is runtime version, `LIBTEXB_VERSION_NUM` is compile-time version.
**/
size_t TEXB_version();

/**
 * @brief Get readable string of the current libTEXB version.
 * @return String of the current libTEXB version.
**/
const char *TEXB_version_string();

/**
 * @brief Get last error message of TEXB_* function calls.
 * 
 * Note that:
 * * Successful TEXB_* calls does NOT change the return value of this function.
 * * The returned pointer may be invalidated on next failure calls on TEXB_* functions.
 * * If you want to store the error message longer, you need to copy it yourself.
 * * Error message is thread-local.
 * @return Reason why last TEXB_* call fails. This function never return NULL.
**/
const char *TEXB_get_last_error();

/**
 * @brief Create new texture bank atlas with the specified width and height.
 * @param width Atlas width.
 * @param height Atlas height.
 * @return New texture bank, or NULL on failure.
**/
TEXB_TextureBank *TEXB_create(uint32_t width, uint32_t height);

/**
 * @brief Open existing texture bank atlas.
 * @param path Path to texb file.
 * @return New texture bank, or NULL on failure.
**/
TEXB_TextureBank *TEXB_open_from_file(const char *path);

/**
 * @brief Open existing texture bank using custom reader callback and userdata.
 * @param handle Userdata
 * @param reader Function with signature similar to fread.
 * @return New texture bank, or NULL on failure.
**/
TEXB_TextureBank *TEXB_open_from_callback(void *handle, size_t(*reader)(void*, size_t, size_t, void*));

/**
 * @brief Open existing texture bank in-memory.
 * @param data Pointer to texture bank file data.
 * @param size Length of the texture bank file data.
 * @return New texture bank, or NULL on failure.
**/
TEXB_TextureBank *TEXB_open_from_memory(const void *data, size_t size);

/**
 * @brief Duplicate current texture bank.
 * @param texb Existing texture bank.
 * @return New texture bank, or NULL on failure.
**/
TEXB_TextureBank *TEXB_clone(TEXB_TextureBank *texb);

/**
 * @brief Free a texture bank and the associated texture image memory.
 * @param t1 Pointer to texture bank. Can be NULL.
 * @param t2 Pointer to pointer to texture bank. Can be NULL. Set the pointer value to NULL.
**/
void TEXB_free(TEXB_TextureBank *t1, TEXB_TextureBank **t2);

/**
 * @brief Get texture bank name as defined in the file.
 * @param texb Existing texture bank.
 * @return Name of the texture bank, or empty string.
 * @note Don't assume the lifetime of the pointer. Copy it if necessary!
**/
const char *TEXB_get_name(TEXB_TextureBank *texb);

/**
 * @brief Set the texture bank name.
 * @param texb Existing texture bank.
 * @param newname New name for this texture bank.
 * @return 1 on success, 0 on failure.
**/
int TEXB_set_name(TEXB_TextureBank *texb, const char *newname);

/**
 * @brief Get texture atlas width.
 * @param texb Existing texture bank.
 * @return Texture atlas width
**/
uint32_t TEXB_get_atlas_width(TEXB_TextureBank *texb);

/**
 * @brief Get texture atlas height.
 * @param texb Existing texture bank.
 * @return Texture atlas height
**/
uint32_t TEXB_get_atlas_height(TEXB_TextureBank *texb);

/**
 * @brief Get texture atlas raw buffer.
 * @param texb Existing texture bank.
 * @return Texture atlas raw buffer.
 * @note `TEXB_flush()` can modify certain areas of the raw buffer. Use with caution!
**/
void *TEXB_get_atlas_contents(TEXB_TextureBank *texb);

/**
 * @brief Get texture atlas flags. The flags also contains channel kind and pixel format.
 * @param texb Existing texture bank.
 * @return Texture bank flags.
**/
uint16_t TEXB_get_flags(TEXB_TextureBank *texb);

/**
 * @brief Get amount of images in the texture atlas.
 * @param texb Existing texture bank.
 * @return Amount of images in the texture atlas.
**/
size_t TEXB_get_image_count(TEXB_TextureBank *texb);

TEXB_TextureImage *TEXB_get_image(TEXB_TextureBank *texb, size_t index, const char *name);
TEXB_TextureImage *TEXB_insert_image(TEXB_TextureBank *texb, TEXB_TextureImage *timg, uint32_t top, uint32_t left, uint32_t bottom, uint32_t right, size_t *index);
TEXB_TextureImage *TEXB_create_image_in_atlas(TEXB_TextureBank *texb, const char *name, uint32_t top, uint32_t left, uint32_t bottom, uint32_t right, uint32_t width, uint32_t height, size_t *index);
TEXB_TextureImage *TEXB_create_new_image(const char *name, TEXB_CHANNEL_KIND channelkind, TEXB_PIXEL_FORMAT pixelformat, uint32_t width, uint32_t height);
const char *TEXB_get_image_name(TEXB_TextureImage *timg);
int TEXB_set_image_name(TEXB_TextureImage *timg, const char *newname);
void *TEXB_get_image_contents(TEXB_TextureImage *timg);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
