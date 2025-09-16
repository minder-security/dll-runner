#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ID_DLL_PATH      101
#define ID_RETURN_TYPE   102
#define ID_FUNC_NAME     103
#define ID_ARGS          104
#define ID_CALL_BUTTON   105
#define ID_OUTPUT        106

HWND hwndOutput;

// Forward declaration
void CallFunction(HWND hwndDlg);

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
        case WM_CREATE:
        {
            CreateWindow("STATIC", "DLL Path:", WS_VISIBLE | WS_CHILD, 10, 10, 80, 20, hwnd, NULL, NULL, NULL);
            CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, 100, 10, 400, 20, hwnd, (HMENU)ID_DLL_PATH, NULL, NULL);

            CreateWindow("STATIC", "Return Type:", WS_VISIBLE | WS_CHILD, 10, 40, 80, 20, hwnd, NULL, NULL, NULL);
            CreateWindow("EDIT", "void", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, 100, 40, 100, 20, hwnd, (HMENU)ID_RETURN_TYPE, NULL, NULL);

            CreateWindow("STATIC", "Function Name:", WS_VISIBLE | WS_CHILD, 10, 70, 80, 20, hwnd, NULL, NULL, NULL);
            CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, 100, 70, 200, 20, hwnd, (HMENU)ID_FUNC_NAME, NULL, NULL);

            CreateWindow("STATIC", "Arguments (comma-separated):", WS_VISIBLE | WS_CHILD, 10, 100, 180, 20, hwnd, NULL, NULL, NULL);
            CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, 200, 100, 300, 20, hwnd, (HMENU)ID_ARGS, NULL, NULL);

            CreateWindow("BUTTON", "Call Function", WS_VISIBLE | WS_CHILD, 10, 130, 150, 30, hwnd, (HMENU)ID_CALL_BUTTON, NULL, NULL);

            hwndOutput = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY, 
                10, 170, 490, 150, hwnd, (HMENU)ID_OUTPUT, NULL, NULL);
        }
        break;

        case WM_COMMAND:
            if (LOWORD(wParam) == ID_CALL_BUTTON) {
                CallFunction(hwnd);
            }
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

// Helper: trim leading/trailing spaces
char* trim(char* str) {
    char* end;
    while (isspace((unsigned char)*str)) str++;
    if (*str == 0) return str;
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    *(end + 1) = 0;
    return str;
}

// Parse arguments by splitting on commas (very simple)
int parse_args(char* input, char* argv[], int max_args) {
    int argc = 0;
    char* token = strtok(input, ",");
    while(token && argc < max_args) {
        argv[argc++] = trim(token);
        token = strtok(NULL, ",");
    }
    return argc;
}

// Core: load DLL, find function, call with very limited signature support
void CallFunction(HWND hwndDlg) {
    char dllPath[512], returnType[64], funcName[128], argsText[512];
    GetWindowText(GetDlgItem(hwndDlg, ID_DLL_PATH), dllPath, sizeof(dllPath));
    GetWindowText(GetDlgItem(hwndDlg, ID_RETURN_TYPE), returnType, sizeof(returnType));
    GetWindowText(GetDlgItem(hwndDlg, ID_FUNC_NAME), funcName, sizeof(funcName));
    GetWindowText(GetDlgItem(hwndDlg, ID_ARGS), argsText, sizeof(argsText));

    // Load DLL
    HMODULE hMod = LoadLibraryA(dllPath);
    if (!hMod) {
        SetWindowText(hwndOutput, "Failed to load DLL.");
        return;
    }

    FARPROC rawFunc = GetProcAddress(hMod, funcName);
    if (!rawFunc) {
        SetWindowText(hwndOutput, "Function not found in DLL.");
        FreeLibrary(hMod);
        return;
    }

    // Parse args
    char* argv[10];
    int argc = parse_args(argsText, argv, 10);

    char outputBuffer[1024];
    outputBuffer[0] = 0;

    // Simple calls for demo (expand as needed)
    if (strcmp(returnType, "void") == 0) {
        if (argc == 0) {
            typedef void (*FuncType)(void);
            ((FuncType)rawFunc)();
            strcpy(outputBuffer, "Called void func(void)");
        }
        else if (argc == 1) {
            // Try int or string guess
            if (isdigit(argv[0][0]) || (argv[0][0] == '-' && isdigit(argv[0][1]))) {
                typedef void (*FuncType)(int);
                ((FuncType)rawFunc)(atoi(argv[0]));
                sprintf(outputBuffer, "Called void func(int): %d", atoi(argv[0]));
            } else {
                typedef void (*FuncType)(const char*);
                ((FuncType)rawFunc)(argv[0]);
                sprintf(outputBuffer, "Called void func(const char*): %s", argv[0]);
            }
        }
        else if (argc == 2) {
            typedef void (*FuncType)(int, const char*);
            ((FuncType)rawFunc)(atoi(argv[0]), argv[1]);
            sprintf(outputBuffer, "Called void func(int, const char*): %d, %s", atoi(argv[0]), argv[1]);
        }
        else {
            strcpy(outputBuffer, "Unsupported argument count for void return type.");
        }
    }
    else if (strcmp(returnType, "int") == 0) {
        if (argc == 0) {
            typedef int (*FuncType)(void);
            int res = ((FuncType)rawFunc)();
            sprintf(outputBuffer, "Called int func(void), returned %d", res);
        }
        else if (argc == 2) {
            typedef int (*FuncType)(const char*, unsigned int);
            int res = ((FuncType)rawFunc)(argv[0], (unsigned int)atoi(argv[1]));
            sprintf(outputBuffer, "Called int func(const char*, unsigned int), returned %d", res);
        }
        else {
            strcpy(outputBuffer, "Unsupported argument count for int return type.");
        }
    }
    else {
        strcpy(outputBuffer, "Unsupported return type.");
    }

    SetWindowText(hwndOutput, outputBuffer);

    FreeLibrary(hMod);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "XLoaderWndClass";

    if (!RegisterClass(&wc)) {
        MessageBox(NULL, "Failed to register window class", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    HWND hwnd = CreateWindow(wc.lpszClassName, "DLL Function Caller", WS_OVERLAPPEDWINDOW,
                             CW_USEDEFAULT, CW_USEDEFAULT, 530, 370,
                             NULL, NULL, hInstance, NULL);

    if (!hwnd) {
        MessageBox(NULL, "Failed to create window", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}
