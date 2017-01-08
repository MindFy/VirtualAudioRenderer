#include "VirtualAudioRenderer.h"

#include <Dshow.h>
#include <uuids.h>

// {95824D89-E231-4FBE-85DF-8ECE0D7837FE}
static const GUID CLSID_VirtualAudioRender = { 0x95824d89, 0xe231, 0x4fbe,{ 0x85, 0xdf, 0x8e, 0xce, 0xd, 0x78, 0x37, 0xfe } };

static const TCHAR* g_pszName = _T("Virtual Audio Renderer");

// Utility Macros
//
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// The _VCRT_DECLARE_ALTERNATE_NAME macro provides an architecture-neutral way
// of specifying /alternatename comments to the linker.  It prepends the leading
// underscore for x86 and leaves names unmodified for other architectures.
#if defined _M_IX86
#define _VCRT_DECLARE_ALTERNATE_NAME_PREFIX "_"
#elif defined _M_X64 || defined _M_ARM || defined _M_ARM64
#define _VCRT_DECLARE_ALTERNATE_NAME_PREFIX ""
#else
#error Unsupported architecture
#endif

#define _VCRT_DECLARE_ALTERNATE_NAME(name, alternate_name)  \
    __pragma(comment(linker,                                \
        "/alternatename:"                                   \
        _VCRT_DECLARE_ALTERNATE_NAME_PREFIX #name           \
        "="                                                 \
        _VCRT_DECLARE_ALTERNATE_NAME_PREFIX #alternate_name \
        ))

_VCRT_DECLARE_ALTERNATE_NAME(__acrt_initialize, __scrt_stub_for_acrt_initialize)
_VCRT_DECLARE_ALTERNATE_NAME(__acrt_uninitialize, __scrt_stub_for_acrt_uninitialize)
_VCRT_DECLARE_ALTERNATE_NAME(__acrt_uninitialize_critical, __scrt_stub_for_acrt_uninitialize_critical)
_VCRT_DECLARE_ALTERNATE_NAME(__acrt_thread_attach, __scrt_stub_for_acrt_thread_attach)
_VCRT_DECLARE_ALTERNATE_NAME(__acrt_thread_detach, __scrt_stub_for_acrt_thread_detach)
_VCRT_DECLARE_ALTERNATE_NAME(_is_c_termination_complete, __scrt_stub_for_is_c_termination_complete)

_VCRT_DECLARE_ALTERNATE_NAME(__vcrt_initialize, __scrt_stub_for_acrt_initialize)
_VCRT_DECLARE_ALTERNATE_NAME(__vcrt_uninitialize, __scrt_stub_for_acrt_uninitialize)
_VCRT_DECLARE_ALTERNATE_NAME(__vcrt_uninitialize_critical, __scrt_stub_for_acrt_uninitialize_critical)
_VCRT_DECLARE_ALTERNATE_NAME(__vcrt_thread_attach, __scrt_stub_for_acrt_thread_attach)
_VCRT_DECLARE_ALTERNATE_NAME(__vcrt_thread_detach, __scrt_stub_for_acrt_thread_detach)

CVirtualAudioRenderer::CVirtualAudioRenderer(const TCHAR* pName, LPUNKNOWN pUnknown, HRESULT* phResult) : CBaseRenderer(CLSID_VirtualAudioRender, const_cast<TCHAR*>(pName), pUnknown, phResult)
{
}

ULONG CVirtualAudioRenderer::AddRef()
{
	return CBaseRenderer::AddRef();
}

ULONG CVirtualAudioRenderer::Release()
{
	return CBaseRenderer::Release();
}

STDMETHODIMP CVirtualAudioRenderer::QueryInterface(REFIID InterfaceIdentifier, void** pInstance)
{
	if (pInstance)
	{
		return CBaseRenderer::QueryInterface(InterfaceIdentifier, pInstance);
	}
	return E_POINTER;
}


// Public method that returns a new instance. 
CUnknown * WINAPI CreateVirtualAudioRender(LPUNKNOWN pUnkown, HRESULT *phResult)
{
	CVirtualAudioRenderer* pVirtualAudioRender = nullptr;
	if (phResult)
	{
		pVirtualAudioRender = new CVirtualAudioRenderer(_T("Virtual Audio Renderer"), pUnkown, phResult);
		if (pVirtualAudioRender == nullptr)
		{
			*phResult = E_OUTOFMEMORY;
		}
	}
	return pVirtualAudioRender;
}

CFactoryTemplate g_Templates[1] =
{
	{
		L"Virtual Audio Renderer",	// Name
		&CLSID_VirtualAudioRender,	// CLSID
		CreateVirtualAudioRender,	// Method to create an instance of MyComponent
		NULL,						// Initialization function
		NULL						// Set-up information (for filters)
	}
};

int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);

STDAPI DllRegisterServer()
{
	HRESULT hResult = AMovieDllRegisterServer2(TRUE);
	if (SUCCEEDED(hResult))
	{
		CComPtr<IFilterMapper2> pFilterMapper;
		if (SUCCEEDED(pFilterMapper.CoCreateInstance(CLSID_FilterMapper2)))
		{
			REGPINTYPES* pPinMediaTypes = new REGPINTYPES[2];
			pPinMediaTypes[0].clsMajorType = &MEDIATYPE_Audio;
			pPinMediaTypes[0].clsMinorType = &MEDIASUBTYPE_IEEE_FLOAT;
			pPinMediaTypes[1].clsMajorType = &MEDIATYPE_Audio;
			pPinMediaTypes[1].clsMinorType = &MEDIASUBTYPE_PCM;
			const REGFILTERPINS2 Pins =
			{
				REG_PINFLAG_B_RENDERER,
				1,
				2,
				pPinMediaTypes,
				0,
				nullptr,
				&CLSID_AudioRendererCategory
			};
			const REGFILTER2 Filter =
			{
				2,
				MERIT_DO_NOT_USE,
				1,
				reinterpret_cast<const REGFILTERPINS*>(&Pins)
			};
			hResult = pFilterMapper->RegisterFilter(
				CLSID_VirtualAudioRender,		// Filter CLSID. 
				g_pszName,						// Filter name.
				NULL,							// Device moniker. 
				&CLSID_AudioRender,				// Video compressor category.
				g_pszName,						// Instance data.
				&Filter						// Pointer to filter information.
			);
			delete[] pPinMediaTypes;
		}
	}
	return hResult;
}

extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

BOOL APIENTRY DllMain(HANDLE hModule, DWORD dwReason, LPVOID lpReserved)
{
	return DllEntryPoint((HINSTANCE)(hModule), dwReason, lpReserved);
}

STDAPI DllUnregisterServer()
{
	HRESULT hResult = AMovieDllRegisterServer2(FALSE);
	if (SUCCEEDED(hResult))
	{
		CComPtr<IFilterMapper2> pFilterMapper;
		if (SUCCEEDED(pFilterMapper.CoCreateInstance(CLSID_FilterMapper2)))
		{
			hResult = pFilterMapper->UnregisterFilter(&CLSID_AudioRender, g_pszName, CLSID_VirtualAudioRender);
		}
	}
	return hResult;
}