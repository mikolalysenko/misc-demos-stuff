module seidel;

private import vector, std.stdio;

//An epsilon factor.  Since nearly feasible regions are permissible, this should
//be set to some small, but positive factor.
private float EPSILON = 1.0 / 65536.0;

version = NO_SSE;

/**
 * Intersect 3 planes in homogeneous coordinates as a single point.
 *
 * Note: This can probably be made a good deal more robust.  One important
 *	case which is not properly handled is when there are 2 coplanar faces.
 *	Fortunately, the BSP construction strategy mostly prevents this event
 *	from happening.
 */
private Vector intersect
(
	Vector c,
	Vector d,
	Vector e
)
{
version(NO_SSE)
{
    Vector z = Vector(
        d.y * e.z - d.z * e.y,
        d.z * e.x - d.x * e.z,
        d.x * e.y - d.y * e.x);

	z =     (
			 z * c.w +
			 e.cross(c) * d.w +
		     c.cross(d) * e.w
			) /
			-(	c.x * z.x +
				c.y * z.y + 
				c.z * z.z );
    
    z.w = 1.0f;
    
    return z;
}
else
{
	Vector r = void;
	float *cp = c.v.ptr;
	float *dp = d.v.ptr;
	float *ep = e.v.ptr;
	float *rp = r.v.ptr;
	
	asm
	{
		//Load up pointers
		mov EAX, cp;
		mov EBX, dp;
		mov ECX, ep;
		mov EDX, rp;
		
		//Load up SSE registers
		movups XMM0, [EAX];
		movups XMM1, [EBX];
		movups XMM2, [ECX];
		
		//Compute cross products & store into XMM3-5
		
		// XMM3 = XMM1 x XMM2
		// XMM4 = XMM2 x XMM0
		// XMM5 = XMM0 x XMM1
		
		//Compute the cross product for XMM3
		movaps XMM6, XMM2;
		movaps XMM7, XMM1;
		shufps XMM6, XMM6, 0b11_00_10_01;
		shufps XMM7, XMM7, 0b11_01_00_10;
		mulps XMM6, XMM7;
		
		movaps XMM3, XMM2;
		shufps XMM3, XMM3, 0b11_01_00_10;
		shufps XMM7, XMM7, 0b11_01_00_10;
		mulps XMM3, XMM7;
		
		subps XMM3, XMM6;
		
		
		//Compute the cross product for XMM4
		movaps XMM6, XMM0;
		movaps XMM7, XMM2;
		shufps XMM6, XMM6, 0b11_00_10_01;
		shufps XMM7, XMM7, 0b11_01_00_10;
		mulps XMM6, XMM7;
		
		movaps XMM4, XMM0;
		shufps XMM4, XMM4, 0b11_01_00_10;
		shufps XMM7, XMM7, 0b11_01_00_10;
		mulps XMM4, XMM7;
		
		subps XMM4, XMM6;
		
		//Compute the cross product for XMM5
		movaps XMM6, XMM1;
		movaps XMM7, XMM0;
		shufps XMM6, XMM6, 0b11_00_10_01;
		shufps XMM7, XMM7, 0b11_01_00_10;
		mulps XMM6, XMM7;
		
		movaps XMM5, XMM1;
		shufps XMM5, XMM5, 0b11_01_00_10;
		shufps XMM7, XMM7, 0b11_01_00_10;
		mulps XMM5, XMM7;
		
		subps XMM5, XMM6;
		
		//Compute the numerator & store in XMM5
		movaps XMM6, XMM2;
		shufps XMM6, XMM6, 0b11111111;
		mulps XMM5, XMM6;
		
		movaps XMM6, XMM1;
		shufps XMM6, XMM6, 0b11111111;
		mulps XMM6, XMM4;
		addps XMM5, XMM6;
		
		movaps XMM6, XMM0;
		shufps XMM6, XMM6, 0b11111111;
		mulps XMM6, XMM3;
		addps XMM5, XMM6;
		
		//Compute the denominator & store in XMM3
		mulps XMM3, XMM0;
		
		movaps XMM6, XMM3;
		shufps XMM6, XMM6, 0x4e;
		addps XMM3, XMM6;
		
		movaps XMM6, XMM3;
		shufps XMM6, XMM6, 0x11;
		addps XMM3, XMM6;
		
		//Now divide through to get final result
		xorps XMM7, XMM7;
		divps XMM5, XMM3;
		subps XMM7, XMM5;
		
		movups [EDX], XMM7;
	}
	
	return r;
}
}

private bool test_pt
(
	Vector x,
	Vector p
)
{
version(NO_SSE)
{
	return 
		x.v[0] * p.v[0] +
		x.v[1] * p.v[1] +
		x.v[2] * p.v[2] +
		p.v[3] <= EPSILON;
}
else
{
	float * xp = x.v.ptr;
	float * pp = p.v.ptr;
	int r;
	
	asm
	{
		mov EAX, xp;
		mov EBX, pp;
		
		movups XMM1, [EAX];
		movups XMM2, [EBX];

		mulps XMM1, XMM2;

		movaps XMM2, XMM1;
		shufps XMM2, XMM2, 0x4e;
		addps XMM1, XMM2;

		movss XMM0, EPSILON;
		
		movaps XMM2, XMM1;
		shufps XMM2, XMM2, 0x11;
		addps XMM1, XMM2;

		//cmpless
		cmpss XMM1, XMM0, 0x02;
		
		movss r, XMM1;
	}
	
	return cast(bool)r;
}
}


/**
 * Performs Seidel's linear programming algorithm on the given
 * problem specification.
 *
 * Params:
 *	h = The set of constraints.
 *	c = The objective function we are trying to optimize.
 *	v = The current best guess for the vector.
 *
 * Returns:
 *	true if a solution is found, false if infeasible.
 */
public bool seidel_lp
(
	Vector h[],		//Constraints
)
{
	if(h.length <= 3)
		return true;
	
	//Constraint violates initial value, so it must be a boundary
    Vector x = intersect(h[$-3], h[$-2], h[$-1]);
	
	for(int i=h.length-4; i>=0; i--)
	{
		if( !test_pt(x, h[i]) )
		{
			//Recursively re-evaluate seidel's algorithm with 2
			x = intersect(h[i], h[$-2],  h[$-1]);
			
			for(int j=h.length - 3; j > i; j--)
			{
				if( !test_pt(x, h[j]) )
				{
					//Final iteration, just do basic feasibility test
					x = intersect(h[i], h[j], h[$-1]);
                    
                    for(int k=h.length - 2; k>j; k--)
                    {
						if( !test_pt(x, h[k]) )
						{
                            x = intersect(h[i], h[j], h[k]);
                            
                            for(int l=h.length - 1; l > k; l--)
                            {
                                if(!test_pt(x, h[l]))
                                {
                                    return false;
                                }
                            }
						}
					}
				}
			}
		}
	}
	
	return true;
}
