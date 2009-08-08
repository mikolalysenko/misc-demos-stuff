//Eigen stuff
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <Eigen/StdVector>

//Engine stuff
#include "common/sys_includes.h"
#include "common/input.h"
#include "common/simplex.h"

//Project stuff
#include "project/game.h"

//STL stuff
#include <iostream>
#include <deque>
#include <vector>
#include <cmath>
#include <algorithm>

using namespace std;
using namespace Eigen;
using namespace Common;


namespace Game
{

//Config variables
int XRes			= 640;
int YRes			= 480;
bool fullscreen		= false;
float fov			= 45.0f;
float z_near		= 0.5f;
float z_far			= 1200.0f;
float delta_t		= 0.01	;

	
float next_piece_center[3] = {0, 0, 0};
	
float slide_rate = 20.;
	
float floor 		= -300.;
float gravity[3] = {0, -.008, 0};

float row_fall_rate = 0.09;
float camera_spin_rate = 0.08;


const int shapes[7][4][4] =
{
	{{0, 0, 1, 0},
	 {0, 0, 1, 0},
	 {0, 0, 1, 0},
	 {0, 0, 1, 0}},
	
	{{0, 0, 0, 0},
	 {0, 1, 1, 0},
	 {0, 1, 1, 0},
	 {0, 0, 0, 0}},
	 
	{{0, 0, 0, 0},
	 {0, 1, 0, 0},
	 {1, 1, 1, 0},
	 {0, 0, 0, 0}},
	 
	{{0, 1, 0, 0},
	 {0, 1, 0, 0},
	 {0, 1, 1, 0},
	 {0, 0, 0, 0}},
	
	{{0, 0, 1, 0},
	 {0, 0, 1, 0},
	 {0, 1, 1, 0},
	 {0, 0, 0, 0}},
	 
	{{0, 0, 0, 0},
	 {1, 1, 0, 0},
	 {0, 1, 1, 0},
	 {0, 0, 0, 0}},
	
	{{0, 0, 0, 0},
	 {0, 0, 1, 1},
	 {0, 1, 1, 0},
	 {0, 0, 0, 0}},
};

int box_list;
void init_box_list(int res, double p)
{
	box_list = glGenLists(1);
	glNewList(box_list, GL_COMPILE);
	
	float nu[6][3] =
	{
		{1, 0, 0},
		{-1, 0, 0},
		{0, 1, 0},
		{0, -1, 0},
		{0, 0, 1},
		{0, 0, -1},
	};
	
	float nv[6][3] =
	{
		{0, 1, 0},
		{0, 1, 0},
		{0, 0, 1},
		{0, 0, 1},
		{1, 0, 0},
		{1, 0, 0},
	};
	
	float nn[6][3] = 
	{
		{0, 0, 1},
		{0, 0,-1},
		{1, 0, 0},
		{-1, 0, 0},
		{0, 1, 0},
		{0,-1, 0},
	};
	
	
	for(int f=0; f<6; f++)
	{
		for(int u=0; u<res; u++)
		{
			glBegin(GL_TRIANGLE_STRIP);
			for(int v=0; v<=res; v++)
			for(int du=1; du>=0; du--)
			{
				float x[3], xp=0,
					  dxu[3], dxv[3], dup=0, dvp=0,
					  fu = ((float)(u + du) / (float)res -.5) * 2.,
					  fv = ((float)(v) / (float)(res) - .5) * 2.;
				
				for(int i=0; i<3; i++)
				{
					x[i] = fu * nu[f][i] + fv * nv[f][i] + nn[f][i];
					dxu[i] = (fu+0.001) * nu[f][i] + fv * nv[f][i] + nn[f][i];
					dxv[i] = fu * nu[f][i] + (fv + 0.001) * nv[f][i] + nn[f][i];
					xp += pow((double)fabsf(x[i]), p);
					dup += pow((double)fabsf(dxu[i]), p);
					dvp += pow((double)fabsf(dxv[i]), p);
				}
				
				xp = pow((double)xp, 1./p);
				dup = pow((double)dup, 1./p);
				dvp = pow((double)dvp, 1./p);
				
				for(int i=0; i<3; i++)
				{
					x[i] /= xp;
					dxu[i] = dxu[i] / dup - x[i];
					dxv[i] = dxv[i] / dvp - x[i];
				}
				
				float n[3];
				n[0] = dxu[1] * dxv[2] - dxu[2] * dxv[1];
				n[1] = dxu[2] * dxv[0] - dxu[0] * dxv[2];
				n[2] = dxu[0] * dxv[1] - dxu[1] * dxv[0];
				
				float nmag = sqrt(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
				
				for(int i=0; i<3; i++)
					n[i] /= nmag;
				
				
				glNormal3fv(n);
				glVertex3fv(x);
			}
			glEnd();
		}
	}
	
	glEndList();
}


struct Palette
{
	float  bg[4],
		   board_color[4],
		   diffuse_colors[7][4],
		   ambient_colors[7][4];
	
	void setShapeParameters()
	{
		glEnable(GL_LIGHTING);
		glMaterialf(GL_FRONT_AND_BACK,
			GL_SHININESS,
			5);
		
		glDisable(GL_COLOR_MATERIAL);
		
		for(int i=1; i<16; i++)
			glDisable(GL_LIGHT0+i);
		
		glEnable(GL_LIGHT0);
		
		float white[4] = {1.,1.,1.,1.},
			  grey[4] = {.4,.4,.4,1.},
			  black[4] = {0., 0., 0., 0.};
			  
		glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 1);
		glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
			  
		glLightfv(GL_LIGHT0, GL_AMBIENT, white);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, white);
		glLightfv(GL_LIGHT0, GL_SPECULAR, white);
		glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 180);
		glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 0);
			  
		glColor4f(1.,1.,1.,1.);
	}
	
	void setShapeMaterial(int i)
	{
		glMaterialfv(GL_FRONT_AND_BACK,
			GL_AMBIENT,
			ambient_colors[i]);
		glMaterialfv(GL_FRONT_AND_BACK,
			GL_DIFFUSE,
			diffuse_colors[i]);
	}
	
	void setBoardMaterial()
	{
		glDisable(GL_LIGHTING);
		glColor4fv(board_color);
	}
	
	void setBackground()
	{
		glClearColor(bg[0], bg[1], bg[2], bg[3]);
	}
};

Palette interp_palette(const Palette& a, const Palette& b, float t)
{
	float ti = 1. - t;
	
	Palette result;
	
	for(int i=0; i<4; i++)
	{
		result.bg[i] = a.bg[i] * ti + b.bg[i] * t;
		result.board_color[i] = a.board_color[i] * ti + b.board_color[i] * t;
	}
	
	for(int i=0; i<7; i++)
	{
		for(int j=0; j<4; j++)
		{
			result.diffuse_colors[i][j] = a.diffuse_colors[i][j] * ti + b.diffuse_colors[i][j] * t;
			result.ambient_colors[i][j] = a.ambient_colors[i][j] * ti + b.ambient_colors[i][j] * t;
		}
	}
	
	return result;
}



struct Piece
{
	int r, c, rot, shape;
	float fall;
};

void rotate(int& nr, int& nc, int r, int c, int rot)
{
	switch(rot)
	{
		case 0:
			nr = r;
			nc = c;
		break;
		
		case 1:
			nr = 3 - c;
			nc = r;
		break;
		
		case 2:
			nr = 3-r;
			nc = 3-c;
		break;
		
		case 3:
			nr = c;
			nc = 3-r;
		break;
	}
}

struct Board
{
	bool game_over;
	
	long long score;
	int level;
	int rows, cols;
	float height, radius, piece_size;
	vector<int> board;
	
	double current_rotation;
	
	Piece cur, next;
	Palette pal;
	
	//Drop columns while clearing rows
	deque<int> falling_rows;
	float fall_time;
	
	bool fall_fast;
	float left_slide, right_slide;
	
	
	GLuint list;
	
	
	
	struct BoxPart
	{
		Board* board;
		
		float pos[3], vel[3],
			  base_rot, ang_rot[4],
			  omega;
		int shape;
		
		
		bool update(float delta_t)
		{
			//Update position
			for(int i=0; i<3; i++)
			{
				pos[i] += vel[i] * delta_t;
				vel[i] += gravity[i] * delta_t;
			}
			
			ang_rot[3] += omega * delta_t;
			
			return pos[1] > floor;
		}
		
		void draw()
		{
			glPushMatrix();
			
			glRotatef(base_rot, 0, 1, 0);
			glTranslatef(pos[0], pos[1], pos[2]);
			glRotatef(ang_rot[3], ang_rot[0], ang_rot[1], ang_rot[2]);
			glScalef(board->piece_size, board->piece_size, board->piece_size);
			board->pal.setShapeMaterial(shape);
			glCallList(box_list);
			
			glPopMatrix();
		}
	};

	//Box particles
	vector<BoxPart> particles;
	
	void init()
	{
		list = glGenLists(1);
		
		glNewList(list, GL_COMPILE);

		//Draw circles
		for(int r=0; r<=rows; r++)
		{
			float z = ((float)(r) / (float)(rows) - .5) * height;
			float d_theta = M_PI / 80.;
			
			glBegin(GL_LINE_LOOP);
			for(float theta=-M_PI; theta<M_PI; theta+=d_theta)
			{
				float x0 = cos(theta) * radius,
					  y0 = sin(theta) * radius,
					  x1 = cos(theta+d_theta) * radius,
					  y1 = sin(theta+d_theta) * radius;
				
				glVertex3f(x0,z,y0);
				glVertex3f(x1,z,y1);
			}
			glEnd();
		}
		
		//Draw lines
		glBegin(GL_LINES);
		for(int c=0; c<cols; c++)
		{
			float theta = (float)(c) / (float)(cols) * 2. * M_PI;
			float x = cos(theta) * radius,
					y = sin(theta) * radius;
			
			glVertex3f(x,-height/2,y);
			glVertex3f(x,height/2,y);
		}
		glEnd();

		glEndList();
		
		board.resize(rows * cols);
	}
	
	void spawn_particle(int r, int c, int x)
	{
		BoxPart tmp;
			
		tmp.board = this;
	
		tmp.pos[0] = tmp.pos[1] = tmp.pos[2] = 
		tmp.vel[0] = tmp.vel[1] = tmp.vel[2] = 0.;
		
		
		tmp.pos[0] = radius;
		tmp.pos[1] = height * (((float)(r)+.5) / rows - .5);
		tmp.vel[0] = drand48() * .3;
		tmp.vel[1] = drand48() * .5;
		
		tmp.base_rot = 360.* ((float)c + .5) / (float)cols;
		tmp.omega = drand48() * 2.;
		tmp.shape = x;
		
		float s = 0.;
		for(int i=0; i<3; i++)
		{
			tmp.ang_rot[i] = drand48() - .5;
			s += tmp.ang_rot[i] * tmp.ang_rot[i];
		}
		
		s = sqrt(s);
		for(int i=0; i<3; i++)
		{
			tmp.ang_rot[i] /= s;
		}
		tmp.ang_rot[3] = 0;
		
		particles.push_back(tmp);		
	}
	
	void reset()
	{
		game_over = false;
		
		level = 0;
		score = 0;
		
		current_rotation = cols/2;
		
		cur.c = 0;
		next_piece();
		next_piece();
		
		for(int i=0; i<rows*cols; i++)
			board[i] = -1;
	}
	
	float fall_rate()
	{
		if(fall_fast)
			return 0.2;
		
		return 0.005 * (.8 * level*level + level + 1);
	}
	
	void next_piece()
	{
		next.c = cur.c;
		cur = next;
		
		next.r = rows;
		next.c = 0;
		next.shape = (rand()>>10) % 7;
		next.rot = rand() % 4;
		
		fall_fast = false;
		left_slide = 0.;
		right_slide = 0.;
	}
	
	bool check_collision(Piece p)
	{
		for(int i=0; i<4; i++)
		for(int j=0; j<4; j++)
		{
			int nr, nc;
			rotate(nr, nc, i, j, p.rot);
			
			if(shapes[p.shape][i][j] != 0)
			{
				nr = (p.r + nr);
				if(nr < 0)
					return true;
				if(nr >= rows)
					continue;
				nc = (p.c + nc) % cols;
				if(board[nc + nr * cols] >= 0)
					return true;
			}
		}
		return false;
	}
	
	bool insert_piece(Piece p)
	{
		bool dead = false;
		for(int i=0; i<4; i++)
		for(int j=0; j<4; j++)
		{
			int nr, nc;
			rotate(nr, nc, i, j, p.rot);
			
			if(shapes[p.shape][i][j] != 0)
			{
				nr = (p.r + nr);
				if(nr >= rows)
				{
					dead = true;
					continue;
				}
				nc = (p.c + nc) % cols;
				board[nc + nr * cols] = p.shape;
			}
		}
		return dead;
	}
	
	void update(float delta_t)
	{
		for(int i=0; i<(int)particles.size(); i++)
		{
			if(!particles[i].update(delta_t))
			{
				particles[i] = particles[particles.size() - 1];
				particles.resize(particles.size() - 1);
				i--;
			}
		}
		
		//Check for row clears
		
		for(int r=0; r<rows; r++)
		{
			bool clear = true;
			for(int c=0; c<cols; c++)
			{
				if(board[c + r * cols] == -1)
				{
					clear = false;
					break;
				}
			}
			
			if(!clear)
				continue;
			
			falling_rows.push_back(r);
			
			for(int c=0; c<cols; c++)
			{
				spawn_particle(r, c, board[c + r * cols]);
				board[c+r*cols] = -1;
			}
		}
		
		sort(falling_rows.begin(), falling_rows.end());
		
		if(falling_rows.size() > 0)
		{
			fall_time += row_fall_rate * delta_t;

			while(fall_time > 1. && falling_rows.size() > 0)
			{
				for(int r=falling_rows[0]+1; r<rows; r++)
				for(int c=0; c<cols; c++)
				{
					board[c + (r-1)*cols] = board[c + r * cols];
				}
				
				falling_rows.pop_front();
				fall_time -= 1.;
				
				for(int i=0; i<falling_rows.size(); i++)
					falling_rows[i]--;
			}
		}
		
		if(falling_rows.size() == 0)
		{
			fall_time = 0.;
		}
		
	}
	
	void draw()
	{
		pal.setBackground();
		
		glPushMatrix();
		
		glRotatef(-360. * current_rotation / (float)(cols), 0, 1, 0);
		
		
		pal.setBoardMaterial();
		glCallList(list);
		
		
		pal.setShapeParameters();
		for(int c=0; c<cols; c++)
		{
			glPushMatrix();
			
			glRotatef(360. * ((float)c + .5) / (float)cols, 0, 1, 0);
			glTranslatef(radius, height * (.5/(float)(rows) - .5), 0);
			
			glScalef(piece_size, piece_size, piece_size);
			
			for(int r=0; r<rows; r++)
			{
				float fd = 0.;
				
				if(falling_rows.size() > 0 && falling_rows[0] == r)
				{
					fd = fall_time;
				}
				
				if(board[c + r * cols] >= 0)
				{
					pal.setShapeMaterial(board[c + r*cols]);
					glCallList(box_list);
				}
				
				glTranslatef(0, (1. - fd) * height/(piece_size*(float)rows), 0);
			}
			
			glPopMatrix();
		}
		
		//Draw current piece
		pal.setShapeMaterial(cur.shape);
		for(int i=0; i<4; i++)
		for(int j=0; j<4; j++)
		{
			int nr, nc;
			
			rotate(nr, nc, i, j, cur.rot);
			
			if(shapes[cur.shape][i][j] != 0)
			{
				nr += cur.r;
				nc += cur.c;
				
				glPushMatrix();
				
				
				Piece tmp = cur;
				tmp.r --;
				float fall_mult = (check_collision(tmp) ? 0 : -1);
				
				
				glRotatef(360. * ((float)nc + .5) / (float)cols, 0, 1, 0);
				glTranslatef(radius, height * (((float)nr + fall_mult * cur.fall + .5)/(float)(rows) - .5), 0);
				glScalef(piece_size, piece_size, piece_size);
				
				glCallList(box_list);
				
				glPopMatrix();
			}
		}
		
		
		for(int i=0; i<(int)particles.size(); i++)
		{
			particles[i].draw();
		}
		
		glPopMatrix();
		
		
		glPushMatrix();
		
		
		
		glPopMatrix();
	}
};
	
	
Board game_board;



Palette level_palettes[] = 
{
{
	{ 0.8, 0.9, .95, 1, },
	
	{ 0.35, 0.25, 0.3, 1. },
	
	{
		{0.34, 0.89, 0.6, 1.},
		{0.88, 0.76, 0.3, 1.},
		{0.55, 0.4, 0.1, 1.},
		{0.8, 0.8, 0.2, 1.},
		{1, 1., .2, 1},
		{0.8, 0.4, 0.8, 1.},
		{.7, .5, .3, 1},
	},
	
	{
		{0.8, 0.4, 0.7, .1},
		{0.25, 0.7, 0.6, 1.},
		{0.3, 0.6, 0.9, 1},
		{.7,.65,.3, 1},
		{.8, .5, .5, 1},
		{.7,.4,.8, 1},
		{.6,.6,.5, 1},
	},
	
},


{
	{ 0., 0., .0, 1, },
	
	{ 1, 1, 1, 1. },
	
	{
		{0.9, 0.9, 0.9, 1.},
		{0.9, 0.9, 0.9, 1.},
		{0.9, 0.9, 0.9, 1.},
		{0.9, 0.9, 0.9, 1.},
		{0.9, 0.9, 0.9, 1.},
		{0.9, 0.9, 0.9, 1.},
		{0.9, 0.9, 0.9, 1.},
	},
	
	{
		{.6, .6, .6, .1},
		{0.6, 0.1, 0.7, 1.},
		{0.8, 0.25, 0.2, 1},
		{.1,.2,.8, 1},
		{.1, .6, .3, 1},
		{.6,.6,.1, 1},
		{.1,.5,.5, 1},
	},
	
},
};


//Initialization
void init()
{
	init_box_list(10, 6.);
	
	game_board.rows = 10;
	game_board.cols = 20;
	game_board.height = 80.;
	game_board.radius = 30.;
	game_board.piece_size = 3.2;
	
	game_board.pal = level_palettes[1];
	
	game_board.init();
	game_board.reset();
	
	
}



//Update the game state
void update(float delta_t)
{
	game_board.cur.fall += delta_t * game_board.fall_rate();
	
	while(game_board.cur.fall > 1.)
	{
		game_board.cur.fall -= 1.;
		game_board.cur.r --;
		if(game_board.check_collision(game_board.cur))
		{
			game_board.fall_fast = false;
			
			game_board.cur.r ++;
			if(game_board.insert_piece(game_board.cur))
			{
				//Game over
			}
			else
			{
				game_board.next_piece();
			}
		}
	}
	
	if(key_press(SDLK_UP))
	{
		Piece tmp = game_board.cur;
		for(int i=1; i<4; i++)
		{
			tmp.rot = (game_board.cur.rot + i) % 4;
			if(!game_board.check_collision(tmp))
			{
				game_board.cur = tmp;
				break;
			}
		}
	}
	
	if(key_down(SDLK_LEFT))
	{
		game_board.left_slide -= delta_t;
	
		if(game_board.left_slide < 0.)
		{
			game_board.left_slide += slide_rate;
			Piece tmp = game_board.cur;
			tmp.c = (game_board.cur.c - 1 + game_board.cols) % game_board.cols;
			if(!game_board.check_collision(tmp))
				game_board.cur = tmp;
		}
	}
	else
		game_board.left_slide = 0.001;
	
	if(key_down(SDLK_RIGHT))
	{
		game_board.right_slide -= delta_t;
		
		if(game_board.right_slide < 0)
		{
			game_board.right_slide += slide_rate;
			
			Piece tmp = game_board.cur;
			tmp.c = (game_board.cur.c + 1) % game_board.cols;
			if(!game_board.check_collision(tmp))
				game_board.cur = tmp;
		}
	}
	else
		game_board.right_slide = 0.001;
	
	if(key_press(SDLK_DOWN))
	{
		Piece tmp = game_board.cur;
		tmp.r = (game_board.cur.r - 1);
		if(!game_board.check_collision(tmp))
			game_board.cur = tmp;
		else
		{
			game_board.cur.fall += 1.;
		}

		game_board.fall_fast = true;
	}
	
	if(key_release(SDLK_DOWN))
	{
		game_board.fall_fast = false;
	}
	
	//Adjust rotation
	if(game_board.current_rotation != game_board.cur.c+2)
	{
		float d = (float)game_board.cur.c+2. - game_board.current_rotation;
		
		if(d < -game_board.cols)
			d += game_board.cols;
		
		if(d > game_board.cols)
			d -= game_board.cols;
		
		float sign = -1;	//check for cw/ccw
		if(d <= -game_board.cols/2 ||
			(d >= 0 && d <= game_board.cols/2))
		{
			sign = 1;
		}
		
		
		double v = sign * delta_t * camera_spin_rate;
		
		if(abs(d) <= abs(v))
		{
			game_board.current_rotation = game_board.cur.c+2.;
		}
		else
		{
			game_board.current_rotation += v;
			if(game_board.current_rotation < 0)
				game_board.current_rotation += game_board.cols;
			if(game_board.current_rotation >= game_board.cols)
				game_board.current_rotation -= game_board.cols;
		}
		
	}
	
	game_board.update(delta_t);
}


//Draw stuff
void draw()
{


	glEnable(GL_FOG);
	
	glFogi(GL_FOG_MODE, GL_LINEAR);
	
	glFogf(GL_FOG_DENSITY, 1.);
	glFogf(GL_FOG_START, 100.);
	glFogf(GL_FOG_END, 800.);
	
	float light_pos[3] = {30, 150, 180};
	glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
	
	static float t = 0.;
	
	gluLookAt(
		140, 80, 0,
		0, 0, 0,
		0, 1, 0);
	
	
	game_board.draw();
	
	glDisable(GL_FOG);
}


//Handle overlays
void overlays()
{
}

};

