#version 120

uniform float waterLevel;
uniform float grassLevel;
uniform float hillLevel;
uniform float snowLevel;
uniform float u_time;
uniform vec2 u_resolution;

uniform sampler2DRect tex0;
varying vec2 texCoordVarying;

#define TAU 6.28318530718
#define MAX_ITER 5

// Water turbulence effect by joltz0r 2013-07-04, improved 2013-07-07 by David Hoskins

void main() {
	// WATER SHADER
	float time = u_time * .5+23.0;
	vec2 uv = gl_FragCoord.xy / u_resolution.xy;    
    vec2 p = mod(uv*TAU, TAU)-250.0;
	vec2 i = vec2(p);
	float c = 1.0;
	float inten = .005;
	for (int n = 0; n < MAX_ITER; n++) {
		float t = time * (1.0 - (3.5 / float(n+1)));
		i = p + vec2(cos(t - i.x) + sin(t + i.y), sin(t - i.y) + cos(t + i.x));
		c += 1.0/length(vec2(p.x / (sin(i.x+t)/inten),p.y / (cos(i.y+t)/inten)));
	}
	c /= float(MAX_ITER);
	c = 1.17-pow(c, 1.4);
	vec3 waterColor = vec3(pow(abs(c), 8.0));
    waterColor = clamp(waterColor + vec3(0.0, 0.35, 0.5), 0.0, 1.0);
	// END WATER SHADER    

    // pick and choose shader per elevation level
	vec4 textureColor = texture2DRect(tex0, texCoordVarying);
	// TODO: check more than just the R channel
	if(textureColor.r < waterLevel){
		gl_FragColor = vec4(waterColor, 1.0);
	} else if(textureColor.r < grassLevel){
		gl_FragColor = texture2DRect(tex0, texCoordVarying);//vec4(0.0, 0.0, 0.0, 1.0);
	} else if(textureColor.r < hillLevel){
		gl_FragColor = texture2DRect(tex0, texCoordVarying);//vec4(0.0, 0.0, 0.0, 1.0);
	} else if(textureColor.r < snowLevel){
		gl_FragColor = texture2DRect(tex0, texCoordVarying);//vec4(0.0, 0.0, 0.0, 1.0);
	} else{
    	gl_FragColor = texture2DRect(tex0, texCoordVarying);
    }
}