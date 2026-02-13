#version 400

layout(location = 0) out vec4 color;

// Interpolated values from the vertex shaders
in vec2 uv;

uniform sampler2D textureMap;
uniform float brightness;
uniform float threshold;
const float root2 = sqrt(2.0);
uniform mat3 G[9] = mat3[](
                           1.0/(2.0*root2) * mat3( 1.0, root2, 1.0, 0.0, 0.0, 0.0, -1.0, -root2, -1.0 ),
                           1.0/(2.0*root2) * mat3( 1.0, 0.0, -1.0, root2, 0.0, -root2, 1.0, 0.0, -1.0 ),
                           1.0/(2.0*root2) * mat3( 0.0, -1.0, root2, 1.0, 0.0, -1.0, -root2, 1.0, 0.0 ),
                           1.0/(2.0*root2) * mat3( root2, -1.0, 0.0, -1.0, 0.0, 1.0, 0.0, 1.0, -root2 ),
                           1.0/2.0 * mat3( 0.0, 1.0, 0.0, -1.0, 0.0, -1.0, 0.0, 1.0, 0.0 ),
                           1.0/2.0 * mat3( -1.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, -1.0 ),
                           1.0/6.0 * mat3( 1.0, -2.0, 1.0, -2.0, 4.0, -2.0, 1.0, -2.0, 1.0 ),
                           1.0/6.0 * mat3( -2.0, 1.0, -2.0, 1.0, 4.0, 1.0, -2.0, 1.0, -2.0 ),
                           1.0/3.0 * mat3( 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 )
                           );


void main(void)
{
	mat3 I;
	float cnv[9];
	vec3 texSample;
	
	/* fetch the 3x3 neighbourhood and use the RGB vector's length as intensity value */
	for (int i=0; i<3; i++) {
        for (int j=0; j<3; j++) {
            texSample = texelFetch( textureMap, ivec2(gl_FragCoord) + ivec2(i-1,j-1), 0 ).rgb;
            I[i][j] = length(texSample);
        }
	}
	/* calculate the convolution values for all the masks */
	for (int i=0; i<9; i++) {
		float dp3 = dot(G[i][0], I[0]) + dot(G[i][1], I[1]) + dot(G[i][2], I[2]);
		cnv[i] = dp3 * dp3; 
	}
    
	float M = (cnv[0] + cnv[1]) + (cnv[2] + cnv[3]);
	float S = (cnv[4] + cnv[5]) + (cnv[6] + cnv[7]) + (cnv[8] + M); 
	
    float lum = sqrt(M/S);
    if ( lum > threshold ) {
        color = vec4(lum);
    } else {
        color = texture(textureMap,uv)*brightness;
    }
}
