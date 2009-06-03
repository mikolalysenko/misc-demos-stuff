/**
 * Shell utilities
 */

module cshell;

import
    std.string,
    std.c.stdio,
    std.c.string;

private extern (C) FILE * popen(char * cmd, char * type);
private extern (C) int pclose(FILE * stream);

//Run a shell command and pull out the text from the result
public char[] shell(char[] cmd)
{
    FILE * pipe = popen(toStringz(cmd), "r");
    char[] result;
    char tmp_buf[256];
    
    while(fgets(tmp_buf.ptr, 256, pipe) !is null)
    {
        result ~= tmp_buf[0..strlen(tmp_buf.ptr)];
    }
    
    if(pclose(pipe) == -1)
        throw new Exception("Error executing: " ~ cmd);
    
    return result;
}

//Escape for shell commands
public char[] shell_escape(char[] str)
{
    char[] result;
    
    foreach(c; str)
    {
        if(c == ' ' || c == '\\' || c == '&' || c == '<' || c == '>' || c == '|')
            result ~= '\\';
        result ~= c;
    }
    
    return result;
}

//Run a shell command
public char[] shell(char[][] args)
{
    char[] cmd;
    
    foreach(arg; args)
        cmd ~= shell_escape(arg) ~ " ";
    
    return shell(cmd);
}


