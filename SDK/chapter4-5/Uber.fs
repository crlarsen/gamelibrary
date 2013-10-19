#ifdef LIGHTING_SHADER
    uniform mediump vec3 LIGHTPOSITION;
    uniform lowp vec3 AMBIENT_COLOR;
    uniform lowp vec3 DIFFUSE_COLOR;
    uniform lowp vec3 SPECULAR_COLOR;
    uniform mediump float SHININESS;
    uniform lowp float DISSOLVE;
    
    varying mediump vec3 position;
    varying lowp vec3 normal;
#endif

uniform sampler2D DIFFUSE;

varying mediump vec2 texcoord0;
 
void main(void) {
    lowp vec4 diffuse_color = texture2D(DIFFUSE, texcoord0);
    
#ifdef LIGHTING_SHADER
    /* Remember the alpha value of the texture. */
    lowp float alpha = diffuse_color.a;
    /* Use the light position in eye space with the vertex position in
     * eye space to calculate the light direction vector.
     */
    mediump vec3 L = normalize(LIGHTPOSITION - position);
    /* Invert the eye position vertex. */
    mediump vec3 E = normalize(-position);
    /* Calculate the reflection vector of the light direction and the
     * current vertex normal that will be used for specular color
     * calculation.
     */
    mediump vec3 R = normalize(-reflect(L, normal));
    /* Assign the ambient color. */
    mediump vec4 ambient = vec4(AMBIENT_COLOR, 1.0);
    /* Calculate the final diffuse color. This calculation is based on
     * the multiplication of the material diffuse color with the
     * diffuse texture color and finally adjusted by the light
     * intensity calculation.
     */
    mediump vec4 diffuse = vec4(DIFFUSE_COLOR * diffuse_color.rgb, 1.0) *
                           max(dot(normal, L), 0.0);
    /* Calculate the final specular color. This calculation is based on
     * the specular color of the material affected by the reflection
     * vector of the light and boosted by the shininess of the material.
     */
    mediump vec4 specular = vec4(SPECULAR_COLOR, 1.0) *
                            pow(max(dot(R, E), 0.0), SHININESS * 0.3);
    /* Calculate the final fragment color by adding together all the
     * different colors that you have calculated above.
     */
    diffuse_color = vec4(0.1) +
                    /* Scene Color. */ ambient + diffuse + specular;
    /* In the calculation above, you lose the original alpha of the
     * texture, so you have to make sure to reassign it.
     */
    diffuse_color.a = alpha;
#endif


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
    gl_FragColor.a = DISSOLVE;
#endif
}
