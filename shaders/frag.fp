// basic fragment shader for use with one light source, with it's 
// direction vector passed in as L

// incoming normal and light vecs
varying vec3 N, L;

// flag set when you want behavior as if glIsEnabled(GL_LIGHTING) == false
uniform bool DisableLighting;

// This gets the contribution of material + environment not
// obtained from the other FrontMaterial properties
// FrontMaterial.emissive + FrontMaterial.ambient + LightModel.ambient
uniform vec4 FrontLightModelProduct;

// computatation of directional light, or
// point light without attenuation
// assumes N and L are normalized
// params:
// i: which light
// norm: normal in eye coords
// lightv: light vec in eye coords
void direct_light(const in int i,
						const in vec3 norm,
						const in vec3 lightv,
						const in float specular_exp,
						inout vec4 ambient,
						inout vec4 diffuse,
						inout vec4 specular)
{
	// diffuse color
	float NdotL = max(0.0, dot(norm, lightv));
	// halfway vec
	vec3 H = normalize(lightv + vec3(0.0, 0.0, 1.0));
	// specular color
	float NdotH = max(0.0, dot(norm, H));
	// specular factor
	float spec_factor;
	if(NdotL == 0.0) {
		spec_factor = 0.0;
	} else {
		spec_factor = pow(NdotH, specular_exp);
	}
	ambient += gl_LightSource[i].ambient;
	diffuse += gl_LightSource[i].diffuse * NdotL;
	specular += gl_LightSource[i].specular * spec_factor;
}

void main()
{
	// if lighting disabled, just use vertex color
	if(DisableLighting) {
		gl_FragColor = gl_Color; //gl_FrontColor; //vec4(1, 0, 0, 1); 
		return;
	}
	vec4 ambient, diffuse, specular;
	vec3 n = normalize(N);
	vec3 l = normalize(L);
	
	// start with global ambient light
	ambient = gl_LightModel.ambient;
	diffuse = vec4(0.0);
	specular = vec4(0.0);
	
	// compute the contribution to the color from light 0 
	direct_light(0, n, l, gl_FrontMaterial.shininess,
					ambient, diffuse, specular);
					
	// add in material properties + whatever is in the uniform
	// note we are just handling the specular here rather than
	// using a secondary
	vec3 color = (//FrontLightModelProduct + 
					ambient * gl_FrontMaterial.ambient + 
					diffuse * gl_FrontMaterial.diffuse + 
					specular * gl_FrontMaterial.specular).rgb;

	gl_FragColor.rgb = clamp(color,	0.0, 1.0);
	gl_FragColor.a = gl_Color.a;
	
	// diffuse color
//	float NdotL = max(0.0, dot(N, L));
	
    // calculate diffuse lighting
    //float intensity = max(0.0, dot(NN, NL));
//    vec3 diffuse = gl_Color.rgb * NdotL;

	// halfway vec
//	vec3 H = normalize(L + vec3(0.0, 0.0, 1.0));

    // calculate specular lighting
//    vec3 specular = vec3(0.0);
//    if (NdotL > 0.0)
 //   {
//        vec3 NdotH = max(0.0, dot(N, H));
        //specular = vec3(pow(NdotL, specular_exp));
//        specular = clamp(pow(NdotH, specular_exp), 0.0, 1.0);
//    }

    // sum the diffuse and specular components
    //gl_FragColor.rgb = diffuse + specular;
//    gl_FragColor.rgb = clamp(diffuse + specular, 0.0, 1.0);
//    gl_FragColor.a = gl_Color.a;
	

}
