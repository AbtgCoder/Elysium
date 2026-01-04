#version 330 core

out vec4 FragColor; // this is written into the first texture attached to framebuffer
out int o_EntityID;  // this is written into the second texture attached to framebuffer (and we can read that texture to get our entity id)

in vec4 v_Color;
in vec3 v_LocalPos;
flat in int v_EntityID;


void main()
{
	// calulate distance and fill circle
	float r = length(v_LocalPos);

	// Full circle, no ring
	float circle = 1.0 - step(1.0, r);

	if (circle == 0.0)
    	discard;

	FragColor = v_Color;
	FragColor.a *= circle;

	o_EntityID = v_EntityID;
}