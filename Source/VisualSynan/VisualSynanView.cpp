// VisualSynanView.cpp : implementation of the CVisualSynanView class
//

#include "stdafx.h"
#include "VisualSynan.h"

#include "VisualSynanDoc.h"
#include "VisualSynanView.h"
#include "resource.h"
#include "WINGDI.h"

CFont	CVisualSynanView::m_FontForWords;
CFont	CVisualSynanView::m_FontForGroupNames;
CFont	CVisualSynanView::m_BoldFontForWords;
CFont	CVisualSynanView::m_BoldUnderlineFontForWords;
CFont	CVisualSynanView::m_UnderlineFontForWords;


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define ID_HOMONYMS_MENU_ITEM 1000
#define ID_WORD_TOOL 1100
/////////////////////////////////////////////////////////////////////////////
// CVisualSynanView

IMPLEMENT_DYNCREATE(CVisualSynanView, CScrollView)

BEGIN_MESSAGE_MAP(CVisualSynanView, CScrollView)
	ON_WM_CONTEXTMENU()
	//{{AFX_MSG_MAP(CVisualSynanView)
	ON_WM_PAINT()
	ON_WM_RBUTTONDOWN()
	ON_WM_SIZE()
	ON_WM_VSCROLL()
	ON_COMMAND(ID_VIEW_TEST, OnViewTest)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_NOTIFY_EX( TTN_NEEDTEXT, 0, OnNeedText)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CVisualSynanView construction/destruction

CVisualSynanView::CVisualSynanView()
{
	// TODO: add construction code here
	m_pHomonymsArray = NULL;
	m_bDefaultFont = TRUE;
	m_bExistUsefulFont = FALSE;
	m_iActiveWordTT = -1;
	m_iActiveSentenceTT = -1;
	m_iActiveWord = -1;
	m_iActiveSentence = -1;
	m_bShowGroups = TRUE;
	m_bFirsTime = TRUE;
	m_bMore = TRUE;
	m_iStartSent = 0;
	m_iStartLine = 0;
	m_iOffset = 0;
	SetScrollSizes(MM_TEXT, CSize(0,0));
}

CVisualSynanView::~CVisualSynanView()
{
	
}

BOOL CVisualSynanView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CVisualSynanView drawing

void CVisualSynanView::OnDraw(CDC* pDC)
{
	CVisualSynanDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	// TODO: add draw code for native data here
}

/////////////////////////////////////////////////////////////////////////////
// CVisualSynanView printing

BOOL CVisualSynanView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CVisualSynanView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CVisualSynanView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// CVisualSynanView diagnostics

#ifdef _DEBUG
void CVisualSynanView::AssertValid() const
{
	CView::AssertValid();
}

void CVisualSynanView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CVisualSynanDoc* CVisualSynanView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CVisualSynanDoc)));
	return (CVisualSynanDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CVisualSynanView message handlers

void CVisualSynanView::OnPaint() 
{
	CString S = GetDocument()->m_VisualSentences.m_WorkTimeStr;
	((CFrameWnd*)((CVisualSynanApp*)AfxGetApp())->m_pMainWnd)->SetMessageText(S);
	ASSERT( GetDocument() );


	CClientDC clDC(this);
	CView::OnPaint();

	CRect clientRect;	
	CDC memDC;
	CBitmap bmBmp;
	
	CRect rectDevice;

	//creating memory DC
	GetClientRect(&clientRect);
	rectDevice = clientRect;
	OnPrepareDC(&clDC);
	clDC.DPtoLP(&clientRect);
	bmBmp.CreateCompatibleBitmap(&clDC, rectDevice.right , rectDevice.bottom);
	memDC.CreateCompatibleDC(&clDC);

	CBitmap* pOldBitmap = memDC.SelectObject(&bmBmp);
	CBrush brBackground(::GetSysColor(COLOR_WINDOW));
	memDC.FillRect(&rectDevice, &brBackground);
	memDC.SetBkColor(::GetSysColor(COLOR_WINDOW));

	//selecting choosen font
	CFont* pOldFont;

	if( m_bExistUsefulFont)
	{
		pOldFont = memDC.SelectObject(&m_FontForWords);
	}



	if( m_bFirsTime && !GetDocument()->NoSentences() )
	{
		m_bFirsTime = FALSE;
		GetDocument()->CalculateCoordinates(&memDC,clientRect.right, m_bShowGroups);
	}


	int iOffset = clientRect.top - rectDevice.top;

	//drawing sentences in memory DC
	GetDocument()->PrintSentences(&memDC,clientRect, iOffset);

	//drawing it on the screen
	clDC.BitBlt(0,clientRect.top, rectDevice.right,rectDevice.bottom,&memDC,0,0,SRCCOPY);

	//restoring old bitmap and old font
	memDC.SelectObject(pOldBitmap);
	if( m_bExistUsefulFont)
		memDC.SelectObject(pOldFont);
	memDC.DeleteDC();	
	
	
	ResizeScroll();
}

void CVisualSynanView::OnRButtonDown(UINT nFlags, CPoint point) 
{

	CView::OnRButtonDown(nFlags, point);
}

void CVisualSynanView::OnContextMenu(CWnd*, CPoint point)
{
	CClientDC dc(NULL);
	OnPrepareDC(&dc);
	CPoint ClientPoint = point;
	ScreenToClient(&ClientPoint);
	dc.DPtoLP(&ClientPoint);		
	
	

	// CG: This block was added by the Pop-up Menu component
	{
		if (point.x == -1 && point.y == -1){
			//keystroke invocation
			CRect rect;
			GetClientRect(rect);
			//ClientToScreen(rect);
			ClientToScreen(rect);
			point = rect.TopLeft();
			point.Offset(5, 5);
		}

				
		BOOL bInSomeWord;

		bInSomeWord = GetDocument()->GetHomonymsArray(ClientPoint,&m_pHomonymsArray,&m_iActiveSentence,&m_iActiveWord);
		if(bInSomeWord)
		{
			CMenu menu;
			menu.CreatePopupMenu();
			
			CString strLemma;
			for(int i = 0 ; i < m_pHomonymsArray->GetSize() ; i++)
			{
				strLemma = ((CSynHomonym*)m_pHomonymsArray->GetAt(i))->m_strLemma;
				CString grm = ((CSynHomonym*)m_pHomonymsArray->GetAt(i))->m_strCommonGrammems;
				if (!grm.IsEmpty())
					strLemma += " " + grm;;
				menu.AppendMenu(MF_STRING | MF_ENABLED,ID_HOMONYMS_MENU_ITEM + i, strLemma);
			}

			menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y,
				this);
		}
	}
}

BOOL CVisualSynanView::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo) 
{

	if(pHandlerInfo == NULL)
	{
		if(m_pHomonymsArray)
			if(( (nID >= ID_HOMONYMS_MENU_ITEM) && (nID < ID_HOMONYMS_MENU_ITEM + (UINT)(m_pHomonymsArray->GetSize()))) && (nCode == CN_COMMAND) )			
			{
				BOOL bInvalidate;
				bInvalidate = GetDocument()->SetActiveHomonym(m_iActiveSentence, m_iActiveWord,nID - ID_HOMONYMS_MENU_ITEM);
				if(bInvalidate)
				{
					CClientDC dc(this);
					Recalculate(dc);
					Invalidate();
				}
			}
	}
	return CView::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}






int CALLBACK GetFefaultFontEx(
  const LOGFONT FAR * elfLogFont,  // pointer to logical-font data
  const TEXTMETRIC FAR *lpntm,  // pointer to physical-font data
  unsigned long FontType,            // type of font
  LPARAM lParam            ) // pointer to application-defined data);
{
	if( (FontType & TRUETYPE_FONTTYPE) && (elfLogFont->lfCharSet & RUSSIAN_CHARSET) )
	{
		CVisualSynanView* pView = (CVisualSynanView*)lParam;
		
		pView->m_LogFontForWords.lfCharSet = RUSSIAN_CHARSET;//elfLogFont->lfCharSet;
		pView->m_LogFontForWords.lfClipPrecision = elfLogFont->lfClipPrecision;
		pView->m_LogFontForWords.lfEscapement = elfLogFont->lfEscapement;
		wcscpy(pView->m_LogFontForWords.lfFaceName,elfLogFont->lfFaceName);
		pView->m_LogFontForWords.lfHeight = 24;//elfLogFont->lfHeight;
		pView->m_LogFontForWords.lfItalic = elfLogFont->lfItalic;
		pView->m_LogFontForWords.lfOrientation = elfLogFont->lfOrientation;
		pView->m_LogFontForWords.lfOutPrecision = elfLogFont->lfOutPrecision;
		pView->m_LogFontForWords.lfPitchAndFamily = elfLogFont->lfPitchAndFamily;
		pView->m_LogFontForWords.lfQuality = elfLogFont->lfQuality;
		pView->m_LogFontForWords.lfStrikeOut = elfLogFont->lfStrikeOut;
		pView->m_LogFontForWords.lfUnderline = elfLogFont->lfUnderline;
		pView->m_LogFontForWords.lfWeight = elfLogFont->lfWeight;
		pView->m_LogFontForWords.lfWidth = 0;//elfLogFont->lfWidth;		

		pView->m_bExistUsefulFont = TRUE;
		
		return 0;
	}

	return 1;
}


int CALLBACK TestIfTrueTypeEx(
  const LOGFONT FAR *lpelf,  // pointer to logical-font data
  const TEXTMETRIC FAR *lpntm,  // pointer to physical-font data
  unsigned long FontType,            // type of font
  LPARAM lParam            ) // pointer to application-defined data);
{

	CVisualSynanView* pView = (CVisualSynanView*)lParam;
	if( !wcscmp(pView->m_LogFontForWords.lfFaceName,lpelf->lfFaceName))
	{
		if( FontType & TRUETYPE_FONTTYPE )		
			pView->m_bExistUsefulFont = TRUE;
		else
			pView->m_bExistUsefulFont = FALSE;

		return 0;
	}

	return 1;
}





void CVisualSynanView::UpdateFontsFromLogFont() 
{
	m_FontForWords.CreateFontIndirect(&m_LogFontForWords);


	m_FontForGroupNames.CreateFont((m_LogFontForWords.lfHeight/3)*2,
									(m_LogFontForWords.lfWidth/3) * 2,
									m_LogFontForWords.lfEscapement,
									m_LogFontForWords.lfOrientation,
									m_LogFontForWords.lfWeight,
									m_LogFontForWords.lfItalic,
									m_LogFontForWords.lfUnderline,
									m_LogFontForWords.lfStrikeOut,
									m_LogFontForWords.lfCharSet,
									m_LogFontForWords.lfOutPrecision,
									m_LogFontForWords.lfClipPrecision,
									m_LogFontForWords.lfQuality,
									m_LogFontForWords.lfPitchAndFamily,
									m_LogFontForWords.lfFaceName);

	m_BoldFontForWords.CreateFont(		m_LogFontForWords.lfHeight,
									m_LogFontForWords.lfWidth,
									m_LogFontForWords.lfEscapement,
									m_LogFontForWords.lfOrientation,
									FW_BOLD,									
									m_LogFontForWords.lfItalic,									
									m_LogFontForWords.lfUnderline,
									m_LogFontForWords.lfStrikeOut,
									m_LogFontForWords.lfCharSet,
									m_LogFontForWords.lfOutPrecision,
									m_LogFontForWords.lfClipPrecision,
									m_LogFontForWords.lfQuality,
									m_LogFontForWords.lfPitchAndFamily,
									m_LogFontForWords.lfFaceName);	

	m_BoldUnderlineFontForWords.CreateFont(		m_LogFontForWords.lfHeight,
									m_LogFontForWords.lfWidth,
									m_LogFontForWords.lfEscapement,
									m_LogFontForWords.lfOrientation,
									FW_BOLD,									
									m_LogFontForWords.lfItalic,									
									TRUE,
									m_LogFontForWords.lfStrikeOut,
									m_LogFontForWords.lfCharSet,
									m_LogFontForWords.lfOutPrecision,
									m_LogFontForWords.lfClipPrecision,
									m_LogFontForWords.lfQuality,
									m_LogFontForWords.lfPitchAndFamily,
									m_LogFontForWords.lfFaceName);	

	m_UnderlineFontForWords.CreateFont(		m_LogFontForWords.lfHeight,
									m_LogFontForWords.lfWidth,
									m_LogFontForWords.lfEscapement,
									m_LogFontForWords.lfOrientation,
									m_LogFontForWords.lfWeight,									
									m_LogFontForWords.lfItalic,									
									TRUE,
									m_LogFontForWords.lfStrikeOut,
									m_LogFontForWords.lfCharSet,
									m_LogFontForWords.lfOutPrecision,
									m_LogFontForWords.lfClipPrecision,
									m_LogFontForWords.lfQuality,
									m_LogFontForWords.lfPitchAndFamily,
									m_LogFontForWords.lfFaceName);	

};

void CVisualSynanView::OnInitialUpdate() 
{
	CView::OnInitialUpdate();	
	CClientDC dc(this);
	LOGFONT lfFont;
	wcscpy(lfFont.lfFaceName,_T("Times New Roman"));
	lfFont.lfCharSet = RUSSIAN_CHARSET;
	EnumFontFamiliesEx(dc.m_hDC, &lfFont , &GetFefaultFontEx,(LPARAM)this,0);
	if( !m_bExistUsefulFont )
		EnumFontFamiliesEx(dc.m_hDC, NULL, &GetFefaultFontEx,(LPARAM)this,0);

	UpdateFontsFromLogFont();

	//creating tooltip ctrl
	EnableToolTips();
	m_ctrlToolTip.Create(this);
	CRect StupidRect(0,0,0,0);//some unuseful rect
								//we will change this rect dinamicly
	m_ctrlToolTip.AddTool( this, LPSTR_TEXTCALLBACK, StupidRect ,ID_WORD_TOOL);
	m_ctrlToolTip.Activate(TRUE);
	m_ctrlToolTip.SetDelayTime(TTDT_AUTOPOP,1000000);
	ResizeScroll();
}

void CVisualSynanView::ResizeScroll()
{
	CClientDC dc(NULL);
	OnPrepareDC(&dc);
	CSize sizeDoc = GetDocument()->GetSentencesSize();
	dc.LPtoDP(&sizeDoc);
	SetScrollSizes(MM_TEXT, sizeDoc);
}

 void CVisualSynanView::Recalculate(CDC& clDC, CPrintInfo* pInfo)
{
	if( GetDocument()->NoSentences() )
		return;

	//selecting choosen font
	CFont* pOldFont = NULL;
	
	//creating memory DC
	CRect clientRect;
	GetClientRect(&clientRect);
	OnPrepareDC(&clDC,pInfo);
	clDC.DPtoLP(&clientRect);

	if( m_bExistUsefulFont)
	{
		pOldFont = clDC.SelectObject(&m_FontForWords);
	}

	//calculating sentences coordinates 
	GetDocument()->CalculateCoordinates(&clDC,clientRect.right, m_bShowGroups);

	if( pOldFont )
		clDC.SelectObject(pOldFont);

}


void CVisualSynanView::OnSize(UINT nType, int cx, int cy) 
{
			
	ResizeScroll();
	CView::OnSize(nType, cx, cy);	
	CClientDC clDC(this);
	Recalculate(clDC);
	Invalidate();


}

void CVisualSynanView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	if( lHint != NULL)
	{
		ResizeScroll();
		CVisualSynanView::OnUpdate(pSender, lHint, pHint);
	}
}


void CVisualSynanView::OnPrepareDC(CDC* pDC, CPrintInfo* pInfo)
{
	
	CScrollView::OnPrepareDC(pDC, pInfo);
	if( pInfo )
	{		
		if( m_iStartSent == -1 )
		{
			pInfo->m_bContinuePrinting = FALSE;
			m_iStartSent = 0;
			m_iStartLine = 0;
			m_iOffset = 0;
			CClientDC dc(this);
			Recalculate(dc);
			return;
		}

		pInfo->m_bContinuePrinting = TRUE;
		
		pDC->SetMapMode(MM_ANISOTROPIC);		
		CSize sizeDoc = GetDocument()->GetSentencesSize();		
		if( (sizeDoc.cx == 0) && (sizeDoc.cy == 0) )
		{
			sizeDoc.cx = 200;
			sizeDoc.cy = 200;
		}
		
		pDC->SetWindowExt(sizeDoc); 

		int xLogPixPerInch = pDC->GetDeviceCaps(LOGPIXELSX);
		int yLogPixPerInch = pDC->GetDeviceCaps(LOGPIXELSY);

		long xExt = (long)sizeDoc.cx * xLogPixPerInch;
		xExt /= 100 ;
		long yExt = (long)sizeDoc.cy * yLogPixPerInch;
		yExt /= 100 ;
		pDC->SetViewportExt((int)xExt, (int)yExt);
	}

}

void CVisualSynanView::LogFontCpy(LOGFONT* dstFont, LOGFONT srcFont)
{
		dstFont->lfCharSet = srcFont.lfCharSet;
		dstFont->lfClipPrecision = srcFont.lfClipPrecision;
		dstFont->lfEscapement = srcFont.lfEscapement;
		wcscpy(dstFont->lfFaceName,srcFont.lfFaceName);
		dstFont->lfHeight = srcFont.lfHeight;
		dstFont->lfItalic = srcFont.lfItalic;
		dstFont->lfOrientation = srcFont.lfOrientation;
		dstFont->lfOutPrecision = srcFont.lfOutPrecision;
		dstFont->lfPitchAndFamily = srcFont.lfPitchAndFamily;
		dstFont->lfQuality = srcFont.lfQuality;
		dstFont->lfStrikeOut = srcFont.lfStrikeOut;
		dstFont->lfUnderline = srcFont.lfUnderline;
		dstFont->lfWeight = srcFont.lfWeight;
		dstFont->lfWidth = srcFont.lfWidth;		
}

BOOL CVisualSynanView::PreTranslateMessage(MSG* pMsg) 
{
	if(	pMsg->message== WM_LBUTTONDOWN ||
		pMsg->message== WM_LBUTTONUP ||
		pMsg->message== WM_MOUSEMOVE) 
	{
		CClientDC dc(NULL);
		OnPrepareDC(&dc);
		BOOL bInSomeWord;
		CPoint ClientPoint = pMsg->pt;
		ScreenToClient(&ClientPoint);
		dc.DPtoLP(&ClientPoint);
		bInSomeWord = GetDocument()->GetHomonymsArray(ClientPoint,NULL,&m_iActiveSentenceTT,&m_iActiveWordTT);

		if(bInSomeWord)
		{				
			dc.LPtoDP(&ClientPoint);
			CRect rect(ClientPoint.x - 1, ClientPoint.y - 1, ClientPoint.x + 1,ClientPoint.y + 1);
			m_ctrlToolTip.SetToolRect(this,ID_WORD_TOOL,rect);
			m_ctrlToolTip.RelayEvent(pMsg);
		}			
		else
		{
			m_iActiveWordTT = -1;
			m_iActiveSentenceTT = -1;
		}
	}
	return CScrollView::PreTranslateMessage(pMsg);
}

int CVisualSynanView::OnNeedText( UINT id, NMHDR * pNMHDR, LRESULT * pResult )
{
	try
	{
		if( (m_iActiveSentenceTT != -1) && (m_iActiveWordTT != -1))
		{
			TOOLTIPTEXT *pTTT = (TOOLTIPTEXT *)pNMHDR;
			CString strLemma, strGramChar;

			//getting lemma and grammatiacl characteristics of the active word
			BOOL bRes = GetDocument()->GetActiveHomDescr(m_iActiveSentenceTT,m_iActiveWordTT,strLemma,strGramChar);
			if(bRes)
			{
				CString s = strLemma + CString(" ") + strGramChar;
				if (s.GetLength() > 80)
				{
					s.Delete(76, s.GetLength() - 76); 
					s += "...";
				};
				ASSERT(s.GetLength() < 80);
				wcscpy(pTTT->szText, s);

				return FALSE;
			}
		}
		return TRUE;
	}
	catch(...)
	{
		return TRUE;
	}
}


void CVisualSynanView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	
	CScrollView::OnVScroll(nSBCode, nPos, pScrollBar);

}

void CVisualSynanView::Reset()
{
	m_bFirsTime = TRUE;
	GetDocument()->Reset();
}

void CVisualSynanView::OnBuildRels(CString& str) 
{
	GetDocument()->BuildRels(str);		
}


void CVisualSynanView::OnViewTest() 
{
	// TODO: Add your command handler code here
	
}
