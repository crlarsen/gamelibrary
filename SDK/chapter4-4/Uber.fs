uniform sampler2D DIFFUSE;

varying mediump vec2 texcoord0;
 
void main(void) {
    lowp vec4 diffuse_color = texture2D(DIFFUSE, texcoord0);
  
#ifdef SOLID_OBJECT
    gl_FragColor = diffuse_color;
#endif

#ifdef ALPHA_TESTED_OBJECT
    /* NEW! If the alpha value of the texture diffuse color is less than 0.5,
     * discard the fragment.
     */
    if (diffuse_color.a < 0.1)
        discard;
    else
        gl_FragColor = diffuse_color;
#endif

#ifdef TRANSPARENT_OBJECT
    gl_FragColor = diffuse_color;
    /* NEW! Override the texture diffuse color alpha value (if any) with
     * the one contained in the material (you only have one at the moment,
     * so you can hardcode the value in the shader for now).
     */
    gl_FragColor.a = 0.65;
#endif
}
