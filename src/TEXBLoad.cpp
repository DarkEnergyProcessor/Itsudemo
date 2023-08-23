/**
* TEXBLoad.cpp
* Load and parse Texture Bank file
**/

#include "TEXB.h"
#include "TEXBPixel.h"
#include "xy2uv.h"

#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <memory>
#include <stdexcept>

#include <cerrno>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <cstdlib>

#include <stdint.h>

#include "zlib.h"

struct MemoryReader
{
	const void *buffer;
	size_t size, pos;

	static size_t read(void *dest, size_t elemSize, size_t elemCount, void *mreader)
	{
		MemoryReader *reader = (MemoryReader *) mreader;
		size_t elemRead = std::min(elemCount, (reader->size - reader->pos) / elemSize);

		memcpy(dest, reader->buffer, elemSize * elemRead);
		reader->pos += elemRead * elemSize;
		return elemRead;
	}
};

TextureBank* TextureBank::FromMemory(const uint8_t* _mem,size_t _n)
{
	MemoryReader reader = {_mem, _n, 0};
	return FromCallback(&reader, MemoryReader::read);
}

TextureBank* TextureBank::FromFile(const std::string &Filename)
{
	TextureBank* texb = nullptr;
	FILE* file = fopen(Filename.c_str(), "rb");

	if (file == nullptr)
		throw int(errno);

	try
	{
		texb = FromCallback(file, (size_t(*)(void*, size_t, size_t, void*)) fread);
	}
	catch (std::bad_alloc &)
	{
		throw int(ENOMEM);
	}
	catch (std::exception &)
	{
		throw int(EBADF);
	}

	fclose(file);
	return texb;
}

TextureBank *TextureBank::FromCallback(void *handle, size_t(*reader)(void*, size_t, size_t, void*))
{
	std::unique_ptr<TextureBank> texb = nullptr;
	std::vector<uint8_t> scratchpadVector(40);
	uint8_t *scratchpad = scratchpadVector.data();

	auto readData = [&scratchpadVector, &scratchpad, handle, reader](size_t size, void *dest = nullptr)
	{
		if (dest == nullptr)
		{
			if (size > scratchpadVector.size())
			{
				scratchpadVector.resize(size);
				scratchpad = scratchpadVector.data();
			}

			dest = scratchpad;
		}

		if (reader(dest, 1, size, handle) < 4)
			throw std::runtime_error("read error");
	};

	// Find header
	readData(4);
	if(memcmp(scratchpad, "TEXB", 4) != 0)
		throw std::runtime_error("not TEXB");

	readData(4);
	ptrdiff_t texbSize = (scratchpad[0] << 24) | (scratchpad[1] << 16) | (scratchpad[2] << 8) | scratchpad[3];
	texb.reset(new TextureBank);
	
	// TEXB Name without T prefix and extension
	{
		readData(2); texbSize -= 2;
		size_t texbNameLen = (scratchpad[0] << 8U) | scratchpad[1];
		readData(texbNameLen); texbSize -= texbNameLen;
		std::string texbName((char *) scratchpad);
		texb->Name = texbName.substr(1, texbName.find(".texb") - 1);	// Discard .texb
	}
	
	// Width & Height
	uint16_t tWidth = 0, tHeight = 0;
	readData(4); texbSize -= 4;
	texb->RawImageWidth = (scratchpad[0] << 8) | scratchpad[1];
	texb->RawImageHeight = (scratchpad[2] << 8) | scratchpad[3];

	// Flags
	readData(2); texbSize -= 2;
	uint16_t TexbFlags = (scratchpad[0] << 8) | scratchpad[1];
	texb->_Flags = TexbFlags;

	// Vertex & Index count
	readData(6); texbSize -= 6;
	/*
	uint16_t VertexCount = (scratchpad[0] << 8) | scratchpad[1];
	uint16_t IndexCount = (scratchpad[2] << 8) | scratchpad[3];
	*/
	uint16_t ImageCount = (scratchpad[4] << 8) | scratchpad[5];

	for(size_t i = 0; i < ImageCount; i++)
	{
		readData(6); texbSize -= 6;
		ptrdiff_t imageDefLen = (scratchpad[4] << 8U) | scratchpad[5];

		if (memcmp(scratchpad, "TIMG", 4) == 0)
		{
			std::unique_ptr<TextureImage> timg(new TextureImage);
			timg->from_texb = texb.get();

			// Texture Image name
			readData(2); imageDefLen -= 2; texbSize -= 2;
			size_t timgNameLen = (scratchpad[0] << 8) | scratchpad[1];
			readData(timgNameLen); imageDefLen -= timgNameLen;
			std::string timgName((char*) scratchpad);
			timg->Name = timgName.substr(1, timgName.find(".png.imag") - 1);	// Discard .png.imag

			// Subimages
			readData(2); imageDefLen -= 2; texbSize -= 2;
			uint16_t Subimgs = (scratchpad[0] << 8) | scratchpad[1];
			bool cancel = false;

			if(Subimgs == 65535)
			{
				readData(2); imageDefLen -= 2; texbSize -= 2;
				size_t attributes = (scratchpad[0] << 8) | scratchpad[1];

				for(size_t j = 0; j < attributes; j++)
				{
					// Based from extb.c, it just skips the attribute.
					uint8_t keyAndType[2];
					readData(2, keyAndType); imageDefLen -= 2; texbSize -= 2;

					switch(keyAndType[1])
						{
						case 0:
						case 1:
						{
							readData(4); imageDefLen -= 4; texbSize -= 4;
							break;
						}
						case 2:
						{
							readData(2); imageDefLen -= 2; texbSize -= 2;
							size_t skipLen = (scratchpad[0] << 8) | scratchpad[1];
							readData(skipLen); imageDefLen -= skipLen; texbSize -= skipLen;
							break;
						}
						default:
						{
							// Based from extb.c, this is unknown and the program just throws assert failed. Delete all memory and exit
							// But let's ignore this whole section instead.
							cancel = true;
							break;
						}
					}

					if (cancel)
						break;
				}
			}

			if (!cancel)
			{
				if (Subimgs == 65535)
				{
					readData(2); imageDefLen -= 2; texbSize -= 2;
					Subimgs = (scratchpad[0] << 8) | scratchpad[1];
				}

				if(Subimgs > 1)
				{
					// Currently not supported. It breaks the TextureImage structure
					cancel = true;
				}
			}

			uint8_t verts = 0;
			uint8_t idxs = 0;

			if (!cancel)
			{
				readData(6); imageDefLen -= 6; texbSize -= 6;
				uint8_t verts = scratchpad[0];
				uint8_t idxs = scratchpad[1];
				timg->Width = (scratchpad[2] << 8) | scratchpad[3];
				timg->Height = (scratchpad[4] << 8) | scratchpad[5];

				if (verts != 4 || idxs != 6)
				{
					// TODO non-rectangular
					cancel = true;
				}
			}

			std::unique_ptr<FixedVertexIndexFormat> vibuf(nullptr);

			if (!cancel)
			{
				// CenterX, CenterY
				readData(4); imageDefLen -= 4; texbSize -= 4;

				// Vertices
				uint32_t vsize = verts * sizeof(uint32_t) * 4;
				vibuf.reset(new FixedVertexIndexFormat);

				for(size_t l = 0; l < 4; l++)
				{
					readData(16); imageDefLen -= 16; texbSize -= 16;
					uint32_t x = (scratchpad[0] << 24) | (scratchpad[1] << 16) | (scratchpad[2] << 8) | scratchpad[3];
					uint32_t y = (scratchpad[4] << 24) | (scratchpad[5] << 16) | (scratchpad[6] << 8) | scratchpad[7];
					uint32_t u = (scratchpad[8] << 24) | (scratchpad[9] << 16) | (scratchpad[10] << 8) | scratchpad[11];
					uint32_t v = (scratchpad[12] << 24) | (scratchpad[13] << 16) | (scratchpad[14] << 8) | scratchpad[15];
					vibuf->Vertices[l] = {x, y, u, v};
				}

				// Indices
				readData(6); imageDefLen -= 6; texbSize -= 6;
				std::copy(scratchpad, scratchpad + 6, vibuf->Indices.begin());

			}

			if ((!cancel && imageDefLen != 0) || (cancel && imageDefLen < 0))
				throw std::runtime_error("image definition size mismatch");

			if (cancel)
			{
				readData(imageDefLen);
				texbSize -= imageDefLen;
			}
			else
			{
				texb->ImageList_Id.push_back(timg.get());
				texb->VertexIndexUVs.push_back(vibuf.get());
				timg.release();
				vibuf.release();
			}
		}
	}

	size_t dataSize = GetBytePerPixel(TexbFlags) * tWidth * tHeight;
	std::unique_ptr<uint8_t[]> rawData(new uint8_t[dataSize]);

	if(TexbFlags & TEXB_FLAG_COMPRESSED)
	{
		// Read 4 more byte to get the compression type.
		readData(4); texbSize -= 4;
		uint32_t compressType = (scratchpad[0] << 24) | (scratchpad[1] << 16) | (scratchpad[2] << 8) | scratchpad[3];

		if(compressType == 0)
		{
			// deflate compression.
			readData(texbSize);
			uLongf destLen = (uLongf) dataSize;
			uLong sourceLen = (uLong) texbSize; texbSize = 0;

			int result = uncompress2(rawData.get(), &destLen, scratchpad, &sourceLen);

			if (result != Z_OK)
				throw std::runtime_error("zlib decompress error");
		}
		else if (compressType == 0x8D64)
			throw std::runtime_error("TODO ETC1 decompression");
		else
			// Upps, I don't know the kind of the compression.
			throw std::runtime_error("unknown or unsupported compression");
	}
	else
		readData(dataSize, rawData.get());

	std::unique_ptr<uint8_t[]> rgbaData(new uint8_t[tWidth * tHeight * 4]);
	convert_map(rawData.get(), tWidth, tHeight, TexbFlags, rgbaData.get());

	texb->RawImage = rgbaData.get();
	rawData.release();
	rgbaData.release();

	for(size_t i = 0; i < texb->ImageList_Id.size(); i++)
	{
		TextureImage* timg = texb->ImageList_Id[i];
		texb->ImageList_Names[timg->Name] = i;
		
		FixedVertexIndexFormat* Vrtx = texb->VertexIndexUVs[i];
		Point v[4] = {
			{Vrtx->Vertices[0].X / 65536, Vrtx->Vertices[0].Y / 65536},
			{Vrtx->Vertices[1].X / 65536, Vrtx->Vertices[1].Y / 65536},
			{Vrtx->Vertices[2].X / 65536, Vrtx->Vertices[2].Y / 65536},
			{Vrtx->Vertices[3].X / 65536, Vrtx->Vertices[3].Y / 65536},
		};
		UVPoint t[4]={
			{std::min(Vrtx->Vertices[0].U / 65536.0, 1.0), std::min(Vrtx->Vertices[0].V / 65536.0, 1.0)},
			{std::min(Vrtx->Vertices[1].U / 65536.0, 1.0), std::min(Vrtx->Vertices[1].V / 65536.0, 1.0)},
			{std::min(Vrtx->Vertices[2].U / 65536.0, 1.0), std::min(Vrtx->Vertices[2].V / 65536.0, 1.0)},
			{std::min(Vrtx->Vertices[3].U / 65536.0, 1.0), std::min(Vrtx->Vertices[3].V / 65536.0, 1.0)},
		};

		auto exes = {v[0].X, v[1].X, v[2].X, v[3].X};
		auto ways = {v[0].Y, v[1].Y, v[2].Y, v[3].Y};
		uint32_t min_x = std::min(exes), max_x = std::max(exes);
		uint32_t min_y = std::min(ways), max_y = std::max(ways);

		size_t rawBmpSize = timg->Width * timg->Height;
		uint32_t* texbBmp = (uint32_t *) texb->RawImage; // uh type punning
		std::unique_ptr<uint32_t[]> rawBmp(new uint32_t[rawBmpSize]);

		memset(rawBmp.get(), 0, rawBmpSize * sizeof(uint32_t));

		for(uint32_t y = min_y; y < max_y; y++)
		{
			for(uint32_t x = min_x; x < max_x; x++)
			{
				UVPoint uv = xy2uv(x, y, v[0], v[1], v[2], v[3], t[0], t[1], t[2], t[3]);
				size_t idx = x + y * timg->Width;

				if (idx >= 0 && idx < rawBmpSize)
					// TODO: Bilinear interpolate
					rawBmp[idx] = texbBmp[uint32_t(uv.U * tWidth + 0.5) + uint32_t(uv.V * tHeight + 0.5) * tWidth];
			}
		}

		timg->RawImage = (uint8_t *) rawBmp.release(); // uh type punning again
	}

	return texb.release();
}

size_t TextureBank::GetImageCount() const noexcept
{
	return ImageList_Id.size();
}
