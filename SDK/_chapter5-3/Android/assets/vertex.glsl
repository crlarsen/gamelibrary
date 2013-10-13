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

uniform mediump mat4 MODELVIEWMATRIX;

uniform mediump mat4 PROJECTIONMATRIX;

uniform lowp mat3 NORMALMATRIX;

uniform mediump vec3 LIGHTPOSITION;


attribute mediump vec3 POSITION;

attribute lowp vec2 TEXCOORD0;

attribute lowp vec3 NORMAL;

attribute lowp vec3 TANGENT0;


varying lowp vec2 texcoord0;

varying lowp vec3 position;

varying lowp vec3 lightdirection_ts;


void main( void )
{
	mediump vec3 tmp;

	lowp vec3 normal   = NORMALMATRIX * NORMAL;
	lowp vec3 tangent  = NORMALMATRIX * TANGENT0;
	lowp vec3 binormal = cross( normal, tangent );

	position = vec3( MODELVIEWMATRIX * vec4( POSITION, 1.0 ) );

	gl_Position = PROJECTIONMATRIX * vec4( position, 1.0 );

	lowp vec3 lightdirection_es = normalize( LIGHTPOSITION - position );

	lightdirection_ts.x = dot( lightdirection_es, tangent );
	lightdirection_ts.y = dot( lightdirection_es, binormal );
	lightdirection_ts.z = dot( lightdirection_es, normal );

	tmp.x = dot( position, tangent );
	tmp.y = dot( position, binormal );
	tmp.z = dot( position, normal );

	position = -normalize( tmp );

	texcoord0 = TEXCOORD0;
}
