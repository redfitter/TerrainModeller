#version 330

layout (location = 0) in vec4 position;

void main()
{
   //Simple pass through shader
   gl_Position = position;
  
}
