#include <Windows.h>
#include <UIAutomation.h>
#include <iostream>
#include <cwctype>
#include <atlcomcli.h>

IUIAutomation *pIUIAutomation;

std::wstring removeInvisibleCharacters(const std::wstring &str)
{
    std::wstring result;
    for (wchar_t c : str)
    {
        if (!std::iswspace(c) && c != L'\0')
        {
            result += c;
        }
    }
    return result;
}
std::wstring BSTRToWString(BSTR bstr)
{
    std::wstring result;
    if (bstr != nullptr)
    {
        result = bstr;
    }
    return result;
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

    // Print element information (you can modify this part as needed)
    for (int i = 0; i < depth; ++i)
        std::cout << "----";
    std::cout << "|";

    std::wcout << (name ? BSTRToWString(name) : L"(No Name)") << std::endl;

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

int main()
{
    CoInitialize(NULL);

    CoCreateInstance(__uuidof(CUIAutomation), NULL, CLSCTX_INPROC_SERVER, IID_IUIAutomation, (void **)&pIUIAutomation);

    if (!pIUIAutomation)
    {
        std::cerr << "Failed to create instance of UI Automation." << std::endl;
        CoUninitialize();
        return 1;
    }

    // Get the root element of the desktop
    IUIAutomationElement *pRootElement;
    pIUIAutomation->GetRootElement(&pRootElement);

    if (!pRootElement)
    {
        std::cerr << "Failed to get root element." << std::endl;
        pIUIAutomation->Release();
        CoUninitialize();
        return 1;
    }

    IUIAutomationElement *pWindow;
    IUIAutomationCondition *pCondition;

    BSTR name;
    pRootElement->get_CurrentName(&name);

    pIUIAutomation->CreatePropertyCondition(UIA_ClassNamePropertyId, CComVariant(L"Notepad"), &pCondition);

    pRootElement->FindFirst(TreeScope_Children, pCondition, &pWindow);
    if (pWindow != NULL)
    {

        BSTR name;
        pWindow->get_CurrentName(&name);
        BSTR className;
        pWindow->get_CurrentClassName(&className);
        TraverseElements(pWindow);
    }

    pCondition->Release();

    // Traverse all elements recursively starting from the root

    pRootElement->Release();
    pIUIAutomation->Release();
    CoUninitialize();

    return 0;
}
