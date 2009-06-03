/*******************************************************
 *	This module has basic operations for matrix-matrix
 *	types as well as useful geometric conversions.
 *	Supported operations:
 *		Matrix Addition
 *		Matrix Scalar multiplication
 *		Matrix Multiplication
 *		Inversion
 *		Determinant
 *		Transpose
 *
 *	A vector matrix transform is implemented separately.
 *******************************************************/
module matrix;

//Imports
private import
	std.stdio,
	std.math,
	std.string,
	derelict.opengl.gl,
	vector,
	quaternion;

//Shear types
enum SHEAR_TYPE
{
	NONE,
	XY,
	XZ,
	YX,
	YZ,
	ZX,
	ZY,
}

/*******************************************************
 * Matrix
 *	An implementation of a matrix data type.
 *******************************************************/
final struct Matrix
{
	/***************************************************
	 * identity
	 *	Returns the identity matrix.
	 ***************************************************/
	final const static Matrix identity =
	{
		m:
		[
			1,0,0,0,
			0,1,0,0,
			0,0,1,0,
			0,0,0,1,
		]
	};
	
	/***************************************************
	 * OpenGL interface constructors
	 ***************************************************/
	static Matrix getModelView()
	{
		Matrix r;
		
		glGetFloatv(GL_MODELVIEW_MATRIX, r.m.ptr);
		
		return r;
	}
	
	static Matrix getProjection()
	{
		Matrix r;
		
		glGetFloatv(GL_PROJECTION_MATRIX, r.m.ptr);
		
		return r;
	}
	
	static Matrix getTexture()
	{
		Matrix r;
		
		glGetFloatv(GL_TEXTURE_MATRIX, r.m.ptr);
		
		return r;
	}
	
	
	/***************************************************
	 * Axis Angles
	 *	Creates an augmented 3x3 rotation matrix from
	 *	an axis angle.
	 ***************************************************/
	Matrix rotateAxis(float angle, float x, float y, float z)
	{
		return (*this) * fromAxis(angle, x, y, z);
	}
	
	Matrix rotateAxis(Vector axis)
	{
		return (*this) * fromAxis(axis);
	}
	
	static Matrix fromAxis(Vector axis)
	{
		return fromAxis(axis.w, axis.x, axis.y, axis.z);
	}
	
	static Matrix fromAxis(float angle, float x, float y, float z)
	{
		Matrix r = Matrix.identity;
		
		float c = cos(angle);
		float s = sin(angle);
		float t = 1 - c;
		
		r.m00 = c + x * x * t;
		r.m11 = c + y * y * t;
		r.m22 = c + z * z * t;
		
		
		float tmp1 = x * y * t;
		float tmp2 = z * s;
		r.m10 = tmp1 + tmp2;
		r.m01 = tmp1 - tmp2;
		
		tmp1 = x * z * t;
		tmp2 = y * s;
		r.m20 = tmp1 - tmp2;
		r.m02 = tmp1 + tmp2;
		
		tmp1 = y * z * t;
		tmp2 = x * s;
		r.m21 = tmp1 + tmp2;
		r.m12 = tmp1 - tmp2;
		
		return r;
	}
	
	/***************************************************
	 * Euler
	 *	Creates a matrix from a set of Euler angles.
	 ***************************************************/
	Matrix rotateEuler(float pitch, float yaw, float roll)
	{
		return (*this) * fromEuler(pitch, yaw, roll);
	}
	
	static Matrix fromEuler(float pitch, float yaw, float roll)
	{
		float cp = cos(pitch);
		float sp = sin(pitch);
		float cy = cos(yaw);
		float sy = sin(yaw);
		float cr = cos(roll);
		float sr = sin(roll);
		
		Matrix r = Matrix.identity;
		
		r.m00 = cp * cy;
		r.m01 = sp*sr - cp*sy*cr;
		r.m02 = cp*sy*sr + sp*cr;
		r.m10 = sy;
		r.m11 = cy*cr;
		r.m12 = -cy*sr;
		r.m20 = -sp*cy;
		r.m21 = sp*sy*cr + cp*sr;
		r.m22 = -sp*sy*sr + cp*cr;
        
		return r;
	}
	
	/***************************************************
	 * Quaternion
	 *	Creates a matrix from a quaternion.
	 ***************************************************/
	static Matrix fromQuat(Quat q)
	{
		float xx = q.x * q.x;
		float xy = q.x * q.y;
		float xz = q.x * q.z;
		float xw = q.x * q.w;
		float yy = q.y * q.y;
		float yz = q.y * q.z;
		float yw = q.y * q.w;
		float zz = q.z * q.z;
		float zw = q.z * q.w;
		float ww = q.w * q.w;
		
		Matrix r = Matrix.identity;
		
		r.m00 = ww + xx - yy - zz;
		r.m01 = 	2 * (xy - zw);
		r.m02 =		2 * (xz + yw);
		
		r.m10 = 	2 * (xy + zw);
		r.m11 =	ww - xx + yy - zz;
		r.m12 =		2 * (yz - xw);
		
		r.m20 =		2 * (xz - yw);
		r.m21 = 	2 * (yz + xw);
		r.m22 = ww - xx - yy + zz;
		
		r.m03 = r.m13 = r.m23 = r.m30 = r.m31 = r.m32 = 0;
		r.m33 = 1;
		
		return r;
	}
	
	Matrix rotateQuat(Quat q)
	{
		return (*this) * fromQuat(q);
	}
	
	/***************************************************
	 * Shearing
	 ***************************************************/
	Matrix shear(SHEAR_TYPE st, float shear_factor)
	{
		return (*this) * fromShear(st, shear_factor);
	}
	
	static Matrix fromShear(SHEAR_TYPE st, float shear_factor)
	{
		Matrix r = identity;
		
		switch(st)
		{
			case SHEAR_TYPE.XY:
				r[0,1] = shear_factor;
			break;
			
			case SHEAR_TYPE.XZ:
				r[0,2] = shear_factor;
			break;
			
			case SHEAR_TYPE.YX:
				r[1,0] = shear_factor;
			break;
			
			case SHEAR_TYPE.YZ:
				r[1,2] = shear_factor;
			break;
			
			case SHEAR_TYPE.ZX:
				r[2,0] = shear_factor;
			break;
			
			case SHEAR_TYPE.ZY:
				r[2,1] = shear_factor;
			break;
			
			default:
		}
		
		return r;
	}
	
	/***************************************************
	 * scale
	 *	Creates a scaling matrix.
	 ***************************************************/
	Matrix scale(float x,float y,float z,float w)
	{
		return (*this) * fromScale(x, y, z, w);
	}
	
	Matrix scale(float x, float y, float z)
	{
		return (*this) * fromScale(x, y, z);
	}

	Matrix scale(Vector v)
	{
		return (*this) * fromScale(v);
	}
	
	Matrix scale(float s)
	{
		return (*this) * fromScale(s);
	}
	
	
	static Matrix fromScale(float x, float y, float z)
	{
		return fromScale(x, y, z, 1);
	}
	
	static Matrix fromScale(Vector v)
	{
		return fromScale(v.x, v.y, v.z, v.w);
	}

	static Matrix fromScale(float s)
	{
		return fromScale(s, s, s, 1);
	}
	
	static Matrix fromScale(float x, float y, float z, float w)
	{
		Matrix r = identity;
		
		r[0,0] = x;
		r[1,1] = y;
		r[2,2] = z;
		r[3,3] = w;
		
		return r;
	}
	
	
	
	/***************************************************
	 * Translation matrices
	 ***************************************************/
	Matrix translate(float x, float y, float z)
	{
		return (*this) * fromTranslate(x, y, z);
	}
	
	Matrix translate(Vector v)
	{
		return (*this) * fromTranslate(v);
	}
	
	static Matrix fromTranslate(Vector v)
	{
		return fromTranslate(v.x, v.y, v.z);
	}
	
	static Matrix fromTranslate(float x, float y, float z)
	{
		Matrix r = identity;
		
		r[3,0] = x;
		r[3,1] = y;
		r[3,2] = z;
		
		return r;
	}
	
	
	/***************************************************
	 * Transpose
	 ***************************************************/
	Matrix transpose()
	{
		Matrix r;
		
		for(int i=0; i<4; i++)
		for(int j=0; j<4; j++)
			r[i, j] = m[i + (j<<2)];
		
		return r;
	}

	/***************************************************
	 * Adjoint
	 ***************************************************/
	Matrix adjoint()
	{
		Matrix r;
		
		r.m00=	-m13*m22*m31 + m12*m23*m31 + m13*m21*m32 - m11*m23*m32 - m12*m21*m33 + m11*m22*m33; 
		r.m01=	 m03*m22*m31 - m02*m23*m31 - m03*m21*m32 + m01*m23*m32 + m02*m21*m33 - m01*m22*m33; 
		r.m02=	-m03*m12*m31 + m02*m13*m31 + m03*m11*m32 - m01*m13*m32 - m02*m11*m33 + m01*m12*m33;
		r.m03=	 m03*m12*m21 - m02*m13*m21 - m03*m11*m22 + m01*m13*m22 + m02*m11*m23 - m01*m12*m23;
		r.m10=	 m13*m22*m30 - m12*m23*m30 - m13*m20*m32 + m10*m23*m32 + m12*m20*m33 - m10*m22*m33;
		r.m11=	-m03*m22*m30 + m02*m23*m30 + m03*m20*m32 - m00*m23*m32 - m02*m20*m33 + m00*m22*m33;
		r.m12=	 m03*m12*m30 - m02*m13*m30 - m03*m10*m32 + m00*m13*m32 + m02*m10*m33 - m00*m12*m33;
		r.m13=	-m03*m12*m20 + m02*m13*m20 + m03*m10*m22 - m00*m13*m22 - m02*m10*m23 + m00*m12*m23;
		r.m20=	-m13*m21*m30 + m11*m23*m30 + m13*m20*m31 - m10*m23*m31 - m11*m20*m33 + m10*m21*m33; 
		r.m21=	 m03*m21*m30 - m01*m23*m30 - m03*m20*m31 + m00*m23*m31 + m01*m20*m33 - m00*m21*m33;
		r.m22=	-m03*m11*m30 + m01*m13*m30 + m03*m10*m31 - m00*m13*m31 - m01*m10*m33 + m00*m11*m33;
		r.m23=	 m03*m11*m20 - m01*m13*m20 - m03*m10*m21 + m00*m13*m21 + m01*m10*m23 - m00*m11*m23;
		r.m30=	 m12*m21*m30 - m11*m22*m30 - m12*m20*m31 + m10*m22*m31 + m11*m20*m32 - m10*m21*m32;
		r.m31=	-m02*m21*m30 + m01*m22*m30 + m02*m20*m31 - m00*m22*m31 - m01*m20*m32 + m00*m21*m32;
		r.m32=	 m02*m11*m30 - m01*m12*m30 - m02*m10*m31 + m00*m12*m31 + m01*m10*m32 - m00*m11*m32;
		r.m33=	-m02*m11*m20 + m01*m12*m20 + m02*m10*m21 - m00*m12*m21 - m01*m10*m22 + m00*m11*m22;
		
		return r;
	}
	
	/***************************************************
	 * Determinant
	 ***************************************************/
	float determinant()
	{
		return ((  m33*m22 - m32*m23)*m11 +
				((-m33*m21 + m31*m23)*m12 + 
				(  m32*m21 - m31*m22)*m13))*m00 + 
				(((-m33*m22+ m32*m23)*m10 + 
				(( m33*m20 - m30*m23)*m12 + 
				( -m32*m20 + m30*m22)*m13))*m01 + 
				(((m33*m21 - m31*m23)*m10 + 
				((-m33*m20 + m30*m23)*m11 + 
				(  m31*m20 - m30*m21)*m13))*m02 + 
				((-m32*m21 + m31*m22)*m10 + 
				(( m32*m20 - m30*m22)*m11 + 
				( -m31*m20 + m30*m21)*m12))*m03));
	}
	
	
	/***************************************************
	 * Trace
	 ***************************************************/
	float trace()
	{
		return m00 + m11 + m22 + m33;
	}
	
	
	/***************************************************
	 * Inverse
	 ***************************************************/
	Matrix inverse()
	in
	{
		assert(determinant != 0);
	}
	body
	{
		return adjoint() / determinant();
	}
	

	/***************************************************
	 * Indexing Operations
	 ***************************************************/
	float opIndex(int i, int j)
	in
	{
		assert(0 <= i && i <= 3);
		assert(0 <= j && j <= 3);
	}
	body
	{
		return m[(i<<2) + j];
	}
	
	float *opIndex(int i)
	in
	{
		assert(0 <= i && i <= 3);
	}
	body
	{
		return &m[i<<2];
	}
	
	float opIndexAssign(float f, int i, int j)
	in
	{
		assert(0 <= i && i <= 3);
		assert(0 <= j && j <= 3);
	}
	body
	{
		return m[(i<<2) + j] = f;
	}
	
	/***************************************************
	 * String Conversion
	 ***************************************************/
	char[] toString()
	{
		return "{" ~ 
			row[0].toString() ~ ", " ~
			row[1].toString() ~ ", " ~
			row[2].toString() ~ ", " ~
			row[3].toString() ~ "}";
	}
	
	
	/***************************************************
	 * Basic Arithmetic
	 ***************************************************/
	Matrix opAdd(Matrix other)
	{
		Matrix r;
		
		version(SSE)
		{
			float *a = m.ptr;
			float *b = other.m.ptr;
			float *c = r.m.ptr;
			
			asm
			{
				mov EAX, a;
				mov EBX, b;
				mov ECX, c;
				
				movups XMM0, [EAX];
				movups XMM1, [EBX];
				addps XMM0, XMM1;
				movups [ECX], XMM0;
				
				movups XMM0, [EAX+16];
				movups XMM1, [EBX+16];
				addps XMM0, XMM1;
				movups [ECX+16], XMM0;
				
				movups XMM0, [EAX+32];
				movups XMM1, [EBX+32];
				addps XMM0, XMM1;
				movups [ECX+32], XMM0;
				
				movups XMM0, [EAX+48];
				movups XMM1, [EBX+48];
				addps XMM0, XMM1;
				movups [ECX+48], XMM0;
			}
		}
		else
		{
			for(int i=0; i<16; i++)
				r.m[i] = m[i] + other.m[i];
		}
		
		return r;
	}
	
	Matrix opAddAssign(Matrix other)
	{
		version(SSE)
		{
			float *a = m.ptr;
			float *b = other.m.ptr;
			
			asm
			{
				mov EAX, a;
				mov EBX, b;
				
				movups XMM0, [EAX];
				movups XMM1, [EBX];
				addps XMM0, XMM1;
				movups [EAX], XMM0;
				
				movups XMM0, [EAX+16];
				movups XMM1, [EBX+16];
				addps XMM0, XMM1;
				movups [EAX+16], XMM0;
				
				movups XMM0, [EAX+32];
				movups XMM1, [EBX+32];
				addps XMM0, XMM1;
				movups [EAX+32], XMM0;
				
				movups XMM0, [EAX+48];
				movups XMM1, [EBX+48];
				addps XMM0, XMM1;
				movups [EAX+48], XMM0;
			}
		}
		else
		{
			for(int i=0; i<16; i++)
				m[i] += other.m[i];
		}
		
		return *this;
	}
	
	Matrix opSub(Matrix other)
	{
		Matrix r;
		
		
		version(SSE)
		{
			float *a = m.ptr;
			float *b = other.m.ptr;
			float *c = r.m.ptr;
			
			asm
			{
				mov EAX, a;
				mov EBX, b;
				mov ECX, c;
				
				movups XMM0, [EAX];
				movups XMM1, [EBX];
				subps XMM0, XMM1;
				movups [ECX], XMM0;
				
				movups XMM0, [EAX+16];
				movups XMM1, [EBX+16];
				subps XMM0, XMM1;
				movups [ECX+16], XMM0;
				
				movups XMM0, [EAX+32];
				movups XMM1, [EBX+32];
				subps XMM0, XMM1;
				movups [ECX+32], XMM0;
				
				movups XMM0, [EAX+48];
				movups XMM1, [EBX+48];
				subps XMM0, XMM1;
				movups [ECX+48], XMM0;
			}
		}
		else
		{
			for(int i=0; i<16; i++)
				r.m[i] = m[i] - other.m[i];
		}
		
		return r;
	}
	
	Matrix opSubAssign(Matrix other)
	{
		version(SSE)
		{
			float *a = m.ptr;
			float *b = other.m.ptr;
			
			asm
			{
				mov EAX, a;
				mov EBX, b;
				
				movups XMM0, [EAX];
				movups XMM1, [EBX];
				subps XMM0, XMM1;
				movups [EAX], XMM0;
				
				movups XMM0, [EAX+16];
				movups XMM1, [EBX+16];
				subps XMM0, XMM1;
				movups [EAX+16], XMM0;
				
				movups XMM0, [EAX+32];
				movups XMM1, [EBX+32];
				subps XMM0, XMM1;
				movups [EAX+32], XMM0;
				
				movups XMM0, [EAX+48];
				movups XMM1, [EBX+48];
				subps XMM0, XMM1;
				movups [EAX+48], XMM0;
			}
		}
		else
		{
			for(int i=0; i<16; i++)
				m[i] -= other.m[i];
		}
		
		return *this;
	}
	
	
	Matrix opMul(float s)
	{
		Matrix r;
		
		version(SSE)
		{
			float *a = m.ptr;
			float *b = &s;
			float *c = r.m.ptr;
			
			asm
			{
				mov EAX, a;
				mov EBX, b;
				mov ECX, c;
				
				movss XMM0, [EBX];
				shufps XMM0, XMM0, 0;
				
				movups XMM1, [EAX];
				mulps XMM1, XMM0;
				movups [ECX], XMM1;
				
				movups XMM1, [EAX+16];
				mulps XMM1, XMM0;
				movups [ECX+16], XMM1;
				
				movups XMM1, [EAX+32];
				mulps XMM1, XMM0;
				movups [ECX+32], XMM1;
				
				movups XMM1, [EAX+48];
				mulps XMM1, XMM0;
				movups [ECX+48], XMM1;
			}
		}
		else
		{
			for(int i=0; i<16; i++)
				r.m[i] = m[i] * s;
		}
		
		return r;
	}
	
	Matrix opMulAssign(float s)
	{
		version(SSE)
		{
			float *a = m.ptr;
			float *b = &s;
			
			asm
			{
				mov EAX, a;
				mov EBX, b;
				
				movss XMM0, [EBX];
				shufps XMM0, XMM0, 0;
				
				movups XMM1, [EAX];
				mulps XMM1, XMM0;
				movups [EAX], XMM1;
				
				movups XMM1, [EAX+16];
				mulps XMM1, XMM0;
				movups [EAX+16], XMM1;
				
				movups XMM1, [EAX+32];
				mulps XMM1, XMM0;
				movups [EAX+32], XMM1;
				
				movups XMM1, [EAX+48];
				mulps XMM1, XMM0;
				movups [EAX+48], XMM1;
			}
		}
		else
		{
			for(int i=0; i<16; i++)
				m[i] *= s;
		}
		
		return *this;
	}
	
	Matrix opDiv(float s)
	{
		Matrix r;
		
		version(SSE)
		{
			float *a = m.ptr;
			float *b = &s;
			float *c = r.m.ptr;
			
			asm
			{
				mov EAX, a;
				mov EBX, b;
				mov ECX, c;
				
				movss XMM0, [EBX];
				shufps XMM0, XMM0, 0;
				
				movups XMM1, [EAX];
				divps XMM1, XMM0;
				movups [ECX], XMM1;
				
				movups XMM1, [EAX+16];
				divps XMM1, XMM0;
				movups [ECX+16], XMM1;
				
				movups XMM1, [EAX+32];
				divps XMM1, XMM0;
				movups [ECX+32], XMM1;
				
				movups XMM1, [EAX+48];
				divps XMM1, XMM0;
				movups [ECX+48], XMM1;
			}
		}
		else
		{
			for(int i=0; i<16; i++)
				r.m[i] = m[i] / s;
		}
		
		return r;
	}
	
	Matrix opDivAssign(float s)
	{
		version(SSE)
		{
			float *a = m.ptr;
			float *b = &s;
			
			asm
			{
				mov EAX, a;
				mov EBX, b;
				
				movss XMM0, [EBX];
				shufps XMM0, XMM0, 0;
				
				movups XMM1, [EAX];
				divps XMM1, XMM0;
				movups [EAX], XMM1;
				
				movups XMM1, [EAX+16];
				divps XMM1, XMM0;
				movups [EAX+16], XMM1;
				
				movups XMM1, [EAX+32];
				divps XMM1, XMM0;
				movups [EAX+32], XMM1;
				
				movups XMM1, [EAX+48];
				divps XMM1, XMM0;
				movups [EAX+48], XMM1;
			}
		}
		else
		{
			for(int i=0; i<16; i++)
				m[i] /= s;
		}
		
		return *this;
	}
	
	
	/***************************************************
	 * Matrix multiplication
	 ***************************************************/
	Matrix opMul(Matrix other)
	{
		Matrix r;
		
		for(int i=0; i<4; i++)
		for(int j=0; j<4; j++)
		{
			r.m[(i<<2) + j] = 0;
			
			for(int t=0; t<4; t++)
				r.m[(i<<2) + j] += m[(i<<2)+t] * other.m[(t<<2)+j];
		}
		
		return r;
	}
	
	
	union
	{
		float [16] m = void;
		
		Vector [4] row;
		
		struct
		{
			float m00, m01, m02, m03;
			float m10, m11, m12, m13;
			float m20, m21, m22, m23;
			float m30, m31, m32, m33;
		}
	}
	
	unittest
	{
		writefln("Testing Matrix");
		
		//Test identity
		writefln("I:", identity.toString());
		assert(identity[0][0]==1);
		assert(identity[0][1]==0);
		assert(identity[0][2]==0);
		assert(identity[0][3]==0);
		assert(identity[1][0]==0);
		assert(identity[1][1]==1);
		assert(identity[1][2]==0);
		assert(identity[1][3]==0);
		assert(identity[2][0]==0);
		assert(identity[2][1]==0);
		assert(identity[2][2]==1);
		assert(identity[2][3]==0);
		assert(identity[3][0]==0);
		assert(identity[3][1]==0);
		assert(identity[3][2]==0);
		assert(identity[3][3]==1);
		
		
		//Test axis angle
		{
			//Unimplemented
			//static assert(0);
		}
		
		//Test euler angle
		{
			//Unimplemented
			//static assert(0);
		}
		
		//Test scale
		{
			Matrix a = fromScale(3,5,7,9);
			
			writefln("scale(3,5,7,9)=", a.toString());
			
			assert(a[0][0]==3);
			assert(a[0][1]==0);
			assert(a[0][2]==0);
			assert(a[0][3]==0);
			assert(a[1][0]==0);
			assert(a[1][1]==5);
			assert(a[1][2]==0);
			assert(a[1][3]==0);
			assert(a[2][0]==0);
			assert(a[2][1]==0);
			assert(a[2][2]==7);
			assert(a[2][3]==0);
			assert(a[3][0]==0);
			assert(a[3][1]==0);
			assert(a[3][2]==0);
			assert(a[3][3]==9);
		}
		
		//Test translate
		{
			Matrix a = fromTranslate(1, 2, 3);
			
			writefln("translate(1,2,3)=",a.toString());
			
			assert(a[0][0]==1);
			assert(a[0][1]==0);
			assert(a[0][2]==0);
			assert(a[0][3]==0);
			assert(a[1][0]==0);
			assert(a[1][1]==1);
			assert(a[1][2]==0);
			assert(a[1][3]==0);
			assert(a[2][0]==0);
			assert(a[2][1]==0);
			assert(a[2][2]==1);
			assert(a[2][3]==0);
			assert(a[3][0]==1);
			assert(a[3][1]==2);
			assert(a[3][2]==3);
			assert(a[3][3]==1);
		}
		
		
		//Test arithmetic
		{
			static Matrix a = { m:
			[
				 1, 2, 3, 4,
				 5, 6, 7, 8,
				 9,10,11,12,
				13,14,15,16,
			]};
			
			static Matrix b = { m:
			[
				1,0,1,0,
				0,1,0,1,
				1,0,1,0,
				0,1,0,1,
			]};
			
			static Matrix c;
			
			static Matrix M = { m:
			[
				10, 3, 8, 7,
				11,31, 1, 6,
				59,61, 2, 4,
				 2, 1, 5,37,
			]};
			
			
			writefln("a=",a.toString());
			writefln("b=",b.toString());
			writefln("M=",M.toString());
			
			//Test transpose
			c = a.transpose();
			writefln("a^T=",c.toString());
			assert(c[0,0]==1);
			assert(c[0,1]==5);
			assert(c[0,2]==9);
			assert(c[0,3]==13);
			assert(c[1,0]==2);
			assert(c[1,1]==6);
			assert(c[1,2]==10);
			assert(c[1,3]==14);
			assert(c[2,0]==3);
			assert(c[2,1]==7);
			assert(c[2,2]==11);
			assert(c[2,3]==15);
			assert(c[3,0]==4);
			assert(c[3,1]==8);
			assert(c[3,2]==12);
			assert(c[3,3]==16);
			
			//Test adjoint
			c = M.adjoint();
			writefln("adj(M)=", c.toString());
			{
				static Matrix adjM = { m:
				[
					1239, 15741, -8029, -1919,
					-165, -14823, 2719, 2141,
					-43020, -15564, 9868, 9596,
					5751, 1653, -973, -9143,
				]};
				
				assert(c == adjM);
			}
			
			//Test inverse
			c = M.inverse();
			writefln("M^-1=", c.toString());
			{
				static Matrix invM = { m:
				[
					-0.004243034437, -0.05390605737, 0.02749582203, 0.006571737761, 
					0.0005650530122, 0.05076230788, -0.009311388729, -0.007331990904,
					0.1473247308, 0.05329990959, -0.03379359470, -0.03286211337,
					-0.01969466590, -0.005660803814, 0.003332100490, 0.03131078601,
				]};
				
				assert(c == invM);
			}
			
			//addition
			c = a + b;
			writefln("a+b=",c.toString());
			assert(c[0,0]==2);
			assert(c[0,1]==2);
			assert(c[0,2]==4);
			assert(c[0,3]==4);
			assert(c[1,0]==5);
			assert(c[1,1]==7);
			assert(c[1,2]==7);
			assert(c[1,3]==9);
			assert(c[2,0]==10);
			assert(c[2,1]==10);
			assert(c[2,2]==12);
			assert(c[2,3]==12);
			assert(c[3,0]==13);
			assert(c[3,1]==15);
			assert(c[3,2]==15);
			assert(c[3,3]==17);
			
			//addition + assignment
			c = a;
			c += b;
			writefln("a+b=",c.toString());
			assert(c[0,0]==2);
			assert(c[0,1]==2);
			assert(c[0,2]==4);
			assert(c[0,3]==4);
			assert(c[1,0]==5);
			assert(c[1,1]==7);
			assert(c[1,2]==7);
			assert(c[1,3]==9);
			assert(c[2,0]==10);
			assert(c[2,1]==10);
			assert(c[2,2]==12);
			assert(c[2,3]==12);
			assert(c[3,0]==13);
			assert(c[3,1]==15);
			assert(c[3,2]==15);
			assert(c[3,3]==17);
			
			//subtraction
			c = a - b;
			writefln("a-b=",c.toString());
			assert(c[0,0]==0);
			assert(c[0,1]==2);
			assert(c[0,2]==2);
			assert(c[0,3]==4);
			assert(c[1,0]==5);
			assert(c[1,1]==5);
			assert(c[1,2]==7);
			assert(c[1,3]==7);
			assert(c[2,0]==8);
			assert(c[2,1]==10);
			assert(c[2,2]==10);
			assert(c[2,3]==12);
			assert(c[3,0]==13);
			assert(c[3,1]==13);
			assert(c[3,2]==15);
			assert(c[3,3]==15);
			
			//subtraction + assignment
			c = a;
			c -= b;
			writefln("a-b=",c.toString());
			assert(c[0,0]==0);
			assert(c[0,1]==2);
			assert(c[0,2]==2);
			assert(c[0,3]==4);
			assert(c[1,0]==5);
			assert(c[1,1]==5);
			assert(c[1,2]==7);
			assert(c[1,3]==7);
			assert(c[2,0]==8);
			assert(c[2,1]==10);
			assert(c[2,2]==10);
			assert(c[2,3]==12);
			assert(c[3,0]==13);
			assert(c[3,1]==13);
			assert(c[3,2]==15);
			assert(c[3,3]==15);
			
			//Scalar multipilication
			c = a * 10;
			writefln("a*10=",c.toString());
			assert(c[0,0]==10);
			assert(c[0,1]==20);
			assert(c[0,2]==30);
			assert(c[0,3]==40);
			assert(c[1,0]==50);
			assert(c[1,1]==60);
			assert(c[1,2]==70);
			assert(c[1,3]==80);
			assert(c[2,0]==90);
			assert(c[2,1]==100);
			assert(c[2,2]==110);
			assert(c[2,3]==120);
			assert(c[3,0]==130);
			assert(c[3,1]==140);
			assert(c[3,2]==150);
			assert(c[3,3]==160);
			
			c = a;
			c *= 10;
			writefln("a*10=",c.toString());
			assert(c[0,0]==10);
			assert(c[0,1]==20);
			assert(c[0,2]==30);
			assert(c[0,3]==40);
			assert(c[1,0]==50);
			assert(c[1,1]==60);
			assert(c[1,2]==70);
			assert(c[1,3]==80);
			assert(c[2,0]==90);
			assert(c[2,1]==100);
			assert(c[2,2]==110);
			assert(c[2,3]==120);
			assert(c[3,0]==130);
			assert(c[3,1]==140);
			assert(c[3,2]==150);
			assert(c[3,3]==160);
			
			//Matrix multiplication
			{
				c = identity * a;
				writefln("I*a=", c.toString());
				assert(c == a);
				
				c = a * identity;
				writefln("a*I=", c.toString());
				assert(c == a);
				
				c = a * b;
				writefln("a*b=", c.toString());
				static Matrix axb = { m:
				[
					4, 6, 4, 6,
					12, 14, 12, 14,
					20, 22, 20, 22,
					28, 30, 28, 30,
				]};
				assert(c == axb);
			}
			writefln("Matrix Pass!");
		}
	}
}

/*******************************************************
 * OpenGL macros
 *******************************************************/
void glLoadMatrix(Matrix m) { glLoadMatrixf(m.m.ptr); }
void glMultMatrix(Matrix m) { glMultMatrixf(m.m.ptr); }

