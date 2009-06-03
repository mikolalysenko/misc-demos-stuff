module macho;

import
    std.stdio,
    std.string,
    misc,
    cshell;


//A map region is a file region
public struct Section
{
    uint file_addr;
    uint virt_addr;
    uint size;
}

//Read in the sections for a ubinary
public Section[] get_sections(char[] ubinary)
{
    auto lines = splitlines(shell("otool -l " ~ ubinary));
    Section[] result;
    
    uint vm_offset = 0;
    uint file_offset = 0;
    
    for(int i=0; i<lines.length; i++)
    {
        if(lines[i] == "Section")
        {
            Section sect;
            
            sect.file_addr = std.string.atoi(std.string.split(lines[i + 5])[1]) +  0x1000; //+ 0x5f000;
            sect.virt_addr = from_hex(std.string.split(lines[i + 3])[1]);
            sect.size      = from_hex(std.string.split(lines[i + 4])[1]);
            
            if(sect.file_addr == 0)
                continue;
           /* 
            writefln("Got section: vm:%08x, fo:%08x, sz:%08x",
                sect.virt_addr,
                sect.file_addr,
                sect.size);
            */
            result ~= sect;
        }
        else
        {
            char[][] line = std.string.split(lines[i]);
            
            if(line[0] == "vmaddr")
            {
                vm_offset = from_hex(line[1]);
            }
            else if(line[0] == "fileoff")
            {
                file_offset = std.string.atoi(line[1]);
            }
        }
    }
    
    return result;
}

//Compute the virtual address from a file offset
public uint compute_vaddr(uint file_offset, Section[] sects)
{
    foreach(s; sects)
    {
        if(s.file_addr <= file_offset)
        {
            uint t = file_offset - s.file_addr;
            
            if(t < s.size)
                return s.virt_addr + t;
        }
    }
    
    return 0;
}

//Compute the file offset given a virtual address
public uint compute_offset(uint vaddr, Section[] sects)
{
    uint r = 0;
    
    foreach(s; sects)
    {
        if(s.virt_addr <= vaddr && vaddr < s.virt_addr + s.size)
        {
            r = (vaddr - s.virt_addr) + s.file_addr;
            return r;
        }
    }
    
    return r;
}
