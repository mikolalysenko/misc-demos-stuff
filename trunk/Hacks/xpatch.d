import 
    std.file,
    std.format, 
    std.regexp, 
    std.string, 
    std.stdio, 
    cshell,
    misc,
    macho;
    
char[] log_filename(char[] filename)
{
    return filename ~ ".xpatch";
}

ubyte to_hex(char c)
{
    if('0' <= c && c <= '9')
        return c - '0';
    
    if('a' <= c && c <= 'f')
        return c - 'a' + 10;
    
    return 0;
}

ubyte[] to_hex_bytes(char[] str)
{
    ubyte[] r;
    
    for(int i=0; i<str.length; i+=2)
    {
        r ~= to_hex(str[i]) * 16 + to_hex(str[i+1]);
    }
    
    return r;
}

char[] cat_lines(char[][] lines)
{
    char[] result;
    
    foreach(l; lines)
    {
        result ~= l ~ "\n";
    }
    
    return result;
}


void undo_write(char[] filename, uint offset)
{
    //Check if log file exists
    char[] logname = log_filename(filename);
    
    if(!exists(logname))
    {
        throw new Exception("Log file does not exist.");
    }
    
    char[][] logfile_data = splitlines(cast(char[])read(logname));
    
    for(int i=logfile_data.length - 1; i>=0; i--)
    {
        char[][] line = std.string.split(logfile_data[i], ":");
        
        uint x = from_hex(line[0]);
        
        //Check for hit
        if(x == offset)
        {
            uint start = x;
            uint end = x + line[0].length / 2;
            
            for(int j=i+1; j<logfile_data.length; j++)
            {
                char[][] rline = std.string.split(logfile_data[j], ":");
                
                if(rline.length < 2)
                    continue;
                
                uint rstart = from_hex(rline[0]);
                uint rend = rstart + rline[1].length / 2;
                
                if((rstart <= start && start < rend) ||
                    (rstart <= end && end < rend))
                {
                    throw new Exception(format("Can not undo patch %08x until patch at %08x is undone.", start, rstart));
                }
            }

            
            
            //Execute hex patch
            ubyte[] hex = to_hex_bytes(line[1]);
            
            //Patch file
            ubyte[] file_data = cast(ubyte[])read(filename);
            file_data[offset..offset+hex.length] = hex;
            write(filename, cast(void[])file_data);
            
            //Remove from log file
            write(logname, cat_lines(logfile_data[0..i] ~ logfile_data[i+1..$]));
            
            return;
        }
        
    }
    
    //Couldn't find a valid patch :(
    throw new Exception("No patch exists for given address.");
}

void do_write(char[] filename, uint offset, ubyte[] values)
{
    ubyte[] bits = cast(ubyte[])read(filename);
    
    if(bits.length < offset + values.length)
        throw new Exception("Cannot write past EOF.\n");
    
    //Create log record
    char[] log_msg = format("%08x:", offset);
    foreach(b; bits[offset..offset+values.length])
        log_msg ~= format("%02x", b);
    log_msg ~= "\n";
    
    //Save change to log file
    append(log_filename(filename), log_msg);
    
    //Patch file in memory
    bits[offset..offset+values.length] = values;
    
    //Save changes
    write(filename, bits);
}



void write_asm(char[] filename, uint vaddr, uint offset, char[][] asm_strings)
{
    writefln("writing asm: %08x, %08x", vaddr, offset);
    
    //Use nasm to assemble arguments with semicolon separators
    char[] asm_code = format("[org 0x%x]\n[bits 32]\n", vaddr);
    foreach(cmd; asm_strings)
    {
        asm_code ~= std.string.replace(cmd, ";", "\n") ~ " ";
    }
    
    //Write asm to file
    write("/tmp/xpatch.asm", cast(void[])asm_code);
    
    //Assemble code
    char[] result = shell("nasm /tmp/xpatch.asm");
    std.file.remove("/tmp/xpatch.asm");
    
    if(result.length != 0)
    {
        throw new Exception("ASM syntax error:\n" ~ result);
    }
    
    //Read in asm data
    ubyte[] asm_data = cast(ubyte[])read("/tmp/xpatch");
    std.file.remove("/tmp/xpatch");
    
    do_write(filename, offset, asm_data);
}

void write_hex(char[] filename, uint offset, char[][] hex_strings)
{
    char[] hex_bytes;
    
    foreach(arg; hex_strings)
        hex_bytes ~= RegExp(r"\s").replace(tolower(arg), "");
    
    if(hex_bytes.length % 2 != 0)
    {
        throw new Exception("Invalid hex string");
    }
    
    do_write(filename, offset, to_hex_bytes(hex_bytes));
}

void write_str(char[] filename, uint offset, char[][] strings)
{
    char[] str;
    
    foreach(arg; strings)
        str ~= arg;
    
    do_write(filename, offset, cast(ubyte[])str);
}



void show_help(char[] err_msg)
{
    writefln(
"Error: %s\n"
"\n"
"Description:\n"
"   Mac OS X Mach-O file patcher.  Simplifies the problem of editing binary files on MacOS X.\n"
"\n"
"Usage:\n"
"   xpatch <filename> -[f|m] <address> -[u|a|b|s] <arguments>\n"
"\n"
"Where:\n"
"   filename = The file which is being patched.\n"
"   -f       = Use file based addressing\n"
"   -m       = Use memory mapped addressing (default)\n"
"   address  = The address to patch\n"
"   -[u|a|b|s|v] = Action to perform:\n"
"\n"
"Actions:\n"
"   -u (Undo) = Undo the last patch applied to the given memory address in file.\n"
"   -a (Assembler) = Patch in the specified assembly instructions (separated by semicolons).\n"
"   -b (Binary) = Patch in hexadecimal values\n"
"   -s (String) = Patch in string values\n"
"\n", err_msg);
    return;
}


int main(char[][] args)
{
    char[] filename;    
    if(args.length < 4)
    {
        show_help("Incorrect arguments");
        return -1;
    }
    filename = args[1];
    
    if(!exists(filename))
    {
        writefln("File %s does not exist.\n", filename);
        return -1;
    }
    
    //Current argument
    int x = 2;
    
    //Handle addressing mode
    bool use_mmap = true;
    if(args[2][0] == '-')
    {
        if(args.length < 5 ||
            args[2].length != 2)
        {
            show_help("Incorrect number of arguments or bad addressing mode\n");
            return -1;
        }
        
        switch(args[2][1])
        {
            case 'f':
                use_mmap = false;
            break;
            
            case 'm':
                use_mmap = true;
            break;
            
            default:
                show_help("Unknown or incorrect addressing mode");
                return -1;
        }
        
        x++;
    }
    
    //Read in the address
    Section[] sections = get_sections(shell_escape(filename));
    uint address = from_hex(args[x]);
    x++;
    
    //Handle addressing mode computation
    if(use_mmap)
    {
        uint naddr = compute_offset(address, sections);
        
        if(naddr == 0)
        {
            writefln("Unmapped virtual address 0x%08x in Mach-O %s.", address, filename);
        }
        
        address = naddr;
    }
    
    
    //Determine command
    if(args[x].length != 2 && args[x][0] != '-')
    {
        show_help("Unknown or missing command");
        return -1;
    }
    
    //Pull out arguments
    char[][] cmd_args = args[x+1..$];
    
    switch(args[x][1])
    {
        case 'u':
            //Undo last action on given memory address
            undo_write(filename, address);
        break;
        
        case 'a':
            //Assemble command at address
            write_asm(filename, compute_vaddr(address, sections), address, cmd_args);
        break;
        
        case 'b':
            //Overwrite binary stuff at address
            write_hex(filename, address, cmd_args);
        break;
        
        case 's':
            //Overwrite string at address
            write_str(filename, address, cmd_args);
        break;
        
        default:
            show_help("Invalid command");
            return -1;
    }
        
    return 0;
}