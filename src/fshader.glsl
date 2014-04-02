#version 130

out vec4 color;
in vec3 LVertexPos2D_v;

void main() {
	color = vec4(vec2(.5,.5)+ LVertexPos2D_v.xy,1,1 );
}
