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

varying lowp vec3 lightdirection_ts;
varying lowp vec3 position;
varying lowp vec2 texcoord0;
uniform mediump float SHININESS;
uniform lowp vec3 SPECULAR_COLOR;
uniform lowp vec3 DIFFUSE_COLOR;
uniform sampler2D BUMP;
uniform sampler2D DIFFUSE;
void main ()
{
  lowp vec3 tmpvar_1;
  tmpvar_1 = ((texture2D (BUMP, texcoord0).xyz * 2.0) - 1.0);
  lowp float tmpvar_2;
  tmpvar_2 = max (dot (lightdirection_ts, tmpvar_1), 0.0);
  gl_FragColor = vec4(0.1, 0.1, 0.1, 0.1);
  if ((tmpvar_2 > 0.0)) {
    lowp vec4 tmpvar_3;
    tmpvar_3.w = 1.0;
    tmpvar_3.xyz = DIFFUSE_COLOR;
    lowp vec4 tmpvar_4;
    tmpvar_4.w = 1.0;
    tmpvar_4.xyz = SPECULAR_COLOR;
    gl_FragColor = (vec4(0.1, 0.1, 0.1, 0.1) + (((texture2D (DIFFUSE, texcoord0) * tmpvar_3) * tmpvar_2) + (tmpvar_4 * pow (max (dot (normalize (-(reflect (lightdirection_ts, tmpvar_1))), position), 0.0), SHININESS))));
  };
}

