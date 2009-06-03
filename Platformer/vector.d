/*******************************************************
 *	This module contains basic vector-vector operations.
 *		Addition
 *		Multiplication
 *		Scalar operations
 *		Dot/Cross product
 *
 *	Everything is written in unoptimized SSE assembler.
 *	Once I figure out how to align the vector struct
 *	on a 16-byte boundary I will go back and fix it up.
 *******************************************************/
module vecmat.vector;

//Imports from phobos, etc.
private import std.string,
	std.stdio,
	std.math,
	std.intrinsic,
	derelict.opengl.gl;

/*******************************************************
 * Vector
 *	This is the struct which handles all vector type
 *	operations.  It is very versatile and can be used
 *	to represent vectors of size 1-4 easily.
 *******************************************************/
final struct Vector
{
	/***************************************************
	 * Constructor Methods
	 *	Each of these initialize the first 0-4 elements
	 *	and clear the rest to 0.
	 ***************************************************/
	static Vector opCall()
	{
		Vector r;
		r[0] = r[1] = r[2] = r[3] = 0.0f;
		return r;
	}
	
	static Vector opCall(float x)
	{
		Vector r;
		r[0] = x;
		r[1] = 0.0f;
		r[2] = 0.0f;
		r[3] = 0.0f;
		return r;
	}
	
	static Vector opCall(float x, float y)
	{
		Vector r;
		r[0] = x;
		r[1] = y;
		r[2] = 0.0f;
		r[3] = 0.0f;
		return r;
	}
	
	static Vector opCall(float x, float y, float z)
	{
		Vector r;
		r[0] = x;
		r[1] = y;
		r[2] = z;
		r[3] = 0.0f;
		return r;
	}
	
	static Vector opCall(float x, float y, float z, float w)
	{
		Vector r;
		r[0] = x;
		r[1] = y;
		r[2] = z;
		r[3] = w;
		return r;
	}
	
	static Vector opCall(float[3] arr)
	{
		Vector r;
		r[0] = arr[0];
		r[1] = arr[1];
		r[2] = arr[2];
		r[3] = 0;
		return r;
	}
	
	/***************************************************
	 * Indexing Operations
	 ***************************************************/
	float opIndex(int i)
	in
	{
		assert(i >= 0 && i <= 3);
	}
	body
	{
		return v[i];
	}
	
	float opIndexAssign(float f, int i)
	in
	{
		assert(i >= 0 && i <= 3);
	}
	body
	{
		return v[i] = f;
	}
	
    /***************************************************
     * Convert to pointer.
	 ***************************************************/
	float * ptr()
    {
        return v.ptr;
    }
	
	/***************************************************
	 * String Conversion
	 *	Works nicely with writef.
	 ***************************************************/
	char[] toString()
	{
		return "{" ~ 
			std.string.toString(v[0]) ~ ", " ~
			std.string.toString(v[1]) ~ ", " ~
			std.string.toString(v[2]) ~ ", " ~
			std.string.toString(v[3]) ~ "}";
	}
	
	/***************************************************
	 * Basic Arithmetic
	 ***************************************************/
	Vector opNeg()
	{
		Vector r;
		
		r[0] = -v[0];
		r[1] = -v[1];
		r[2] = -v[2];
		r[3] = -v[3];
		
		return r;
	}
	
	Vector opAdd(Vector x)
	{
		Vector r;
		
		version(SSE)
		{
			
			float *a = x.v.ptr;
			float *b = v.ptr;
			float *c = r.v.ptr;
			
			asm
			{
				mov EAX, a;
				mov EBX, b;
				mov ECX, c;
				
				movups XMM1, [EAX];
				movups XMM2, [EBX];
				addps XMM1, XMM2;
				movups [ECX], XMM1;
			}
		}
		else
		{
			r[0] = v[0] + x[0];
			r[1] = v[1] + x[1];
			r[2] = v[2] + x[2];
			r[3] = v[3] + x[3];
		}
		
		return r;
	}
	
	Vector opSub(Vector x)
	{
		Vector r;
		
		version(SSE)
		{
			float *a = x.v.ptr;
			float *b = v.ptr;
			float *c = r.v.ptr;
			
			asm
			{
				mov EAX, a;
				mov EBX, b;
				mov ECX, c;
				
				movups XMM1, [EAX];
				movups XMM2, [EBX];
				subps XMM2, XMM1;
				movups [ECX], XMM2;
			}
		}
		else
		{
			r[0] = v[0] - x[0];
			r[1] = v[1] - x[1];
			r[2] = v[2] - x[2];
			r[3] = v[3] - x[3];
		}
		
		return r;
	}
	
	Vector opMul(Vector x)
	{
		Vector r;
		
		version(SSE)
		{
			float *a = x.v.ptr;
			float *b = v.ptr;
			float *c = r.v.ptr;
			
			asm
			{
				mov EAX, a;
				mov EBX, b;
				mov ECX, c;
				
				movups XMM1, [EAX];
				movups XMM2, [EBX];
				mulps XMM1, XMM2;
				movups [ECX], XMM1;
			}
		}
		else
		{
			r[0] = v[0] * x[0];
			r[1] = v[1] * x[1];
			r[2] = v[2] * x[2];
			r[3] = v[3] * x[3];
		}
		
		return r;
	}
	
	Vector opDiv(Vector x)
	{
		Vector r;
		
		version(SSE)
		{
			float *a = x.v.ptr;
			float *b = v.ptr;
			float *c = r.v.ptr;
			
			asm
			{
				mov EAX, a;
				mov EBX, b;
				mov ECX, c;
				
				movups XMM1, [EAX];
				movups XMM2, [EBX];
				divps XMM2, XMM1;
				movups [ECX], XMM2;
			}
		}
		else
		{
			r[0] = v[0] / x[0];
			r[1] = v[1] / x[1];
			r[2] = v[2] / x[2];
			r[3] = v[3] / x[3];
		}
		
		
		return r;
	}
	
	Vector opMul(float s)
	{
		Vector r;
		
		
		version(SSE)
		{
			float *a = v.ptr;
			float *b = &s;
			float *c = r.v.ptr;
			
			asm
			{
				mov EAX, a;
				mov EBX, b;
				mov ECX, c;
				
				movss XMM2, [EBX];
				movups XMM1, [EAX];
				shufps XMM2, XMM2, 0;
				mulps XMM1, XMM2;
				
				movups [ECX], XMM1;
			}
		}
		else
		{
			r[0] = v[0] * s;
			r[1] = v[1] * s;
			r[2] = v[2] * s;
			r[3] = v[3] * s;
		}
		
		
		return r;
	}
	
	Vector opDiv(float s)
	{
		Vector r;
		
		
		version(SSE)
		{
			float *a = v.ptr;
			float *b = &s;
			float *c = r.v.ptr;
			
			asm
			{
				mov EAX, a;
				mov EBX, b;
				mov ECX, c;
				
				movss XMM2, [EBX];
				movups XMM1, [EAX];
				shufps XMM2, XMM2, 0;
				divps XMM1, XMM2;
				
				movups [ECX], XMM1;
			}
		}
		else
		{
			r[0] = v[0] / s;
			r[1] = v[1] / s;
			r[2] = v[2] / s;
			r[3] = v[3] / s;
		}
		
		
		
		return r;
	}
	
	
	/***************************************************
	 * Arithmetic With Assignment
	 ***************************************************/
	Vector opAddAssign(Vector x)
	{
		version(SSE)
		{
			float *a = x.v.ptr;
			float *b = v.ptr;
			
			asm
			{
				mov EAX, a;
				mov EBX, b;
				
				movups XMM1, [EAX];
				movups XMM2, [EBX];
				addps XMM1, XMM2;
				movups [EBX], XMM1;
			}
		}
		else
		{
			v[0] += x[0];
			v[1] += x[1];
			v[2] += x[2];
			v[3] += x[3];
		}
		
		
		return *this;
	}
	
	Vector opSubAssign(Vector x)
	{
		version(SSE)
		{
			float *a = x.v.ptr;
			float *b = v.ptr;
			
			asm
			{
				mov EAX, a;
				mov EBX, b;
				
				movups XMM1, [EAX];
				movups XMM2, [EBX];
				subps XMM2, XMM1;
				movups [EBX], XMM2;
			}
		}
		else
		{
			v[0] -= x[0];
			v[1] -= x[1];
			v[2] -= x[2];
			v[3] -= x[3];
		}
		
		
		return *this;
	}
	
	Vector opMulAssign(Vector x)
	{
		version(SSE)
		{
			float *a = x.v.ptr;
			float *b = v.ptr;
			
			asm
			{
				mov EAX, a;
				mov EBX, b;
				
				movups XMM1, [EAX];
				movups XMM2, [EBX];
				mulps XMM1, XMM2;
				movups [EBX], XMM1;
			}
		}
		else
		{
			v[0] *= x[0];
			v[1] *= x[1];
			v[2] *= x[2];
			v[3] *= x[3];
		}
		
		
		return *this;
	}
	
	Vector opDivAssign(Vector x)
	{
		version(SSE)
		{
			float *a = x.v.ptr;
			float *b = v.ptr;
			
			asm
			{
				mov EAX, a;
				mov EBX, b;
				
				movups XMM1, [EAX];
				movups XMM2, [EBX];
				divps XMM2, XMM1;
				movups [EBX], XMM2;
			}
		}
		else
		{
			v[0] /= x[0];
			v[1] /= x[1];
			v[2] /= x[2];
			v[3] /= x[3];
		}
		
		
		return *this;
	}
	
	Vector opMulAssign(float s)
	{
		version(SSE)
		{
			float *a = v.ptr;
			float *b = &s;
			
			asm
			{
				mov EAX, a;
				mov EBX, b;
				
				movss XMM2, [EBX];
				movups XMM1, [EAX];
				shufps XMM2, XMM2, 0;
				mulps XMM1, XMM2;
				
				movups [EAX], XMM1;
			}
		}
		else
		{
			v[0] *= s;
			v[1] *= s;
			v[2] *= s;
			v[3] *= s;
		}
		
		
		return *this;
	}
	
	Vector opDivAssign(float s)
	{
		version(SSE)
		{
			float *a = v.ptr;
			float *b = &s;
			
			asm
			{
				mov EAX, a;
				mov EBX, b;
				
				movss XMM2, [EBX];
				movups XMM1, [EAX];
				shufps XMM2, XMM2, 0;
				divps XMM1, XMM2;
				
				movups [EAX], XMM1;
			}
		}
		else
		{
			v[0] /= s;
			v[1] /= s;
			v[2] /= s;
			v[3] /= s;
		}
		
		
		return *this;
	}
	
	
	/***************************************************
	 * Cross Product
	 *	Only operates on the first 3 components of the
	 *	vector.
	 ***************************************************/
	Vector cross(Vector x)
	{
		Vector r;
		
		version(SSE)
		{
			float *a = v.ptr;
			float *b = x.v.ptr;
			float *c = r.v.ptr;
			
			asm
			{
				mov EAX, a;
				mov EBX, b;
				mov ECX, c;
				
				movups XMM0, [EAX];
				movups XMM1, [EBX];
				
				movaps XMM2, XMM0;
				
				shufps XMM2, XMM2, 0b11001001;
				shufps XMM1, XMM1, 0b11010010;
				
				mulps XMM2, XMM1;
				
				shufps XMM0, XMM0, 0b11010010;
				shufps XMM1, XMM1, 0b11010010;
				
				mulps XMM0, XMM1;
				
				subps XMM2, XMM0;
				
				movups [ECX], XMM2;
			}
		}
		else
		{
			r[0] = v[1] * x[2] - v[2] * x[1];
			r[1] = v[2] * x[0] - v[0] * x[2];
			r[2] = v[0] * x[1] - v[1] * x[0];
			r[3] = 0;
		}
		
		
		return r;
	}
	
	/***************************************************
	 * Dot Product
	 *	Computes the dot product of the vector with
	 *	another vector.
	 ***************************************************/
	float dot(Vector x)
	{
		version(SSE)
		{
			float r;
			float *a = v.ptr;
			float *b = x.v.ptr;
			float *c = &r;
			
			asm
			{
				mov EAX, a;
				mov EBX, b;
				mov ECX, c;
				
				movups XMM1, [EAX];
				movups XMM2, [EBX];
				
				mulps XMM1, XMM2;
				
				movaps XMM2, XMM1;
				shufps XMM2, XMM2, 0x4e;
				addps XMM1, XMM2;
				
				movaps XMM2, XMM1;
				shufps XMM2, XMM2, 0x11;
				addps XMM1, XMM2;
				
				movss [ECX], XMM1;
			}
			
			return r;
		}
		else
		{
			return	v[0] * x[0] +
					v[1] * x[1] +
					v[2] * x[2] +
					v[3] * x[3];
		}
	}
	
	/***************************************************
	 * Magnitude
	 *	Returns the length of the vector
	 ***************************************************/
	float mag()
	{
		version(SSE)
		{
			float r;
			float *a = v.ptr;
			float *b = &r;
			float *c;
			
			asm
			{
				mov EAX, a;
				mov EBX, b;
				
				movups XMM1, [EAX];
				
				mulps XMM1, XMM1;
				
				movaps XMM2, XMM1;
				shufps XMM2, XMM2, 0x4e;
				addps XMM1, XMM2;
				
				movaps XMM2, XMM1;
				shufps XMM2, XMM2, 0x11;
				addps XMM1, XMM2;
				
				sqrtps XMM2, XMM1;
				//db 0x0f, 0x51, 0xd1;
				
				movss [EBX], XMM2;
			}
		}
		else
		{
			return sqrt(dot(*this));
		}
		
		
		return r;
	}
    
    float magSquared()
    {
        return x*x + y*y + z*z;
    }
	
	
	/***************************************************
	 * Normalization
	 *	Normalizes the vector to unit length.
	 ***************************************************/
	Vector norm()
	{
		Vector r;
		
		version(SSE)
		{
			float *a = v.ptr;
			float *b = r.v.ptr;
			
			asm
			{
				mov EAX, a;
				mov EBX, b;
				
				movups XMM0, [EAX];
				movaps XMM1, XMM0;
				
				mulps XMM1, XMM1;
				
				movaps XMM2, XMM1;
				shufps XMM2, XMM2, 0x4e;
				addps XMM1, XMM2;
				
				movaps XMM2, XMM1;
				shufps XMM2, XMM2, 0x11;
				addps XMM1, XMM2;
				
				sqrtps XMM2, XMM1;
				//db 0x0f, 0x51, 0xd1;
				
				divps XMM0, XMM2;
				
				movups [EBX], XMM0;
			}
		}
		else
		{
			return *this / mag();
		}
		
		
		return r;
	}
    
    /***************************************************
	 * Parallel & Perpendicular.
     * Breaks the vector into its parallel/perpendicular
     * components wrt to another vector.
	 ***************************************************/
    Vector para(Vector p)
    {
        float d = p.dot(*this);
        return (*this) * d;
    }
    
    Vector perp(Vector p)
    {
        return p - para(p);
    }
	

	/***************************************************
	 * Data Members
	 *	There are wrappers for standard vector names
	 *	(x,y,z,w) as well as (r,g,b,a) for colors.
	 ***************************************************/
	union
	{
		float[4] v;
		
		struct
		{
			float r;
			float g;
			float b;
			float a;
		}
		
		struct
		{
			float x;
			float y;
			float z;
			float w;
		}
		
		struct
		{
			float nx;
			float ny;
			float nz;
			float d;
		}
	}
	
	/***************************************************
	 * Unittests
	 ***************************************************/
	unittest
	{
		writefln("Testing Vector");
		
		assert(Vector.sizeof == 16);
		
		{
			Vector a = Vector();
			Vector e = Vector(1);
			Vector d = Vector(1, 2);
			Vector b = Vector(1, 2, 3);
			Vector c = Vector(1, 2, 3, 4);
			
			writefln("%s", a.toString());
			assert(a[0] == 0.0f &&
				a[1] == 0.0f &&
				a[2] == 0.0f &&
				a[3] == 0.0f);
			
			writefln("%s", e.toString());
			assert(e[0] == 1.0f &&
				e[1] == 0.0f &&
				e[2] == 0.0f &&
				e[3] == 0.0f);
			
			writefln("%s", d.toString());
			assert(d[0] == 1.0f &&
				d[1] == 2.0f &&
				d[2] == 0.0f &&
				d[3] == 0.0f);
			
			writefln("%s", b.toString());
			assert(b[0] == 1.0f &&
				b[1] == 2.0f &&
				b[2] == 3.0f &&
				b[3] == 0.0f);
			
			writefln("%s", c.toString());
			assert(c[0] == 1.0f &&
				c[1] == 2.0f &&
				c[2] == 3.0f &&
				c[3] == 4.0f);
				
			Vector t = -c;
			writefln("%s", t.toString());
			assert(t[0] == -1.0f &&
				t[1] == -2.0f &&
				t[2] == -3.0f &&
				t[3] == -4.0f);
		}
		
		{
			Vector a = Vector(10, 20, 30, 40);
			Vector b = Vector(1, 2, 3, 4);
			Vector c = Vector();
			
			writefln("a=%s", a.toString());
			writefln("b=%s", b.toString());
			
			c = a + b;
			writefln("a+b=%s",c.toString());
			assert(c[0] == 11.0f &&
				c[1] == 22.0f &&
				c[2] == 33.0f &&
				c[3] == 44.0f);
			
			c = a - b;
			writefln("a-b=%s",c.toString());
			assert(c[0] == 9.0f &&
				c[1] == 18.0f &&
				c[2] == 27.0f &&
				c[3] == 36.0f);
			
			c = a * b;
			writefln("a*b=%s",c.toString());
			assert(c[0] == 10.0f &&
				c[1] == 40.0f &&
				c[2] == 90.0f &&
				c[3] == 160.0f);
			
			c = a / b;
			writefln("a/b=%s",c.toString());
			assert(c[0] == 10.0f &&
				c[1] == 10.0f &&
				c[2] == 10.0f &&
				c[3] == 10.0f);
			
			c = a * 2.0f;
			writefln("a*2.0=%s",c.toString());
			assert(c[0] == 20.0f &&
				c[1] == 40.0f &&
				c[2] == 60.0f &&
				c[3] == 80.0f);
			
			c = a / 5.0f;
			writefln("a/5.0f=%s", c.toString());
			assert(c[0]==2.0f &&
				c[1] == 4.0f &&
				c[2] == 6.0f &&
				c[3] == 8.0f);
			
			c = a;
			c += b;
			writefln("a+b=%s",c.toString());
			assert(c[0] == 11.0f &&
				c[1] == 22.0f &&
				c[2] == 33.0f &&
				c[3] == 44.0f);
			
			c = a;
			c -= b;
			writefln("a-b=%s",c.toString());
			assert(c[0] == 9.0f &&
				c[1] == 18.0f &&
				c[2] == 27.0f &&
				c[3] == 36.0f);
			
			c = a;
			c *= b;
			writefln("a*b=%s",c.toString());
			assert(c[0] == 10.0f &&
				c[1] == 40.0f &&
				c[2] == 90.0f &&
				c[3] == 160.0f);
			
			c = a;
			c /= b;
			writefln("a/b=%s",c.toString());
			assert(c[0] == 10.0f &&
				c[1] == 10.0f &&
				c[2] == 10.0f &&
				c[3] == 10.0f);
			
			c = a;
			c *= 2.0f;
			writefln("a*2.0=%s",c.toString());
			assert(c[0] == 20.0f &&
				c[1] == 40.0f &&
				c[2] == 60.0f &&
				c[3] == 80.0f);
			
			c = a;
			c /= 5.0f;
			writefln("a/5.0f=%s", c.toString());
			assert(c[0]==2.0f &&
				c[1] == 4.0f &&
				c[2] == 6.0f &&
				c[3] == 8.0f);
			
			
			
			
			float f = a.dot(b);
			writefln("a.b=%f", f);
			assert(f == 300.0f);
			
			
			Vector up = Vector(1, 0, 0);
			Vector right = Vector(0, 1, 0);
			
			c = up.cross(right);
			
			writefln("%s x %s=%s", 
				up.toString(), 
				right.toString(), 
				c.toString());
				
			assert(c[0] == 0 &&
				c[1] == 0 &&
				c[2] == 1.0f &&
				c[3] == 0);
			
			c = Vector(1.0f, 1.0f, 1.0f, 1.0f);
			f = c.mag();
			writefln("|1,1,1,1|=%f", f);
			assert(f == 2.0f);
			
			c = c.norm();
			writefln("_|_ {1,1,1,1}=%s", c.toString());
			assert(c[0] == 0.5 &&
				c[1] == 0.5 &&
				c[2] == 0.5 &&
				c[3] == 0.5);
		}
		writefln("Vector Pass!");
	}
}

/******************************************************
 * OpenGL wrappers
 ******************************************************/
void glVertex2(Vector x)	{ glVertex2fv(x.v.ptr); }
void glVertex3(Vector x)	{ glVertex3fv(x.v.ptr); }
void glVertex4(Vector x)	{ glVertex4fv(x.v.ptr); }
void glTexCoord1(Vector x){ glTexCoord1fv(x.v.ptr); }
void glTexCoord2(Vector x){ glTexCoord2fv(x.v.ptr); }
void glTexCoord3(Vector x){ glTexCoord3fv(x.v.ptr); }
void glTexCoord4(Vector x){ glTexCoord4fv(x.v.ptr); }
void glColor3(Vector x)	{ glColor3fv(x.v.ptr); }
void glColor4(Vector x)	{ glColor4fv(x.v.ptr); }
void glRasterPos2(Vector x){ glRasterPos2fv(x.v.ptr); }
void glRasterPos3(Vector x){ glRasterPos3fv(x.v.ptr); }
void glRasterPos4(Vector x){ glRasterPos4fv(x.v.ptr); }
void glNormal(Vector x)	{ glNormal3fv(x.v.ptr); }
void glTranslate(Vector x) { glTranslatef(x.x, x.y, x.z); }
void glScale(Vector x)     { glScalef(x.x, x.y, x.z); }
