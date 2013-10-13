/*

Book:      	Game and Graphics Programming for iOS and Android with OpenGL(R) ES 2.0
Author:    	Romain Marucchi-Foino
ISBN-10: 	1119975913
ISBN-13: 	978-1119975915
Publisher: 	John Wiley & Sons	

Copyright (C) 2011 Romain Marucchi-Foino

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of
this software. Permission is granted to anyone who either own or purchase a copy of
the book specified above, to use this software for any purpose, including commercial
applications subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that
you wrote the original software. If you use this software in a product, an acknowledgment
in the product would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be misrepresented
as being the original software.

3. This notice may not be removed or altered from any source distribution.

*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "glsl_optimizer.h"

void print_usage( void )
{
	printf("Usage: glsloptimizerCL -in <inputfilename> -out <outputfilename> [-profile <shadertype> ] [-help]\n\n" );
	printf("\t-in        The input shader filename.\n");
	printf("\t-out       The output shader filename (if optimization is successfull).\n");
	printf("\t-profile   The type of shader to optimize, either GL_VERTEX_SHADER (default) or GL_FRAGMENT_SHADER.\n");
	printf("\t-help      Displays help information for glsloptimizerCL.\n");
}


int main( int argc, char * const argv[] )
{
	int err_code = 0;
	
	unsigned int i = 1,
				 p;
	
	char in_file [ 256 ] = {""},
		 out_file[ 256 ] = {""},
		 profile [ 50  ] = {""};
	
	glslopt_ctx *ctx = NULL;

	glslopt_shader_type shader_type = kGlslOptShaderVertex;

	glslopt_shader *shader = NULL;
	
	
	if( argc == 1 )
	{
		print_usage();
		goto cleanup;
	}
	

	while( i != argc )
	{
		p = ( i + 1 );
	
		if( p == argc ) break;
		
		if( !strcmp( argv[ i ], "-in" ) )
		{
			strcpy( in_file, argv[ p ] );
			++i;
		}
		
		else if( !strcmp( argv[ i ], "-out" ) )
		{
			strcpy( out_file, argv[ p ] );
			++i;
		}		

		else if( !strcmp( argv[ i ], "-profile" ) )
		{
			strcpy( profile, argv[ p ] );
			
			if( !strcmp( profile, "GL_VERTEX_SHADER" ) ) shader_type = kGlslOptShaderVertex;
			
			else if( !strcmp( profile, "GL_FRAGMENT_SHADER" ) ) shader_type = kGlslOptShaderFragment;

			else
			{
				printf( "ERROR: Invalid shader profile.\n" );
				err_code = 1;
				goto cleanup;	
			}
			
			++i;
		}
		
		else if( !strcmp( argv[ i ], "-help" ) )
		{
			print_usage();
			goto cleanup;
		}		
	
		++i;
	}
	
	
	if( !strlen( in_file  ) ||
		!strlen( out_file ) ||
		!strlen( profile  ) )
	{
		printf( "ERROR: Invalid parameter.\n" );
		err_code = 2;
		goto cleanup;
	}
	else
	{
		char *code = NULL;
	
		unsigned int size = 0;
		
		FILE *f = fopen( in_file, "rb" );
			
		if( !f )
		{
			printf( "ERROR: Unable to open %s.\n", in_file );
			err_code = 3;
			goto cleanup;
		}

		fseek( f, 0, SEEK_END );
		size = ftell( f );
		fseek( f, 0, SEEK_SET );

		code = ( char * ) calloc( 1, ( size + 1 ) );
		fread( code, size, 1, f );
		fclose( f );
	
		ctx = glslopt_initialize( true );

		shader = glslopt_optimize( ctx, shader_type, code, 0 );

		if( glslopt_get_status( shader ) )
		{
			f = fopen( out_file, "wb" );
			
			if( !f )
			{
				printf( "ERROR: Unable to write %s.\n", out_file );
				err_code = 4;
				goto cleanup;
			}

			printf( "Writing %s ", out_file );
			
			fprintf( f, "%s", glslopt_get_output( shader ) );

			fclose( f );
			
			printf( "[ OK ]\n" );
		}
		else 
		{
			printf("ERROR: %s\n", glslopt_get_log( shader ) );
			err_code = 5;
		}
	}

cleanup:	
	
	if( shader ) glslopt_shader_delete( shader );
	
	if( ctx ) glslopt_cleanup( ctx );

	return err_code;
}
