#import <Foundation/NSAutoreleasePool.h>
#import <AppKit/NSApplication.h>

@interface NSApplication(SDL_Missing_Methods)
- (void)setAppleMenu:(NSMenu *)menu;
@end


NSAutoreleasePool *globalPool = nil;

typedef struct CPSProcessSerNum
{
	unsigned int		lo;
	unsigned int		hi;
} CPSProcessSerNum;

extern int	CPSGetCurrentProcess( CPSProcessSerNum *psn);
extern int 	CPSEnableForegroundOperation( CPSProcessSerNum *psn, unsigned int _arg2, unsigned int _arg3, unsigned int _arg4, unsigned int _arg5);
extern int	CPSSetFrontProcess( CPSProcessSerNum *psn);

void nsAppInit()
{
    CPSProcessSerNum PSN;
    
    NSApplicationLoad();
    globalPool = [[NSAutoreleasePool alloc] init];

    CPSGetCurrentProcess(&PSN);
    CPSEnableForegroundOperation(&PSN,0x03,0x3C,0x2C,0x1103);
    CPSSetFrontProcess(&PSN);
}

void nsAppCleanup()
{
   [globalPool release];
}