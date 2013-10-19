/* Declare a new entry inside your Uber Shader */
#ifdef LIGHTING_SHADER
    uniform mediump mat4 MODELVIEWMATRIX;
    uniform mediump mat4 PROJECTIONMATRIX;
    uniform mediump mat3 NORMALMATRIX;
    
    attribute lowp vec3 NORMAL;

    /* Since the lighting calculation will be done inside the fragment
     * shader, declare two variables to bridge the vertex normal and the
     * vertex position to send them over.
    */
    varying lowp vec3 normal;
    varying mediump vec3 position;
#else
    uniform mediump mat4 MODELVIEWPROJECTIONMATRIX;
#endif

attribute mediump vec3 POSITION;
attribute mediump vec2 TEXCOORD0;

varying mediump vec2 texcoord0;

void main(void) {
    texcoord0 = TEXCOORD0;
#ifdef LIGHTING_SHADER
    /* Calculate the vertex position in eye space. */
    position = vec3(MODELVIEWMATRIX * vec4(POSITION, 1.0));
    
    /* Adjust the current vertex normal with the normal matrix. */
    normal = normalize(NORMALMATRIX * NORMAL);
    
    /* Multiply the eye position with the projection matrix to be able to
     * position the vertex onscreen.
     */
    gl_Position = PROJECTIONMATRIX * vec4(position, 1.0);
#else
    gl_Position = MODELVIEWPROJECTIONMATRIX * vec4(POSITION, 1.0);
#endif
}
