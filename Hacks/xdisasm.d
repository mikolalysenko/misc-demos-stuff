import 
    std.format, 
    std.regexp, 
    std.string, 
    std.stdio, 
    cshell,
    misc,
    macho;

//Grab strings from otool
void get_otool_strings(inout char[][uint] result, char[] ubinary)
{
    auto lines = splitlines(shell("otool -sd __TEXT __cstring " ~ ubinary));
    uint start_address = from_hex(std.string.split(lines[2])[0]);
    
    char[] s_buffer;
    int off = 0;
    
    foreach(line; lines[2..$])
    {
        foreach(num; std.string.split(line)[1..$])
        {
            uint n = from_hex(num);
            
            if(is_ascii(n))
            {
                s_buffer ~= cast(char)(n);
            }
            else
            {
                if(s_buffer.length > 3)
                {
                    result[start_address] = s_buffer.dup;
                }
                
                start_address += s_buffer.length + 1;
                s_buffer.length = 0;
            }
        }
    }
}

//Grab strings using strings
void get_raw_strings(inout char[][uint] result, char[] ubinary)
{
    Section[] sects = get_sections(ubinary);
    
    foreach(line; splitlines(shell("strings -o " ~ ubinary)))
    {
        //Strip out leading number
        int x;
        for(x=1; x<line.length; x++)
        {
            if(line[x] == ' ')
            {
                break;
            }
        }
        
        uint vaddr = compute_vaddr(std.string.atoi(line[0..x]), sects);
        char[] str = line[x+1..$].dup;
        
        //Get string and put it in the buffer
        result[vaddr] = str;
    }
}

//Extract the strings and their corresponding relative virtual addresses
char[][uint] get_strings(char[] ubinary)
{
    char[][uint] result;

    get_otool_strings(result, ubinary);
    get_raw_strings(result, ubinary);
    
    return result;
}

//Get the asm values
char[][] get_asm(char[] ubinary)
{
   return splitlines(shell("otool -t -V -v " ~ ubinary)); 
}

//Insert strings into assembler code
char[][] process_strings(char[][] code, char[][uint] offs)
{
    char[][] result;
    
    foreach(line; code)
    {
        result ~= line;
        
        foreach(m; RegExp(r"0x[0-9a-f]*").search(line))
        {
            uint x = from_hex(m.match(0));
            
            if(x in offs)
            {
                result[$-1] ~= format(" ; string: 0x%08x = \"%s\"", x, offs[x]);
            }
        }
    }
    
    return result;
}

//Get a list of all jump targets
uint[][uint] get_jump_targets(char[][] code)
{
    uint[][uint] targets;
    
    foreach(line; code)
    {
        foreach(m; RegExp(r"([0-9a-f]{8})\s*(call|calll|j\S*)\s*0x([0-9a-f]{8})").search(line))
        {
            uint l = from_hex(m.match(1));
            uint t = from_hex(m.match(3));
            
            targets[t] ~= l;
        }
    }
    
    return targets;
}

//Process all jumps
char[][] process_jumps(char[][] code, uint[][uint] jumps, char[][uint] subs)
{
    char[][] result;
    
    foreach(line; code)
    {
        if(line.length < 8)
        {
            result ~= line;
            continue;
        }
        
        uint vaddr = from_hex(line[0..8]);
        
        if(vaddr in jumps)
        {
            result ~= line ~ "\t; jump from: "; 
            
            foreach(jmp; jumps[vaddr])
                result[$-1] ~= format("%08x, ", jmp);
        }
        else
            result ~= line;
    }
    
    return result;
}

//Get a list of subroutines
char[][uint] get_subroutines(char[][] code)
{
    char[][uint] subs;
    
    for(int i=0; i<code.length - 1; i++)
    {
        if( RegExp(r"[0-9a-f]{8}\tpushl\t%ebp").test(code[i]) &&
            RegExp(r"[0-9a-f]{8}\tmovl\t%esp,%ebp").test(code[i+1]))
        {
            uint vaddr = from_hex(code[i][0..8]);
            
            if(i > 0 && code[i-1].length > 0 && code[i-1][$-1] == ':')
            {
                subs[vaddr] = code[i-1][0..$-1];
            }
            else
            {
                subs[vaddr] = format("sub_%08x", vaddr);
            }
        }
    }
    
    return subs;
}

//Traverse all lines and add annotations for unlabeled subroutines
char[][] process_subroutines(char[][] code, char[][uint] subs)
{
    char[][] result;
    
    for(int i=0; i<code.length; i++)
    {
        if(code[i].length < 8)
        {
            result ~= code[i];
            continue;
        }
        
        uint vaddr = from_hex(code[i][0..8]);
        
        if(vaddr in subs)
        {
            char[] sub_name = subs[vaddr];
            
            result ~= format("%s:", sub_name);
        }
        
        //Replace all occurences of subnames with a sub
        char[][] tok = RegExp(r"0x[0-9a-f]{8}").split(code[i]);
        char[][] sub_names;
        
        foreach(m; RegExp(r"0x([0-9a-f]{8})").search(code[i]))
        {
            uint vaddr = from_hex(m.match(1));
            
            if(vaddr in subs)
            {
                sub_names ~= subs[vaddr];
            }
            else
            {
                sub_names ~= format("0x%08x", vaddr);
            }
        }
        
        char[] str;
        for(int j=0; j<sub_names.length; j++)
            str ~= tok[j] ~ sub_names[j];
        str ~= tok[$-1];
        
        result ~= str;
    }
    
    return result;
}

//Fix up all symbols
char[][] fix_symbols(char[][] code)
{
    char[][] result;
    
    foreach(line; code)
    {
        char[][] toks = RegExp(r"; symbol stub for: ").split(line);
        
        if(toks.length == 2)
        {
            result ~= toks[0][0..$-11] ~ toks[1];
        }
        else
        {
            result ~= toks[0];
        }
    }
    
    return result;
}

//Get the names of objective C subroutines
void get_obj_c_subroutines(char[] ubinary, inout char[][uint] subroutines)
{
    char[][] names = splitlines(shell("otool -o -v " ~ ubinary));
    
    for(int i=0; i<names.length; i++)
    {
        foreach(m; RegExp(r"method_name 0x([0-9a-f]{8}) (.*)").search(names[i]))
        {
            char[] sym_name = m.match(2);
            uint sym_addr = from_hex(names[i+2][$-8..$]);
            
            subroutines[sym_addr] = sym_name;
        }
    }
}


int main(char[][] args)
{
    if(args.length != 2)
    {
        writefln("Incorrect arguments.");
        return -1;
    }
    
    auto ubinary = shell_escape(args[1]);
    auto asm_code = get_asm(ubinary);
    auto str_names = get_strings(ubinary);
    
    char[][uint] string_offs;
    char[][] strings;
    foreach(uint k, char[] str; str_names)
    {
        strings ~= format("%08x - %s", k, str);
        for(uint i=0; i<str.length; i++)
            string_offs[k + i] = str;
    }
    
    auto targets = get_jump_targets(asm_code);
    auto subroutines = get_subroutines(asm_code);
    
    get_obj_c_subroutines(ubinary, subroutines);
    
    auto str_code = fix_symbols(asm_code);
    str_code = process_subroutines(str_code, subroutines);
    str_code = process_jumps(str_code, targets, subroutines);
    str_code = process_strings(str_code, string_offs);    

    //Print out code
    writefln("---------------------------------\ncode:\n---------------------------------\n");
    foreach(line; str_code)
        writefln("%s", line);
    
    
    
    //Print out list of all string constants (comment out if not needed)
    writefln("\n---------------------------------\nstrings:\n---------------------------------\n");
    foreach(str; strings.sort)
        writefln("%s", str);
    
    return 0;
}