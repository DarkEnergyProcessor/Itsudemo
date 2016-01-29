/**
* TEXBPixel.h
* Forward-declaration of functions in TEXBPixel.cpp
**/

#ifndef TEXB_PIXEL_H_
#define TEXB_PIXEL_H_

#include <stdint.h>

void convert_map(uint8_t *raw, uint32_t w, uint32_t h, uint16_t texb_flags, uint8_t *output);

void copy_1bpp_luma(uint8_t *raw, int len, uint8_t *output);
void copy_1bpp_alpha(uint8_t *raw, int len, uint8_t *output);
void copy_2bpp_lumalpha(uint8_t *raw, int len, uint8_t *output);
void copy_2bpp_rgb565(uint8_t *raw, int len, uint8_t *output);
void copy_2bpp_rgba5551(uint8_t *raw, int len, uint8_t *output);
void copy_2bpp_rgba4444(uint8_t *raw, int len, uint8_t *output);
void copy_3bpp_rgb(uint8_t *raw, int len, uint8_t *output);

#endif
