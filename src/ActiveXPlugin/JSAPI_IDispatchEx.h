/**********************************************************\ 
Original Author: Richard Bateman (taxilian)

Created:    Oct 30, 2009
License:    Dual license model; choose one of two:
            Eclipse Public License - Version 1.0
            http://www.eclipse.org/legal/epl-v10.html
            - or -
            GNU Lesser General Public License, version 2.1
            http://www.gnu.org/licenses/lgpl-2.1.html

Copyright 2009 Richard Bateman, Firebreath development team
\**********************************************************/

#ifndef H_JSAPI_IDISPATCHEX
#define H_JSAPI_IDISPATCHEX

#include "ActiveXBrowserHost.h"
#include "APITypes.h"
#include "JSAPI.h"
#include "TypeIDMap.h"
#include "COM_config.h"

#include "axmain.h"
#include "IDispatchAPI.h"
#include "dispex.h"
#include <map>

template <class T, class IDISP, const IID* piid>
class JSAPI_IDispatchEx :
    public IDISP,
    public IConnectionPointContainer,
    public IConnectionPoint
{
    typedef CComEnum<IEnumConnectionPoints, &__uuidof(IEnumConnectionPoints), IConnectionPoint*,
        _CopyInterface<IConnectionPoint> > CComEnumConnectionPoints;
    typedef std::map<DWORD, FB::AutoPtr<IDispatchAPI>> ConnectionPointMap;

public:
    JSAPI_IDispatchEx(void) { };
    virtual ~JSAPI_IDispatchEx(void) { };
    void setAPI(FB::JSAPI *api, ActiveXBrowserHost *host)
    {
        m_api = api;
        m_host = host;
    }

protected:
    FB::AutoPtr<FB::JSAPI> m_api;
    FB::AutoPtr<ActiveXBrowserHost> m_host;
    ConnectionPointMap m_connPtMap;
    
    bool m_valid;

public:
    /* IConnectionPointContainer members */
    HRESULT STDMETHODCALLTYPE EnumConnectionPoints(IEnumConnectionPoints **ppEnum);
    HRESULT STDMETHODCALLTYPE FindConnectionPoint(REFIID riid, IConnectionPoint **ppCP);

    /* IConnectionPoint members */
    HRESULT STDMETHODCALLTYPE GetConnectionInterface(IID *pIID);
    HRESULT STDMETHODCALLTYPE GetConnectionPointContainer(IConnectionPointContainer **ppCPC);
    HRESULT STDMETHODCALLTYPE Advise(IUnknown *pUnkSink, DWORD *pdwCookie);
    HRESULT STDMETHODCALLTYPE Unadvise(DWORD dwCookie);
    HRESULT STDMETHODCALLTYPE EnumConnections(IEnumConnections **ppEnum);

    /* IDispatch members */
    HRESULT STDMETHODCALLTYPE GetTypeInfoCount(UINT *pctinfo);
    HRESULT STDMETHODCALLTYPE GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo **ppTInfo);
    HRESULT STDMETHODCALLTYPE GetIDsOfNames(REFIID riid, LPOLESTR *rgszNames, UINT cNames, LCID lcid,
        DISPID *rgDispId);
    HRESULT STDMETHODCALLTYPE Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, 
        DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr);

    /* IDispatchEx members */
    HRESULT STDMETHODCALLTYPE GetDispID(BSTR bstrName, DWORD grfdex, DISPID *pid);
    HRESULT STDMETHODCALLTYPE InvokeEx(DISPID id, LCID lcid, WORD wFlags,
        DISPPARAMS *pdp, VARIANT *pvarRes, EXCEPINFO *pei, IServiceProvider *pspCaller);
    HRESULT STDMETHODCALLTYPE DeleteMemberByName(BSTR bstrName, DWORD grfdex);
    HRESULT STDMETHODCALLTYPE DeleteMemberByDispID(DISPID id);
    HRESULT STDMETHODCALLTYPE GetMemberProperties(DISPID id, DWORD grfdexFetch,
        DWORD *pgrfdex);
    HRESULT STDMETHODCALLTYPE GetMemberName(DISPID id, BSTR *pbstrName);
    HRESULT STDMETHODCALLTYPE GetNextDispID(DWORD grfdex, DISPID id, DISPID *pid);
    HRESULT STDMETHODCALLTYPE GetNameSpaceParent(IUnknown **ppunk);
};

/* IConnectionPointContainer methods */
template <class T, class IDISP, const IID* piid>
HRESULT JSAPI_IDispatchEx<T,IDISP,piid>::EnumConnectionPoints(IEnumConnectionPoints **ppEnum)
{
    if (ppEnum == NULL)
        return E_POINTER;
    *ppEnum = NULL;
    CComEnumConnectionPoints* pEnum = NULL;

    pEnum = new CComObject<CComEnumConnectionPoints>;
    if (pEnum == NULL)
        return E_OUTOFMEMORY;

    IConnectionPoint *connectionPoint[1] = { NULL };
    static_cast<T*>(this)->QueryInterface(IID_IConnectionPoint, (void **)connectionPoint);

    HRESULT hRes = pEnum->Init(connectionPoint, &connectionPoint[1],
        reinterpret_cast<IConnectionPointContainer*>(this), AtlFlagCopy);

    if (FAILED(hRes))
    {
        delete pEnum;
        return hRes;
    }
    hRes = pEnum->QueryInterface(__uuidof(IEnumConnectionPoints), (void**)ppEnum);
    if (FAILED(hRes))
        delete pEnum;
    return hRes;
}

template <class T, class IDISP, const IID* piid>
HRESULT JSAPI_IDispatchEx<T,IDISP,piid>::FindConnectionPoint(REFIID riid, IConnectionPoint **ppCP)
{
    if (ppCP == NULL)
        return E_POINTER;
    *ppCP = NULL;
    HRESULT hRes = CONNECT_E_NOCONNECTION;

    if (InlineIsEqualGUID(*piid, riid)) {
        hRes = static_cast<T*>(this)->QueryInterface(__uuidof(IConnectionPoint), (void**)ppCP);
    }

    return hRes;
}

/* IConnectionPoint methods */
template <class T, class IDISP, const IID* piid>
HRESULT JSAPI_IDispatchEx<T,IDISP,piid>::GetConnectionInterface(IID *pIID)
{
    *pIID = *piid;
    return S_OK;
}

template <class T, class IDISP, const IID* piid>
HRESULT JSAPI_IDispatchEx<T,IDISP,piid>::GetConnectionPointContainer(IConnectionPointContainer **ppCPC)
{
    if (ppCPC == NULL)
        return E_POINTER;

    return static_cast<T*>(this)->QueryInterface(__uuidof(IConnectionPointContainer), (void**)ppCPC);
}

template <class T, class IDISP, const IID* piid>
HRESULT JSAPI_IDispatchEx<T,IDISP,piid>::Advise(IUnknown *pUnkSink, DWORD *pdwCookie)
{
    IDispatch *idisp(NULL);
    if (SUCCEEDED(pUnkSink->QueryInterface(IID_IDispatch, (void**)&idisp))) {
        FB::AutoPtr<IDispatchAPI> obj(new IDispatchAPI(idisp, m_host));
        m_connPtMap[(DWORD)obj.ptr()] = obj;
        *pdwCookie = (DWORD)obj.ptr();
        m_api->registerEventInterface(static_cast<FB::EventHandlerObject *>(obj.ptr()));
        return S_OK;
    } else {
        return CONNECT_E_CANNOTCONNECT;
    }
}

template <class T, class IDISP, const IID* piid>
HRESULT JSAPI_IDispatchEx<T,IDISP,piid>::Unadvise(DWORD dwCookie)
{
    ConnectionPointMap::iterator fnd = m_connPtMap.find(dwCookie);
    if (fnd == m_connPtMap.end()) {
        return E_UNEXPECTED;
    } else {
        m_api->registerEventInterface(static_cast<FB::EventHandlerObject *>(fnd->second.ptr()));
        m_connPtMap.erase(fnd);
        return S_OK;
    }
}

template <class T, class IDISP, const IID* piid>
HRESULT JSAPI_IDispatchEx<T,IDISP,piid>::EnumConnections(IEnumConnections **ppEnum)
{
    return E_NOTIMPL;
}

/* IDispatch methods */
template <class T, class IDISP, const IID* piid>
HRESULT JSAPI_IDispatchEx<T,IDISP,piid>::GetTypeInfoCount(UINT *pctinfo)
{
    return E_NOTIMPL;
}

template <class T, class IDISP, const IID* piid>
HRESULT JSAPI_IDispatchEx<T,IDISP,piid>::GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo **ppTInfo)
{
    return E_NOTIMPL;
}

template <class T, class IDISP, const IID* piid>
HRESULT JSAPI_IDispatchEx<T,IDISP,piid>::GetIDsOfNames(REFIID riid, LPOLESTR *rgszNames, 
                                                  UINT cNames, LCID lcid, 
                                                  DISPID *rgDispId)
{
    HRESULT rv = S_OK;
    for (UINT i = 0; i < cNames; i++) {
        CComBSTR bstr(rgszNames[i]);
        rv = GetDispID(bstr, 0, &rgDispId[i]);
        if (!SUCCEEDED(rv)) {
            return rv;
        }
    }
    return rv;
}

template <class T, class IDISP, const IID* piid>
HRESULT JSAPI_IDispatchEx<T,IDISP,piid>::Invoke(DISPID dispIdMember, REFIID riid, LCID lcid,
                                           WORD wFlags, DISPPARAMS *pDispParams, 
                                           VARIANT *pVarResult, EXCEPINFO *pExcepInfo,
                                           UINT *puArgErr)
{
    return InvokeEx(dispIdMember, lcid, wFlags, pDispParams, pVarResult, pExcepInfo, NULL);
}


/* IDispatchEx members */
template <class T, class IDISP, const IID* piid>
HRESULT JSAPI_IDispatchEx<T,IDISP,piid>::GetDispID(BSTR bstrName, DWORD grfdex, DISPID *pid)
{
    if (m_api.ptr() == NULL)
        return E_NOTIMPL;

    std::string sName(CW2A(bstrName).m_psz);

    if (m_api->HasProperty(sName) || m_api->HasMethod(sName) || m_api->HasEvent(sName)) {
        *pid = AxIdMap.getIdForValue(sName);
    } else {
        *pid = -1;
    }
    return S_OK;
}

template <class T, class IDISP, const IID* piid>
HRESULT JSAPI_IDispatchEx<T,IDISP,piid>::InvokeEx(DISPID id, LCID lcid, WORD wFlags,
                                             DISPPARAMS *pdp, VARIANT *pvarRes, 
                                             EXCEPINFO *pei, 
                                             IServiceProvider *pspCaller)
{
    if (m_api.ptr() == NULL || !AxIdMap.idExists(id)) {
        return E_NOTIMPL;
    }
    try {
        std::string sName = AxIdMap.getValueForId<std::string>(id);

        if (wFlags & DISPATCH_PROPERTYGET && m_api->HasProperty(sName)) {

            FB::variant rVal = m_api->GetProperty(sName);

            m_host->getComVariant(pvarRes, rVal);

        } else if (wFlags & DISPATCH_PROPERTYPUT && m_api->HasProperty(sName)) {

            FB::variant newVal = m_host->getVariant(&pdp->rgvarg[0]);

            m_api->SetProperty(sName, newVal);

        } else if (wFlags & DISPATCH_METHOD && m_api->HasMethod(sName)) {

            std::vector<FB::variant> params;
            for (int i = pdp->cArgs - 1; i >= 0; i--) {
                params.push_back(m_host->getVariant(&pdp->rgvarg[i]));
            }
            FB::variant rVal = m_api->Invoke(sName, params);

            m_host->getComVariant(pvarRes, rVal);

        } else {
            throw FB::invalid_member("Invalid method or property name");
        }
    } catch (FB::invalid_member) {
        return E_NOTIMPL;
    } catch (FB::script_error se) {
        if (pei != NULL) {
            pei->bstrSource = CComBSTR(ACTIVEX_PROGID);
            pei->bstrDescription = CComBSTR(se.what());
            pei->bstrHelpFile = NULL;
            pei->pfnDeferredFillIn = NULL;
            pei->scode = E_NOTIMPL;
        }
        return DISP_E_EXCEPTION;
    } catch (...) {
        return E_NOTIMPL;
    }

    return S_OK;
}

template <class T, class IDISP, const IID* piid>
HRESULT JSAPI_IDispatchEx<T,IDISP,piid>::DeleteMemberByName(BSTR bstrName, DWORD grfdex)
{
    return E_NOTIMPL;
}

template <class T, class IDISP, const IID* piid>
HRESULT JSAPI_IDispatchEx<T,IDISP,piid>::DeleteMemberByDispID(DISPID id)
{
    return E_NOTIMPL;
}

template <class T, class IDISP, const IID* piid>
HRESULT JSAPI_IDispatchEx<T,IDISP,piid>::GetMemberProperties(DISPID id, DWORD grfdexFetch,
                                                        DWORD *pgrfdex)
{
    return E_NOTIMPL;
}

template <class T, class IDISP, const IID* piid>
HRESULT JSAPI_IDispatchEx<T,IDISP,piid>::GetMemberName(DISPID id, BSTR *pbstrName)
{
    try {
        CComBSTR outStr(AxIdMap.getValueForId<std::string>(id).c_str());

        *pbstrName = outStr.Detach();
        return S_OK;
    } catch (...) {
        return DISP_E_UNKNOWNNAME;
    }
    return E_NOTIMPL;
}

template <class T, class IDISP, const IID* piid>
HRESULT JSAPI_IDispatchEx<T,IDISP,piid>::GetNextDispID(DWORD grfdex, DISPID id, DISPID *pid)
{
    return E_NOTIMPL;
}

template <class T, class IDISP, const IID* piid>
HRESULT JSAPI_IDispatchEx<T,IDISP,piid>::GetNameSpaceParent(IUnknown **ppunk)
{
    return E_NOTIMPL;
}

#endif // H_JSAPI_IDISPATCHEX