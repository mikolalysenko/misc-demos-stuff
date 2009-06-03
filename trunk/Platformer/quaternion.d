/*******************************************************
 *	This module has operations for quaternions.
 *	I need to go back someday and add SSE optimizations to
 *	these classes, as well as finish the unittests.
 *
 *  Currently incomplete.
 *******************************************************/
module quaternion;

//Imports
private import
	std.stdio,
	std.math,
	derelict.opengl.gl,
	vector,
	matrix;

/*******************************************************
 * Quat
 *******************************************************/
final struct Quat
{
	/***************************************************
	 * Constructor Methods
	 ***************************************************/
	static Quat opCall()
	{
		Quat r;
		r.x = r.y = r.z = 0.0f;
		r.w = 1.0f;
		return r;
	}
	
	static Quat opCall(Vector vec)
	{
		Quat r;
		r.v = vec;
		return r;
	}
	
	static Quat opCall(float w, Vector vec)
	{
		Quat r;
		r.v = vec;
		r.w = w;
		return r;
	}
	
	static Quat opCall(float x, float y, float z, float w)
	{
		Quat r;
		r.x = x;
		r.y = y;
		r.z = z;
		r.w = w;
		return r;
	}
	
	
	/***************************************************
	 * String Conversion
	 *	Works nicely with writef.
	 ***************************************************/
	char[] toString()
	{
		return v.toString();
	}
	
	
	/***************************************************
	 * Basic Arithmetic
	 ***************************************************/
	Quat opAdd(Quat x)
	{
		return Quat(v + x.v);
	}
	
	Quat opSub(Quat x)
	{
		return Quat(v - x.v);
	}
	
	Quat opMul(Quat q)
	{
		Quat r;
		
		r.x = w * q.x + x * q.w + y * q.z - z * q.y;
		r.y = w * q.y - x * q.z + y * q.w + z * q.x;
		r.z = w * q.z + x * q.y - y * q.x + z * q.w;
		r.w = w * q.w - x * q.x - y * q.y - z * q.z;
		
		return r;
	}
	
	Quat opDiv(Quat q)
	{
		return q * conj();
	}
	
	
	/***************************************************
	 * Basic Arithmetic with assignment
	 ***************************************************/
	Quat opAddAssign(Quat x)
	{
		v += x.v;
		return *this;
	}
	
	Quat opSubAssign(Quat x)
	{
		v -= x.v;
		return *this;
	}

	
	/***************************************************
	 * Conjugate
	 *	Returns the quaternion conjugate.
	 ***************************************************/
	Quat conj()
	{
		Quat r;
		
		r.x = -x;
		r.y = -y;
		r.z = -z;
		r.w = w;
		
		return r;
	}
	
	/***************************************************
	 * Magnitude
	 *	Returns the length of the quaternion
	 ***************************************************/
	float mag()
	{
		return v.mag();
	}
	
	/***************************************************
	 * Normalization
	 *	Makes the quaternion into a unit quaternion
	 ***************************************************/
	Quat norm()
	{
		return Quat(v.norm);
	}
	
	
	
	/***************************************************
	 * Data Members
	 ***************************************************/
	union
	{
		Vector v;
		
		struct
		{
			float x;
			float y;
			float z;
			float w;
		}
	}
	
	/***************************************************
	 * Unittests
	 ***************************************************/
	unittest
	{
		writefln("Testing Quat");
		
		assert(Quat.sizeof == 16);
		
		
		writefln("Quat Pass!");
	}
}

/*******************************************************
 * OpenGL macros
 *******************************************************/
void glLoadQuat(Quat q) { glLoadMatrix(Matrix.fromQuat(q)); }
void glMultQuat(Quat q) { glMultMatrix(Matrix.fromQuat(q)); }

