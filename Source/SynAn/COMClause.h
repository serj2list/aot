#pragma once 

#include "resource.h"       // main symbols
#include "StdAfx.h"
#include "ComClauseVariant.h"
#include "Synan_i.h"

class CClause;
class CSyntaxOpt;

class ATL_NO_VTABLE CCOMClause : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CCOMClause, &CLSID_Clause>,
	public IDispatchImpl<IClause, &IID_IClause, &LIBID_SYNANLib>
{
public:
	CCOMClause();

DECLARE_REGISTRY_RESOURCEID(IDR_CLAUSE)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CCOMClause)
	COM_INTERFACE_ENTRY(IClause)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

// IClause
public:
	STDMETHOD(get_ClauseVariant)(/*[in]*/ long lVarNum, /*[out, retval]*/ IClauseVariant* *pVal);
	STDMETHOD(get_VariantsCount)(/*[out, retval]*/ long *pVal);
	STDMETHOD(get_LastWord)(/*[out, retval]*/ long *pVal);
	STDMETHOD(get_AntecedentWord)(/*[out, retval]*/ long *pVal);
	STDMETHOD(get_RelativeWord)(/*[out, retval]*/ long *pVal);
	STDMETHOD(get_FirstWord)(/*[out, retval]*/ long *pVal);
	STDMETHOD(get_ConjStr)(/*[in]*/int iNum, /*[out, retval]*/ BSTR *pVal);
	STDMETHOD(get_ConjsCount)(/*[out, retval]*/ long *pVal);
	STDMETHOD(get_Description)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(get_ClauseRoots)(/*[in]*/long lNum, /*[out, retval]*/ IClauseRoot* *pVal);
	STDMETHOD(get_ClauseRootsCount)(/*[out, retval]*/ long *pVal);
	BOOL Init(const CClause* pClause, const CSentence* pSentence);
	const CSyntaxOpt* GetOpt() const;
	
protected:
	const CClause* m_pClause;	
	SClauseType m_ClauseEmptyType;

};
