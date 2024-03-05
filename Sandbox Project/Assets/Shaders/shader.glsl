// #version 120

varying in vec4 vert_pos;
uniform sampler2D currentTexture;
uniform float time;
uniform vec3 LightColor;
uniform vec2 LightPos;
uniform float LightRadius;

void main()
{
    // ambient light
    vec4 ambient = vec4(0.02, 0.02, 0.02, 1);

    vec2 coord = gl_TexCoord[0].xy;
    vec4 pixel_color = texture2D(currentTexture, coord);

   // LightPos = (gl_ModelViewProjectionMatrix*vec4(LightPos, 0, 1)).xy;

    vec2 lightToFrag = LightPos - vert_pos.xy;
    //lightToFrag.y = lightToFrag.y / 1.7;

    float vecLength = clamp(length(lightToFrag)*LightRadius, 0.0, 1.0);

    //gl_FragColor = gl_Color * pixel_color * clamp(ambient + vec4(1.0-vecLength,1.0-vecLength,1.0-vecLength, 1.0), 0.0, 1.0);
    float brightness = clamp(dot(normalize(lightToFrag), vec2(0.0, -1.0)), 0.0, 1.0);
    brightness *= clamp(1.0 - (length(lightToFrag)/LightRadius), 0.0, 1.0);
    
    // gl_FragColor = pixel_color * vec4(LightColor, 1.0) * brightness;
    gl_FragColor = gl_Color * vec4(LightColor, 1.0);// * brightness;
    //gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);

}