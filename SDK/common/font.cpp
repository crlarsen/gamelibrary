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

#include "gfx.h"


FONT *FONT_init( char *name )
{
	FONT *font = ( FONT * ) calloc( 1, sizeof( FONT ) );
	
	strcpy( font->name, name );
	
	font->program = PROGRAM_init( name );
	
	font->program->vertex_shader = SHADER_init( name, GL_VERTEX_SHADER );
	
	SHADER_compile( font->program->vertex_shader,
					"uniform mediump mat4 MODELVIEWPROJECTIONMATRIX;"
					"attribute mediump vec2 POSITION;"
					"attribute lowp vec2 TEXCOORD0;"
					"varying lowp vec2 texcoord0;"
					"void main( void ) {"
					"texcoord0 = TEXCOORD0;"
					"gl_Position = MODELVIEWPROJECTIONMATRIX * vec4( POSITION.x, POSITION.y, 0.0, 1.0 ); }",
					0 );

	font->program->fragment_shader = SHADER_init( name, GL_FRAGMENT_SHADER );
	
	SHADER_compile( font->program->fragment_shader,
					"uniform sampler2D DIFFUSE;"
					"uniform lowp vec4 COLOR;"
					"varying lowp vec2 texcoord0;"
					"void main( void ) {"
					"lowp vec4 color = texture2D( DIFFUSE, texcoord0 );"
					"color.x = COLOR.x;"
					"color.y = COLOR.y;"
					"color.z = COLOR.z;"
					"color.w *= COLOR.w;"
					"gl_FragColor = color; }",
					0 );

	PROGRAM_link( font->program, 0 );

	return font;
}


FONT *FONT_free( FONT *font )
{
	if( font->program )
	{
		SHADER_free( font->program->vertex_shader );

		SHADER_free( font->program->fragment_shader );
	
		PROGRAM_free( font->program );
	}

	if( font->character_data ) free( font->character_data );

	if( font->tid ) glDeleteTextures( 1, &font->tid );

	free( font );
	return NULL;
}


unsigned char FONT_load( FONT			*font,
						 char			*filename,
						 unsigned char	relative_path,
						 float			font_size,
						 unsigned int	texture_width,
						 unsigned int	texture_height,
						 int			first_character,
						 int			count_character )
{
	MEMORY *m = mopen( filename, relative_path );

	if( m )
	{
		unsigned char *texel_array = ( unsigned char * ) malloc( texture_width * texture_height );
		
		font->character_data = ( stbtt_bakedchar * ) malloc( count_character * sizeof( stbtt_bakedchar ) );
		
		font->font_size = font_size;
		
		font->texture_width = texture_width;
		
		font->texture_height = texture_height;
		
		font->first_character = first_character;
		
		font->count_character = count_character;
		
		stbtt_BakeFontBitmap( m->buffer,
							  0,
							  font_size,
							  texel_array,
							  texture_width,
							  texture_height,
							  first_character,
							  count_character,
							  font->character_data );

		mclose( m );
		
		glGenTextures(1, &font->tid );
		
		glBindTexture( GL_TEXTURE_2D, font->tid );
		
		glTexImage2D( GL_TEXTURE_2D,
					  0,
					  GL_ALPHA,
					  texture_width,
					  texture_height,
					  0,
					  GL_ALPHA,
					  GL_UNSIGNED_BYTE,
					  texel_array );
		
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		
		free( texel_array );
	}
	
	return 0;
}


void FONT_print( FONT *font, float x, float y, char *text, vec4 *color )
{
	char vertex_attribute =  PROGRAM_get_vertex_attrib_location( font->program,
																 ( char * )"POSITION" ),
																 
		 texcoord_attribute = PROGRAM_get_vertex_attrib_location( font->program,
																 ( char * )"TEXCOORD0" );

	glBindVertexArrayOES( 0 );

	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

	glDisable( GL_CULL_FACE );
	
	glDisable( GL_DEPTH_TEST );
	
	glDepthMask( GL_FALSE );

	glEnable( GL_BLEND );
		
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	
	PROGRAM_draw( font->program );

	glUniformMatrix4fv( PROGRAM_get_uniform_location( font->program, ( char * )"MODELVIEWPROJECTIONMATRIX"),
						1,
						GL_FALSE, 
						( float * )GFX_get_modelview_projection_matrix() );

	glUniform1i( PROGRAM_get_uniform_location( font->program, ( char * )"DIFFUSE" ), 0 );
	
	if( color ) glUniform4fv( PROGRAM_get_uniform_location( font->program, ( char * )"COLOR" ), 1, ( float * )color );

	glActiveTexture( GL_TEXTURE0 );

	glBindTexture( GL_TEXTURE_2D, font->tid );
	
	glEnableVertexAttribArray( vertex_attribute );
	
	glEnableVertexAttribArray( texcoord_attribute );

	while( *text )
	{
		if( *text >= font->first_character &&
			*text <= ( font->first_character + font->count_character ) )
		{
			vec2 vert[ 4 ];
			
			vec2 uv[ 4 ];
			
			stbtt_aligned_quad quad;
			
			stbtt_bakedchar *bakedchar = font->character_data + ( *text - font->first_character );
			
			int round_x = STBTT_ifloor( x + bakedchar->xoff );
			int round_y = STBTT_ifloor( y - bakedchar->yoff );
			
			quad.x0 = ( float )round_x;
			quad.y0 = ( float )round_y;
			quad.x1 = ( float )round_x + bakedchar->x1 - bakedchar->x0;
			quad.y1 = ( float )round_y - bakedchar->y1 + bakedchar->y0;
			
			quad.s0 = bakedchar->x0 / ( float )font->texture_width;
			quad.t0 = bakedchar->y0 / ( float )font->texture_width;
			quad.s1 = bakedchar->x1 / ( float )font->texture_height;
			quad.t1 = bakedchar->y1 / ( float )font->texture_height;
			
			x += bakedchar->xadvance;
			
			vert[ 0 ].x = quad.x1; vert[ 0 ].y = quad.y0;
			uv  [ 0 ].x = quad.s1; uv  [ 0 ].y = quad.t0;

			vert[ 1 ].x = quad.x0; vert[ 1 ].y = quad.y0;
			uv  [ 1 ].x = quad.s0; uv  [ 1 ].y = quad.t0;

			vert[ 2 ].x = quad.x1; vert[ 2 ].y = quad.y1;
			uv  [ 2 ].x = quad.s1; uv  [ 2 ].y = quad.t1;

			vert[ 3 ].x = quad.x0; vert[ 3 ].y = quad.y1;
			uv  [ 3 ].x = quad.s0; uv  [ 3 ].y = quad.t1;

			glVertexAttribPointer( vertex_attribute,
								   2,
								   GL_FLOAT,
								   GL_FALSE,
								   0,
								   ( float * )&vert[ 0 ] );

			glVertexAttribPointer( texcoord_attribute,
								   2,
								   GL_FLOAT,
								   GL_FALSE,
								   0,
								   ( float * )&uv[ 0 ] );

			glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
		}
	
		++text;
	}

	glEnable( GL_CULL_FACE );
	
	glEnable( GL_DEPTH_TEST );
	
	glDepthMask( GL_TRUE );

	glDisable( GL_BLEND );
}


float FONT_length( FONT *font, char *text )
{
	float length = 0;
	
	while( *text )
	{
		if( *text >= font->first_character &&
			*text <= ( font->first_character + font->count_character ) )
		{
			stbtt_bakedchar *bakedchar = font->character_data + ( *text - font->first_character );
			
			length += bakedchar->xadvance;
		}
	
		++text;
	}
	
	return length;
}

