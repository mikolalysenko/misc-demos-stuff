module misc;

import 
    std.c.stdio,
    std.string;

//Convert a hex string into an int
uint from_hex(char[] str)
{
    uint r;
    sscanf(toStringz(str), "%x", &r); 
    return r;
}

//Check if a character is ascii valued
bool is_ascii(uint c)
{
    return 0x20 <= c && c < 0x80;
}
