varying vec4 color;

void main()
{
    gl_Position = ftransform();
    color = vec4(gl_Normal.xyz, 0);
    gl_TexCoord[0] = gl_MultiTexCoord0;
}