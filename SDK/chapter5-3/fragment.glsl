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

varying lowp vec2 texcoord0;

/* Notice that originally in the vertex shader, this variable precision
 * qualifier was declared as mediump.  But since you normalized it
 * before sending it to the fragment shader, you can now declare it here
 * as lowp.
 */
varying lowp vec3 position;
varying lowp vec3 lightdirection_ts;

void main( void )
{
    /* Get the current RGB data from the bump map and convert it into
     * a normal value.
     */
    lowp vec3 normal = texture2D( BUMP, texcoord0 ).rgb * 2.0 - 1.0;

    /* Now calculate the intensity (aka Lambert factor) based on the
     * light direction vector and the normal, which are now both in tangent
     * space.
     */
    lowp float intensity = max( dot( lightdirection_ts, normal), 0.0 );

    /* Set the default ambient color. */
    gl_FragColor = vec4( 0.1 );

    /* Check if you have an intensity. */
    if ( intensity > 0.0 ) {
        /* Add to the current fragment the result of the intensity with the
         * current diffuse color of the material.
         */
        gl_FragColor += texture2D( DIFFUSE, texcoord0 ) * vec4( DIFFUSE_COLOR, 1.0 ) * intensity;
    }
}
