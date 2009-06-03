#version 110

uniform sampler2D image;
vec2 size = vec2(512, 512);

void main()
{
    vec2 pos = gl_TexCoord[0].xy * size;

    float weights = 0.0;
    vec4 res = vec4(0, 0, 0, 0);
    
    for(int i=-2; i<=2; i++)
    for(int j=-2; j<=2; j++)
    {
        float w = exp(-0.5 * float(i * i + j * j)) / (2.0 * 3.14159265);
        
        weights += w;
        res += w * texture2D(image, (pos + vec2(i, j)) / size);
    }
    
    gl_FragColor = res / weights;
}