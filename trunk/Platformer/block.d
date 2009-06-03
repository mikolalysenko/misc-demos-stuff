module block;

import
    vector,
    matrix,
    xform,
    misc,
    std.stdio,
    std.math,
    derelict.opengl.gl;

const float CUTOFF = 0.577;

//A block
class Block
{
    float slipperiness;
    Matrix mat;
    Vector color;
    
    this(Matrix mat, Vector color, float slip)
    {
        this.slipperiness = slip;
        this.mat = mat;
        this.color = color;
    }
    
    void draw()
    {
        glPushMatrix();
        
        glMultMatrix(mat);
        
        glBegin(GL_QUADS);
        
            glColor4(color);
        
            glVertex3f( 1, 1, 1);
            glVertex3f(-1, 1, 1);
            glVertex3f(-1,-1, 1);
            glVertex3f( 1,-1, 1);
        
            glVertex3f( 1, 1,-1);
            glVertex3f(-1, 1,-1);
            glVertex3f(-1,-1,-1);
            glVertex3f( 1,-1,-1);
        
            glVertex3f( 1, 1, 1);
            glVertex3f(-1, 1, 1);
            glVertex3f(-1, 1,-1);
            glVertex3f( 1, 1,-1);

            glVertex3f( 1,-1, 1);
            glVertex3f(-1,-1, 1);
            glVertex3f(-1,-1,-1);
            glVertex3f( 1,-1,-1);
        
            glVertex3f( 1, 1, 1);
            glVertex3f( 1,-1, 1);
            glVertex3f( 1,-1,-1);
            glVertex3f( 1, 1,-1);
        
            glVertex3f(-1, 1, 1);
            glVertex3f(-1,-1, 1);
            glVertex3f(-1,-1,-1);
            glVertex3f(-1, 1,-1);

        glEnd();
        
        glPopMatrix();
    }
    
    Vector getHitNormal(Vector pos)
    {
        pos.w = 1;
        pos = xform.xform(mat.inverse, pos);
        pos.w = 0;
        
        float m = pos.mag;
        
        if(m < 0.001)
            return Vector(0, 0, 0);
        
        pos /= m;
        
        //Threshold
        for(int i=0; i<3; i++)
        {
            if(pos[i] < -CUTOFF)
                pos[i] = -1;
            else if(pos[i] > CUTOFF)
                pos[i] = 1;
            else
                pos[i] = 0;
        }
        
        Vector v = xform.xform(mat, pos);
        v.w = 0;
        return v.norm;
    }
}
