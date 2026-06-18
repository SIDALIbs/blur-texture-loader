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
    STDMETHOD(GetTexture)(THIS_ DWORD Stage,IDirect3DBaseTexture9** ppTexture) { return realDevice->GetTexture(Stage, ppTexture); }
    STDMETHOD(SetTexture)(THIS_ DWORD Stage,IDirect3DBaseTexture9* pTexture) { return realDevice->SetTexture(Stage, pTexture); }
    STDMETHOD(GetTextureStageState)(THIS_ DWORD Stage,D3DTEXTURESTAGESTATETYPE Type,DWORD* pValue) { return realDevice->GetTextureStageState(Stage, Type, pValue); }
    STDMETHOD(SetTextureStageState)(THIS_ DWORD Stage,D3DTEXTURESTAGESTATETYPE Type,DWORD Value) { return realDevice->SetTextureStageState(Stage, Type, Value); }
    STDMETHOD(GetSamplerState)(THIS_ DWORD Sampler,D3DSAMPLERSTATETYPE Type,DWORD* pValue) { return realDevice->GetSamplerState(Sampler, Type, pValue); }
    STDMETHOD(SetSamplerState)(THIS_ DWORD Sampler,D3DSAMPLERSTATETYPE Type,DWORD Value) { return realDevice->SetSamplerState(Sampler, Type, Value); }
    STDMETHOD(ValidateDevice)(THIS_ DWORD* pNumPasses) { return realDevice->ValidateDevice(pNumPasses); }
    STDMETHOD(SetPaletteEntries)(THIS_ UINT PaletteNumber,const PALETTEENTRY* pEntries) { return realDevice->SetPaletteEntries(PaletteNumber, pEntries); }
    STDMETHOD(GetPaletteEntries)(THIS_ UINT PaletteNumber,PALETTEENTRY* pEntries) { return realDevice->GetPaletteEntries(PaletteNumber, pEntries); }
    STDMETHOD(SetCurrentTexturePalette)(THIS_ UINT PaletteNumber) { return realDevice->SetCurrentTexturePalette(PaletteNumber); }
    STDMETHOD(GetCurrentTexturePalette)(THIS_ UINT *PaletteNumber) { return realDevice->GetCurrentTexturePalette(PaletteNumber); }
    STDMETHOD(SetScissorRect)(THIS_ const RECT* pRect) { return realDevice->SetScissorRect(pRect); }
    STDMETHOD(GetScissorRect)(THIS_ RECT* pRect) { return realDevice->GetScissorRect(pRect); }
    STDMETHOD(SetSoftwareVertexProcessing)(THIS_ BOOL bSoftware) { return realDevice->SetSoftwareVertexProcessing(bSoftware); }
    STDMETHOD_(BOOL, GetSoftwareVertexProcessing)(THIS) { return realDevice->GetSoftwareVertexProcessing(); }
    STDMETHOD(SetNPatchMode)(THIS_ float nSegments) { return realDevice->SetNPatchMode(nSegments); }
    STDMETHOD_(float, GetNPatchMode)(THIS) { return realDevice->GetNPatchMode(); }
    STDMETHOD(DrawPrimitive)(THIS_ D3DPRIMITIVETYPE PrimitiveType,UINT StartVertex,UINT PrimitiveCount) { return realDevice->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount); }
    STDMETHOD(DrawIndexedPrimitive)(THIS_ D3DPRIMITIVETYPE Type,INT BaseVertexIndex,UINT MinVertexIndex,UINT NumVertices,UINT startIndex,UINT primCount) { return realDevice->DrawIndexedPrimitive(Type, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount); }
    STDMETHOD(DrawPrimitiveUP)(THIS_ D3DPRIMITIVETYPE PrimitiveType,UINT PrimitiveCount,const void* pVertexStreamZeroData,UINT VertexStreamZeroStride) { return realDevice->DrawPrimitiveUP(PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride); }
    STDMETHOD(DrawIndexedPrimitiveUP)(THIS_ D3DPRIMITIVETYPE PrimitiveType,UINT MinVertexIndex,UINT NumVertices,UINT PrimitiveCount,const void* pIndexData,D3DFORMAT IndexDataFormat,const void* pVertexStreamZeroData,UINT VertexStreamZeroStride) { return realDevice->DrawIndexedPrimitiveUP(PrimitiveType, MinVertexIndex, NumVertices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride); }
    STDMETHOD(ProcessVertices)(THIS_ UINT SrcStartIndex,UINT DestIndex,UINT VertexCount,IDirect3DVertexBuffer9* pDestBuffer,IDirect3DVertexDeclaration9* pSelectDeclaration,DWORD Flags) { return realDevice->ProcessVertices(SrcStartIndex, DestIndex, VertexCount, pDestBuffer, pSelectDeclaration, Flags); }
    STDMETHOD(CreateVertexDeclaration)(THIS_ const D3DVERTEXELEMENT9* pVertexElements,IDirect3DVertexDeclaration9** ppDecl) { return realDevice->CreateVertexDeclaration(pVertexElements, ppDecl); }
    STDMETHOD(SetVertexDeclaration)(THIS_ IDirect3DVertexDeclaration9* pDecl) { return realDevice->SetVertexDeclaration(pDecl); }
    STDMETHOD(GetVertexDeclaration)(THIS_ IDirect3DVertexDeclaration9** ppDecl) { return realDevice->GetVertexDeclaration(ppDecl); }
    STDMETHOD(SetFVF)(THIS_ DWORD FVF) { return realDevice->SetFVF(FVF); }
    STDMETHOD(GetFVF)(THIS_ DWORD* pFVF) { return realDevice->GetFVF(pFVF); }
    STDMETHOD(CreateVertexShader)(THIS_ const DWORD* pFunction,IDirect3DVertexShader9** ppShader) { return realDevice->CreateVertexShader(pFunction, ppShader); }
    STDMETHOD(SetVertexShader)(THIS_ IDirect3DVertexShader9* pShader) { return realDevice->SetVertexShader(pShader); }
    STDMETHOD(GetVertexShader)(THIS_ IDirect3DVertexShader9** ppShader) { return realDevice->GetVertexShader(ppShader); }
    STDMETHOD(SetVertexShaderConstantF)(THIS_ UINT StartRegister,const float* pConstantData,UINT Vector4fCount) { return realDevice->SetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount); }
    STDMETHOD(GetVertexShaderConstantF)(THIS_ UINT StartRegister,float* pConstantData,UINT Vector4fCount) { return realDevice->GetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount); }
    STDMETHOD(SetVertexShaderConstantI)(THIS_ UINT StartRegister,const int* pConstantData,UINT Vector4iCount) { return realDevice->SetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount); }
    STDMETHOD(GetVertexShaderConstantI)(THIS_ UINT StartRegister,int* pConstantData,UINT Vector4iCount) { return realDevice->GetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount); }
    STDMETHOD(SetVertexShaderConstantB)(THIS_ UINT StartRegister,const BOOL* pConstantData,UINT  BoolCount) { return realDevice->SetVertexShaderConstantB(StartRegister, pConstantData, BoolCount); }
    STDMETHOD(GetVertexShaderConstantB)(THIS_ UINT StartRegister,BOOL* pConstantData,UINT BoolCount) { return realDevice->GetVertexShaderConstantB(StartRegister, pConstantData, BoolCount); }
    STDMETHOD(SetStreamSource)(THIS_ UINT StreamNumber,IDirect3DVertexBuffer9* pStreamData,UINT OffsetInBytes,UINT Stride) { return realDevice->SetStreamSource(StreamNumber, pStreamData, OffsetInBytes, Stride); }
    STDMETHOD(GetStreamSource)(THIS_ UINT StreamNumber,IDirect3DVertexBuffer9** ppStreamData,UINT* pOffsetInBytes,UINT* pStride) { return realDevice->GetStreamSource(StreamNumber, ppStreamData, pOffsetInBytes, pStride); }
    STDMETHOD(SetStreamSourceFreq)(THIS_ UINT StreamNumber,UINT Setting) { return realDevice->SetStreamSourceFreq(StreamNumber, Setting); }
    STDMETHOD(GetStreamSourceFreq)(THIS_ UINT StreamNumber,UINT* pSetting) { return realDevice->GetStreamSourceFreq(StreamNumber, pSetting); }
    
    // Fixed param matching official SDK layout
    STDMETHOD(SetIndices)(THIS_ IDirect3DIndexBuffer9* pIndexData) { return realDevice->SetIndices(pIndexData); }
    STDMETHOD(GetIndices)(THIS_ IDirect3DIndexBuffer9** ppIndexData) { return realDevice->GetIndices(ppIndexData); }
    
    STDMETHOD(CreatePixelShader)(THIS_ const DWORD* pFunction,IDirect3DPixelShader9** ppShader) { return realDevice->CreatePixelShader(pFunction, ppShader); }
    STDMETHOD(SetPixelShader)(THIS_ IDirect3DPixelShader9* pShader) { return realDevice->SetPixelShader(pShader); }
    STDMETHOD(GetPixelShader)(THIS_ IDirect3DPixelShader9** ppShader) { return realDevice->GetPixelShader(ppShader); }
    STDMETHOD(SetPixelShaderConstantF)(THIS_ UINT StartRegister,const float* pConstantData,UINT Vector4fCount) { return realDevice->SetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount); }
    STDMETHOD(GetPixelShaderConstantF)(THIS_ UINT StartRegister,float* pConstantData,UINT Vector4fCount) { return realDevice->GetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount); }
    STDMETHOD(SetPixelShaderConstantI)(THIS_ UINT StartRegister,const int* pConstantData,UINT Vector4iCount) { return realDevice->SetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount); }
    STDMETHOD(GetPixelShaderConstantI)(THIS_ UINT StartRegister,int* pConstantData,UINT Vector4iCount) { return realDevice->GetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount); }
    STDMETHOD(SetPixelShaderConstantB)(THIS_ UINT StartRegister,const BOOL* pConstantData,UINT BoolCount) { return realDevice->SetPixelShaderConstantB(StartRegister, pConstantData, BoolCount); }
    STDMETHOD(GetPixelShaderConstantB)(THIS_ UINT StartRegister,BOOL* pConstantData,UINT BoolCount) { return realDevice->GetPixelShaderConstantB(StartRegister, pConstantData, BoolCount); }
    STDMETHOD(DrawRectPatch)(THIS_ UINT Handle,const float* pNumSegs,const D3DRECTPATCH_INFO* pRectPatchInfo) { return realDevice->DrawRectPatch(Handle, pNumSegs, pRectPatchInfo); }
    STDMETHOD(DrawTriPatch)(THIS_ UINT Handle,const float* pNumSegs,const D3DTRIPATCH_INFO* pTriPatchInfo) { return realDevice->DrawTriPatch(Handle, pNumSegs, pTriPatchInfo); }
    STDMETHOD(DeletePatch)(THIS_ UINT Handle) { return realDevice->DeletePatch(Handle); }
    STDMETHOD(CreateQuery)(THIS_ D3DQUERYTYPE Type,IDirect3DQuery9** ppQuery) { return realDevice->CreateQuery(Type, ppQuery); }
    STDMETHOD(Present)(THIS_ const RECT* pSourceRect, const RECT* pDestinationRect, HWND hDestWindowOverride, const RGNDATA* pDirtyRegion) { return realDevice->Present(pSourceRect, pDestinationRect, hDestWindowOverride, pDirtyRegion); }
};

// Wrapper object for the master IDirect3D9 system instance
class DummyInterface9 : public IDirect3D9 {
private:
    IDirect3D9* realD3D;
public:
    DummyInterface9(IDirect3D9* original) : realD3D(original) {}

    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj) { return realD3D->QueryInterface(riid, ppvObj); }
    STDMETHOD_(ULONG,AddRef)(THIS) { return realD3D->AddRef(); }
    STDMETHOD_(ULONG,Release)(THIS) { ULONG res = realD3D->Release(); if (res == 0) { delete this; } return res; }
    STDMETHOD(RegisterSoftwareDevice)(THIS_ void* pInitializeFunction) { return realD3D->RegisterSoftwareDevice(pInitializeFunction); }
    STDMETHOD_(UINT, GetAdapterCount)(THIS) { return realD3D->GetAdapterCount(); }
    STDMETHOD(GetAdapterIdentifier)(THIS_ UINT Adapter,DWORD Flags,D3DADAPTER_IDENTIFIER9* pIdentifier) { return realD3D->GetAdapterIdentifier(Adapter, Flags, pIdentifier); }
    STDMETHOD_(UINT, GetAdapterModeCount)(THIS_ UINT Adapter,D3DFORMAT Format) { return realD3D->GetAdapterModeCount(Adapter, Format); }
    STDMETHOD(EnumAdapterModes)(THIS_ UINT Adapter,D3DFORMAT Format,UINT Mode,D3DDISPLAYMODE* pMode) { return realD3D->EnumAdapterModes(Adapter, Format, Mode, pMode); }
    STDMETHOD(GetAdapterDisplayMode)(THIS_ UINT Adapter,D3DDISPLAYMODE* pMode) { return realD3D->GetAdapterDisplayMode(Adapter, pMode); }
    STDMETHOD(CheckDeviceType)(THIS_ UINT Adapter,D3DDEVTYPE DevType,D3DFORMAT DisplayFormat,D3DFORMAT BackBufferFormat,BOOL bWindowed) { return realD3D->CheckDeviceType(Adapter, DevType, DisplayFormat, BackBufferFormat, bWindowed); }
    STDMETHOD(CheckDeviceFormat)(THIS_ UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT AdapterFormat,DWORD Usage,D3DRESOURCETYPE RType,D3DFORMAT CheckFormat) { return realD3D->CheckDeviceFormat(Adapter, DeviceType, AdapterFormat, Usage, RType, CheckFormat); }
    STDMETHOD(CheckDeviceMultiSampleType)(THIS_ UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT SurfaceFormat,BOOL Windowed,D3DMULTISAMPLE_TYPE MultiSampleType,DWORD* pQualityLevels) { return realD3D->CheckDeviceMultiSampleType(Adapter, DeviceType, SurfaceFormat, Windowed, MultiSampleType, pQualityLevels); }
    STDMETHOD(CheckDepthStencilMatch)(THIS_ UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT AdapterFormat,D3DFORMAT RenderTargetFormat,D3DFORMAT DepthStencilFormat) { return realD3D->CheckDepthStencilMatch(Adapter, DeviceType, AdapterFormat, RenderTargetFormat, DepthStencilFormat); }
    STDMETHOD(CheckDeviceFormatConversion)(THIS_ UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT SourceFormat,D3DFORMAT TargetFormat) { return realD3D->CheckDeviceFormatConversion(Adapter, DeviceType, SourceFormat, TargetFormat); }
    STDMETHOD(GetDeviceCaps)(THIS_ UINT Adapter,D3DDEVTYPE DeviceType,D3DCAPS9* pCaps) { return realD3D->GetDeviceCaps(Adapter, DeviceType, pCaps); }
    STDMETHOD_(HMONITOR, GetAdapterMonitor)(THIS_ UINT Adapter) { return realD3D->GetAdapterMonitor(Adapter); }
    
    // Intercept CreateDevice step clean to hand off our wrapped proxy device interface
    STDMETHOD(CreateDevice)(THIS_ UINT Adapter,D3DDEVTYPE DeviceType,HWND hFocusWindow,DWORD BehaviorFlags,D3DPRESENT_PARAMETERS* pPresentationParameters,IDirect3DDevice9** ppReturnedDeviceInterface) {
        IDirect3DDevice9* baseDevice = NULL;
        HRESULT hr = realD3D->CreateDevice(Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, &baseDevice);
        if (SUCCEEDED(hr) && baseDevice) {
            *ppReturnedDeviceInterface = new DummyDevice9(baseDevice);
            return S_OK;
        }
        return hr;
    }
};

// Main system entry point hook
extern "C" IDirect3D9* WINAPI Direct3DCreate9(UINT SDKVersion) {
    char sysPath[MAX_PATH];
    GetSystemDirectoryA(sysPath, MAX_PATH);
    std::string realD3DPath = std::string(sysPath) + "\\d3d9.dll";
    
    HMODULE hRealD3D = LoadLibraryA(realD3DPath.c_str());
    if (!hRealD3D) return NULL;

    typedef IDirect3D9* (WINAPI* D3DCreate9_t)(UINT);
    D3DCreate9_t pRealCreate = (D3DCreate9_t)GetProcAddress(hRealD3D, "Direct3DCreate9");
    IDirect3D9* originalD3D = pRealCreate(SDKVersion);

    if (originalD3D) {
        return new DummyInterface9(originalD3D);
    }
    return NULL;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    return TRUE;
}
