#include <Windows.h>
#include <UIAutomation.h>
#include <iostream>
#include <cwctype>
#include <atlcomcli.h>
#include <tlhelp32.h>
#include <string.h>

IUIAutomation *pIUIAutomation;

DWORD FindProcessIdByName(const wchar_t *procname)
{
    HANDLE hSnapshot;
    PROCESSENTRY32W pe;
    int pid = 0;
    BOOL hResult;

    // snapshot of all processes in the system
    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (INVALID_HANDLE_VALUE == hSnapshot)
        return 0;

    // initializing size: needed for using Process32First
    pe.dwSize = sizeof(PROCESSENTRY32W);

    // info about first process encountered in a system snapshot
    hResult = Process32FirstW(hSnapshot, &pe);

    // retrieve information about the processes
    // and exit if unsuccessful
    while (hResult)
    {

        // std::wcout << pe.szExeFile << std::endl;
        // if we find the process: return process ID
        if (wcscmp(procname, pe.szExeFile) == 0)
        {
            pid = pe.th32ProcessID;
            break;
        }
        hResult = Process32NextW(hSnapshot, &pe);
    }

    // closes an open handle (CreateToolhelp32Snapshot)
    CloseHandle(hSnapshot);
    return pid;
}
struct FindWindowData
{
    DWORD processId;
    HWND hwnd;
};

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
    FindWindowData &data = *reinterpret_cast<FindWindowData *>(lParam);
    DWORD windowProcessId;
    GetWindowThreadProcessId(hwnd, &windowProcessId);
    if (windowProcessId == data.processId)
    {
        data.hwnd = hwnd;
        return FALSE; // Stop enumeration
    }
    return TRUE; // Continue enumeration
}

HWND FindWindowHandleFromProcessId(DWORD processId)
{
    FindWindowData data = {processId, nullptr};
    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&data));
    return data.hwnd;
}

std::wstring GetWindowClassName(HWND hwnd)
{
    wchar_t className[256]; // Adjust buffer size as needed
    if (GetClassNameW(hwnd, className, sizeof(className) / sizeof(wchar_t)) == 0)
    {
        // Error handling, if needed
        return L"";
    }
    return std::wstring(className);
}

void PrintTreeLayout(int depth, bool start = true)
{
    for (int i = 0; i < depth; ++i)
    {
        if (start)
        {
            std::cout << "----";
        }
        else
        {
            std::cout << "    ";
        }
    }
    std::cout << "|";
}
void TraverseElements(IUIAutomationElement *pElement, int depth = 0)
{
    if (!pElement)
        return;

    IUIAutomationCondition *pTrueCondition;
    pIUIAutomation->CreateTrueCondition(&pTrueCondition);

    // Get element properties
    BSTR name;
    pElement->get_CurrentName(&name);
    BSTR localizedControlType;
    pElement->get_CurrentLocalizedControlType(&localizedControlType);
    BSTR automationId;
    pElement->get_CurrentAutomationId(&automationId);

    // Print element information (you can modify this part as needed)
    PrintTreeLayout(depth);
    std::wcout << L"Name: " << (name ? name : L"(No Name)") << std::endl;
    PrintTreeLayout(depth, false);
    std::wcout << L"LocalizedControlType: " << (localizedControlType ? localizedControlType : L"(empty)") << std::endl;
    PrintTreeLayout(depth, false);
    std::wcout << L"AutomationID: " << (automationId ? automationId : L"(empty)") << std::endl;

    // Get child elements
    IUIAutomationElementArray *pChildren;
    pElement->FindAll(TreeScope_Children, pTrueCondition, &pChildren);
    int count;
    pChildren->get_Length(&count);

    // Recursively traverse child elements
    for (int i = 0; i < count; ++i)
    {
        IUIAutomationElement *pChild;
        pChildren->GetElement(i, &pChild);
        TraverseElements(pChild, depth + 1);
        pChild->Release();
    }
    pChildren->Release();
    SysFreeString(name);
}

int wmain(int argc, wchar_t *argv[])
{

    if (argc < 2)
    {
        std::cout << "No second command-line argument provided." << std::endl;
        return 1;
    }

    CoInitialize(NULL);

    CoCreateInstance(__uuidof(CUIAutomation), NULL, CLSCTX_INPROC_SERVER, IID_IUIAutomation, (void **)&pIUIAutomation);

    if (pIUIAutomation == NULL)
    {
        std::cerr << "Failed to create instance of UI Automation." << std::endl;
        CoUninitialize();
        return 1;
    }

    // Get the root element of the desktop
    IUIAutomationElement *pRootElement;
    pIUIAutomation->GetRootElement(&pRootElement);

    if (pRootElement == NULL)
    {
        std::cerr << "Failed to get root element." << std::endl;
        pIUIAutomation->Release();
        CoUninitialize();
        return 1;
    }

    IUIAutomationElement *pWindow;
    IUIAutomationCondition *pCondition;

    DWORD processID = FindProcessIdByName(argv[1]);

    HWND handleWindow = FindWindowHandleFromProcessId(processID);

    std::wstring classNameWindow = GetWindowClassName(handleWindow);

    pIUIAutomation->CreatePropertyCondition(UIA_ClassNamePropertyId, CComVariant(classNameWindow.c_str()), &pCondition);
    pRootElement->FindFirst(TreeScope_Children, pCondition, &pWindow);

    if (pWindow == NULL)
    {
        std::cerr << "Failed to get window element." << std::endl;
        pIUIAutomation->Release();
        pRootElement->Release();
        CoUninitialize();
        return 1;
    }

    while (true)
    {
        // Check if the 'Z' key is pressed
        if (GetAsyncKeyState('Z') & 0x8000)
        {
            // std::cout << "'Z' key pressed!" << std::endl;
            system("cls");

            BSTR name;
            pWindow->get_CurrentName(&name);
            BSTR className;
            pWindow->get_CurrentClassName(&className);
            TraverseElements(pWindow);
        }

        // Check if the Escape key is pressed to exit the loop
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
        {
            break;
        }

        // Add a small delay to reduce CPU usage
        Sleep(10); // Sleep for 10 milliseconds
    }
    pCondition->Release();
    pRootElement->Release();
    pIUIAutomation->Release();
    CoUninitialize();
    return 0;
}
