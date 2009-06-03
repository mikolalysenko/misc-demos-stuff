/*******************************************************
 * Geometric transformations are implemented here.
 * Any operation acting on a mixture of vectors, matrices
 * or quaternions is implemented here.
 *
 *  Supported Operations:
 *      Matrix/Vector transformation
 *
 * Currently incomplete.
 *******************************************************/
module xform;

private import
    std.stdio,
    std.math,
	matrix,
    vector,
    quaternion;

/**
 * Transform a single vector by a matrix.
 *
 * Params:
 *  m = The matrix we are transforming by
 *  vec = The vector we are transforming.
 *
 * Returns: A transformed vector
 */
Vector xform(Matrix m, Vector vec)
{
    Vector res;
    
    for(int i=0; i<4; i++)
    {
        res.v[i] = 0.0;
        
        for(int j=0; j<4; j++)
            res.v[i] += m[j, i] * vec[j];
    }
    
    return res;
}
