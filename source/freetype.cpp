/******************************************************************************/
/**** FREETYPE START ****/
/* This is a very rough implementation if freetype using GRRLIB */

#include "freetype.h"

static FT_Library ftLibrary;
static FT_Face ftFace;

/* Static function prototypes */
static unsigned int DrawText(const wchar_t *string, unsigned int fontSize, void *buffer);
static bool BlitGlyph(FT_Bitmap *bitmap, int offset, int top, void *buffer);
static void BitmapTo4x4RGBA(const unsigned char *src, void *dst, const unsigned int src_width, const unsigned int src_height, const unsigned int dst_width, const unsigned int dst_height);
extern void* GRRLIB_TextToTexture(const char *string, unsigned int fontSize, unsigned int fontColour);

GX_Texture* GX_Text(const char *string, uint fontSize, uint color) {
	size_t length = strlen(string);
	wchar_t *utf32 = (wchar_t*)malloc((length+1) * sizeof(wchar_t));
	mbstowcs(utf32, string, length);
	utf32[length] = 0;

	GX_Texture *ret = GX_Text(utf32, fontSize, color);
	free (utf32);
	return ret;
}

/**
 * Shameless wrapper around the handy FreeTypeGRRLIB
 */
GX_Texture* GX_Text(const wchar_t *string, uint fontSize, uint color)
{
	unsigned int error = FT_Set_Pixel_Sizes(ftFace, 0, fontSize);
	if (error) {
		/* Failed to set the font size to the requested size. 
		 * You probably should set a default size or something. 
		 * I'll leave that up to the reader. */
	}
	const unsigned int height = fontSize*2;
	const unsigned int tempWidth = 640;
	
	/* Set the font colour, alpha is determined when we blit the glyphs */
	color = color << 8;
	
	/* Create a tempory buffer, 640 pixels wide, for freetype draw the glyphs */
	void *tempTextureBuffer = (void*) malloc(tempWidth * height * 4);
	if (tempTextureBuffer == NULL) {
		/* Oops! Something went wrong! */
		exit(0);
	}
	
	/* Set the RGB pixels in tempTextureBuffer to the requested colour */
	unsigned int *p = (unsigned int*)tempTextureBuffer;
	unsigned int loop = 0;
	for (loop = 0; loop < (tempWidth * height); ++loop) {
		*(p++) = color;	
	}
	/* Render the glyphs on to the temp buffer */
	unsigned int width = DrawText(string, fontSize, tempTextureBuffer);
	width = ((width + 3) / 4) * 4; // round up to the nearest mod 4 number
	
	GX_InvalidateTexAll();
	
	/* Create a new buffer, this time to hold the final texture 
	 * in a format suitable for the Wii */
	void *texture = memalign(32, width * height * 4);
	/* Convert the RGBA temp buffer to a format usuable by GX */
	BitmapTo4x4RGBA((const unsigned char *)tempTextureBuffer,texture, tempWidth, height, width, height);
	DCFlushRange(texture, 640 * (fontSize*2) * 4);
	
	/* The temp buffer is no longer required */
	free(tempTextureBuffer);
	
	return new GX_Texture(width, height, GX_TF_RGBA8, (u8*)texture);
}

extern void GX_InitFreetype(void) {
	unsigned int error = FT_Init_FreeType(&ftLibrary);
	if (error) {
		exit(0);
	}
	
	error = FT_New_Memory_Face(ftLibrary, helvetica, helvetica_size, 0, &ftFace);
	/* Note: You could also directly load a font from the SD card like this:
	 error = FT_New_Face(ftLibrary, "fat3:/apps/myapp/font.ttf", 0, &ftFace); */
	
	if (error == FT_Err_Unknown_File_Format) {
		exit(0);
	} else if (error) {
		/* Some other error */
		exit(0);
	}
}

#include <wchar.h>

static unsigned int DrawText(const wchar_t *string, unsigned int fontSize, void *buffer) {
	unsigned int error = 0;
	int penX = 0;
	int penY = fontSize;
	FT_GlyphSlot slot = ftFace->glyph;
	FT_UInt glyphIndex = 0;
	FT_UInt previousGlyph = 0;
	FT_Bool hasKerning = FT_HAS_KERNING(ftFace);
	
	/* Convert the string to UTF32 */
	//size_t length = strlen(string);
	const wchar_t *utf32 =  string;//(wchar_t*)malloc(length * sizeof(wchar_t)); 
	size_t length = wcslen(utf32);//mbstowcs(utf32, string, length);
	
	/* Loop over each character, drawing it on to the buffer, until the 
	 * end of the string is reached, or until the pixel width is too wide */
	unsigned int loop = 0;
	for (loop = 0; loop < length; ++loop) {
		glyphIndex = FT_Get_Char_Index(ftFace, utf32[ loop ]);
		
		/* To the best of my knowledge, none of the other freetype 
		 * implementations use kerning, so my method ends up looking
		 * slightly better :) */
		if (hasKerning && previousGlyph && glyphIndex) {
			FT_Vector delta;
			FT_Get_Kerning(ftFace, previousGlyph, glyphIndex, FT_KERNING_DEFAULT, &delta);
			
			penX += delta.x >> 6;
		}
		
		error = FT_Load_Glyph(ftFace, glyphIndex, FT_LOAD_RENDER);
		if (error) {
			/* Whoops, something went wrong trying to load the glyph 
			 * for this character... you should handle this better */
			continue;
		}
		
		if (BlitGlyph(&slot->bitmap, penX + slot->bitmap_left, penY - slot->bitmap_top, buffer) == true) {
			/* The glyph was successfully blitted to the buffer, move the pen forwards */
			penX += slot->advance.x >> 6;
			previousGlyph = glyphIndex;
		} else {
			/* BlitGlyph returned false, the line must be full */
			//free(utf32);
			return penX;
		}
	}
	
	//free(utf32);
	return penX;
}

/* Returns true if the character was draw on to the buffer, false if otherwise */
static bool BlitGlyph(FT_Bitmap *bitmap, int offset, int top, void *buffer) {
	uint bitmapWidth = bitmap->width;
	uint bitmapHeight = bitmap->rows;
	
	if (offset + bitmapWidth > 640) {
		/* Drawing this character would over run the buffer, so don't draw it */
		return false;
	}
	
	/* Draw the glyph onto the buffer, blitting from the bottom up */
	/* CREDIT: Derived from a function by DragonMinded */
	unsigned char *p = (unsigned char*)buffer;
	unsigned int y = 0;
	for (y = 0; y < bitmapHeight; ++y) {
		int sywidth = y * bitmapWidth;
		int dywidth = (y + top) * 640;
		
		unsigned int column = 0;
		for (column = 0; column < bitmapWidth; ++column) {
			unsigned int srcloc = column + sywidth;
			unsigned int dstloc = ((column + offset) + dywidth) << 2;
			/* Copy the alpha value for this pixel into the texture buffer */
			p[ dstloc + 3 ] = bitmap->buffer[ srcloc ];
		}
	}
	
	return true;
}

static void BitmapTo4x4RGBA(const unsigned char *src, void *dst, const unsigned int src_width, const unsigned int src_height, const unsigned int dst_width, const unsigned int dst_height)
{
	unsigned int block = 0;
	unsigned int i = 0;
	unsigned int c = 0;
	unsigned int ar = 0;
	unsigned int gb = 0;
	unsigned char *p = (unsigned char*)dst;
	
	for (block = 0; block < dst_height; block += 4) {
		for (i = 0; i < dst_width; i += 4) {
			/* Alpha and Red */
			
			for (c = 0; c < 4; ++c) {
				for (ar = 0; ar < 4; ++ar) {
					/* Alpha pixels */
					*p++ = src[(((i + ar) + ((block + c) * src_width)) * 4) + 3];
					/* Red pixels */	
					*p++ = src[((i + ar) + ((block + c) * src_width)) * 4];
				}
			}
			
			/* Green and Blue */
			for (c = 0; c < 4; ++c) {
				for (gb = 0; gb < 4; ++gb) {
					/* Green pixels */
					*p++ = src[(((i + gb) + ((block + c) * src_width)) * 4) + 1];
					/* Blue pixels */
					*p++ = src[(((i + gb) + ((block + c) * src_width)) * 4) + 2];
				}
			}
		} /* i */
	} /* block */
}

/**** FREETYPE END ****/
/******************************************************************************/