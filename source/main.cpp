#include "Application.h"

#include <windows.h>
#include <iostream>

int main()
{
	try
	{
		Application app{};
	}
	catch (const std::exception& e)
	{
		MessageBoxA(nullptr, e.what(), "Error", MB_OK | MB_ICONERROR);
		return 1;
	}
}
