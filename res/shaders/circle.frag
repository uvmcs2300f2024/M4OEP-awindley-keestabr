#version 330 core

out vec4 FragColor;
in vec2 FragPos;

uniform vec4 shapeColor;
uniform float radius;
uniform vec2 center;

void main()
{
    // Calculate the distance from the center to the fragment
    if (length(FragPos - center) > radius) {
        // Discard all fragments outside the circle
        discard;
    }
    FragColor = shapeColor;
}
