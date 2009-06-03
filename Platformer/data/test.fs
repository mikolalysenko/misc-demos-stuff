uniform sampler2D depth_tex;

void main()
{
    vec4 depth = texture2D(depth_tex, gl_TexCoord[0].xy);
    gl_FragColor = vec4((1.0 - depth.r)*10.0, 0, 0, 0);
}