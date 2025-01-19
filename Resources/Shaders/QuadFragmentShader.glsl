#version 330 core

out vec4 FragColor; // this is written into the first texture attached to framebuffer
out int o_EntityID;  // this is written into the second texture attached to framebuffer (and we can read that texture to get our entity id)

in vec4 v_Color;
flat in int v_EntityID;

void main()
{
	FragColor = v_Color;

	o_EntityID = v_EntityID;
}