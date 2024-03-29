#define MAX_LIGHTS 8
#define NUM_LIGHTS 2
varying vec3 normal, lightDir[MAX_LIGHTS], eyeVec;
uniform int numLights;
void main()
{
  vec4 final_color = gl_FrontLightModelProduct.sceneColor;
  vec3 N = normalize(normal);
  int i;
  for (i=0; i<NUM_LIGHTS; ++i)
  {  
    vec3 L = normalize(lightDir[i]);
    float lambertTerm = dot(N,L);
    if (lambertTerm > 0.0)
    {
      final_color += gl_LightSource[i].diffuse * gl_FrontMaterial.diffuse * lambertTerm;	
      vec3 E = normalize(eyeVec);
      vec3 R = reflect(-L, N);
      float specular = pow( max(dot(R, E), 0.0), gl_FrontMaterial.shininess );
      final_color += gl_LightSource[i].specular * gl_FrontMaterial.specular * specular;	
    }
  }
  gl_FragColor = final_color;			
}
