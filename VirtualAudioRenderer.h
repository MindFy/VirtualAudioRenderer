#pragma once

#include "streams.h"
#include "renbase.h"

#include <amaudio.h>
#include <atlbase.h>
#include <atlcom.h>

class ATL_NO_VTABLE CVirtualAudioRenderer :
	public CBaseRenderer,
	public IDispatchImpl<IBasicAudio>
{
public:
	CVirtualAudioRenderer(const TCHAR* pName, LPUNKNOWN pUnknown, HRESULT* phResult);

	// IUnknown
	ULONG STDMETHODCALLTYPE AddRef() override;
	ULONG STDMETHODCALLTYPE Release() override;
	STDMETHOD(QueryInterface)(REFIID InterfaceIdentifier, void** pInstance) override;

	// CBaseRenderer
	HRESULT DoRenderSample(IMediaSample *pMediaSample) override;
	HRESULT CheckMediaType(const CMediaType *) override;

	// IBasicAudio
	STDMETHOD(put_Volume)(long lVolume) override;
	STDMETHOD(get_Volume)(long *plVolume) override;
	STDMETHOD(put_Balance)(long lBalance) override;
	STDMETHOD(get_Balance)(long *plBalance) override;

private:
	CComCriticalSection m_DataCriticalSection;
	LONG m_nReferenceCount;
};