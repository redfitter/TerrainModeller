#version 330

uniform sampler2D Snowimage;
uniform sampler2D Rockimage;
uniform sampler2D Waterimage;
uniform sampler2D Grassimage;
uniform float watLoc;
uniform float snwLoc;

in float height;
in float lightingFactor;
in vec2 texcoords;
in float fogStr;

void main() 
{
     vec4 colour = vec4(0, 0, 1, 1);
     vec4 SnowTex  = texture(Snowimage, texcoords);
     vec4 RockTex  = texture(Rockimage, texcoords);
     vec4 WaterTex = texture(Waterimage, texcoords);
     vec4 GrassTex = texture(Grassimage, texcoords);

     float grassStr = 0, waterStr = 0, rockStr = 0, snowStr = 0;
     //Get the strength of the texture at the height of the coordinates
     snowStr = max(0, min((10.0*(height - snwLoc + 0.1)), 1));
     rockStr = max(0, min((10.0*(height - snwLoc + 0.3)), 1)) * min(1, max((-3.0*(height - snwLoc - 0.3)), 0));
     grassStr = min(1, max((-10.0*(height - snwLoc + 0.2)), 0));

     //if is water texture overwrite equations above.
     if (height < (watLoc + 0.000001)){
          waterStr = 1;
          grassStr = 0;
          rockStr = 0;
          snowStr = 0;
     }
     colour = (SnowTex * snowStr) +  (RockTex * rockStr) + ((WaterTex) * waterStr) + (GrassTex * grassStr);

     //adding the fog strength
     colour = max((1 - fogStr) * colour + fogStr * vec4(1.0), vec4(fogStr));

     // Uncomment following line to see flat colour for lighting.
     //colour = vec4(0,1,0,0);
     gl_FragColor = colour * lightingFactor;
}
