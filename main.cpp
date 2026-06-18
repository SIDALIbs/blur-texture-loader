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
        // Dynamically load the graphics extension helper to preserve compatibility
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

// Function pointer signature for the original CreateDevice method
typedef HRESULT(WINAPI* CreateDevice_t)(void* pD3D, UINT Adapter, int DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, void* pPresentationParameters, void** ppReturnedDeviceInterface);
CreateDevice_t OriginalCreateDevice = NULL;

// Hooked CreateDevice to tap the rendering device the moment the game builds it
HRESULT WINAPI HookedCreateDevice(void* pD3D, UINT Adapter, int DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, void* pPresentationParameters, void** ppReturnedDeviceInterface) {
    HRESULT hr = OriginalCreateDevice(pD3D, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);
    
    if (SUCCEEDED(hr) && ppReturnedDeviceInterface && *ppReturnedDeviceInterface) {
        void* pDevice = *ppReturnedDeviceInterface;
        uintptr_t* vtable = *(uintptr_t**)pDevice;

        // Index 23 is the CreateTexture function in the Direct3D 9 Device VTable
        if ((CreateTexture_t)vtable[23] != HookedCreateTexture) {
            OriginalCreateTexture = (CreateTexture_t)vtable[23];
            
            // Unprotect the memory page to swap the pointer
            DWORD oldProtect;
            VirtualProtect(&vtable[23], sizeof(uintptr_t), PAGE_EXECUTE_READWRITE, &oldProtect);
            vtable[23] = (uintptr_t)HookedCreateTexture;
            VirtualProtect(&vtable[23], sizeof(uintptr_t), oldProtect, &oldProtect);
        }
    }
    return hr;
}

// Main proxy interception at initial startup bootstrap
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
        
        // Index 16 is the CreateDevice function in the IDirect3D9 VTable
        if ((CreateDevice_t)vtable[16] != HookedCreateDevice) {
            OriginalCreateDevice = (CreateDevice_t)vtable[16];
            
            DWORD oldProtect;
            VirtualProtect(&vtable[16], sizeof(uintptr_t), PAGE_EXECUTE_READWRITE, &oldProtect);
            vtable[16] = (uintptr_t)HookedCreateDevice;
            VirtualProtect(&vtable[16], sizeof(uintptr_t), oldProtect, &oldProtect);
        }
    }
    return pD3DObject;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    return TRUE;
}
