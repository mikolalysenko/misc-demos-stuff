module shader;

import 
    std.file,
    std.string,
    derelict.opengl.gl;

interface UniformAttachment
{
    void bindUniform(GLint location, inout int texture_id);
}

class Shader
{
    this(char[] filename, GLenum type)
    {
        assert(exists(filename));
        this(filename, cast(char[])read(filename), type);
    }
    
    this(char[] filename, char[] source, GLenum type)
    {
        shader = glCreateShader(type);
        char * cp = source.ptr;
        int ip = source.length;
        glShaderSource(
            shader, 
            1, 
            &cp, 
            &ip);
        glCompileShader(shader);
        
        int status;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
        
        if(status != GL_TRUE)
        {
            int len;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
            char[] log = new char[len];
            glGetShaderInfoLog(shader, log.length, &len, log.ptr);
            log.length = len;
            throw new Exception("Shader Compile Error (" ~ filename ~ "):\n" ~ log);
        }
    }
    
    ~this()
    {
        glDeleteShader(shader);
    }
    
    GLuint shader;
}

class VertShader : Shader
{
    this(char[] filename) { super(filename, GL_VERTEX_SHADER); }
    this(char[] filename, char[] src) { super(filename, src, GL_VERTEX_SHADER); }
}

class FragShader : Shader
{
    this(char[] filename) { super(filename, GL_FRAGMENT_SHADER); }
    this(char[] filename, char[] src) { super(filename, src, GL_FRAGMENT_SHADER); }
}

class ShaderProg
{
    GLuint prog;
    GLuint[char[]] uniform_vars;
    UniformAttachment[GLuint] attachments;
    VertShader vert_s;
    FragShader frag_s;
    
    this(VertShader vs, FragShader fs)
    {
        vert_s = vs;
        frag_s = fs;
        
        prog = glCreateProgram();
        if(vs !is null)
            glAttachShader(prog, vs.shader);
        if(fs !is null)
            glAttachShader(prog, fs.shader);
        glLinkProgram(prog);
        
        int status;
        glGetProgramiv(prog, GL_LINK_STATUS, &status);
        
        if(status != GL_TRUE)
        {
            int len;
            glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
            
            char[] log = new char[len];
            glGetProgramInfoLog(prog, log.length, &len, log.ptr);
            log.length = len;
            throw new Exception("Shader Program Compile Error:\n" ~ log);
        }
    }

    this(FragShader fs)
    {
        this(null, fs);
    }
    
    this(VertShader vs)
    {
        this(vs, null);
    }
    
    ~this()
    {
        glDeleteProgram(prog);
    }
    
    void bind()
    {
        glUseProgram(prog);
        int texnum = 0;
        foreach(GLuint loc, UniformAttachment att; attachments)
        {
            att.bindUniform(loc, texnum);
        }
    }
    
    void attach(char[] str, UniformAttachment att)
    {
        GLint loc = -1;
        
        if(str in uniform_vars)
            loc = uniform_vars[str];
        else
            loc = glGetUniformLocation(prog, toStringz(str));
        
        if(loc == -1)
            return;
        
        attachments[loc] = att;
    }
    
    static void disable()
    {
        glUseProgram(0);
    }
}

class Attachment(T, uint D=1) : UniformAttachment
{
    T[D]* value;
    
    static if(D == 1)
    {
        this(T* value)
        {
            this.value = cast(T[1]*)value;
        }
    }

    this(T[D]* value)
    {
        this.value = value;
    }
    
    void bindUniform(GLint loc, inout int texture_id)
    {
        static if(is(T == int))
        {
            static if(D == 1)
            {
                glUniform1iv(loc, 1, (*value).ptr);
            }
            else static if(D == 2)
            {
                glUniform2iv(loc, 1, (*value).ptr);
            }
            else static if(D == 3)
            {
                glUniform3iv(loc, 1, (*value).ptr);
            }
            else static if(D == 4)
            {
                glUniform4iv(loc, 1, (*value).ptr);
            }
        }
        else static if(is(T == float))
        {
            static if(D == 1)
            {
                glUniform1fv(loc, 1, (*value).ptr);
            }
            else static if(D == 2)
            {
                glUniform2fv(loc, 1, (*value).ptr);
            }
            else static if(D == 3)
            {
                glUniform3fv(loc, 1, (*value).ptr);
            }
            else static if(D == 4)
            {
                glUniform4fv(loc, 1, (*value).ptr);
            }
        }
    }
}

