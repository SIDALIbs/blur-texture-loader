#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <time.h>
#include <stdio.h>

// Global font interface for our on-screen text
static ID3DXFont* g_pFont = NULL;

class DummyDevice9 : public IDirect3DDevice9 {
private:
    IDirect3DDevice9* realDevice;
public:
    DummyDevice9(IDirect3DDevice9* original) : realDevice(original) {}

    // Hooking EndScene to draw the clock
    STDMETHOD(EndScene)(THIS) {
        if (!g_pFont) {
            D3DXCreateFont(realDevice, 30, 0, FW_BOLD, 0, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial", &g_pFont);
        }

        if (g_pFont) {
            time_t rawtime;
            struct tm timeinfo;
            char buffer[80];
            time(&rawtime);
            localtime_s(&timeinfo, &rawtime);
            strftime(buffer, sizeof(buffer), "%H:%M:%S", &timeinfo);

            RECT rect = { 10, 10, 300, 50 };
            g_pFont->DrawText(NULL, buffer, -1, &rect, DT_LEFT, D3DCOLOR_ARGB(255, 255, 255, 255));
        }

        return realDevice->EndScene();
    }

    // Boilerplate redirectors (same as before)
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj) { HRESULT hr = realDevice->QueryInterface(riid, ppvObj); if (SUCCEEDED(hr) && *ppvObj == realDevice) *ppvObj = this; return hr; }
    STDMETHOD_(ULONG,AddRef)(THIS) { return realDevice->AddRef(); }
    STDMETHOD_(ULONG,Release)(THIS) { ULONG res = realDevice->Release(); if (res == 0) delete this; return res; }
    
    // ... [Include all other standard D3D9 methods here just like the previous block] ...
    // NOTE: Keep the rest of the methods the same as the previous block to ensure full implementation
    
    // Quick helper to redirect everything else to avoid compiler errors
    STDMETHOD(TestCooperativeLevel)(THIS) { return realDevice->TestCooperativeLevel(); }
    // (Fill in the remaining methods from the previous block here)
};

// ... [Keep DummyInterface9 and Direct3DCreate9 the same as before] ...
