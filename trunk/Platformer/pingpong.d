module pingpong;

import
    framebuffer,
    shader,
    image,
    derelict.opengl.glext,
    derelict.opengl.gl;


/**
 * A PingPong buffer is a buffer which supports front/back swapping
 */
class PingPong
{
    private FrameBuffer fbo;
    private int current_buffer, num_targets;
    private UniformAttachment size_attachment;
    private float size[2];
    
    int width() { return fbo.width; }
    int height() { return fbo.height; }
    
    /**
     * Creates a pingpong buffer
     */
    this(int w, int h, int t, int d)
    {
        //Create framebuffer
        fbo = new FrameBuffer(w, h, t * 2, d);
        
        //Set buffer values
        num_targets = t;
        current_buffer = 0;
        
        //Size attachment
        size[0] = w;
        size[1] = h;
        size_attachment = new Attachment!(float, 2)(&size);
    }
    
    /**
     * Binds the ping pong buffer and sets active render targets
     */
    void bind()
    {
        static GLuint color_buffers[] =
        [
            GL_COLOR_ATTACHMENT0_EXT,
            GL_COLOR_ATTACHMENT1_EXT,
            GL_COLOR_ATTACHMENT2_EXT,
            GL_COLOR_ATTACHMENT3_EXT,
            GL_COLOR_ATTACHMENT4_EXT,
            GL_COLOR_ATTACHMENT5_EXT,
            GL_COLOR_ATTACHMENT6_EXT,
            GL_COLOR_ATTACHMENT7_EXT,
            GL_COLOR_ATTACHMENT8_EXT,
            GL_COLOR_ATTACHMENT9_EXT,
            GL_COLOR_ATTACHMENT10_EXT,
            GL_COLOR_ATTACHMENT11_EXT,
            GL_COLOR_ATTACHMENT12_EXT,
            GL_COLOR_ATTACHMENT13_EXT,
            GL_COLOR_ATTACHMENT14_EXT,
            GL_COLOR_ATTACHMENT15_EXT,
        ];
        
        //Bind frame buffer
        fbo.bind;
        fbo.setOneToOne;
        
        //Set render targets
        glDrawBuffers(num_targets, &color_buffers[current_buffer * num_targets]);
    }
    
    /**
     * Use a single image
     */
    void init(Image data)
    {
        init([data]);
    }
    
    /**
     * Initialize the buffer with image data
     */
    void init(Image data[])
    in
    {
        assert(data.length == num_targets);
    }
    body
    {
        //Bind the buffer
        bind;
        fbo.setOneToOne;
        ShaderProg.disable;
        
        //Set new buffers
        glActiveTexture(GL_TEXTURE0);
        glEnable(GL_TEXTURE_2D);
        for(int i=0; i<data.length; i++)
        {
            glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT + i + current_buffer * num_targets);
            data[i].bind;
            fbo.drawRect;
        }
        glDisable(GL_TEXTURE_2D);
        
        //Swap
        swap;
    }
    
    /**
     * Clears out the buffer
     */
    void clear(float[] data)
    in
    {
        assert(data.length == 4 * num_targets);
    }
    body
    {
        bind;
        for(int i=0; i<data.length; i+=4)
        {
            glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT + (i/4) + current_buffer * num_targets);
            glClearColor(data[i], data[i+1], data[i+2], data[i+3]);
            glClear(GL_COLOR_BUFFER_BIT);
        }
        swap;
    }
    
    
    
    /**
     * Bind uniforms, execute shader program
     */
    void apply(ShaderProg prog)
    {
        //Set draw buffers
        static char[7] buf_name = "buffer0";
        
        for(int i=0; i<num_targets; i++)
        {
            buf_name[$-1] = i + '0';
            prog.attach(buf_name[0..$], texture(i));
        }
        
        //Bind size
        prog.attach("dims", size_attachment);
        
        bind;
        prog.bind;
        fbo.drawRect;
        swap;
    }
    
    /**
     * Swap the buffers
     */
    void swap()
    {
        current_buffer ^= 1;
    }
    
    /**
     * Get textures from the back buffer
     */
    Image texture(int i)
    {
        return fbo.texture((current_buffer ^ 1) * num_targets + i);
    }    
}