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

/* This matrix is already normalized (since you do not use scale),
 * so there's no need to use any other precision qualifier than lowp.
 */
uniform lowp mat3 NORMALMATRIX;

/* This light position is not normalized, so you have no choice but to
 * use at least mediump.
 */
uniform mediump vec3 LIGHTPOSITION;

attribute mediump vec3 POSITION;

attribute lowp vec2 TEXCOORD0;

/* Always use lowp when dealing with the normal attribute, because it
 * should always be normalized outside the shader.
 */
attribute lowp vec3 NORMAL;

/* As with the normal, the tangent should always be normalized. */
attribute lowp vec3 TANGENT0;

varying lowp vec2 texcoord0;

/* The current vertex position in tangent space. */
varying mediump vec3 position;

/* The light direction in tangent space. */
varying lowp vec3 lightdirection_ts;

void main( void )
{
    mediump vec3 tmp;

    /* Rotate the normal and the tangent by the current normal matrix.  Since
     * the normal, tangent, and normal matrix are all normalized, you can save two
     * normalize instruction calls.
     */
    lowp vec3 normal  = NORMALMATRIX * NORMAL;
    lowp vec3 tangent = NORMALMATRIX * TANGENT0;

    /* Calculate the binormal (or bi-tangent if you prefer) based on the
     * current normal and tangent.  The cross-product of the two normalized vectors
     * will always be normalized, which explains the lowp precision qualifier.
     */
    lowp vec3 binormal = cross( normal, tangent );

    /* Calculate the current vertex position in eye space. */
    position = vec3( MODELVIEWMATRIX * vec4( POSITION, 1.0 ) );

    /* Since you just calculated the position in eye space, you can
     * use the current value and multiply it by the projection matrix to be
     * able to see the current vertex on screen. */
    gl_Position = PROJECTIONMATRIX * vec4( position, 1.0 );

    /* Calculate the light direction in eye space and normalize it */
    lowp vec3 lightdirection_es = normalize( LIGHTPOSITION - position );

    /* Transform the light direction in eye space to tangent space. */
    lightdirection_ts.x = dot( lightdirection_es, tangent );
    lightdirection_ts.y = dot( lightdirection_es, binormal );
    lightdirection_ts.z = dot( lightdirection_es, normal );

    /* Since the light calculation will be done in tangent space,
     * also convert the current position in eye space to tangent space and
     * invert it.
     */
    tmp.x = dot( position, tangent );
    tmp.y = dot( position, binormal );
    tmp.z = dot( position, normal );
    position = -normalize( tmp );

    /* Bridge over the texture coordinate. */
    texcoord0 = TEXCOORD0;
}
