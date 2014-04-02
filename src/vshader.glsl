#version 130

uniform mat4 projection;

in vec3 position;
out vec3 LVertexPos2D_v;
void main() {
	gl_Position = vec4( position.x, position.y, 0, 1 );
	LVertexPos2D_v=position;
}
