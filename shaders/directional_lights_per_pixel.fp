// vertex shader
// directional lights
// per pixel lighting


#define MAX_LIGHTS 8

// modify and recompile for different number of lights
#define NUM_LIGHTS 2

// Normal in eye coords
varying vec3 N;

// Light direction vector and halfway vector for each light source
varying vec3 L[NUM_LIGHTS], H[NUM_LIGHTS]; 

// Color collects frontlightmodel scene color + ambient from each light
// Diffuse holds diffuse for each light
varying vec4 Color, Diffuse[NUM_LIGHTS];

void main() {	
	vec3 n, h;
	float NdotH, NdotL, spec_factor;
	int i;
	// color has all ambient components
	vec4 color = Color;
	
	n = normalize(N);
	
	for(i=0; i<NUM_LIGHTS; i++) {		
		NdotL = max( 0.0, dot(n, L[i]));
		if(NdotL > 0.00001) {
			h = normalize(H[i]);
			NdotH = max( 0.0, dot(n, h));
			spec_factor = pow(NdotH, gl_FrontMaterial.shininess);
			
			// add diffuse and specular colors
			color += 	Diffuse[i] * NdotL +
						gl_FrontMaterial.specular * gl_LightSource[i].specular * spec_factor;
		}
	}
	gl_FragColor = clamp(color, 0.0, 1.0);

}