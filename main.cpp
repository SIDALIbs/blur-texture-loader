#include <windows.h>
#include <string>
#include <d3d9.h>

// Dynamic function pointers for runtime extensions
typedef HRESULT(WINAPI* D3DXCreateTextureFromFileA_t)(IDirect3DDevice9* Device, const char* pSrcFile, IDirect3DTexture9** ppTexture);

// Forward declaration of our wrapper device
class DummyDevice9 : public IDirect3DDevice9 {
private:
    IDirect3DDevice9* realDevice;
public:
    DummyDevice9(IDirect3DDevice9* original) : realDevice(original) {}

    // The only function we actually care about intercepting
    STDMETHOD(CreateTexture)(THIS_ UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle) {
        char hashStr[32];
        unsigned int textureHash = (Width ^ Height ^ Format ^ Usage) * 0x9E3779B1;
        _snprintf_s(hashStr, sizeof(hashStr), _TRUNCATE, "0x%08x", textureHash);

        std::string filePath = ".\\textures\\";
        filePath += hashStr;
        filePath += ".dds";

        DWORD fileAttr = GetFileAttributesA(filePath.c_str());
        if (fileAttr != INVALID_FILE_ATTRIBUTES && !(fileAttr & FILE_ATTRIBUTE_DIRECTORY)) {
            HMODULE hD3DX = LoadLibraryA("d3dx9_43.dll");
            if (!hD3DX) hD3DX = LoadLibraryA("d3dx9_42.dll");
            
            if (hD3DX) {
                D3DXCreateTextureFromFileA_t pD3DXLoad = (D3DXCreateTextureFromFileA_t)GetProcAddress(hD3DX, "D3DXCreateTextureFromFileA");
                if (pD3DXLoad && SUCCEEDED(pD3DXLoad(realDevice, filePath.c_str(), ppTexture))) {
                    FreeLibrary(hD3DX);
                    return S_OK; // Custom texture successfully injected!
                }
                FreeLibrary(hD3DX);
            }
        }
        return realDevice->CreateTexture(Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle);
    }

    // Pass every other standard method straight down the line to the real GPU interface
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj) { return realDevice->QueryInterface(riid, ppvObj); }
    STDMETHOD_(ULONG,AddRef)(THIS) { return realDevice->AddRef(); }
    STDMETHOD_(ULONG,Release)(THIS) { ULONG res = realDevice->Release(); if (res == 0) { delete this; } return res; }
    STDMETHOD(TestCooperativeLevel)(THIS) { return realDevice->TestCooperativeLevel(); }
    STDMETHOD_(UINT, GetAvailableTextureMem)(THIS) { return realDevice->GetAvailableTextureMem(); }
    STDMETHOD(EvictManagedResources)(THIS) { return realDevice->EvictManagedResources(); }
    STDMETHOD(GetDirect3D)(THIS_ IDirect3D9** ppD3D9) { return realDevice->GetDirect3D(ppD3D9); }
    STDMETHOD(GetDeviceCaps)(THIS_ D3DCAPS9* pCaps) { return realDevice->GetDeviceCaps(pCaps); }
    STDMETHOD(GetDisplayMode)(THIS_ UINT iSwapChain,D3DDISPLAYMODE* pMode) { return realDevice->GetDisplayMode(iSwapChain, pMode); }
    STDMETHOD(GetCreationParameters)(THIS_ D3DDEVICE_CREATION_PARAMETERS *pParameters) { return realDevice->GetCreationParameters(pParameters); }
    STDMETHOD(SetCursorProperties)(THIS_ UINT XHotSpot,UINT YHotSpot,IDirect3DSurface9* pCursorBitmap) { return realDevice->SetCursorProperties(XHotSpot, YHotSpot, pCursorBitmap); }
    STDMETHOD_(void, SetCursorPosition)(THIS_ int X,int Y,DWORD Flags) { realDevice->SetCursorPosition(X, Y, Flags); }
    STDMETHOD_(BOOL, ShowCursor)(THIS_ BOOL bShow) { return realDevice->ShowCursor(bShow); }
    STDMETHOD(CreateAdditionalSwapChain)(THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters,IDirect3DSwapChain9** pSwapChain) { return realDevice->CreateAdditionalSwapChain(pPresentationParameters, pSwapChain); }
    STDMETHOD(GetSwapChain)(THIS_ UINT iSwapChain,IDirect3DSwapChain9** pSwapChain) { return realDevice->GetSwapChain(iSwapChain, pSwapChain); }
    STDMETHOD_(UINT, GetNumberOfSwapChains)(THIS) { return realDevice->GetNumberOfSwapChains(); }
    STDMETHOD(Reset)(THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters) { return realDevice->Reset(pPresentationParameters); }
    STDMETHOD(GetBackBuffer)(THIS_ UINT iSwapChain,UINT iBackBuffer,D3DBACKBUFFER_TYPE Type,IDirect3DSurface9** ppBackBuffer) { return realDevice->GetBackBuffer(iSwapChain, iBackBuffer, Type, ppBackBuffer); }
    STDMETHOD(GetRasterStatus)(THIS_ UINT iSwapChain,D3DRASTER_STATUS* pRasterStatus) { return realDevice->GetRasterStatus(iSwapChain, pRasterStatus); }
    STDMETHOD(SetDialogBoxMode)(THIS_ BOOL bEnableDialogs) { return realDevice->SetDialogBoxMode(bEnableDialogs); }
    STDMETHOD_(void, SetGammaRamp)(THIS_ UINT iSwapChain,DWORD Flags,const D3DGAMMARAMP* pRamp) { realDevice->SetGammaRamp(iSwapChain, Flags, pRamp); }
    STDMETHOD_(void, GetGammaRamp)(THIS_ UINT iSwapChain,D3DGAMMARAMP* pRamp) { realDevice->GetGammaRamp(iSwapChain, pRamp); }
    STDMETHOD(CreateVolumeTexture)(THIS_ UINT Width,UINT Height,UINT Depth,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DVolumeTexture9** ppVolumeTexture,HANDLE* pSharedHandle) { return realDevice->CreateVolumeTexture(Width, Height, Depth, Levels, Usage, Format, Pool, ppVolumeTexture, pSharedHandle); }
    STDMETHOD(CreateCubeTexture)(THIS_ UINT EdgeLength,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DCubeTexture9** ppCubeTexture,HANDLE* pSharedHandle) { return realDevice->CreateCubeTexture(EdgeLength, Levels, Usage, Format, Pool, ppCubeTexture, pSharedHandle); }
    STDMETHOD(CreateVertexBuffer)(THIS_ UINT Length,DWORD Usage,DWORD FVF,D3DPOOL Pool,IDirect3DVertexBuffer9** ppVertexBuffer,HANDLE* pSharedHandle) { return realDevice->CreateVertexBuffer(Length, Usage, FVF, Pool, ppVertexBuffer, pSharedHandle); }
    STDMETHOD(CreateIndexBuffer)(THIS_ UINT Length,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DIndexBuffer9** ppIndexBuffer,HANDLE* pSharedHandle) { return realDevice->CreateIndexBuffer(Length, Usage, Format, Pool, ppIndexBuffer, pSharedHandle); }
    STDMETHOD(CreateRenderTarget)(THIS_ UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuality,BOOL Lockable,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle) { return realDevice->CreateRenderTarget(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle); }
    STDMETHOD(CreateDepthStencilSurface)(THIS_ UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuality,BOOL Discard,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle) { return realDevice->CreateDepthStencilSurface(Width, Height, Format, MultiSample, MultisampleQuality, Discard, ppSurface, pSharedHandle); }
    STDMETHOD(UpdateSurface)(THIS_ IDirect3DSurface9* pSourceSurface,const RECT* pSourceRect,IDirect3DSurface9* pDestinationSurface,const POINT* pDestinationPoint) { return realDevice->UpdateSurface(pSourceSurface, pSourceRect, pDestinationSurface, pDestinationPoint); }
    STDMETHOD(UpdateTexture)(THIS_ IDirect3DBaseTexture9* pSourceTexture,IDirect3DBaseTexture9* pDestinationTexture) { return realDevice->UpdateTexture(pSourceTexture, pDestinationTexture); }
    STDMETHOD(GetRenderTargetData)(THIS_ IDirect3DSurface9* pRenderTarget,IDirect3DSurface9* pDestSurface) { return realDevice->GetRenderTargetData(pRenderTarget, pDestSurface); }
    STDMETHOD(GetFrontBufferData)(THIS_ UINT iSwapChain,IDirect3DSurface9* pDestSurface) { return realDevice->GetFrontBufferData(iSwapChain, pDestSurface); }
    STDMETHOD(StretchRect)(THIS_ IDirect3DSurface9* pSourceSurface,const RECT* pSourceRect,IDirect3DSurface9* pDestinationSurface,const RECT* pDestinationRect,D3DTEXTUREFILTERTYPE Filter) { return realDevice->StretchRect(pSourceSurface, pSourceRect, pDestinationSurface, pDestinationRect, Filter); }
    STDMETHOD(ColorFill)(THIS_ IDirect3DSurface9* pSurface,const RECT* pRect,D3DCOLOR color) { return realDevice->ColorFill(pSurface, pRect, color); }
    STDMETHOD(CreateOffscreenPlainSurface)(THIS_ UINT Width,UINT Height,D3DFORMAT Format,D3DPOOL Pool,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle) { return realDevice->CreateOffscreenPlainSurface(Width, Height, Format, Pool, ppSurface, pSharedHandle); }
    STDMETHOD(SetRenderTarget)(THIS_ DWORD RenderTargetIndex,IDirect3DSurface9* pRenderTarget) { return realDevice->SetRenderTarget(RenderTargetIndex, pRenderTarget); }
    STDMETHOD(GetRenderTarget)(THIS_ DWORD RenderTargetIndex,IDirect3DSurface9** ppRenderTarget) { return realDevice->GetRenderTarget(RenderTargetIndex, ppRenderTarget); }
    STDMETHOD(SetDepthStencilSurface)(THIS_ IDirect3DSurface9* pNewZStencil) { return realDevice->SetDepthStencilSurface(pNewZStencil); }
    STDMETHOD(GetDepthStencilSurface)(THIS_ IDirect3DSurface9** ppZStencilSurface) { return realDevice->GetDepthStencilSurface(ppZStencilSurface); }
    STDMETHOD(BeginScene)(THIS) { return realDevice->BeginScene(); }
    STDMETHOD(EndScene)(THIS) { return realDevice->EndScene(); }
    STDMETHOD(Clear)(THIS_ DWORD Count,const D3DRECT* pRects,DWORD Flags,D3DCOLOR Color,float Z,DWORD Stencil) { return realDevice->Clear(Count, pRects, Flags, Color, Z, Stencil); }
    STDMETHOD(SetTransform)(THIS_ D3DTRANSFORMSTATETYPE State,const D3DMATRIX* pMatrix) { return realDevice->SetTransform(State, pMatrix); }
    STDMETHOD(GetTransform)(THIS_ D3DTRANSFORMSTATETYPE State,D3DMATRIX* pMatrix) { return realDevice->GetTransform(State, pMatrix); }
    STDMETHOD(MultiplyTransform)(THIS_ D3DTRANSFORMSTATETYPE State,const D3DMATRIX* pMatrix) { return realDevice->MultiplyTransform(State, pMatrix); }
    STDMETHOD(SetViewport)(THIS_ const D3DVIEWPORT9* pViewport) { return realDevice->SetViewport(pViewport); }
    STDMETHOD(GetViewport)(THIS_ D3DVIEWPORT9* pViewport) { return realDevice->GetViewport(pViewport); }
    STDMETHOD(SetMaterial)(THIS_ const D3DMATERIAL9* pMaterial) { return realDevice->SetMaterial(pMaterial); }
    STDMETHOD(GetMaterial)(THIS_ D3DMATERIAL9* pMaterial) { return realDevice->GetMaterial(pMaterial); }
    STDMETHOD(SetLight)(THIS_ DWORD Index,const D3DLIGHT9* pLight) { return realDevice->SetLight(Index, pLight); }
    STDMETHOD(GetLight)(THIS_ DWORD Index,D3DLIGHT9* pLight) { return realDevice->GetLight(Index, pLight); }
    STDMETHOD(LightEnable)(THIS_ DWORD Index,BOOL Enable) { return realDevice->LightEnable(Index, Enable); }
    STDMETHOD(GetLightEnable)(THIS_ DWORD Index,BOOL* pEnable) { return realDevice->GetLightEnable(Index, pEnable); }
    STDMETHOD(SetClipPlane)(THIS_ DWORD Index,const float* pPlane) { return realDevice->SetClipPlane(Index, pPlane); }
    STDMETHOD(GetClipPlane)(THIS_ DWORD Index,float* pPlane) { return realDevice->GetClipPlane(Index, pPlane); }
    STDMETHOD(SetRenderState)(THIS_ D3DRENDERSTATETYPE State,DWORD Value) { return realDevice->SetRenderState(State, Value); }
    STDMETHOD(GetRenderState)(THIS_ D3DRENDERSTATETYPE State,DWORD* pValue) { return realDevice->GetRenderState(State, pValue); }
    STDMETHOD(CreateStateBlock)(THIS_ D3DSTATEBLOCKTYPE Type,IDirect3DStateBlock9** ppSB) { return realDevice->CreateStateBlock(Type, ppSB); }
    STDMETHOD(BeginStateBlock)(THIS) { return realDevice->BeginStateBlock(); }
    STDMETHOD(EndStateBlock)(THIS_ IDirect3DStateBlock9** ppSB) { return realDevice->EndStateBlock(ppSB); }
    STDMETHOD(SetClipStatus)(THIS_ const D3DCLIPSTATUS9* pClipStatus) { return realDevice->SetClipStatus(pClipStatus); }
    STDMETHOD(GetClipStatus)(THIS_ D3DCLIPSTATUS9* pClipStatus) { return realDevice->GetClipStatus(pClipStatus); }
    STDMETHOD(GetTexture)(THIS_ DWORD Stage,IDirect3DBaseTexture9** ppTexture) { return realDevice->GetTexture(Stage, ppTexture);
