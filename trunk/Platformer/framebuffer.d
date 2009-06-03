module framebuffer;

import
    image,
    derelict.opengl.glext,
    derelict.opengl.gl,
    derelict.opengl.glu,
    derelict.sdl.sdl;

private int current_fbo = 0;

/**
 * A draw buffer is a wrapper for an OpenGL frame buffer object.  These can be either displayed or
 * drawn offscreen.
 */
class DrawBuffer
{
    //Access controls for binding/unbinding the buffer
    abstract void bind();
    abstract void unbind();
    
    //Recover various elements from the framebuffer
    abstract Image texture(int t);
    abstract Image depth_texture();
    
    //Get the dimensions for the framebuffer
    abstract int width();
    abstract int height();

    //Sets aspect ratio/drawing parameters
    void setOneToOne()
    {
        //Set the viewport
        glViewport(0, 0, width(), height());
        
        //Set projection matrix
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluOrtho2D(0, 1, 0, 1);
        
        //Set modelview matrix
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
    }
    
    //Draws a fullscreen rectangle
    void drawRect()
    {
        glBegin(GL_TRIANGLES);
            glTexCoord2f(-1, 1);
            glVertex2f(-1, 1);
            glTexCoord2f(1, 1);
            glVertex2f(1, 1);
            glTexCoord2f(1, -1);
            glVertex2f(1, -1);
        glEnd();
    }
}


/**
 * A screen buffer is a viewable drawbuffer
 */
class ScreenBuffer : DrawBuffer
{
    private int xres, yres;
    
    this(int w, int h)
    {
        xres = w;
        yres = h;
    }
    
    Image texture(int t) { assert(false); return null; }
    Image depth_texture() { assert(false); return null; }
    
    void bind()
    {
        if(current_fbo != 0)
        {
            glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
            current_fbo = 0;
        }
    }
    
    void set_render_target(GLuint[] targets)
    {
        glDrawBuffers(targets.length, targets.ptr);
    }
    
    void unbind() { }
    int width() { return xres; }
    int height() { return yres; }
    
}

/**
 * A framebuffer is an offscreen draw buffer.
 */
class FrameBuffer : DrawBuffer
{
    private int xres, yres;
    private Image[] textures;
    private Image depth_tex;
    private GLuint fbo;
    private bool mipmapped = false;
    
    /**
     * Create an offscreen render target
     *  @param w : The width of the buffer
     *  @param h : The height of the buffer
     *  @param t : The number of color attachments in the buffer
     *  @param use_depth : If true, add a depth attachment to the buffer
     *  @param use_32bit : If true, use 32 bit color channels, otherwise use 16-bit
     *  @param use_mipmaps : If true, do mipmapping on the color channels
     */
    this(
        int w, int h, 
        int t = 1, 
        int color_bits = 8,
        bool use_depth = false, 
        bool use_mipmaps = false)
    in
    {
        assert(t >= 0);
        assert(t <= 16);
    }
    body
    {
        uint texture_format;
        
        switch(color_bits)
        {
            case 8:
                texture_format = GL_RGBA8;
                break;
            
            case 16:
                texture_format = GL_RGBA16F_ARB;
                break;
            
            case 32:
                texture_format = GL_RGBA32F_ARB;
                break;
            
            default: assert(false, "Unsupported texture format");
        }
        
        //Set dimensions
        xres = w;
        yres = h;
        mipmapped = use_mipmaps;
        
        //Create framebuffer
        glGenFramebuffersEXT(1, &fbo);
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
        
        //Create and bind textures
        textures = new Image[t];
        GLuint tex_ids[] = new GLuint[t];
        glGenTextures(t, tex_ids.ptr);
        
        for(int i=0; i<t; i++)
        {
            glBindTexture(GL_TEXTURE_2D, tex_ids[i]);
            
            glTexImage2D(GL_TEXTURE_2D, 
                0, 
                texture_format,
                xres, yres, 0,
                GL_RGBA,
                GL_FLOAT,
                null);
            
            if(use_mipmaps)
            {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
                glGenerateMipmapEXT(GL_TEXTURE_2D);
            }
            else
            {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
            }
            
            glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
                GL_COLOR_ATTACHMENT0_EXT + i,
                GL_TEXTURE_2D, tex_ids[i], 0);
            
            textures[i] = new Image(tex_ids[i], xres, yres);
        }
        
        //Create depth buffer
        if(use_depth)
        {
            GLuint depth_id;
            glGenTextures(1, &depth_id);
            
            glBindTexture(GL_TEXTURE_2D, depth_id);
            
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
            
            glTexImage2D(GL_TEXTURE_2D, 0,
                GL_DEPTH24_STENCIL8_EXT,
                xres, yres, 0,
                GL_DEPTH_STENCIL_EXT,
                GL_UNSIGNED_INT_24_8_EXT,
                null);
            
            glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
                GL_DEPTH_ATTACHMENT_EXT,
                GL_TEXTURE_2D, depth_id, 0);
            
            glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
                GL_STENCIL_ATTACHMENT_EXT,
                GL_TEXTURE_2D, depth_id, 0);
            
            depth_tex = new Image(depth_id, xres, yres);
        }
        else
        {
            depth_tex = null;
        }
        
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    }
    
    //Accessor methods
    int width() { return xres; }
    int height() { return yres; }
    Image texture(int t) { return textures[t]; }
    Image depth_texture() { return depth_tex; }
    
    //Bind the framebuffer
    void bind()
    {
        if(current_fbo != fbo)
        {
            glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
            current_fbo = fbo;
        }
    }
    
    //Unbind and fix up mipmaps
    void unbind()
    {
        if(mipmapped)
        foreach(t; textures)
        {
            glBindTexture(GL_TEXTURE_2D, t.texture);
            glGenerateMipmapEXT(GL_TEXTURE_2D);
        }
    }
}