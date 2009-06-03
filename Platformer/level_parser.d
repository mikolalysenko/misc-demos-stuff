module level_parser;

import
    vector,
    matrix,
    block,
    std.math,
    std.file,
    std.string,
    std.stdio;
    
    
const float DEFAULT_SLIP = 0.000001f;

private int cur_line;
private char[][] tokens;
private int[] line_numbers;

private char[] next_token()
{
    if(tokens.length == 0)
    {
        throw new Exception("Unexpected EOF");
    }
    
    cur_line = line_numbers[0];
    line_numbers = line_numbers[1..$];
    
    char[] t = tokens[0];
    tokens = tokens[1..$];
    return t;
}

private void tokenize_string(char[] str)
{
    char[] nstr;
    
    foreach(c ; str)
    {
        if(c == '(' || c == ')')
        {
            nstr ~= ' ';
            nstr ~= c;
            nstr ~= ' ';
        }
        else
        {
            nstr ~= c;
        }
    }
    
    foreach(int n, char[] l ; splitlines(nstr))
    {
        foreach(t ; split(l))
        {
            if(t.length > 0 && t[0] == '#')
                break;
            
            tokens ~= t;
            line_numbers ~= n;
        }
    }
}

private class ParseTree
{
    int line_num;
    char[] tok;
    ParseTree[] children;
    
    this()
    {
        tok = next_token();
        line_num = cur_line;
        
        if(tok == "(")
        {
            tok = next_token();
            
            if(tok == ")")
            {
                tok = "";
            }
            else
            
            
            while(tokens.length > 0 && tokens[0] != ")")
            {
                children ~= new ParseTree;
            }
            
            if(next_token != ")")
                throw new Exception(format("Expected ) on line %s", cur_line));
        }
    }
    
    float eval_float()
    {
        float f = atof(tok);
        
        if(isnan(f))
        {
            throw new Exception(format("Expected float on line %s", line_num));
        }
        
        return f;
    }
    
    Vector eval_vect()
    {
        float[] v;
        
        v ~= eval_float();
        
        foreach(c ; children)
        {
            v ~= c.eval_float;
        }
        
        switch(v.length)
        {
            case 2:
                return Vector(v[0], v[1]);
            
            case 3:
                return Vector(v[0], v[1], v[2]);
            
            case 4:
                return Vector(v[0], v[1], v[2], v[3]);
            
            default:
                throw new Exception(format("Expected vector on line %s", line_num));
                return Vector();
        }
    }
    
    Block[] generate_blocks(Matrix mat, Vector color, float slip)
    {
        char[] line_str = format(" on line %s", line_num);
        
        switch(tok)
        {
            case "Blocks":
            {
                Block[] res;
                foreach(c ; children)
                    res ~= c.generate_blocks(mat, color, slip);
                return res;
            }
            
            case "Slip":
            {
                if(children.length != 2)
                    throw new Exception("Incorrect arguments for Slip" ~ line_str);
                
                return children[1].generate_blocks(mat, color, children[0].eval_float);
            }
            
            case "Color":
            {
                if(children.length != 2)
                    throw new Exception("Incorrect arguments for Color" ~ line_str);
                
                return children[1].generate_blocks(
                    mat, 
                    children[0].eval_vect(),
                    slip);
            }
            
            case "Trans":
            {
                if(children.length != 2)
                    throw new Exception("Incorrect arguments for Trans" ~ line_str);
                
                Vector t = children[0].eval_vect();
                
                return children[1].generate_blocks(
                    Matrix.fromTranslate(t.x, t.y, t.z) * mat,
                    color,
                    slip);
            }
            
            case "Rotate":
            {
                if(children.length != 3)
                    throw new Exception("Incorrect arguments for Rotate" ~ line_str);
                
                float theta = children[0].eval_float() / PI * 180.0f;
                Vector s = children[1].eval_vect();
                
                return children[2].generate_blocks(
                    Matrix.fromAxis(theta, s.x, s.y, s.z) * mat,
                    color,
                    slip);
            }
            
            case "Scale":
            {
                if(children.length != 2)
                    throw new Exception("Incorrect arguments for Scale" ~ line_str);
                
                Vector s = children[0].eval_vect();
                
                return children[1].generate_blocks(
                    Matrix.fromScale(s.x, s.y, s.z) * mat,
                    color,
                    slip);
            }
            
            case "Box":
            {
                return [new Block(mat, color, slip)];
            }
            
            default:
                throw new Exception("Unrecognized token: " ~ tok ~ line_str);
                return null;
        }
    }
    
    char[] toString()
    {
        char[] result = "(" ~ tok;
        
        foreach(c ; children)
        {
            result ~= " " ~ c.toString();
        }
        
        return result ~ ")";
    }
}

private ParseTree parse_level(char[] str)
{
    tokenize_string(str);
    auto r = new ParseTree();
    
    if(tokens.length > 0)
    {
        throw new Exception("Unmatched )!");
    }
    
    return r;
}

//Create 
public Block[] read_level(char[] filename)
{
    if(!exists(filename))
        throw new Exception("Level " ~ filename ~ " does not exist!");

    
    //Parse out file
    ParseTree t = parse_level("(Blocks " ~ cast(char[])read(filename) ~ ")");
    
    
    return t.generate_blocks(Matrix.identity, Vector(0, 0, 0, 0), DEFAULT_SLIP);
}
