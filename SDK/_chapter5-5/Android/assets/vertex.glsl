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
varying mediump vec3 position;
varying lowp vec2 texcoord0;
attribute lowp vec3 TANGENT0;
attribute lowp vec3 NORMAL;
attribute lowp vec2 TEXCOORD0;
attribute mediump vec3 POSITION;
uniform mediump vec3 LIGHTPOSITION;
uniform lowp mat3 NORMALMATRIX;
uniform mediump mat4 PROJECTIONMATRIX;
uniform mediump mat4 MODELVIEWMATRIX;
void main ()
{
  mediump vec3 tmp;
  lowp vec3 tmpvar_1;
  tmpvar_1 = (NORMALMATRIX * NORMAL);
  lowp vec3 tmpvar_2;
  tmpvar_2 = (NORMALMATRIX * TANGENT0);
  lowp vec3 tmpvar_3;
  tmpvar_3 = cross (tmpvar_1, tmpvar_2);
  mediump vec4 tmpvar_4;
  tmpvar_4.w = 1.0;
  tmpvar_4.xyz = POSITION;
  mediump vec3 tmpvar_5;
  tmpvar_5 = (MODELVIEWMATRIX * tmpvar_4).xyz;
  mediump vec4 tmpvar_6;
  tmpvar_6.w = 1.0;
  tmpvar_6.xyz = tmpvar_5;
  gl_Position = (PROJECTIONMATRIX * tmpvar_6);
  mediump vec3 tmpvar_7;
  tmpvar_7 = normalize ((LIGHTPOSITION - tmpvar_5));
  lowp vec3 lightdirection_es = tmpvar_7;
  lightdirection_ts.x = dot (lightdirection_es, tmpvar_2);
  lightdirection_ts.y = dot (lightdirection_es, tmpvar_3);
  lightdirection_ts.z = dot (lightdirection_es, tmpvar_1);
  tmp.x = dot (tmpvar_5, tmpvar_2);
  tmp.y = dot (tmpvar_5, tmpvar_3);
  tmp.z = dot (tmpvar_5, tmpvar_1);
  position = -(normalize (tmp));
  texcoord0 = TEXCOORD0;
}

