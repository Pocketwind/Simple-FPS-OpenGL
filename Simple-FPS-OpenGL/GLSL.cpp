//GLSL
extern const char* vs = R"(
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
)";
extern const char* fs = R"(
#version 330 core
in vec3 viewpos;
in vec3 fragpos;
in vec3 normal;
in vec3 fragmentColor;
in float ambientStrength;
in float specularStrength;
in float shininess;
in float diffuseStrength;
out vec4 fragmentColorOut;

struct phLight { 
    vec3 color; 
    vec3 position; 
};

vec3 LightingModel(vec3 material_color, vec3 lightColor, 
        vec3 lightPosition, vec3 fragpos, vec3 normal, vec3 viewpos, 
        float ambientStrength_in, float specularStrength_in, float shininess_in, float diffuseStrength_in)
{
    //ambient
    vec3 ambient = ambientStrength_in*material_color*lightColor;

    // diffuse
    vec3 Lvec=normalize(lightPosition-fragpos);
    float cosTheta=max(dot(Lvec,normal),0.0);
    vec3 D=diffuseStrength_in*material_color;
    vec3 diffuse = D*cosTheta*lightColor;

    // specular
    vec3 Vvec=normalize(viewpos-fragpos);
    vec3 Rvec=2*dot(Lvec,normal)*normal-Lvec;
    float exponent=shininess_in;   //32
    float specFactor=pow(max(dot(Rvec,Vvec),0.0),exponent);
    vec3 specular = specFactor*specularStrength_in*lightColor;

    vec3 color =  ambient + diffuse + specular;
    return color;
}

void main()
{
    phLight light;
    light=phLight(vec3(1,1,1),vec3(10,10,10));
    vec3 color = LightingModel(fragmentColor,light.color,
                                light.position,fragpos,normal,viewpos,
                                ambientStrength,specularStrength,shininess,diffuseStrength);
    fragmentColorOut = vec4(color, 1.0);
}
)";

extern const char* vs_base = R"(
#version 330 core
layout (location = 0) in vec3 vertexPosition; 
layout (location = 1) in vec3 vertexColor; 
out vec3 fragmentColor;
uniform mat4 Amat; 
void main()
{
    vec4 point = vec4(vertexPosition, 1.0);
    gl_Position = Amat * point;
    fragmentColor = vertexColor;
}
)";

extern const char* fs_base = R"(
#version 330 core
in vec3 fragmentColor;
out vec4 fragmentColorOut;
void main()
{
   fragmentColorOut = vec4(fragmentColor, 1.0); // alpha
}
)";