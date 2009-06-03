module level;

import 
    vector,
    matrix,
    block,
    misc,
    level_parser,
    std.file,
    std.stdio,
    std.string,
    derelict.opengl.gl;

//Read in level, create octree

class Level
{
    Block[] blocks;
    
    this(char[] filename)
    {
        //Parse out tokens
        blocks = read_level(filename);
    }
    
    void draw()
    {
        foreach(b ; blocks)
        {
            b.draw;
        }
    }
    
    
    Block testCollision(Matrix box)
    {
        foreach(b ; blocks)
        {
            if(intersectOBB(box, b.mat))
            {
                return b;
            }
        }
        
        return null;
    }
    
}
