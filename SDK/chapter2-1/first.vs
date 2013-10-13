uniform mediump mat4    MODELVIEWPROJECTIONMATRIX;
attribute mediump vec4  POSITION;
attribute lowp vec4     COLOR;
varying lowp vec4       color;

void main(void)
{
    gl_Position = MODELVIEWPROJECTIONMATRIX * POSITION;
    color = COLOR;
}