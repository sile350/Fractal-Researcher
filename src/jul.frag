#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform vec2 resolution;
uniform vec2 juliaC; 
uniform float zoom;
uniform vec2 offset;
uniform int maxIterations;

void main() {
    vec2 z = (TexCoord - 0.5) * zoom * vec2(resolution.x / resolution.y, 1.0) + offset;
    vec2 c = juliaC;
    int i;
    for (i = 0; i < maxIterations; i++) {
        float x = z.x * z.x - z.y * z.y + c.x;
        float y = 2.0 * z.x * z.y + c.y;
        z = vec2(x, y);
        if (dot(z, z) > 4.0) break;
    }
    float t = float(i) / float(maxIterations);
    FragColor = vec4(t, t * t, sin(t * 3.14159), 1.0);
}