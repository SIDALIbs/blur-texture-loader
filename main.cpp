#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <string>
#include <stdio.h>

// Link the required DirectX libraries
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

HMODULE hOriginalD3D9 = NULL;

// Typedef for the original Direct3DCreate9 function
typedef IDirect3D9* (WINAPI* Direct3DCreate9_t)(UINT SDKVersion);
Direct3DCreate9_t pOriginalDirect3DCreate9 = NULL;

// Custom Device Wrapper (Only overriding what we need)
class myIDirect3DDevice9 : public IDirect3DDevice9 {
private:
    IDirect3DDevice9* originalDevice;

public:
    myIDirect3DDevice9(IDirect3DDevice9* pOriginal) {
        originalDevice = pOriginal;
    }

    // --- The Core Hook: Intercepting Texture Creation ---
    HRESULT STDMETHODCALLTYPE CreateTexture(UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle) {
        
        // 1. Calculate a unique hash. 
        // (Placeholder: Hashing based on parameters. Replace with CRC32 pixel hashing if exact TexMod parity is required).
        DWORD textureHash = (Width ^ Height ^ Format ^ Usage) * 0x9E3779B1; 
        
        // 2. Format it exactly like your target string (e.g., "0x7a00c450")
        char hashStr[32];
        sprintf_s(hashStr, sizeof(hashStr), "0x%08x", textureHash);

        // 3. Build the file path
        std::string filePath = ".\\textures\\";
        filePath += hashStr;
        filePath += ".dds";

        // 4. Check if the custom .dds file exists in the \textures\ folder
        DWORD fileAttr = GetFileAttributesA(filePath.c_str());
        if (fileAttr != INVALID_FILE_ATTRIBUTES && !(fileAttr & FILE_ATTRIBUTE_DIRECTORY)) {
            // FILE FOUND: Load your custom DDS instead of the game's default
            HRESULT hr = D3DXCreateTextureFromFileExA(
                originalDevice, 
                filePath.c_str(), 
                D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 
                0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, 
                D3DX_DEFAULT, D3DX_DEFAULT, 
                0, NULL, NULL, ppTexture
            );

            if (SUCCEEDED(hr)) {
                return hr; // Successfully swapped texture in GPU memory!
            }
        }

        // FILE NOT FOUND: Pass cleanly to the original function for peak performance
        return originalDevice->CreateTexture(Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle);
    }

    // --- Forwarding all other critical device calls to the original ---
    // (Note: In a full wrapper, you must forward EVERY method of IDirect3DDevice9. 
    // For brevity, assume all other standard methods like Release, BeginScene, DrawIndexedPrimitive, etc., are forwarded like this:)
    HRESULT STDMETHODCALLTYPE Present(CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion) {
        return originalDevice->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
    }
    // ... [Other IDirect3DDevice9 methods are forwarded identically] ...
};

// Custom Direct3D9 Wrapper
class myIDirect3D9 : public IDirect3D9 {
private:
    IDirect3D9* originalD3D;
public:
    myIDirect3D9(IDirect3D9* pOriginal) { originalD3D = pOriginal; }
    
    HRESULT STDMETHODCALLTYPE CreateDevice(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice9** ppReturnedDeviceInterface) {
        IDirect3DDevice9* pOriginalDevice = NULL;
        HRESULT hr = originalD3D->CreateDevice(Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, &pOriginalDevice);
        
        if (SUCCEEDED(hr) && pOriginalDevice != NULL) {
            *ppReturnedDeviceInterface = (IDirect3DDevice9*)new myIDirect3DDevice9(pOriginalDevice);
        }
        return hr;
    }
    // Forward other IDirect3D9 methods similarly...
};

// --- Exported Function to Catch the Engine's Startup ---
extern "C" IDirect3D9* WINAPI Direct3DCreate9(UINT SDKVersion) {
    if (!pOriginalDirect3DCreate9) return NULL;
    
    IDirect3D9* pOriginal = pOriginalDirect3DCreate9(SDKVersion);
    if (pOriginal) {
        return (IDirect3D9*)new myIDirect3D9(pOriginal);
    }
    return NULL;
}

// --- DLL Entry Point ---
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        
        // Load the REAL DirectX 9 from the Windows system folder
        char sysPath[MAX_PATH];
        GetSystemDirectoryA(sysPath, MAX_PATH);
        strcat_s(sysPath, "\\d3d9.dll");
        
        hOriginalD3D9 = LoadLibraryA(sysPath);
        if (hOriginalD3D9) {
            pOriginalDirect3DCreate9 = (Direct3DCreate9_t)GetProcAddress(hOriginalD3D9, "Direct3DCreate9");
        }
    } else if (ul_reason_for_call == DLL_PROCESS_DETACH) {
        if (hOriginalD3D9) FreeLibrary(hOriginalD3D9);
    }
    return TRUE;
}
