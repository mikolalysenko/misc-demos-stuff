module player;

import
    std.stdio,
    misc,
    vector,
    matrix,
    image,
    xform,
    md2loader,
    block,
    level,
    std.math,
    derelict.opengl.gl,
    derelict.opengl.glu;

const float COLLISION_SCALE = 0.4;

const float GRAVITY = 40.0f;
const float AIR_FRICTION = 0.2f;
const float PLAYER_SPEED = 0.3f;
const float PLAYER_AIR_SPEED = 0.02f;

const float CAMERA_DISTANCE = 30.0f;
const float PLAYER_SCALE = 0.1f;
const float STAND_ANIM_RATE = 1.0f;
const float RUN_ANIM_RATE = 2.0f;
const float JUMP_ANIM_RATE = 0.5f;

const float FOOT_BOX_SCALE = 1.0f;

const float JUMP_POWER = 30.0f;
const float DOUBLE_JUMP_POWER = 20.0f;

const float COLLISION_PUSH = 0.5f;
const float BOUNDING_BOX_SCALE_X = 12.0f;
const float BOUNDING_BOX_SCALE_Y = 12.0f;
const float BOUNDING_BOX_SCALE_Z = 22.0f;

enum KEYS
{
    UP,
    DOWN,
    LEFT,
    RIGHT,
    JUMP,
    
    NUM_KEYS,
}

bool[KEYS.NUM_KEYS] keys;

MD2Model    player_model;
Image       player_skin;


int[2][] PLAYER_ANIMS = 
[
    [0, 39],
    [40, 46],
    [66, 71],
];

void init_player()
{
    player_model = new MD2Model("data/models/fonz/tris.MD2");
    player_skin = new Image("data/models/fonz/fonz.PCX");    
}

class Player
{
    //Camera axis
    Vector  up_axis,
            top_axis,
            right_axis;
    
    //Camera tracking parameters
    float camera_height = 20.f;
    float max_camera_distance = 10.f;
    Vector camera_pos;
    
    
    enum STATE
    {
        STAND,
        RUN,
        JUMP,
    }
    
    bool can_double_jump;
    Vector pos;
    Vector vel;
    float theta;
    STATE state;
    float frame;

    this()
    {
        up_axis = Vector(0, 0, 1);
        top_axis = Vector(0, 1, 0);
        right_axis = Vector(-1, 0, 0);
        pos = Vector(0, 0, 0, 0);
        vel = Vector(0, 0, 0, 0);
        theta = 0;
        state = STATE.STAND;
        frame = 0;
        
        
        camera_pos = Vector(-25, 0, 20);
    }
    
    Matrix getMatrix()
    {
        return 
            Matrix.fromScale(PLAYER_SCALE)
                    .rotateAxis(theta, 0, 0, -1)
                    .translate(-pos);
    }
    
    Matrix getBox()
    {
        return Matrix.fromScale(BOUNDING_BOX_SCALE_X, BOUNDING_BOX_SCALE_Y, BOUNDING_BOX_SCALE_Z) * 
            Matrix.fromScale(PLAYER_SCALE)
                    .translate(-pos);
    }
    
    Matrix getFootBox()
    {
        return Matrix.fromScale(BOUNDING_BOX_SCALE_X, BOUNDING_BOX_SCALE_Y, FOOT_BOX_SCALE) * 
                Matrix.fromScale(PLAYER_SCALE)
                    .translate(-pos.x, -pos.y, -pos.z-PLAYER_SCALE * BOUNDING_BOX_SCALE_Z);
    }
    
    void set_camera()
    {
        Vector target = camera_pos - up_axis * camera_height - right_axis * 25;
        
        gluLookAt(
            -camera_pos.x, -camera_pos.y, camera_pos.z,
            -target.x, -target.y, target.z,
            up_axis.x, up_axis.y, up_axis.z);
    }
    
    void update_camera(float delta_t, Level level)
    {
        Vector delta = pos - camera_pos + 25 * right_axis;
        delta -= up_axis * up_axis.dot(delta);
        
        double d = delta.mag;
        
        if(d > max_camera_distance)
        {
            camera_pos += delta_t * delta;
        }
    }
    
    void draw()
    {
        glPushMatrix();
        
        glMultMatrix(getMatrix);
        
        glEnable(GL_TEXTURE_2D);
        player_skin.bind;
        
        float f = PLAYER_ANIMS[cast(int)state][0] * (1 - frame) + 
            PLAYER_ANIMS[cast(int)state][1] * frame;
        
        int start_f = cast(int)f;
        int end_f = start_f + 1;
        float alpha = f - cast(float)start_f;
        
        if(end_f >= PLAYER_ANIMS[cast(int)state][1])
        {
            end_f = PLAYER_ANIMS[cast(int)state][0];
        }
        
        glColor4f(1, 1, 1, 1);
        player_model.draw(alpha, start_f, end_f);
        glDisable(GL_TEXTURE_2D);
        
        
        glPopMatrix();
    }
    
    void update(float delta_t, Level game_level)
    {
        Block standing_block = game_level.testCollision(getFootBox);
        bool standing = standing_block !is null;
        
        Vector v = Vector(0, 0, 0, 0);
        
        //Handle key presses
        if(keys[KEYS.UP])
        {
            v.y -= 1;
        }
        
        if(keys[KEYS.DOWN])
        {
            v.y += 1.;
        }
        
        if(keys[KEYS.LEFT])
        {
            v.x += 1.;
        }
        
        if(keys[KEYS.RIGHT])
        {
            v.x -= 1.;
        }
        
        v = xform.xform(matrix.Matrix.getModelView.inverse, v);
        
        if(standing)
        {
            Vector ground_normal = standing_block.getHitNormal(-pos);
            v -= ground_normal * ground_normal.dot(v);
        }
        
        float m = v.mag;
        
        if(standing)
        {
            float friction = pow(standing_block.slipperiness, delta_t);
            
            vel *= friction;
            if(vel.z > 0)
                vel.z = 0;
            
            if(m > 0.1)
            {
                theta = atan2(-v.y, -v.x);
                v *= PLAYER_SPEED / m;
                state = STATE.RUN;
                frame += delta_t * RUN_ANIM_RATE;
            }
            else
            {
                state = STATE.STAND;
                frame += delta_t * STAND_ANIM_RATE;
            }
            
            if(keys[KEYS.JUMP])
            {
                
                if(abs(vel.z) < 0.8* JUMP_POWER)
                    v.z -= JUMP_POWER;
                
                keys[KEYS.JUMP] = false;
            }
            
            can_double_jump = true;
            vel += v;
        }
        else
        {
            state = STATE.JUMP;
            frame += delta_t * JUMP_ANIM_RATE;
            
            if(m > 0.1)
                vel += v * PLAYER_AIR_SPEED / m;
            
            vel.z += GRAVITY * delta_t;
            vel *= pow(AIR_FRICTION, delta_t);
            
            if(keys[KEYS.JUMP] && can_double_jump)
            {
                keys[KEYS.JUMP] = false;
                can_double_jump = false;
                
                vel.z -= DOUBLE_JUMP_POWER;
            }
        }
                
        //Update position
        pos += vel * delta_t;
        
        //Handle collisions
        Block hit_block = game_level.testCollision(getBox);
        if(hit_block !is null)
        {
            Vector hit_normal = hit_block.getHitNormal(-pos);
            
            pos -= vel * (delta_t + 0.01) + hit_normal * 0.1;
            vel -= (1 + COLLISION_SCALE) * hit_normal.dot(vel) * hit_normal;
        }
        
        while(frame > 1.0)
            frame -= 1.0;
        
        update_camera(delta_t, game_level);
    }
}