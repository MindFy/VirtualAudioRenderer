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

#ifdef _DEBUG
#define CHECK_FUNCTION(hResult, pFunctionName)\
{\
	wchar_t pError[1024];\
	if (SUCCEEDED(hResult))\
	{\
		wsprintf(pError, L"%s: %s succeeded", g_pszName, pFunctionName); \
		OutputDebugString(pError);\
	}\
	else\
	{\
		wsprintf(pError, L"%s: %s failed with 0x%08x", g_pszName, pFunctionName, hResult);\
		OutputDebugString(pError);\
	}\
}
#else
#define CHECK_FUNCTION(hResult, pFunctionName)
#endif
STDAPI DllRegisterServer()
{
#ifdef _DEBUG
	OutputDebugString(L"Virtual Audio Renderer: Entering DllRegisterServer\n");
#endif
	HRESULT hResult = CoInitialize(NULL);
	CHECK_FUNCTION(hResult, L"CoInitialize");
	if (SUCCEEDED(hResult))
	{
		hResult = AMovieDllRegisterServer2(TRUE);
		CHECK_FUNCTION(hResult, L"AMovieDllRegisterServer2");
		if (SUCCEEDED(hResult))
		{
			CComPtr<IFilterMapper2> pFilterMapper;
			if (SUCCEEDED(pFilterMapper.CoCreateInstance(CLSID_FilterMapper2)))
			{
				static const REGPINTYPES g_pPinMediaTypes[2] =
				{
					{
						&MEDIATYPE_Audio,
						&MEDIASUBTYPE_IEEE_FLOAT
					},
					{
						&MEDIATYPE_Audio,
						&MEDIASUBTYPE_PCM
					}
				};
				const static REGFILTERPINS2 g_Pins =
				{
					REG_PINFLAG_B_RENDERER,
					1,
					2,
					g_pPinMediaTypes,
					0,
					nullptr,
					&CLSID_AudioRendererCategory
				};
				static const REGFILTER2 g_Filter =
				{
					2,
					MERIT_DO_NOT_USE,
					1,
					reinterpret_cast<const REGFILTERPINS*>(&g_Pins)
				};
				CHECK_FUNCTION(hResult, L"IFilterMapper2::RegisterFilter");
				hResult = pFilterMapper->RegisterFilter(
					CLSID_VirtualAudioRender,		// Filter CLSID. 
					g_pszName,						// Filter name.
					NULL,							// Device moniker. 
					&CLSID_AudioRendererCategory,	// Video compressor category.
					g_pszName,						// Instance data.
					&g_Filter						// Pointer to filter information.
				);
			}
		}
		CoUninitialize();
	}
#ifdef _DEBUG
	OutputDebugString(L"Virtual Audio Renderer: Leaving DllRegisterServer\n");
#endif
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