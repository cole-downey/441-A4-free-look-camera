#version 120

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 ka;
uniform vec3 kd;
uniform vec3 ks;
uniform float s;

varying vec3 vertPos; 
varying vec3 norm;

void main()
{
	vec3 n = normalize(norm);
	vec3 l = normalize(lightPos - vertPos);
	vec3 eye = normalize(vec3(0) - vertPos);
	vec3 h = normalize(l + eye);

	vec3 ca = ka;
	vec3 cd = kd * max(0, dot(l, n));
	vec3 cs = ks * pow(max(0, dot(h, n)), s);
	vec3 color = vec3(0.0f);
	color.r = lightColor.r * (ca.r + cd.r + cs.r);
	color.g = lightColor.g * (ca.g + cd.g + cs.g);
	color.b = lightColor.b * (ca.b + cd.b + cs.b);
	gl_FragColor = vec4(color, 1.0);

}
