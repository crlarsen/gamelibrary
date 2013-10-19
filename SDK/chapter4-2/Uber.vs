uniform mediump mat4 MODELVIEWPROJECTIONMATRIX;

attribute mediump vec3 POSITION;
attribute mediump vec2 TEXCOORD0;

varying mediump vec2 texcoord0;

void main(void) {
    texcoord0 = TEXCOORD0;
    gl_Position = MODELVIEWPROJECTIONMATRIX * vec4(POSITION, 1.0);
}
