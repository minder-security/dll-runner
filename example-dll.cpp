#include "pch.h"
#include <windows.h>
#include <Shellapi.h>
#include <thread>
#include <chrono>
#include <string>

 
extern "C" __declspec(dllexport)
int ShowMessageBox(const char* text, UINT uType)
{
    return MessageBoxA(NULL, text, "DLL Message", uType);
}


extern "C" __declspec(dllexport)
void SuspendCurrentThread()
{
    std::this_thread::sleep_for(std::chrono::seconds(10));
}

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
