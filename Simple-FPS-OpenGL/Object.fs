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
    light=phLight(vec3(1,1,1),vec3(0,8,0));
    vec3 color = LightingModel(fragmentColor,light.color,
                                light.position,fragpos,normal,viewpos,
                                ambientStrength,specularStrength,shininess,diffuseStrength);
    fragmentColorOut = vec4(color, 1.0);
}