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
#include <fstream>
#include <iostream>
#include <deque>
#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdlib>
#include <ctime>

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

bool quit = false;
	

float cursor_pos = 0.;
float intro_timer = 0;
float intro_text_rate = 50.;
float intro_delay = 1600;
	
bool paused;
int paused_menu_item;
	
float level_interp_time = 120.;
float game_over_seq_time= 140.;
float side_piece_rot[3];
float menu_intro_time = 70.;
float next_piece_center[3] = {0, 0, 0};
float slide_rate = 20.;
float floor 		= -300.;
float gravity[3] = {0, -.008, 0};
float row_fall_rate = 0.09;
float camera_spin_rate = 0.08;


//Currently selected menu item

int menu_item = 0;

enum MENU
{
	MENU_MAIN,
	MENU_INSTRUCTIONS,
	MENU_HIGH_SCORES,
};

MENU menu = MENU_MAIN;


enum STATE
{
	INTRO_STATE,
	MENU_STATE,
	GAME_STATE,
};

STATE app_state;

const int shapes[7][4][4] =
{
	{{0, 1, 0, 0},
	 {0, 1, 0, 0},
	 {0, 1, 0, 0},
	 {0, 1, 0, 0}},
	
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

struct HighScore
{
	int level, score, lines;
	
	bool operator<(const HighScore& other) const
	{
		return score > other.score;
	}
};

vector<HighScore> high_scores;

void load_scores()
{
	ifstream score_file("scores.txt");
	
	HighScore tmp;
	while(score_file >> tmp.level >> tmp.score >> tmp.lines)
	{
		high_scores.push_back(tmp);
	}
	
	sort(high_scores.begin(), high_scores.end());
}

void save_scores()
{
	ofstream score_file("scores.txt");
	sort(high_scores.begin(), high_scores.end());
	for(int i=0; i<min((int)high_scores.size(), 100); i++)
	{
		score_file << high_scores[i].level << ' '
			 << high_scores[i].score << ' '
			 << high_scores[i].lines << endl;
	}
}

void add_score(HighScore hs)
{
	high_scores.push_back(hs);
	save_scores();
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
	
	void setTextMode()
	{
		glDisable(GL_LIGHTING);
		glColor4fv(board_color);
	}
};

#define NUM_PALETTES  8

Palette level_palettes[NUM_PALETTES] = 
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


{
	{ .73, .64, .4, 1, },
	
	{ 0, 0, 0, 1. },
	
	{
		{.3, .3, .3, 1},
		{.3, .3, .3, 1},
		{.3, .3, .3, 1},
		{.3, .3, .3, 1},
		{.3, .3, .3, 1},
		{.3, .3, .3, 1},
		{.3, .3, .3, 1},
	},
	
	{
		{ .3, 0, .3, 1. },
		{ .1, .35, .2, 1. },
		{.05, .05, .4, 1},
		{.23, .3, .05, 1},
		{.05, .27, .33, 1},
		{.1, .43, .15, 1},
		{.1, .1, .03, 1},
	},
	
},

{
	{ 1, 1, 1, 1, },
	
	{ 0, 0, 0, 1. },
	
	{
		{1, 1, 1, 1},
		{1, 1, 1, 1},
		{1, 1, 1, 1},
		{1, 1, 1, 1},
		{1, 1, 1, 1},
		{1, 1, 1, 1},
		{1, 1, 1, 1},
	},
	
	{
		{0, 0, 0, 1},
		{0, 0, 0, 1},
		{0, 0, 0, 1},
		{0, 0, 0, 1},
		{0, 0, 0, 1},
		{0, 0, 0, 1},
		{0, 0, 0, 1},
	},
	
},

{
	{ .1, .08, .23, 1, },
	
	{ 1, 1, 1, 1. },
	
	{
		{1, 1, 1, 1},
		{1, 1, 1, 1},
		{1, 1, 1, 1},
		{1, 1, 1, 1},
		{1, 1, 1, 1},
		{1, 1, 1, 1},
		{1, 1, 1, 1},
	},
	
	{
		{0.75, 0.63, 0.35, 1},
		{0.7, 0.7, 0.7, 1},
		{0.75, 0.63, 0.35, 1},
		{0.7, 0.7, 0.7, 1},
		{0.75, 0.63, 0.35, 1},
		{0.7, 0.7, 0.7, 1},
		{0.75, 0.63, 0.35, 1},
	},
	
},




{
	{ .7, .5, 1, 1, },
	
	{ 0, 0, 0, 1. },
	
	{
		{1, 1, 1, 1},
		{1, 1, 1, 1},
		{1, 1, 1, 1},
		{1, 1, 1, 1},
		{1, 1, 1, 1},
		{1, 1, 1, 1},
		{1, 1, 1, 1},
	},
	
	{
		{1, 1, 0, 1},
		{0.333, 1, 0, 1},
		{0.89, 0, .4, 1},
		{0.6, 0, .2, 1},
		{0.1, 0.55, 0, 1},
		{0.7, 0.7, 0.7, 1},
		{0.55, .55, 0, 1},
	},
	
},


{
	{ 0, 0, 0, 1, },
	
	{ 1, 1, 1, 1. },
	
	{
		{1, 1, 1, 1},
		{1, 1, 1, 1},
		{1, 1, 1, 1},
		{1, 1, 1, 1},
		{1, 1, 1, 1},
		{1, 1, 1, 1},
		{1, 1, 1, 1},
	},
	
	{
		{0.6, 0.6, 0.6, 1},
		{0.6, 0.6, 0.6, 1},
		{0.6, 0.6, 0.6, 1},
		{0.6, 0.6, 0.6, 1},
		{0.6, 0.6, 0.6, 1},
		{0.6, 0.6, 0.6, 1},
		{0.6, 0.6, 0.6, 1},
	},
	
},

	
{
	{ 0, .45, .45, 1. },
	{ .66, 1, 0, 1, },
	
	
	{
		{1, 1, 1, 1},
		{1, 1, 1, 1},
		{1, 1, 1, 1},
		{1, 1, 1, 1},
		{1, 1, 1, 1},
		{1, 1, 1, 1},
		{1, 1, 1, 1},
	},
	
	{
		{1, 0, 0, 1},
		{.7, 0, 0, 1},
		{1, .7, .7, 1},
		{1, .5, .5, 1},
		{.1, .1, .1, 1},
		{.6, 0, 0, 1},
		{.3, .3, .3, 1},

	},
	
},


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
	bool palette_interp;
	float palette_interp_t;
	
	bool game_over;
	float game_over_time;
	
	long long score;
	int level, lines;
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
	
	
	bool intro_menu;
	float intro_pos;
	
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
		palette_interp = false;
		game_over = false;
		
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
		
		
		pal = level_palettes[0];
		
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
		
		palette_interp = false;
		
		pal = level_palettes[0];
		
		game_over = false;
		
		level = 0;
		score = 0;
		lines = 0;
		
		current_rotation = cols/2;
		
		cur.c = 0;
		next_piece();
		next_piece();
		
		for(int i=0; i<rows*cols; i++)
			board[i] = -1;
		
		intro_pos = 0.;
		intro_menu = true;
	}
	
	float fall_rate()
	{
		if(fall_fast)
			return 0.2;
		
		return 0.002 * (level + 1);
	}
	
	void next_piece()
	{
		next.c = cur.c;
		cur = next;
		
		next.r = rows;
		next.c = 0;
		next.shape = (rand()>>10) % 7;
		next.rot = rand() % 4;
		
		for(int i=0; i<3; i++)
			side_piece_rot[i] = drand48() * 360.;
		
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
				}
				nc = (p.c + nc) % cols;
				if(dead)
					spawn_particle(nr, nc, p.shape);
				else
					board[nc + nr * cols] = p.shape;
			}
		}
		return dead;
	}
	
	void update(float delta_t)
	{
		if(palette_interp)
		{
			Palette prev = level_palettes[(level-2) % NUM_PALETTES],
					next = level_palettes[(level-1) % NUM_PALETTES];
			
			palette_interp_t += delta_t / level_interp_time;
			
			if(palette_interp_t >= 1.)
			{
				palette_interp = false;
				pal = next;
			}
			else
				pal = interp_palette(prev, next, palette_interp_t);
		}
		
		if(!game_over)
		{
			intro_pos += delta_t / menu_intro_time;
			
			if(intro_pos >= 1.)
			{
				intro_pos = 1.;
				intro_menu = false;
			}
		}
		for(int i=0; i<(int)particles.size(); i++)
		{
			if(!particles[i].update(delta_t))
			{
				particles[i] = particles[particles.size() - 1];
				particles.resize(particles.size() - 1);
				i--;
			}
		}
		
		if(!game_over)
		{
			//Check for row clears
			int n_clear = 0;
			
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
				
				n_clear ++;
				
				falling_rows.push_back(r);
				
				for(int c=0; c<cols; c++)
				{
					spawn_particle(r, c, board[c + r * cols]);
					board[c+r*cols] = -1;
				}
			}
			
			
			if((lines % 10) + n_clear >= 10 && app_state == GAME_STATE)
			{
				palette_interp = true;
				palette_interp_t = 0.;
			}
			
			lines += n_clear;
			score += 100 * n_clear * n_clear * (level + 1);
			level = (lines / 10) + 1;
			
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
		
		if(!game_over && app_state == GAME_STATE)
		{
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


//Initialization
void init()
{
	load_scores();
	
	srand(time(NULL));
	
	init_box_list(10, 6.);
	
	game_board.rows = 10;
	game_board.cols = 20;
	game_board.height = 80.;
	game_board.radius = 30.;
	game_board.piece_size = 3.2;
	
	game_board.pal = level_palettes[0];
	
	game_board.init();
	
	if(high_scores.size() == 0)
		app_state = INTRO_STATE;
	else
		app_state = MENU_STATE;
}

void start_game()
{
	//Reset the game
	game_board.reset();
	app_state = GAME_STATE;
	paused = false;
}

void reset_menu()
{
	//Sets the state to the menu
	app_state = MENU_STATE;
	menu = MENU_MAIN;
}

void intro_update(float delta_t)
{
	if(key_press(SDLK_RETURN) ||
		key_press(SDLK_ESCAPE) ||
		key_press(SDLK_SPACE))
	{
		reset_menu();
	}
	
	cursor_pos += delta_t / intro_text_rate;
	intro_timer += delta_t;
	
	if(intro_timer >= intro_delay)
		reset_menu();
}

void menu_update(float delta_t)
{
	
	if(menu != MENU_MAIN)
	{
		if(key_press(SDLK_RETURN) ||
			key_press(SDLK_ESCAPE) ||
			key_press(SDLK_SPACE) )
		{
			menu = MENU_MAIN;
		}
	}
	else
	{
		if(key_press(SDLK_UP))
		{
			menu_item = (menu_item + 3) % 4;
		}
		
		if(key_press(SDLK_DOWN))
		{
			menu_item = (menu_item + 1) % 4;
		}
		
		if(key_press(SDLK_RETURN))
		{
			if(menu_item == 0)
			{
				start_game();
			}
			else if(menu_item == 1)
			{
				menu = MENU_INSTRUCTIONS;
			}
			else if(menu_item == 2)
			{
				menu = MENU_HIGH_SCORES;
			}
			else if(menu_item == 3)
			{
				exit(0);
			}
		}
		
		if(key_press(SDLK_ESCAPE))
			exit(0);
	}
	
	
	game_board.update(delta_t);
}

//Update the game state
void game_update(float delta_t)
{
	if(game_board.game_over)
	{
		if(key_press(SDLK_RETURN) ||
			key_press(SDLK_SPACE) ||
			key_press(SDLK_ESCAPE) ||
			key_press(SDLK_a))
		{
			reset_menu();
		}
	}
	
	if(key_press(SDLK_ESCAPE))
	{
		if(!paused)
		{
			paused = true;
			paused_menu_item = 0;
		}
		else
		{
			paused = false;
		}
	}
	
	if(paused)
	{
		if(key_press(SDLK_UP))
		{
			paused_menu_item = (paused_menu_item + 2) % 3;
		}
		
		if(key_press(SDLK_DOWN))
		{
			paused_menu_item = (paused_menu_item + 1) % 3;
		}
		
		if(key_press(SDLK_RETURN))
		{
			if(paused_menu_item == 0)
				paused = false;
			else if(paused_menu_item == 1)
			{
				HighScore tmp;
				tmp.level = game_board.level;
				tmp.score = game_board.score;
				tmp.lines = game_board.lines;
				add_score(tmp);
				
				reset_menu();
			}
			else if(paused_menu_item == 2)
			{
								
				HighScore tmp;
				tmp.level = game_board.level;
				tmp.score = game_board.score;
				tmp.lines = game_board.lines;
				add_score(tmp);
				
				exit(0);

			}
		}
	}
	
	
	if(!paused)
	{
	
	if(!game_board.game_over)
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
				game_board.game_over = true;
				game_board.game_over_time = 1.;
				
				for(int i=0; i<game_board.rows; i++)
				for(int j=0; j<game_board.cols; j++)
				{
					int s = game_board.board[j + i * game_board.cols];
					
					if(s >= 0)
					{
						game_board.spawn_particle(i, j, s);
						game_board.board[j + i * game_board.cols] = -1;
					}
				}
				
				HighScore tmp;
				tmp.level = game_board.level;
				tmp.score = game_board.score;
				tmp.lines = game_board.lines;
				add_score(tmp);
			}
			else
			{
				game_board.score += 5 * game_board.level;
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
	}
	
	side_piece_rot[2] += delta_t * 0.005;
	side_piece_rot[1] += delta_t * 0.008716;
	side_piece_rot[0] += delta_t * 0.0061772;
	
	
	if(game_board.game_over)
	{
		game_board.game_over_time -= delta_t / game_over_seq_time;
		if(game_board.game_over_time < 0.)
			game_board.game_over_time = 0.;
	}
	
	game_board.update(delta_t);
	}
	
	if(quit)
	{
		//Save high score
		HighScore tmp;
		tmp.level = game_board.level;
		tmp.score = game_board.score;
		tmp.lines = game_board.lines;
		add_score(tmp);
	}
}


void update(float delta_t)
{
	switch(app_state)
	{
		case INTRO_STATE:
			intro_update(delta_t);
		break;
		
		case MENU_STATE:
			menu_update(delta_t);
		break;
		
		case GAME_STATE:
			game_update(delta_t);
		break;
	}
	
	if(quit)
		exit(0);
}

//Draw stuff
void draw()
{
	if(app_state == INTRO_STATE)
		return;
	
	glEnable(GL_FOG);
	
	float fog_color[4];
	
	for(int i=0; i<4; i++)
		fog_color[i] = game_board.pal.bg[i] * .3;
	
	glFogfv(GL_FOG_COLOR, fog_color);
	
	glFogi(GL_FOG_MODE, GL_LINEAR);
	
	glFogf(GL_FOG_DENSITY, 1.);
	glFogf(GL_FOG_START, 180.);
	glFogf(GL_FOG_END, 300.);
	
	float light_pos[3] = {30, 150, 180};
	glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
	
	gluLookAt(
		140, 80, 0,
		0, 0, 0,
		0, 1, 0);
	
	game_board.draw();
	
	glDisable(GL_FOG);
	
}


void intro_overlays()
{
	string intro_text = 
	"Hester,\n"
	"    You asked for a tetris game and so I have made one.\n"
	"However, the vanilla tetromino puzzle is much too simple,\n"
	"making it a poor vehicle for flaunting coding skill.  To \n"
	"overcome this issue I decided to try something different...\n"
	"\nEnjoy!\n   -Mik";
	
	glClearColor(0, 0, 0, 0);
	glColor4f(1, 1, 1, 1);
	
	glScalef(0.002, 0.002, 0.002);
	
	glTranslatef(-460, 200, 0);
	
	draw_string(intro_text);
}

void menu_overlays()
{
	game_board.pal.setTextMode();
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	glBegin(GL_QUADS);


	glColor4f(game_board.pal.board_color[0], 
		game_board.pal.board_color[1], 
		game_board.pal.board_color[2],
		.3);
		
		glVertex3f(.5, -1., 0.5);
		glVertex3f(-.5, -1, 0.5);
		glVertex3f(-.5, 1., 0.5);
		glVertex3f(.5, 1, 0.5);
	
	glEnd();
	glDisable(GL_BLEND);
	
	glLineWidth(3);
	glBegin(GL_LINES);
		glVertex3f(-.5, -1, 0);
		glVertex3f(-.5, 1., 0);
		glVertex3f(.5, -1., 0);
		glVertex3f(.5, 1, 0);
	glEnd();
	
	
	glScalef(.01, .01, .01);
	glTranslatef(-49, 65, 0);
	
	if(menu == MENU_MAIN)
	{
		draw_string("TETRIS");
	
		glScalef(.3, .3, .3);
		
		glTranslatef(68, -300, 0);
		
		if(menu_item == 0)
			glLineWidth(4);
		else
			glLineWidth(2);
		draw_string("START GAME");
		

		glTranslatef(0, -50, 0);
		if(menu_item == 1)
			glLineWidth(4);
		else
			glLineWidth(2);
		draw_string("INSTRUCTIONS");
		
		glTranslatef(0, -50, 0);
		if(menu_item == 2)
			glLineWidth(4);
		else
			glLineWidth(2);
		draw_string("HIGH SCORES");

		glTranslatef(0, -50, 0);
		if(menu_item == 3)
			glLineWidth(4);
		else
			glLineWidth(2);
		draw_string("EXIT");
	}
	else if(menu == MENU_INSTRUCTIONS)
	{
		draw_string("HELP");

		glScalef(.25, .25, .25);
		glLineWidth(2);
		
		glTranslatef(10, -200, 0);
		draw_string("UP - Rotate");

		glTranslatef(0, -50, 0);
		draw_string("DOWN - Drop");
		
		glTranslatef(0, -50, 0);
		draw_string("LEFT/RIGHT - Slide");
		
		glTranslatef(0, -50, 0);
		draw_string("ESCAPE - Pause/Back");

		glTranslatef(0, -50, 0);
		draw_string("RETURN - Select");
		
		
		glTranslatef(0, -200, 0);
		draw_string("Press RETURN");
		
	}
	else if(menu == MENU_HIGH_SCORES)
	{
		glScalef(.8,.8,.8);
		glTranslatef(0,5,0);
		draw_string("SCORES");
		
		glLineWidth(1);
		
		glScalef(.25, .25, .25);
		
		for(int i=0; i<20; i++)
		{
			char buffer[1024];
			if(i >= (int)high_scores.size())
				sprintf(buffer, "%2d. ---", i+1);
			else
				sprintf(buffer, "%2d. %10d %6d %2d", i+1, high_scores[i].score, high_scores[i].lines, high_scores[i].level);
			
			
			glTranslatef(0, -40, 0);
			draw_string(buffer);
		}
	}

	glLineWidth(1);

}

//Handle overlays
void game_overlays()
{
	game_board.pal.setTextMode();
	
	if(paused)
	{
			glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	glBegin(GL_QUADS);


	glColor4f(game_board.pal.board_color[0], 
		game_board.pal.board_color[1], 
		game_board.pal.board_color[2],
		.3);
		
		glVertex3f(.5, -1., 0.5);
		glVertex3f(-.5, -1, 0.5);
		glVertex3f(-.5, 1., 0.5);
		glVertex3f(.5, 1, 0.5);
	
	glEnd();
	glDisable(GL_BLEND);
	
	glLineWidth(3);
	glBegin(GL_LINES);
		glVertex3f(-.5, -1, 0);
		glVertex3f(-.5, 1., 0);
		glVertex3f(.5, -1., 0);
		glVertex3f(.5, 1, 0);
	glEnd();
	
	
		glScalef(.008, .008, .008);
		glTranslatef(-60, 65, 0);
	
		draw_string("PAUSED");

		
		glScalef(.4, .4, .4);
		
		glTranslatef(20, -200, 0);
		
		if(paused_menu_item == 0)
			glLineWidth(4);
		else
			glLineWidth(2);
		draw_string("Resume Game");
		
		glTranslatef(0, -50, 0);
		
		if(paused_menu_item == 1)
			glLineWidth(4);
		else
			glLineWidth(2);
		draw_string("End Game");
		
		glTranslatef(0, -50, 0);
		
		if(paused_menu_item == 2)
			glLineWidth(4);
		else
			glLineWidth(2);
		draw_string("Quit");
		
		glLineWidth(1);
		return;
	}
	
	
	glEnable(GL_BLEND);
	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	
	glBegin(GL_TRIANGLES);
	
	glColor4f(game_board.pal.board_color[0], 
		game_board.pal.board_color[1], 
		game_board.pal.board_color[2],
		.3);
	
	glVertex3f(-1, 1, 0.1);
	glVertex3f(-1, 1 - game_board.intro_pos * 1.2, 0.1)	;
	glVertex3f(-1 + game_board.intro_pos * .6, 1, 0.1);
	
	
	glVertex3f(1, -1, 0.5);
	glVertex3f(1-game_board.intro_pos * .6, -1, .5);
	glVertex3f(1, -1 + game_board.intro_pos * 1.5, .5);
	
	glEnd();
	
	glDisable(GL_BLEND);
	
	glLineWidth(3);
	
	glBegin(GL_LINES);
		glVertex3f(-1, 1 - game_board.intro_pos * 1.2, 0.1)	;
		glVertex3f(-1 + game_board.intro_pos * .6, 1, 0.1);
	glVertex3f(1-game_board.intro_pos * .6, -1, .5);
	glVertex3f(1, -1 + game_board.intro_pos * 1.5, .5);

	glEnd();
	
	glLineWidth(1);
	
	
	if(!game_board.intro_menu)
	{	
	
		glPushMatrix();
		glTranslatef(-.97, .9, 0);
		glScalef(.0025,.0025,.0025);
		draw_string("Score:");
		
		char buffer[1024];
		sprintf(buffer, "%9Ld", game_board.score);
		glTranslatef(15, -35, 0);
		draw_string(buffer);
		
		
		glTranslatef(-15, -35, 0);
		draw_string("Lines:");
		
		sprintf(buffer, "%6d", game_board.lines);
		glTranslatef(15, -35, 0);
		draw_string(buffer);

		glTranslatef(-15, -35, 0);
		draw_string("Level:");
		
		sprintf(buffer, "%3d", game_board.level);
		glTranslatef(15, -35, 0);
		draw_string(buffer);
		
		glTranslatef(560, -560, 0);
		draw_string("Next:");
		glPopMatrix();
		
		if(game_board.game_over)
		{
			glPushMatrix();
			glTranslatef(-game_board.game_over_time-.425,0,0);
			glColor4f(1, 0, 0, 1);
			glScalef(.005, .005, .005);
			glLineWidth(3);
			draw_string("GAME");
			glPopMatrix();
			
			glPushMatrix();
			glTranslatef(game_board.game_over_time+.025,0,0);
			glScalef(.005, .005, .005);
			draw_string("OVER");
			glLineWidth(1);
			glPopMatrix();
		}
		
		game_board.pal.setShapeParameters();
		game_board.pal.setShapeMaterial(game_board.next.shape);
		
		float light_pos[3] = {-100, 100, -100};
		
		glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
		
		float white[4] = {1.,1.,1.,1.},
			  grey[4] = {.01,.01,.01,.01},
			  black[4] = {0., 0., 0., 0.};
			  
		glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 0);
			  
		glLightfv(GL_LIGHT0, GL_AMBIENT, white);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, grey);
		glLightfv(GL_LIGHT0, GL_SPECULAR, black);
		glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 180);
		glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 0);

		glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 0);

		glTranslatef(.78,-.66,0); 
		glRotatef(cos(side_piece_rot[0])*30, 1, 0, 0);
		glRotatef(cos(side_piece_rot[1])*10, 0, 1, 0);
		glRotatef(sin(side_piece_rot[2])*40., 0, 0, 1);
		for(int i=0; i<4; i++)
		for(int j=0; j<4; j++)
		{
			if(shapes[game_board.next.shape][i][j] == 0)
				continue;
			glPushMatrix();
				glScalef(.044,.044,.044);
				glTranslatef(2.5*(i-1.5), 2.5*(j-1.5), 0);
				glCallList(box_list);
			glPopMatrix();
		}
	}
}

void overlays()
{
	switch(app_state)
	{
		case INTRO_STATE:
			intro_overlays();
		break;
		
		case MENU_STATE:
			menu_overlays();
		break;
		
		case GAME_STATE:
			game_overlays();
		break;
	}
}


};

