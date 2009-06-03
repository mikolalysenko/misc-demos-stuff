module image;

import
    shader,
    std.math,
    std.stdio,
    std.string,
    derelict.sdl.sdl,
    derelict.sdl.image,
    derelict.opengl.gl,
    derelict.opengl.glu;
    
    
private int round_pow2(int x)
{
    int y;
    for(y=1; y<x; y<<=1) {}
    return y;
}

private int max(int a, int b) { return a < b ? b : a; }

/**
 * Reads in an image
 */
public float[4][][] read_pixels(char[] filename)
{
    SDL_Surface * raw_image = IMG_Load(toStringz(filename));
    assert(raw_image !is null, "Could not find: " ~ filename);

    //Convert image to standard format (need to check endianness here
    int dim = max(round_pow2(raw_image.w), round_pow2(raw_image.h));
    SDL_Surface * image = SDL_CreateRGBSurface(SDL_SWSURFACE, dim, dim, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
    assert(image !is null);
    
    //Copy images
    SDL_BlitSurface(raw_image, null, image, null);    
    
    //Convert to internal d format
    float[4][][] result = new float[4][][image.h];
    
    for(int r=0; r<image.h; r++)
    {
        result[r] = new float[4][image.w];
        
        for(int c=0; c<image.w; c++)
        {
            for(int d=0; d<4; d++)
            {
                result[r][c][d] =  
                    cast(float)((cast(ubyte*)image.pixels)[4 * (r * image.w + c) + d]) / 255.0f;
            }
        }
    }
    
    //Free buffers
    SDL_FreeSurface(raw_image);
    SDL_FreeSurface(image);
    
    return result;
}


public class Image : UniformAttachment
{
    public GLuint texture;
    public int width, height;
    
    /**
     * Create an image
     */
    public this(float[4][][] pixels, bool mipmap = true)
    {
        height = pixels.length;
        width = pixels[0].length;

        //Convert to standard format
        float[4][] tmp_buf;
        foreach(row; pixels)
            tmp_buf ~= row;

        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        
        if(mipmap)
        {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST);
            
            gluBuild2DMipmaps(GL_TEXTURE_2D,
                GL_RGBA,
                width, height, 
                GL_RGBA,
                GL_FLOAT,
                tmp_buf[0].ptr);
        }
        else
        {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            
            glTexImage2D(GL_TEXTURE_2D,
                0,
                GL_RGBA,
                width, height,
                0,
                GL_RGBA,
                GL_FLOAT,
                tmp_buf[0].ptr);
        }
    }
    
    public this(char[] filename, bool mipmap = true)
    {
        SDL_Surface * raw_image = IMG_Load(toStringz(filename));
        assert(raw_image !is null, "Could not find: " ~ filename);

        //Convert image to standard format (need to check endianness here
        int dim = max(round_pow2(raw_image.w), raw_image.h);
        SDL_Surface * image = SDL_CreateRGBSurface(SDL_SWSURFACE, dim, dim, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
        assert(image !is null);
        
        //Copy images
        SDL_BlitSurface(raw_image, null, image, null);
        
        width = image.w;
        height = image.h;
        
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        
        if(mipmap)
        {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST);
            
            gluBuild2DMipmaps(GL_TEXTURE_2D,
                GL_RGBA,
                width, height, 
                GL_RGBA,
                GL_UNSIGNED_BYTE,
                image.pixels);
        }
        else
        {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            
            glTexImage2D(GL_TEXTURE_2D,
                0,
                GL_RGBA,
                width, height,
                0,
                GL_RGBA,
                GL_UNSIGNED_BYTE,
                image.pixels);
        }

        SDL_FreeSurface(raw_image);
        SDL_FreeSurface(image);
    }
    
    public this(GLuint texnum, int w, int h)
    {
        texture = texnum;
        width = w;
        height = h;
    }
    
    void bindUniform(GLint loc, inout int texture_id)
    {
        glActiveTexture(texture_id + GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(loc, texture_id);
        texture_id ++;
    }
    
    void bind()
    {
        glBindTexture(GL_TEXTURE_2D, texture);
    }
}