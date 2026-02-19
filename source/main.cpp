#include <windows.h>
#include "Application.h"

#include <windows.h>
#include <iostream>

int main(int argc, char* argv[])
{
    try
    {
        bool enableDebug = false;
        for (int i = 1; i < argc; ++i)
        {
            if (strcmp(argv[i], "--debuglayer") == 0)
            {
	            enableDebug = true;
            }
        }

        Application app{ enableDebug };
    }
    catch (const std::exception& e)
    {
        MessageBoxA(nullptr, e.what(), "Error", MB_OK | MB_ICONERROR);
        return 1;
    }
}