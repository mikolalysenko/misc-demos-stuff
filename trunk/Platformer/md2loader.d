module md2loader;

import
    std.cstream,
    std.stdio,
    std.file,
    std.math,
    derelict.opengl.gl;

// MD2 File format information
private
{
    struct MD2_HEADER
    {
        int magic;
        int md2Version;
        int skinWidth;
        int skinHeight;
        int frameSize;
        int numSkins;
        int numVertices;
        int numTexCoords;
        int numTriangles;
        int numGlCommands;
        int numFrames;
        int offsetSkins;
        int offsetTexCoords;
        int offsetTriangles;
        int offsetFrames;
        int offsetGlCommands;
        int offsetEnd;
    }

    struct MD2_VERTEX
    {
        ubyte vertex[3];
        ubyte lightNormalIndex;
    }

    struct MD2_FRAME
    {
        float scale[3];
        float translate[3];
        char name[16];
        MD2_VERTEX vertices[1];
    }

    struct MD2_TRIANGLE
    {
        short vertexIndices[3];
        short textureIndices[3];
    }

    struct MD2_TEXCOORD
    {
        short s;
        short t;
    }
}

private int round_pow2(int x)
{
    int y;
    for(y=1; y<x; y<<=1) {}
    return y;
}

private int max(int a, int b) { return a < b ? b : a; }

public class MD2Model
{       
    float[3][][] frames;
    ushort[3][] triangles;
    float[2][3][] tex_coords;
    
    public this(char[] filename)
    {
        void[] data = read(filename);
        MD2_HEADER * header = cast(MD2_HEADER*)data.ptr;

        triangles.length = header.numTriangles;
        tex_coords.length = header.numTriangles;
        frames.length = header.numFrames;
        float dim = cast(float)max(round_pow2(header.skinHeight), round_pow2(header.skinWidth));
        
        MD2_TEXCOORD[] md2_tex_coords = (cast(MD2_TEXCOORD*)(data.ptr + header.offsetTexCoords))[0..header.numTexCoords];
        MD2_TRIANGLE[] md2_triangles = (cast(MD2_TRIANGLE*)(data.ptr + header.offsetTriangles))[0..header.numTriangles];
        
        foreach(int i, MD2_TRIANGLE tri; md2_triangles)
        {
            for(int j=0; j<3; j++)
            {
                triangles[i][j] = tri.vertexIndices[j];
                tex_coords[i][j][0] = cast(float)md2_tex_coords[tri.textureIndices[j]].s / dim;
                tex_coords[i][j][1] = cast(float)md2_tex_coords[tri.textureIndices[j]].t / dim;
            }
        }
        
        //Read in frames
        for(int i=0; i<header.numFrames; i++)
        {
            MD2_FRAME * md2_frame = cast(MD2_FRAME*)(data.ptr + header.offsetFrames + i * header.frameSize);
            MD2_VERTEX[] md2_vertices = md2_frame.vertices.ptr[0..header.numVertices];

            frames[i].length = header.numVertices;
            
            foreach(int j, MD2_VERTEX vert; md2_vertices)
            {
                for(int k=0; k<3; k++)
                    frames[i][j][k] = cast(float)vert.vertex[k] * md2_frame.scale[k] + md2_frame.translate[k];
            }
        }
    }
    
    
    public void draw(float frame)
    {
        int start_f = cast(int)frame;

        if(start_f < 0) start_f = 0;
        if(start_f >= frames.length-1) start_f = frames.length - 2;

        int end_f = start_f + 1;
        
        float alpha = frame - cast(float)start_f;
        if(alpha < 0) alpha = 0;
        if(alpha > 1) alpha = 1;
            
        draw(alpha, start_f, end_f);
    }
    
    public void draw(float alpha, int start_f, int end_f)
    {
        float alpha_1 = 1.0f - alpha;
        
        glBegin(GL_TRIANGLES);
        foreach(int t, ushort[3] tri; triangles)
        {
            for(int i=0; i<3; i++)
            {
                glTexCoord2f(tex_coords[t][i][0], tex_coords[t][i][1]);
                
                float[3] v0 = frames[start_f][tri[i]];
                float[3] v1 = frames[end_f][tri[i]];
                
                glVertex3f(
                    v0[0] * alpha_1 + v1[0] * alpha, 
                    v0[1] * alpha_1 + v1[1] * alpha, 
                    v0[2] * alpha_1 + v1[2] * alpha);
            }
        }
        glEnd();
    }
}