uniform sampler2D DIFFUSE;

varying mediump vec2 texcoord0;

void main(void) {
    gl_FragColor = texture2D(DIFFUSE, texcoord0);
}
