module misc;

import 
    vector, 
    matrix,
    xform,
    seidel,
    std.random, 
    std.math;

float randf() { return rand / 4294967296.0f; }

public T[] shuffle(T)(T[] r)
{
	T[] res = r.dup;
	
	for(int i=0; i<r.length; i++)
	{
		int n = rand() % (i+1);
		
		T tmp = res[i];
		res[i] = res[n];
		res[n] = tmp;
	}
	
	return res;
}

Vector rand_sphere()
{
    float x1 = randf;
    float x2 = randf;
    
    return Vector(
        2 * x1 * sqrt(1 - x1 * x1 - x2 * x2),
        2 * x2 * sqrt(1 - x1 * x1 - x2 * x2),
        1 - 2 * (x1 * x1 + x2 * x2));
}

//Grab the vertices from an oriented bounding box
Vector[] getOBBVerts(Matrix b)
{
    return [
        xform.xform(b, Vector( 1, 1, 1, 1)),
        xform.xform(b, Vector( 1, 1,-1, 1)),
        xform.xform(b, Vector( 1,-1, 1, 1)),
        xform.xform(b, Vector( 1,-1,-1, 1)),
        xform.xform(b, Vector(-1, 1, 1, 1)),
        xform.xform(b, Vector(-1, 1,-1, 1)),
        xform.xform(b, Vector(-1,-1, 1, 1)),
        xform.xform(b, Vector(-1,-1,-1, 1))];
}

Vector[] getOBBPlanes(Matrix b)
{
    b = b.adjoint.transpose;
    
    Vector[] r = [
        xform.xform(b, Vector( 1, 0, 0, -1)),
        xform.xform(b, Vector(-1, 0, 0, -1)),
        xform.xform(b, Vector( 0, 1, 0, -1)),
        xform.xform(b, Vector( 0,-1, 0, -1)),
        xform.xform(b, Vector( 0, 0, 1, -1)),
        xform.xform(b, Vector( 0, 0,-1, -1))];
    
    foreach(inout p ; r)
    {
        p += Vector(randf(), randf(), randf()) * 0.0001;
    }
    
    return r;
}

bool intersectOBB(Matrix a, Matrix b)
{
    return seidel_lp(shuffle(getOBBPlanes(a) ~ getOBBPlanes(b)));
    
    
    
    /*
    Vector bmin = Vector(float.infinity, float.infinity, float.infinity);
    Vector bmax = -bmin;
    
    foreach(v ; getOBBVerts(b))
    {
        for(int d=0; d<3; d++)
        {
            if(v[d] < bmin[d])
                bmin[d] = v[d];
            if(v[d] > bmax[d])
                bmax[d] = v[d];
        }
    }
    
    for(int i=0; i<3; i++)
    {
        if(bmin[i] > 1 || bmax[i] < -1)
            return false;
    }
    
    return true;
    */
}

//Returns the t value for ray-box intersection
float testOBBRay(Matrix box, Vector rd, Vector ro)
{
    Matrix minv = box.inverse;
    
    
    //Check if ray start is inside a box!
    ro = xform.xform(minv, ro);
    if( ro.x > -1 && ro.x < 1 &&
        ro.y > -1 && ro.y < 1 &&
        ro.z > -1 && ro.z < 1)
        return 0.0f;

    //Get direction for ray
    rd = xform.xform(minv, rd);
    
    //Find point of intersection in box with r for each plane
    float tmin = float.infinity;
    
    
    for(int i=0; i<3; i++)
    {
        float d = rd[i];
        if(abs(d) <= float.epsilon)
            continue;
        
        float o = ro[i];
        float t;
        
        if(o < -1)
        {
            if(d < 0) continue;
            t = -(1 + o) / d;
        }
        else if(o > 1)
        {
            if(d > 0) continue;
            t = (1 - o) / d;
        }
        else
        {
            continue;
        }
        
        if(t >= tmin)
            continue;
        
        Vector p = ro + t * rd;
        
        if( p.x >= -1.0001 && p.x <= 1.0001 &&
            p.y >= -1.0001 && p.y <= 1.0001 &&
            p.z >= -1.0001 && p.z <= 1.0001 )
            tmin = t;
    }
    
    return tmin;
}