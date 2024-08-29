#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D scene;
uniform sampler2D bloomBlur;
uniform bool bloom;
uniform bool gammaEnabled;
uniform float exposure;
uniform bool hdr;


void main(){
    const float gamma = 2.2;
    vec3 hdrColor = texture(scene, TexCoords).rgb;
    vec3 bloomColor = texture(bloomBlur, TexCoords).rgb;
    vec3 result;
    if(bloom)
        hdrColor += bloomColor; // additive blending
    // tone mapping
    if(hdr)
        result = vec3(1.0) - exp(-hdrColor * exposure);
    else
        result=hdrColor;

    // gamma correction
    if(gammaEnabled)
        result = pow(result, vec3(1.0 / gamma));
    FragColor = vec4(result, 1.0);
}