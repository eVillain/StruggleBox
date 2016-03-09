#version 400

precision highp float;

layout(location = 0) out vec4 color;

// Interpolated values from the vertex shaders
in vec2 uv;

uniform sampler2D textureMap;
uniform vec2 texelSize;
uniform float brightness;
uniform float threshold;

//float Brightness = 1.0;
//const float threshold = 0.05;
const int NUM = 9;
vec2 c[NUM] = vec2[](vec2(-texelSize.x, texelSize.y),
                           vec2( 0.00       , texelSize.y),
                           vec2( texelSize.x, texelSize.y),
                           vec2(-texelSize.x, 0.00 ),
                           vec2( 0.00       , 0.00 ),
                           vec2( texelSize.x, 0.00 ),
                           vec2(-texelSize.x,-texelSize.y),
                           vec2( 0.00       ,-texelSize.y),
                           vec2( texelSize.x,-texelSize.y) );

void main( void ) {
    vec3 col[NUM];
    int i;
    //    Store the samples from texture to col array.    
    for (i=0; i < NUM; i++) {
        col[i] = texture(textureMap, uv + c[i]).rgb;
    }
    //    Compute the luminance with dot product and store them in lum array.
    vec3 rgb2lum = vec3(0.30, 0.59, 0.11);
    float lum[NUM];
    for (i = 0; i < NUM; i++) {
        lum[i] = dot(col[i].xyz, rgb2lum);
    }
    //    Sobel filter computes new value at the central position by sum the weighted neighbors.
    float M = lum[2]+ lum[8]+2*lum[5]-lum[0]-2*lum[3]-lum[6];
    float S = lum[6]+2*lum[7]+ lum[8]-lum[0]-2*lum[1]-lum[2];
    //    Show the points which values are over the threshold and hide others. Final result is the product of col[5] and edge detector value.
    //    Brightness adjusts the brightness of the image.
    
//    float edge = (M*M + S*S < threshold) ? 1.0 : 0.0;
    float edge = (M*M + S*S);
    if ( edge < threshold ) {
        color = vec4(col[4].xyz,1)*brightness;
    } else {
        color = vec4(0);
    }
//    color = vec4(brightness * col[5].xyz * edge, 1.0);
    
//    color = texture(textureMap, uv);
}
