#include <windows.h>
#include <string>
#include <stdio.h>

// Function pointer signature for the original CreateTexture method
typedef HRESULT(WINAPI* CreateTexture_t)(void* Device, UINT Width, UINT Height, UINT Levels, DWORD Usage, int Format, int Pool, void** ppTexture, HANDLE* pSharedHandle);
CreateTexture_t OriginalCreateTexture = NULL;

// Function pointer signature for the D3DX texture loader function
typedef HRESULT(WINAPI* D3DXCreateTextureFromFileA_t)(void* Device, const char* pSrcFile, void** ppTexture);

// Our custom hooked function that intercepts texture creation
HRESULT WINAPI HookedCreateTexture(void* Device, UINT Width, UINT Height, UINT Levels, DWORD Usage, int Format, int Pool, void** ppTexture, HANDLE* pSharedHandle) {
    char hashStr[32];
    unsigned int textureHash = (Width ^ Height ^ Format ^ Usage) * 0x9E3779B1;
    _snprintf_s(hashStr, sizeof(hashStr), _TRUNCATE, "0x%08x", textureHash);

    std::string filePath = ".\\textures\\";
    filePath += hashStr;
    filePath += ".dds";

    // Look for a replacement asset inside the local .\textures\ folder
    DWORD fileAttr = GetFileAttributesA(filePath.c_str());
    if (fileAttr != INVALID_FILE_ATTRIBUTES && !(fileAttr & FILE_ATTRIBUTE_DIRECTORY)) {
        HMODULE hD3DX = LoadLibraryA("d3dx9_43.dll");
        if (!hD3DX) hD3DX = LoadLibraryA("d3dx9_42.dll");
        
        if (hD3DX) {
            D3DXCreateTextureFromFileA_t pD3DXLoad = (D3DXCreateTextureFromFileA_t)GetProcAddress(hD3DX, "D3DXCreateTextureFromFileA");
            if (pD3DXLoad && SUCCEEDED(pD3DXLoad(Device, filePath.c_str(), ppTexture))) {
                FreeLibrary(hD3DX);
                return S_OK; // Swap successful!
            }
            FreeLibrary(hD3DX);
        }
    }

    // Pass cleanly back to the engine's original function if no loose asset matches
    return OriginalCreateTexture(Device, Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle);
}

// Custom wrapper class for IDirect3D9 to intercept ONLY the device creation cleanly
class CustomIDirect3D9 {
public:
    void* originalD3D;
    
    static HRESULT WINAPI HookedCreateDevice(void* pD3D, UINT Adapter, int DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, void* pPresentationParameters, void** ppReturnedDeviceInterface) {
        // Find the original function location from the VTable dynamically
        uintptr_t* d3dVtable = *(uintptr_t**)pD3D;
        typedef HRESULT(WINAPI* CreateDevice_Fn)(void*, UINT, int, HWND, DWORD, void*, void**);
        CreateDevice_Fn RealCreateDevice = (CreateDevice_Fn)d3dVtable[16];
        
        HRESULT hr = RealCreateDevice(pD3D, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);
        
        if (SUCCEEDED(hr) && ppReturnedDeviceInterface && *ppReturnedDeviceInterface) {
            void* pDevice = *ppReturnedDeviceInterface;
            uintptr_t* deviceVtable = *(uintptr_t**)pDevice;

            // Securely hook the CreateTexture pointer at index 23
            if ((CreateTexture_t)deviceVtable[23] != HookedCreateTexture) {
                OriginalCreateTexture = (CreateTexture_t)deviceVtable[23];
                
                DWORD oldProtect;
                VirtualProtect(&deviceVtable[23], sizeof(uintptr_t), PAGE_EXECUTE_READWRITE, &oldProtect);
                deviceVtable[23] = (uintptr_t)HookedCreateTexture;
                VirtualProtect(&deviceVtable[23], sizeof(uintptr_t), oldProtect, &oldProtect);
            }
        }
        return hr;
    }
};

// Main entry interceptor
extern "C" void* WINAPI Direct3DCreate9(UINT SDKVersion) {
    char sysPath[MAX_PATH];
    GetSystemDirectoryA(sysPath, MAX_PATH);
    std::string realD3DPath = std::string(sysPath) + "\\d3d9.dll";
    
    HMODULE hRealD3D = LoadLibraryA(realD3DPath.c_str());
    if (!hRealD3D) return NULL;

    typedef void* (WINAPI* D3DCreate9_t)(UINT);
    D3DCreate9_t pRealCreate = (D3DCreate9_t)GetProcAddress(hRealD3D, "Direct3DCreate9");
    void* pD3DObject = pRealCreate(SDKVersion);

    if (pD3DObject) {
        uintptr_t* vtable = *(uintptr_t**)pD3DObject;
        
        // Swap the CreateDevice method pointer directly at startup safely
        DWORD oldProtect;
        VirtualProtect(&vtable[16], sizeof(uintptr_t), PAGE_EXECUTE_READWRITE, &oldProtect);
        vtable[16] = (uintptr_t)CustomIDirect3D9::HookedCreateDevice;
        VirtualProtect(&vtable[16], sizeof(uintptr_t), oldProtect, &oldProtect);
    }
    return pD3DObject;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    return TRUE;
}
