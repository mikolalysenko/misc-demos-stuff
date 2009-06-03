module platform;

import derelict.util.loader;

class Platform
{
    static this()
    {
        platformLib = null;
    }

    static ~this()
    {
        cleanup();
    }

    static void init()
    {
        version (darwin)
        {
            if (platformLib is null)
            {
                platformLib = Derelict_LoadSharedLib(PLATFORM_LIB_NAME);
                void function() nsAppInit = cast(void function()) Derelict_GetProc(platformLib, "nsAppInit");
                nsAppInit();
            }
        }
    }

    static void cleanup()
    {
        version (darwin)
        {
            if (platformLib !is null)
            {
                void function() nsAppCleanup = cast(void function()) Derelict_GetProc(platformLib, "nsAppCleanup");
                nsAppCleanup();
                Derelict_UnloadSharedLib(platformLib);
                platformLib = null;
            }
        }
    }

    private
    {
        static SharedLib platformLib;

        version (darwin)
        {
            static const char[] PLATFORM_LIB_NAME = "nsappbits.dylib";
        }
    }
}