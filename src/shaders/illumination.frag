#version 410 core

const float PI = 3.14159265359;

out vec4 colorFrag;

in vec3 lightDir;
in vec3 vNormal;
in vec3 vViewPosition;

uniform vec3 ambientColor;
uniform vec3 diffuseColor;
uniform vec3 specularColor;
uniform float Ka;
uniform float Kd;
uniform float Ks;
uniform float shininess;

vec4 PhongIlluminationModel()
{
    vec3 color = Ka*ambientColor;

    vec3 N = normalize(vNormal);

    vec3 L = normalize(lightDir.xyz);

    float lambertian = max(dot(L,N), 0.0);

    if(lambertian > 0.0)
    {
      vec3 V = normalize( vViewPosition );

      vec3 R = reflect(-L, N);

      float specAngle = max(dot(R, V), 0.0);
      float specular = pow(specAngle, shininess);
      color += vec3( Kd * lambertian * diffuseColor +
                      Ks * specular * specularColor);
    }
    return vec4(color, 1.0);
}

void main(void)
{
  	colorFrag = PhongIlluminationModel();
}
