#version 440

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

uniform vec4 lgtLoc;
uniform mat4 mvpMatrix;
uniform vec4 eyePos;
uniform float watLoc;
uniform int fogEnable;

in EV_OUT
{
    float ev_height;
    vec3 vecNormal;
    vec2 coords;
    bool water;
    float prev_height;
} gs_in[];

out float lightingFactor;
out float height;
out vec2 texcoords;
out float fogStr;

void main()
{
    //Spec term is zero for terrain but value for water
    for(int i = 0; i < gl_in.length(); i++)
    {
        fogStr = 0;
        if (fogEnable == 1) {
            float dist = distance(eyePos.xyz, gl_in[0].gl_Position.xyz);
            float fogMin = 40, fogMax = 130;
            fogStr = (dist - fogMin) / (fogMax - fogMin);
        }
        
        
        vec3 lgtVec = normalize(lgtLoc.xyz - gl_in[i].gl_Position.xyz);
        float diffuseTerm = max(dot(gs_in[i].vecNormal, lgtVec),0);
        //lighting factor = diffuse term + ambient term 0.2
        lightingFactor = diffuseTerm + 0.2;
        
        gl_Position = mvpMatrix * gl_in[i].gl_Position;
        height = gs_in[i].ev_height;
        texcoords = gs_in[i].coords;

        if (gs_in[i].water){    

            vec3 viewVec = normalize(eyePos.xyz - gl_in[0].gl_Position.xyz);
            vec3 reflectDir = reflect(-lgtVec, vec3(0,1,0));

            float specTerm = pow(max(dot(viewVec, reflectDir), 0), 100);

            //calulate new lighting factor for water with normal (0,1,0)
            lightingFactor = max(dot(vec3(0,1,0), lgtVec),0);
            //calculate height difference between water level and water floor to get deeper shading
            lightingFactor *= max(1 - 2* (height - gs_in[i].prev_height), 0.3);
            lightingFactor += specTerm;
            
        } 
        
        EmitVertex();
    }
    EndPrimitive();
}