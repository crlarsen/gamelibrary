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

uniform sampler2D DIFFUSE;

uniform sampler2D BUMP;

uniform lowp vec3 DIFFUSE_COLOR;

uniform lowp vec3 SPECULAR_COLOR;

uniform mediump float SHININESS;

varying lowp vec2 texcoord0;

varying lowp vec3 position;

varying lowp vec3 lightdirection_ts;


void main( void )
{
	lowp vec3 normal = texture2D( BUMP, texcoord0 ).rgb * 2.0 - 1.0;

	lowp float intensity = max( dot( lightdirection_ts, normal ), 0.0 );

	gl_FragColor = vec4( 0.1 );

	if( intensity > 0.0 ) {

		lowp vec3 reflectionvector = normalize( -reflect( lightdirection_ts, normal ) );

		gl_FragColor += texture2D( DIFFUSE, texcoord0 ) * 
						vec4( DIFFUSE_COLOR, 1.0 ) *
						intensity
						   
						+
				   
					    vec4( SPECULAR_COLOR, 1.0 ) * 
						pow( max( dot( reflectionvector, position ), 0.0 ), SHININESS );

	}
}
