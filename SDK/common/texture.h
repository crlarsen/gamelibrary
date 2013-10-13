/*

GFX Lightweight OpenGLES 2.0 Game and Graphics Engine

Copyright (C) 2011 Romain Marucchi-Foino http://gfx.sio2interactive.com

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of
this software. Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it freely,
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that
you wrote the original software. If you use this software in a product, an acknowledgment
in the product would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be misrepresented
as being the original software.

3. This notice may not be removed or altered from any source distribution.

*/

#ifndef TEXTURE_H
#define TEXTURE_H

enum
{
	TEXTURE_CLAMP			= ( 1 << 0 ),
	TEXTURE_MIPMAP			= ( 1 << 1 ),
	TEXTURE_16_BITS			= ( 1 << 2 ),
	TEXTURE_16_BITS_5551	= ( 1 << 3 )
};


enum
{
	TEXTURE_FILTER_0X = 0,
	TEXTURE_FILTER_1X = 1,
	TEXTURE_FILTER_2X = 2,
	TEXTURE_FILTER_3X = 3
};


typedef struct
{
	unsigned int headersize;
	
	unsigned int height;

	unsigned int width;
	
	unsigned int n_mipmap;
	
	unsigned int flags;
	
	unsigned int datasize;
	
	unsigned int bpp;
	
	unsigned int bitred;

	unsigned int bitgreen;

	unsigned int bitblue;
	
	unsigned int bitalpha;
	
	unsigned int tag;
	
	unsigned int n_surface;

} PVRHEADER;


typedef struct
{
	char			name[ MAX_CHAR ];
	
	unsigned int	tid;
	
	unsigned short	width;
	
	unsigned short	height;
	
	unsigned char	byte;
	
	unsigned int	size;

	unsigned int	target;
	
	unsigned int	internal_format;
	
	unsigned int	format;

	unsigned int	texel_type;

	unsigned char	*texel_array;

	unsigned int	n_mipmap;
	
	unsigned int	compression;
		
} TEXTURE;


TEXTURE *TEXTURE_init( char *name );

TEXTURE *TEXTURE_free( TEXTURE *texture );

TEXTURE *TEXTURE_create( char *name, char *filename, unsigned char relative_path, unsigned int flags, unsigned char filter, float anisotropic_filter );

void TEXTURE_load( TEXTURE *texture, MEMORY *memory );

void TEXTURE_load_png( TEXTURE *texture, MEMORY *memory );

void TEXTURE_load_pvr( TEXTURE *texture, MEMORY *memory );

void TEXTURE_convert_16_bits( TEXTURE *texture, unsigned char use_5551 );

void TEXTURE_generate_id( TEXTURE *texture, unsigned int flags, unsigned char filter, float anisotropic_filter );

void TEXTURE_delete_id( TEXTURE *texture );

void TEXTURE_free_texel_array( TEXTURE *texture );

void TEXTURE_draw( TEXTURE *texture );

#endif
