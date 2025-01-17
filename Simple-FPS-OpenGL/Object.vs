#version 330 core
layout (location = 0) in vec3 vertexPosition; 
layout (location = 1) in vec3 vertexNormal; 
out vec3 fragmentColor;
out vec3 viewpos;
out vec3 fragpos;
out vec3 normal;
out float ambientStrength;
out float specularStrength;
out float shininess;
out float diffuseStrength;

uniform mat4 Amat; 
uniform mat4 Mmat; 
uniform vec3 camVec;
uniform vec3 vertexColor;
uniform float ambientStrength_in;
uniform float specularStrength_in;
uniform float shininess_in;
uniform float diffuseStrength_in;


void main()
{
    vec4 point = vec4(vertexPosition, 1.0);
    gl_Position = Amat * point;
    
    // geometric information
    viewpos = camVec;
    fragpos = vec3(Mmat*point); // world (global) coordinates
    normal = normalize(inverse(transpose(mat3(Mmat)))*vertexNormal);

    // Material properties
    vec3 material_color = vertexColor;
    ambientStrength = ambientStrength_in;
    specularStrength = specularStrength_in;
    shininess = shininess_in;
    diffuseStrength = diffuseStrength_in;
    
    fragmentColor = material_color;

}