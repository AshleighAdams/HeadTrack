// Set by the program
uniform float time;

#ifdef VERTEX

void main(void)
{
	vec4 v = vec4(gl_Vertex);
	v.x = v.x + (cos(time + (v.x + v.z) * 5.0) * 0.05);
	gl_Position = gl_ModelViewProjectionMatrix * v;
	
	gl_TexCoord[0] = gl_MultiTexCoord0;
}

#endif

#ifdef FRAGMENT

uniform sampler2D tex;

void main(void)
{
	gl_FragColor =  texture2D(tex, gl_TexCoord[0].xy);
}

#endif