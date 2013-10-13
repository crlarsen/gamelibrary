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

SHADER *SHADER_init( char *name, unsigned int type )
{
	SHADER *shader = ( SHADER * ) calloc( 1, sizeof( SHADER ) );

	strcpy( shader->name, name );

	shader->type = type;
	
	return shader;
}


SHADER *SHADER_free( SHADER *shader )
{
	if( shader->sid ) SHADER_delete_id( shader );

	free( shader );
	return NULL;
}

 
unsigned char SHADER_compile( SHADER *shader, const char *code, unsigned char debug )
{
	char type[ MAX_CHAR ] = {""};
	
	int loglen,
		status;
	
	if( shader->sid ) return 0;
	
	shader->sid = glCreateShader( shader->type );
	
    glShaderSource( shader->sid, 1, &code, NULL );
	
    glCompileShader( shader->sid );
    
	if( debug )
	{
		if( shader->type == GL_VERTEX_SHADER ) strcpy( type, "GL_VERTEX_SHADER" );
		else strcpy( type, "GL_FRAGMENT_SHADER" );
		
		glGetShaderiv( shader->sid, GL_INFO_LOG_LENGTH, &loglen );
		
		if( loglen )
		{
			char *log = ( char * ) malloc( loglen );

			glGetShaderInfoLog( shader->sid, loglen, &loglen, log );
			
			#ifdef __IPHONE_4_0
			
				printf("[ %s:%s ]\n%s", shader->name, type, log );
			#else
				__android_log_print( ANDROID_LOG_ERROR, "", "[ %s:%s ]\n%s", shader->name, type, log );
			#endif
			
			free( log );
		}
	}
    
    glGetShaderiv( shader->sid, GL_COMPILE_STATUS, &status );
	
	if( !status )
	{
		SHADER_delete_id( shader );
		return 0;
	}

	return 1;
}

 
void SHADER_delete_id( SHADER *shader )
{
	if( shader->sid )
	{
		glDeleteShader( shader->sid );
		shader->sid = 0;
	}
}
