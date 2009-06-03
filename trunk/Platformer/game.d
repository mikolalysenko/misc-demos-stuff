module game;

import
    level,
    pingpong,
    framebuffer,
    shader,
    platform,
    md2loader,
    image,
    player,
    block,
    misc,
    vector,
    matrix,
    std.stdio,
    std.string,
    std.math,
    derelict.sdl.sdl,
    derelict.sdl.image,
    derelict.opengl.gl,
    derelict.opengl.glu;

const int X_RES = 800;
const int Y_RES = 600;
const char[] WINDOW_NAME = "GameMode";


ScreenBuffer scr;
Player player_actor;
Level game_level;

void init()
{
    game_level = new Level("data/levels/level0.txt");
    
    scr = new ScreenBuffer(X_RES, Y_RES);
    
    init_player;
    player_actor = new Player();
}

void update(float delta_t)
{
    player_actor.update(delta_t, game_level);
}

void draw()
{
    //Update the screen
    scr.bind;
    
    //Set camera / background
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.4,0.3,0.9,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45, 1, 0.1, 1000);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    //Set camera
    player_actor.set_camera();
    
    //Draw player
    player_actor.draw;
    
    //Draw level
    game_level.draw;
    
    /*
    Block c = game_level.testCollision(player_actor.getFootBox);
    
    if(c is null)
        glColor3f(0, 1, 0);
    else
        glColor3f(1, 0, 0);
    
    glBegin(GL_POINTS);
        foreach(v ; getOBBVerts(player_actor.getFootBox))
            glVertex3(v);
    glEnd();
    */

    scr.unbind;
}

void keyPress(SDLKey key)
{
    switch(key)
    {
        case SDLK_UP:
            keys[KEYS.UP] = true;
        break;
        
        case SDLK_DOWN:
            keys[KEYS.DOWN] = true;
        break;
        
        case SDLK_LEFT:
            keys[KEYS.LEFT] = true;
        break;
        
        case SDLK_RIGHT:
            keys[KEYS.RIGHT] = true;
        break;
        
        case ' ':
            keys[KEYS.JUMP] = true;
        break;
        
        default: break;
    }
}

void keyRelease(SDLKey key)
{
    switch(key)
    {
        case SDLK_UP:
            keys[KEYS.UP] = false;
        break;
        
        case SDLK_DOWN:
            keys[KEYS.DOWN] = false;
        break;
        
        case SDLK_LEFT:
            keys[KEYS.LEFT] = false;
        break;
        
        case SDLK_RIGHT:
            keys[KEYS.RIGHT] = false;
        break;
        
        case ' ':
            keys[KEYS.JUMP] = false;
        break;
        
        default: break;
    }
}

int main()
{
    Platform.init();
    DerelictSDL.load();
    DerelictSDLImage.load();
    DerelictGL.load();
    DerelictGLU.load();

    // initialize SDL's VIDEO module
    SDL_Init(SDL_INIT_VIDEO);
    scope(exit) SDL_Quit();

    // Set attributes
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    // create our OpenGL window
    SDL_SetVideoMode(X_RES, Y_RES, 32, SDL_OPENGL | SDL_HWPALETTE);
    SDL_WM_SetCaption(toStringz(WINDOW_NAME), null);

    //Now that the context is loaded, we can initialize extensions
    DerelictGL.loadVersions(GLVersion.Version20);
    DerelictGL.loadExtensions();

    init();
    
    uint prev_ticks = SDL_GetTicks();
    
    mainLoop:
    while (true)
    {
        SDL_Event event;

        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                // user has clicked on the window's close button
                case SDL_QUIT:
                    break mainLoop;
                
                case SDL_KEYDOWN:
                    if(event.key.keysym.sym == SDLK_ESCAPE)
                    {
                        break mainLoop;
                    }
                    else
                    {
                        keyPress(event.key.keysym.sym);
                    }
                break;
                    
                case SDL_KEYUP:
                    keyRelease(event.key.keysym.sym);
                break;

                // by default, we do nothing => break from the switch
                default:
                    break;
            }
        }
        
        uint cur_ticks = SDL_GetTicks();
        float delta_t = (cast(float)cur_ticks - cast(float)prev_ticks) / 1000.0f;
        prev_ticks = cur_ticks;
        update(delta_t);

        draw();

        SDL_GL_SwapBuffers();
    }
    
    return 0;
}
