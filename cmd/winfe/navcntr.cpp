/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.0 (the "NPL"); you may not use this file except in
 * compliance with the NPL.  You may obtain a copy of the NPL at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the NPL is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the NPL
 * for the specific language governing rights and limitations under the
 * NPL.
 *
 * The Initial Developer of this code under the NPL is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation.  All Rights
 * Reserved.
 */

#include "stdafx.h"
#include "navfram.h"
#include "feimage.h"
#include "net.h"
#include "cxsave.h"
#include "cxicon.h"
#include "fegui.h"
#include "rdfliner.h"
#include "mmsystem.h"
#include "winproto.h"
#include "xp_ncent.h"
#include "pain.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#ifndef _AFXDLL
#undef new
#endif

#ifndef _AFXDLL
#define new DEBUG_NEW
#endif

#include "dropmenu.h"

//////////////////////////
// RDF Event Handlers

// The Main Event Handler for the NavCenter.  Handles events on the selector bar AND within the tree
// views.
void notifyProcedure (HT_Notification ns, HT_Resource n, HT_Event whatHappened,
			void *token, uint32 tokenType) 
{
	CSelector* theSelector = (CSelector*)ns->data;
	if (theSelector == NULL)
		return;

	HT_View theView = HT_GetView(n);
	
	// The pane has to handle some events.  These will go here.
	if (whatHappened == HT_EVENT_VIEW_SELECTED)
	{
		CSelectorButton* pButton = theSelector->GetCurrentButton();

		// If the new selected view is NULL, then we need to make sure our pane is closed.
		if (theView == NULL && pButton != NULL)
		{
			// We're open.  Close the pane.
			((CNSNavFrame*)theSelector->GetParentFrame())->CollapseWindow();
		}
		else if (theView != NULL)
		{
			// We have a view. Select it.
			CSelectorButton* pNewButton = (CSelectorButton*)HT_GetViewFEData(theView);
			theSelector->SetCurrentButton(pNewButton);
			
			if (pButton == NULL)
			{
				// Need to open the pane.
				((CNSNavFrame*)theSelector->GetParentFrame())->ExpandWindow();
			}
		}
		return;
	}
	
	if (theView == NULL)
		return;

	if (whatHappened == HT_EVENT_VIEW_ADDED) 
	{
		theSelector->AddButton(theView);
		theSelector->RearrangeIcons();
	}
	else if (whatHappened == HT_EVENT_VIEW_DELETED)
	{
		// Delete the content view
		CSelectorButton* pButton = (CSelectorButton*)HT_GetViewFEData(theView);
		if (pButton)
		{
			// Delete the button
			delete pButton;

			// Remove our FE data
			HT_SetViewFEData(theView, NULL);
		}
	}
	else if (whatHappened == HT_EVENT_NODE_VPROP_CHANGED && HT_TopNode(theView) == n)
	{
		// Top level node changed its name/icon.  Need to change the button text, window title bar, and 
		// embedded nav menu bar. Also need to update the current tree view and the column headers. (Whew!)
		CSelectorButton* pButton = (CSelectorButton*)HT_GetViewFEData(theView);
		if (pButton && pButton->m_hWnd)
		{
			// Invalidate the button.
			pButton->Invalidate();

			// The remaining invalidation only happens if the selected view is the one that was just
			// changed.
			if (theSelector->GetCurrentButton() != pButton)
				return;

			// Invalidate the title bar.
			CFrameWnd* pFrame = pButton->GetParentFrame();
			if (pFrame->IsKindOf(RUNTIME_CLASS(CNSNavFrame)))
			{
				CNSNavFrame* pNavFrame = (CNSNavFrame*)pFrame;
				if (pNavFrame)
				{
					// Invalidate the title bar.
					CNavTitleBar* pBar = pNavFrame->GetNavTitleBar();
					if (pBar)
						pBar->Invalidate();
					
					// Invalidate the tree view.
					CRDFContentView* theOutlinerView = theSelector->GetContentView();
					if (theOutlinerView)
					{
						CRDFOutlinerParent* pParent = (CRDFOutlinerParent*)(theOutlinerView->GetOutlinerParent());
						if (pParent)
						{
							pParent->Invalidate();
							COutliner* pOutliner = pParent->GetOutliner();
							if (pOutliner)
								pOutliner->Invalidate();
						}
					}
				}
			}
		}
	}
	else if (whatHappened == HT_EVENT_NODE_EDIT && HT_TopNode(theView) == n)
	{
		// Edit being performed on a selector bar item. (STILL TO DO)
	}
	else if (whatHappened == HT_EVENT_VIEW_WORKSPACE_REFRESH)
		theSelector->RearrangeIcons(); // Redraw the selector.

	// If the pane doesn't handle the event, then the tree view does, but only if this button is the current
	// selected button on the selector bar.
	else
	{
		// View needs to exist in order for us to send this event to it.
		CSelectorButton* pButton = (CSelectorButton*)HT_GetViewFEData(theView);
		if (pButton && (pButton == theSelector->GetCurrentButton()))
		{
			CRDFContentView* theOutlinerView = theSelector->GetContentView();
			if (theOutlinerView)
			{
				CRDFOutliner* theOutliner = (CRDFOutliner*)(theOutlinerView->GetOutlinerParent()->GetOutliner());
				if (theOutliner)
					theOutliner->HandleEvent(ns, n, whatHappened);
			}
		}
		else if (pButton && whatHappened == HT_EVENT_NODE_OPENCLOSE_CHANGED && 
				 (pButton->GetDropMenu() != NULL))
		{
			// We opened or closed a node in the popup menu.
			PRBool openState;
			HT_GetOpenState(n, &openState);
			if (openState)
				pButton->FillInMenu(n);
		}
	}
}

// SelectorButton

CSelectorButton::~CSelectorButton() 
{
	m_Node = NULL;
}

int CSelectorButton::Create(CWnd *pParent, int nToolbarStyle, CSize noviceButtonSize, 
							CSize advancedButtonSize,
							LPCTSTR pButtonText, LPCTSTR pToolTipText, 
							LPCTSTR pStatusText,
							CSize bitmapSize, int nMaxTextChars, int nMinTextChars, 
							HT_Resource pNode,
							DWORD dwButtonStyle)
{	
	BOOKMARKITEM bookmark; // For now, create with the pictures style. No text ever.
	BOOL bResult = CRDFToolbarButton::Create(pParent, TB_PICTURES, noviceButtonSize, advancedButtonSize,
		   pButtonText, pToolTipText, pStatusText, bitmapSize, nMaxTextChars, nMinTextChars,
		   bookmark, pNode, dwButtonStyle);
	return bResult;
}

CRDFContentView* CSelectorButton::GetContentView()
{
	return m_pSelector->GetContentView();
}

void CSelectorButton::OnAction()
{
	if (m_pDropMenu == NULL || !m_pDropMenu->IsOpen())
	{
		HT_Pane pane = HT_GetPane(m_HTView);
		if (m_pSelector->GetCurrentButton() == this)
		{
			// View is already selected.  We are now closing the pane.
			HT_SetSelectedView(pane, NULL);
		}
		else HT_SetSelectedView(pane, m_HTView);
	}
}

CSize CSelectorButton::GetButtonSizeFromChars(CString s, int c)
{
    if (m_nToolbarStyle == TB_PICTURES)
		return GetBitmapOnlySize();
	else if(m_nToolbarStyle == TB_TEXT)
		return(GetTextOnlySize(s, c));
	else
		return(GetBitmapOnTopSize(s, c));
}


void CSelectorButton::DrawPicturesMode(HDC hDC, CRect rect)
{

	DrawButtonBitmap(hDC, rect);

}

void CSelectorButton::DrawPicturesAndTextMode(HDC hDC, CRect rect)
{
	//DrawBitmapOnTop(hDC, rect);
	DrawButtonBitmap(hDC, rect);
}

void CSelectorButton::GetPicturesAndTextModeTextRect(CRect &rect)
{
	//GetBitmapOnTopTextRect(rect);
	GetPicturesModeTextRect(rect);
}

void CSelectorButton::GetPicturesModeTextRect(CRect &rect)
{
	//GetBitmapOnTopTextRect(rect);
	CToolbarButton::GetPicturesModeTextRect(rect);
}

void CSelectorButton::DisplayMenuOnDrag()
{
	CPoint point = RequestMenuPlacement();

	if(m_pDropMenu == NULL || !m_pDropMenu->IsOpen())
	{
		int nCount;
		if(m_pDropMenu != NULL)
		{
			nCount = m_pDropMenu->GetMenuItemCount();

			// clean out the menu before adding to it
			for(int i = nCount - 1; i >= 0; i--)
			{
				m_pDropMenu->DeleteMenu(i, MF_BYPOSITION);
			}
			m_pDropMenu->DestroyDropMenu();
			delete m_pDropMenu;
		}

		m_pDropMenu = new CDropMenu;
		SendMessage(NSDRAGMENUOPEN, (WPARAM)GetButtonCommand(), (LPARAM)m_pDropMenu);

		nCount = m_pDropMenu->GetMenuItemCount();

		if(nCount > 0)
		{
			CDropMenuDropTarget *dropTarget = new CDropMenuDropTarget(m_pDropMenu);
			m_pDropMenu->TrackDropMenu(this, point.x, point.y, TRUE, dropTarget, TRUE);
		}
		else
		{
			CDropMenu* theLastMenu = CDropMenu::GetLastDropMenu();
			if(theLastMenu !=NULL)
			{
				//we only want one drop menu open at a time, so close the last one if one is
				//still open. 
				theLastMenu->Deactivate();
				CDropMenu::SetLastDropMenu(NULL);
			}
		}
	}
}

BOOL CSelectorButton::CreateRightMouseMenu()
{
	// Selector buttons should be selected if the tree is visible.
	if (m_pSelector->IsTreeVisible())
	{
		// Actually switch to this view.
		if (!m_bDepressed)
		{
			// Depress.
			OnAction();
		}
	}

	// Build the actual menu.
	m_MenuCommandMap.Clear();
	
	// Set the selected view, but do a mask if we're closed so we don't end up opening the pane.
	
	HT_SetSelectedView(HT_GetPane(m_HTView), m_HTView);
	HT_SetSelection(HT_TopNode(m_HTView));
	
	HT_Cursor theCursor = HT_NewContextualMenuCursor(m_HTView, PR_TRUE, PR_FALSE);
	if (theCursor != NULL)
	{
		// We have a cursor. Attempt to iterate
		HT_MenuCmd theCommand; 
		while (HT_NextContextMenuItem(theCursor, &theCommand))
		{
			char* menuName = HT_GetMenuCmdName(theCommand);
			if (theCommand == HT_CMD_SEPARATOR)
				m_menu.AppendMenu(MF_SEPARATOR);
			else
			{
				// Add the command to our command map
				CRDFMenuCommand* rdfCommand = new CRDFMenuCommand(menuName, theCommand);
				int index = m_MenuCommandMap.AddCommand(rdfCommand);
				m_menu.AppendMenu(MF_ENABLED, index+FIRST_HT_MENU_ID, menuName);
			}
		}
		HT_DeleteCursor(theCursor);
	}
	
	return TRUE;
}

void CSelectorButton::DisplayAndTrackMenu()
{
	CRDFToolbarButton::DisplayAndTrackMenu();
}

/////////////////////////////////////////////////////////////////////////////
// CNavCenterFrame

//IMPLEMENT_DYNAMIC(CSelector, CScrollView)
IMPLEMENT_DYNAMIC(CSelector, CView)
BEGIN_MESSAGE_MAP(CSelector, CView)
	//{{AFX_MSG_MAP(CMainFrame)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_MOUSEMOVE()
	ON_WM_ERASEBKGND()
	ON_WM_PARENTNOTIFY()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_TIMER()
	ON_MESSAGE(NSBUTTONDRAGGING, OnButtonDrag)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CSelector::CSelector(CRDFContentView* pContent)
{
	m_pContentView = pContent;
	m_xpos =  ICONXPOS;
	m_ypos = ICONYPOS;
    m_pDropTarget = NULL;
	m_Pane = NULL;
	m_Notification = NULL;
	m_scrollPos = 0;
	m_pCurButton = NULL;
	m_boundingBox.SetRectEmpty();
	m_hScrollBmp = ::LoadBitmap( AfxGetResourceHandle(), 
										 MAKEINTRESOURCE( IDB_VFLIPPY ));
	BITMAP bm;
	::GetObject( m_hScrollBmp, sizeof( bm ), &bm );
	m_hScrollBmpSize.cx = bm.bmWidth / 4;
	m_hScrollBmpSize.cy = bm.bmHeight;
	m_scrollUp = new CNavSelScrollButton(m_hScrollBmp, m_hScrollBmpSize.cx, 0, m_hScrollBmpSize.cy);
	m_scrollDown = new CNavSelScrollButton(m_hScrollBmp, m_hScrollBmpSize.cx, 2, m_hScrollBmpSize.cy);
	m_scrollDirection = NOSCROLL;

	m_hPaneSwitchTimer = 0;
}

CSelector::~CSelector()
{
	if (m_hScrollBmp) 
	{
		VERIFY( ::DeleteObject( m_hScrollBmp ));
	}
	
	if (m_Pane != NULL)
	{
		int count = HT_GetViewListCount(m_Pane);
		for (int i = 0; i < count; i++)
		{
			HT_View view = HT_GetNthView(m_Pane, i);
		
			if (view && HT_GetViewFEData(view))
			{
				CSelectorButton* theButton = (CSelectorButton*)HT_GetViewFEData(view);
				delete theButton;
				HT_SetViewFEData(view, NULL);
			}
		}
		
		XP_UnregisterNavCenter(m_Pane);
		HT_DeletePane(m_Pane);
	}

	if (m_scrollUp)
		delete m_scrollUp;
	if (m_scrollDown)
		delete m_scrollDown;

	delete m_Notification;
    if(m_pDropTarget) 
	{
        m_pDropTarget->Revoke();
        delete m_pDropTarget;
        m_pDropTarget = NULL;
    }
}

void CSelector::OnDraw(CDC* pDC) 
{
}

int CSelector::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if(!m_pDropTarget) {
        m_pDropTarget = new CSelectorDropTarget(this);
        m_pDropTarget->Register(this);
    }
	CRect tempRect(0, 0, 1, 1);
	m_scrollUp->Create("", BS_OWNERDRAW | BS_PUSHBUTTON , tempRect, this, SCROLLID);
	m_scrollDown->Create("", BS_OWNERDRAW | BS_PUSHBUTTON , tempRect, this, SCROLLID+1);
	m_scrollUp->SetWindowPos( &wndTopMost, 0, 0, 1, 1, 0);
	m_scrollDown->SetWindowPos( &wndTopMost, 0, 0, 1, 1, 0);
  
#ifdef XP_WIN32
	::SetClassLong(GetSafeHwnd(), GCL_HBRBACKGROUND, (COLOR_BTNFACE + 1));
#else
	::SetClassWord(GetSafeHwnd(), GCW_HBRBACKGROUND, (COLOR_BTNFACE + 1));
#endif

	return 0;
}

void CSelector::SelectNthView(int i)
{
	HT_View theView = HT_GetNthView(m_Pane, i);
	if (theView && HT_GetViewFEData(theView))
	{
		CSelectorButton* pButton = (CSelectorButton*)HT_GetViewFEData(theView);
		pButton->OnAction();
	}
}

BOOL CSelector::IsTreeVisible()
{
	return m_pCurButton != NULL;
}

void CSelector::SetCurrentButton(CSelectorButton* pButton)
{
	// Deselect the old button.
	if (m_pCurButton)
	{
		m_pCurButton->SetDepressed(FALSE);
	}

	// Selecting the button.
	m_pCurButton = pButton;
	m_pCurButton->SetDepressed(TRUE);
		
	// adjust the window. here.
	m_pContentView->SwitchHTViews(m_pCurButton->GetHTView());
	m_pContentView->SetFocus();
}

void CSelector::UnSelectAll()
{
	if (m_pCurButton)
		m_pCurButton->SetDepressed(FALSE);
	m_pCurButton = NULL;
}

CRDFContentView* CSelector::GetContentView()
{
	return m_pContentView;
}

CSelectorButton* CSelector::GetCurrentButton()
{
	return m_pCurButton;
}

void CSelector::OnSize( UINT nType, int cx, int cy )
{
	if (cy > m_boundingBox.Height() && m_scrollPos > 0) 
	{  // resize bigger need to scroll down.
		ScrollWindowEx( 0, SCROLLSTEP, NULL, NULL, NULL, NULL, SW_SCROLLCHILDREN | SW_INVALIDATE | SW_ERASE); 
		m_scrollPos -= SCROLLSTEP;

	}
	int32 dockStyle = ((CNSNavFrame*)GetParent())->GetDockStyle();
	if (dockStyle == DOCKSTYLE_HORZTOP || dockStyle == DOCKSTYLE_HORZBOTTOM) 
	{
		int left = cx-m_hScrollBmpSize.cx;
		m_scrollUp->MoveWindow(0, 0, m_hScrollBmpSize.cx,  m_hScrollBmpSize.cy);
		m_scrollUp->setBoundingBox(0, 0, m_hScrollBmpSize.cx, m_hScrollBmpSize.cy);
		m_scrollDown->MoveWindow(left, 0, m_hScrollBmpSize.cx ,  m_hScrollBmpSize.cy);
		m_scrollDown->setBoundingBox(left, 0, left + m_hScrollBmpSize.cx, m_hScrollBmpSize.cy);
	}
	else 
	{
		int top = cy - m_hScrollBmpSize.cy;
		int left = cx-m_hScrollBmpSize.cx;
		m_scrollUp->MoveWindow(left, 0, m_hScrollBmpSize.cx,  m_hScrollBmpSize.cy);
		m_scrollUp->setBoundingBox(left, 0, left + m_hScrollBmpSize.cx , m_hScrollBmpSize.cy);
		m_scrollDown->MoveWindow(left, top, m_hScrollBmpSize.cx ,  m_hScrollBmpSize.cy);
		m_scrollDown->setBoundingBox(left, top, left + m_hScrollBmpSize.cx, top + m_hScrollBmpSize.cy);
	}
	ShowScrollButton(NULL);
	GetClientRect(&m_boundingBox);
	RearrangeIcons();
}



void CSelector::ShowScrollButton(CSelectorButton* button)
{
	CRect tempRect;
	CNSNavFrame* pParent = (CNSNavFrame*)GetParent();
	int32 dockstyle = pParent->GetDockStyle();
	GetClientRect(&tempRect);
	// checked if we need to display the scroll button
	if (dockstyle == DOCKSTYLE_FLOATING || dockstyle == DOCKSTYLE_VERTLEFT ||
		dockstyle == DOCKSTYLE_VERTRIGHT) {
		if ((m_ypos - m_hScrollBmpSize.cy) > (tempRect.Height() - ICONYPOS)) {
			m_scrollUp->ShowWindow(SW_SHOW);
			m_scrollDown->ShowWindow(SW_SHOW);
			m_scrollUp->Invalidate();
			m_scrollDown->Invalidate();
		}
		else {
			m_scrollUp->ShowWindow(SW_HIDE);
			m_scrollDown->ShowWindow(SW_HIDE);
		}
	}
	else {
		if ((m_xpos - m_hScrollBmpSize.cx) > (tempRect.Width() - ICONXPOS)) {
			m_scrollUp->ShowWindow(SW_SHOW);
			m_scrollDown->ShowWindow(SW_SHOW);
			m_scrollUp->Invalidate();
			m_scrollDown->Invalidate();
		}
		else {
			m_scrollUp->ShowWindow(SW_HIDE);
			m_scrollDown->ShowWindow(SW_HIDE);
		}
	}
}


void CSelector::AddButton(HT_View theView)
{
	// Create the selector button.
	CSelectorButton* theNSViewButton = new CSelectorButton(this);

	// Set the HT view for the button.
	theNSViewButton->SetHTView(theView); 
	
	HT_SetViewFEData(theView, theNSViewButton);

	HT_Resource node = HT_TopNode(theView);
	char* pNodeURL = HT_GetNodeURL(node);
	char* pTitle = HT_GetNodeName(node);

	theNSViewButton->Create(this, theApp.m_pToolbarStyle, CSize(60,42), CSize(85, 25), 
							pTitle,
							pTitle, pNodeURL, 
							CSize(23,23), 10, 5, 
							node,
                            TB_HAS_DRAGABLE_MENU | TB_HAS_TIMED_MENU);
	
	CRect rect;
	rect.left = (long)m_xpos;
	rect.top = (long)m_ypos;

	CSize buttonSize = theNSViewButton->GetRequiredButtonSize();
	rect.right = rect.left + buttonSize.cx;
	rect.bottom = rect.top + buttonSize.cy;

	CNSNavFrame* pFrame = (CNSNavFrame*)GetParentFrame();
	int32 dockStyle = pFrame->GetDockStyle();

	if (dockStyle == DOCKSTYLE_HORZTOP || dockStyle == DOCKSTYLE_HORZBOTTOM)
	{
		m_xpos += BUTTON_WIDTH + 2;
		rect.bottom = rect.top + BUTTON_HEIGHT;
	}
	else 
	{
		m_ypos += buttonSize.cy + 2;
		rect.right = rect.left + BUTTON_WIDTH;
	}

	ShowScrollButton(theNSViewButton);
}

void CSelector::PopulatePane()
{
	// Construct the notification struct used by HT
	HT_Notification ns = new HT_NotificationStruct;
	XP_BZERO(ns, sizeof(HT_NotificationStruct));
	ns->notifyProc = notifyProcedure;
	ns->data = this;
	m_Notification = ns;
	
	// Construct the pane and give it our notification struct
	m_Pane = HT_NewPane(ns);
	HT_SetPaneFEData(m_Pane, this);
	
    //  Inform our XP layer of the new nav center.
    MWContext *pDockedCX = NULL;
    CNSNavFrame *pParentFrame = GetParentFrame()->IsKindOf(RUNTIME_CLASS(CNSNavFrame)) ? (CNSNavFrame*)GetParentFrame() : NULL;

    //  Since the XP layer isn't set up yet, we can't use
    //      XP_IsNavCenterDocked in order to tell
    //      if the window is docked.  Use instead wether or
    //      not the frame has a parent.
    if(pParentFrame && pParentFrame->GetParent()) {
        CFrameGlue *pGlue = CFrameGlue::GetFrameGlue(pParentFrame->GetTopLevelFrame());
        if(pGlue && pGlue->GetMainContext()) {
            pDockedCX = pGlue->GetMainContext()->GetContext();
        }
        ASSERT(pDockedCX);
    }
    XP_RegisterNavCenter(m_Pane, pDockedCX);

	// Place any icons that were immediately available.  (The remote ones will trickle in later, and we'll
	// rearrange the selector bar when we get those VIEW_ADDED events.)
	RearrangeIcons();
}

void CSelector::DestroyViews()
{
	int viewCount = HT_GetViewListCount(m_Pane);
	for (int i = 0; i < viewCount; i++)
	{
		HT_View v = HT_GetNthView(m_Pane, i);
		if (v != NULL)
		{
			CRDFContentView* theView = (CRDFContentView*)(HT_GetViewFEData(v));
			delete theView;
		}
	}
}

BOOL CSelector::OnEraseBkgnd( CDC* pDC )
{
	int i = 0;
	return CWnd::OnEraseBkgnd(pDC);
}


void CSelector::ScrollSelector()
{
	if (m_scrollDirection ==  SCROLLUP || m_scrollDirection == SCROLLLEFT) {
		if (m_scrollPos  > 0) {
			if (m_scrollDirection ==  SCROLLUP) 
				ScrollWindowEx( 0, SCROLLSTEP, NULL, NULL, NULL, NULL, SW_SCROLLCHILDREN | SW_INVALIDATE | SW_ERASE); 
			else 
				ScrollWindowEx( SCROLLSTEP, 0, NULL, NULL, NULL, NULL, SW_SCROLLCHILDREN | SW_INVALIDATE | SW_ERASE); 
			m_scrollPos -= SCROLLSTEP;
		}
	} 
	else {
		RECT rect;
		GetClientRect(&rect);
		if (m_scrollDirection ==  SCROLLDOWN) {
			if (m_scrollPos + (rect.bottom - rect.top) < m_ypos) {
				ScrollWindowEx( 0, -SCROLLSTEP, NULL, NULL, NULL, NULL, SW_SCROLLCHILDREN | SW_INVALIDATE | SW_ERASE); 
				m_scrollPos += SCROLLSTEP;
			}
			else if (m_scrollPos + (rect.right - rect.left) < m_xpos) {
				ScrollWindowEx( -SCROLLSTEP, 0, NULL, NULL, NULL, NULL, SW_SCROLLCHILDREN | SW_INVALIDATE | SW_ERASE); 
				m_scrollPos += SCROLLSTEP;
			}
		}
	}
	ShowScrollButton(NULL);
	m_scrollUp->AdjustPos();
	m_scrollDown->AdjustPos();
}


void CSelector::OnParentNotify( UINT message, LPARAM lParam )
{
	if (message == WM_LBUTTONDOWN) {
		POINT pt;
		pt.x = LOWORD(lParam);  // horizontal position of cursor 
		pt.y = HIWORD(lParam);  // vertical position of cursor 
		CNSNavFrame* pParent = (CNSNavFrame*)GetParent();
		int32 dockStyle = pParent->GetDockStyle();
		if (m_scrollUp->IsPtInRect(pt)) {
			if (dockStyle == DOCKSTYLE_HORZTOP)
				m_scrollDirection = SCROLLLEFT;
			else
				m_scrollDirection = SCROLLUP;
		}
		else if (m_scrollDown->IsPtInRect(pt)){	 // scroll down.
			if (dockStyle == DOCKSTYLE_HORZBOTTOM)
				m_scrollDirection = SCROLLRIGHT;
			else
				m_scrollDirection = SCROLLDOWN;
		}
		ScrollSelector();
	}
	CWnd::OnParentNotify( message, lParam );
}

void CSelector::RearrangeIcons()
{
	if (m_Pane == NULL)
		return;

	CNSNavFrame* pFrame = (CNSNavFrame*)GetParentFrame();
	int32 dockStyle = pFrame->GetDockStyle();

	CRect tempRect;
	if (dockStyle == DOCKSTYLE_HORZTOP || dockStyle == DOCKSTYLE_HORZBOTTOM) 
	{
		m_xpos = ICONXPOS;
		m_ypos = 1;
	}
	else {
		m_xpos = 1;
		m_ypos = ICONYPOS;
	}
	CRect parentRect;
	CSelectorButton* theButton = NULL;
	GetClientRect(&parentRect);

	int count = HT_GetViewListCount(m_Pane);
	for (int i = 0; i < count; i++)
	{
		HT_View view = HT_GetNthView(m_Pane, i);
		
		if (view && HT_GetViewFEData(view))
		{
			theButton = (CSelectorButton*)HT_GetViewFEData(view);
		
			CSize buttonSize = theButton->GetRequiredButtonSize();

			if (dockStyle == DOCKSTYLE_FLOATING || dockStyle == DOCKSTYLE_VERTLEFT ||
				dockStyle == DOCKSTYLE_VERTRIGHT) 
			{	 // draw icon from top to bottom
				theButton->MoveWindow((int)m_xpos, (int)m_ypos, 
					BUTTON_WIDTH, buttonSize.cy, TRUE);

				m_ypos += buttonSize.cy + 2; 
			}
			else 
			{	// draw icon from left to right.
				theButton->MoveWindow((int)m_xpos, (int)m_ypos, 
					BUTTON_WIDTH, buttonSize.cy, TRUE);
				m_xpos += BUTTON_WIDTH + 2;
			}
		}
	}
}

CSelectorButton* CSelector::GetButtonFromPoint(CPoint point, int& dragFraction)
{
	CRect buttonRect;
	
	CNSNavFrame* pFrame = (CNSNavFrame*)GetParentFrame();
	int32 dockStyle = pFrame->GetDockStyle();

	CSelectorButton* pButton = NULL;
	int count = HT_GetViewListCount(m_Pane);
	for (int i = 0; i < count; i++)
	{
		HT_View view = HT_GetNthView(m_Pane, i);
		
		if (view && HT_GetViewFEData(view))
		{
			pButton = (CSelectorButton*)HT_GetViewFEData(view);
			pButton->GetClientRect(&buttonRect);
			pButton->MapWindowPoints(this, &buttonRect);
        
			if (buttonRect.PtInRect(point))
			{
				int hitY = point.y;
				if (point.y >= buttonRect.top && point.y <= buttonRect.top + buttonRect.Height()/4.0)
					dragFraction = 1;
				else if (point.y >= buttonRect.bottom - buttonRect.Height()/4.0)
					dragFraction = 3;
				else dragFraction = 2;

				return pButton;
			}

			if (dockStyle == DOCKSTYLE_FLOATING || dockStyle == DOCKSTYLE_VERTLEFT ||
				dockStyle == DOCKSTYLE_VERTRIGHT) 
			{
				if (point.y <= buttonRect.bottom)
				{
					dragFraction = 1;
					return pButton;
				}
			}
			else
			{
				if (point.x <= buttonRect.right)
				{
					dragFraction = 1;
					return pButton;
				}
			}
		}
	}

	dragFraction = 3;
	return pButton;
}

void CSelector::OnTimer(UINT nID)
{
	CWnd::OnTimer(nID);
	if (nID == IDT_PANESWITCH) 
	{
		if (m_hPaneSwitchTimer != 0)
		{
			KillSwitchTimer();
		}

		// Do the pane switch
		if (m_pCurButton != GetDragButton())
		{
			// There's actually something that needs to be done.
			if (m_pCurButton) // Something is depressed
				GetDragButton()->OnAction();
			else	// Nothing is depressed. Use popup menus instead.
				GetDragButton()->DisplayMenuOnDrag();	
		}
	}
}


void CSelector::OnLButtonDown (UINT nFlags, CPoint point )
{
	// Called when the user clicks on the menu bar.  Start a drag or collapse the view.
	if (m_pCurButton)
	{
		if (m_pContentView)
			m_pContentView->SetFocus();
	}

	m_PointHit = point;
	SetCapture();
}


void CSelector::OnMouseMove(UINT nFlags, CPoint point)
{
	if (GetCapture() == this)
	{
		CNSNavFrame* navFrameParent = (CNSNavFrame*)GetParentFrame();
		
		if (abs(point.x - m_PointHit.x) > 3 ||
			abs(point.y - m_PointHit.y) > 3)
		{
			ReleaseCapture();

			// Start a drag
			MapWindowPoints(navFrameParent, &point, 1); 
			navFrameParent->StartDrag(point);
		}
	}
}

void CSelector::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (GetCapture() == this) 
	{
		ReleaseCapture();
	}
}

void CSelector::OnRButtonDown(UINT nFlags, CPoint point)
{
	m_MenuCommandMap.Clear();
	HT_Cursor theCursor = HT_NewContextualMenuCursor(NULL, PR_TRUE, PR_TRUE);
	CMenu menu;
	ClientToScreen(&point);
	if (menu.CreatePopupMenu() != 0 && theCursor != NULL)
	{
		// We have a cursor. Attempt to iterate
		HT_MenuCmd theCommand; 
		while (HT_NextContextMenuItem(theCursor, &theCommand))
		{
			char* menuName = HT_GetMenuCmdName(theCommand);
			if (theCommand == HT_CMD_SEPARATOR)
				menu.AppendMenu(MF_SEPARATOR);
			else
			{
				// Add the command to our command map
				CRDFMenuCommand* rdfCommand = new CRDFMenuCommand(menuName, theCommand);
				int index = m_MenuCommandMap.AddCommand(rdfCommand);
				menu.AppendMenu(MF_ENABLED, index+FIRST_HT_MENU_ID, menuName);
			}
		}
		HT_DeleteCursor(theCursor);

		menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this, NULL);	

		menu.DestroyMenu();
	}
}

BOOL CSelector::OnCommand( WPARAM wParam, LPARAM lParam )
{
	if (wParam >= FIRST_HT_MENU_ID && wParam <= LAST_HT_MENU_ID)
	{
		// A selection was made from the context menu.
		// Use the menu map to get the HT command value
		CRDFMenuCommand* theCommand = (CRDFMenuCommand*)(m_MenuCommandMap.GetCommand((int)wParam-FIRST_HT_MENU_ID));
		if (theCommand)
		{
			HT_MenuCmd htCommand = theCommand->GetHTCommand();
			HT_DoMenuCmd(m_Pane, htCommand);
		}
	}
	return TRUE;
}

CSelectorDropTarget::CSelectorDropTarget(CSelector* pParent) 
{
	fd = 0;
	m_pParent = pParent;
}

CSelectorDropTarget::~CSelectorDropTarget()
{
	delete fd;
}


BOOL CSelectorDropTarget::OnDrop(CWnd* pWnd, 
                COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point)
{
	CSelector* pSelector = (CSelector*)pWnd;
	HT_View theView = pSelector->GetDragButton()->GetHTView();
	pSelector->SetDragButton(NULL);
	if (pSelector->GetSwitchTimer() != 0)
		pSelector->KillSwitchTimer();
	if (theView)
	{
		// Do a drop.
		RDFGLOBAL_PerformDrop(pDataObject, HT_TopNode(theView), pSelector->GetDragFraction());
	}
	
	

/*
    if(!pDataObject || !pWnd)
        return(FALSE);

#ifdef XP_WIN32
    // we only handle text at the moment
    if(! (pDataObject->IsDataAvailable(CF_TEXT) || pDataObject->IsDataAvailable(CF_UNICODETEXT)))
        return(FALSE);
#else
    // we only handle text at the moment
    if(!pDataObject->IsDataAvailable(CF_TEXT))
        return(FALSE);
#endif

    CSelector * cView = (CSelector *) pWnd;

	BOOL bUseUnicodeData = FALSE;

	// get the data
	HGLOBAL hString = pDataObject->GetGlobalData(CF_TEXT);
	if(hString == NULL)
		return(FALSE);

	// get a pointer to the actual bytes
	char *  pString = (char *) GlobalLock(hString);    
	if(!pString)
		return(FALSE);

	m_url = pString;

	GlobalUnlock(hString);
    if	(strnicmp(m_url, "mailto:", 7) == 0) {	// drag from mail and new

	//	Generic brain dead interface to GetUrl.
		pSaveContext = new CSaveCX(NULL, "MCF Viewer", pWnd);
			//  Create the stream which will do the actual work.
	#ifndef XP_WIN16
		pSaveContext->Create(CSaveCX::IDD, m_pParent);
	#elif defined(DEBUG_hyatt) || defined(DEBUG_mhwang) || defined(DEBUG_rjc) || defined(DEBUG_guha)
	// TODO -WIND16 Fix the saving of MCF correctly, and stop using static strings
	#endif
		fd = 0;
		NET_StreamClass *pStream = NET_NewStream("Saving MCF file",
			MCFSaveWrite,
			MCFSaveComplete,
			MCFSaveAbort,
			MCFSaveReady,
			this,
			pSaveContext->GetContext());

		pSaveContext->SetSecondaryStream(pStream);
		//	Load it.
		URL_Struct *pUrlStruct = NET_CreateURLStruct(m_url, NET_DONT_RELOAD);
		pSaveContext->GetUrl(pUrlStruct, FO_CACHE_AND_SAVE_AS, TRUE, FALSE);
	}
	else 
	{
		CWnd* pWindowHit =  m_pParent->ChildWindowFromPoint( point );
		if (pWindowHit)
		{
			CSelectorButton* theButton = (CSelectorButton*)pWindowHit;
			CPoint point(-1, -1);
			return theButton->GetView()->OnDrop(pDataObject, dropEffect, point);
		}
		else 
		{ // create a new work space.
			HT_NewWorkspace(m_url.GetBuffer(m_url.GetLength()), m_pParent, m_pParent->GetHTPane());
		}
	}
	*/
    return(TRUE);

}

LRESULT CSelector::OnButtonDrag(WPARAM wParam, LPARAM lParam)
{
	HWND hWnd = (HWND) lParam;

	CWnd *pButton = CWnd::FromHandle(hWnd);

	COleDataSource * pDataSource = new COleDataSource;  
	CSelectorButton* pSelButton = (CSelectorButton*)pButton;
	
    pSelButton->FillInOleDataSource(pDataSource);
	
	// Need to clear the selection, since I use that for dropping stuff.
	HT_SetSelection(HT_TopNode(pSelButton->GetHTView()));

    // Don't start drag until outside this rect 
    RECT rectDragStart;
	pButton->GetClientRect(&rectDragStart);
	pButton->MapWindowPoints(this, &rectDragStart);

	DROPEFFECT effect;
	CToolbarDropSource * pDropSource = new CToolbarDropSource;

    effect=pDataSource->DoDragDrop(DROPEFFECT_COPY | DROPEFFECT_LINK | DROPEFFECT_MOVE | DROPEFFECT_SCROLL | DROPEFFECT_NONE,
                            &rectDragStart, pDropSource);
	
	delete pDropSource;
	delete pDataSource;
	
	return 1;
}

DROPEFFECT CSelectorDropTarget::OnDragEnter(CWnd* pWnd, 
                COleDataObject* pDataObject, DWORD dwKeyState, CPoint point )
{
	return OnDragOver(pWnd, pDataObject, dwKeyState, point);
}

DROPEFFECT CSelectorDropTarget::OnDragOver(CWnd* pWnd, 
                COleDataObject* pDataObject, DWORD dwKeyState, CPoint point )
{
	CSelector* pSelector = (CSelector*)pWnd;
	int dragFraction;

	CSelectorButton* pButton = pSelector->GetButtonFromPoint(point, dragFraction);
	HT_Resource theNode = pButton ? pButton->GetNode() : NULL;
	
	// Give some drag feedback. 
	DROPEFFECT answer = RDFGLOBAL_TranslateDropAction(theNode, pDataObject, dragFraction);

	if (pButton != pSelector->GetDragButton() || dragFraction != pSelector->GetDragFraction()) 
	{
		// User is over a different button or different part of the same button
		if (pSelector->GetSwitchTimer() != 0)
			pSelector->KillSwitchTimer();	// Kill the existing pane switch timer.
		
		pSelector->SetDragButton(pButton);
		pSelector->SetDragFraction(dragFraction);
		
		if (answer != DROPEFFECT_NONE && dragFraction == 2) // Set the pane switching timer if the user can drop here.
			pSelector->SetSwitchTimer();
		else
		{
			// Kill any menus that are up.
			CDropMenu* theLastMenu = CDropMenu::GetLastDropMenu();
			if(theLastMenu !=NULL)
			{
				//we only want one drop menu open at a time, so close the last one if one is
				//still open. 
				theLastMenu->Deactivate();
				CDropMenu::SetLastDropMenu(NULL);
			}
		}
	}

	return answer;
}

void CSelectorDropTarget::OnDragLeave(CWnd* pWnd)
{
	CSelector* pSelector = (CSelector*)pWnd;
	pSelector->SetDragButton(NULL);
	if (pSelector->GetSwitchTimer() != 0)
	{
		pSelector->KillSwitchTimer();
	}
}


IMPLEMENT_DYNAMIC(CNavSelScrollButton, CButton)
BEGIN_MESSAGE_MAP(CNavSelScrollButton, CButton)
	//{{AFX_MSG_MAP(CMainFrame)
//	ON_WM_LBUTTONUP()
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CNavSelScrollButton::CNavSelScrollButton(HBITMAP hBmp, int width, int index, int height) 
{
	m_hbmp = hBmp; 
	m_width = width;
	m_index = index;
	m_height = height;
	m_selected = FALSE;
}

void CNavSelScrollButton::DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct )
{
	if (lpDrawItemStruct->itemAction == ODA_FOCUS  )
		return; 
	if ( m_hbmp ) {
		RECT rect; 
		GetClientRect(&rect);
		int index = m_index; // normal state bitmap.
		HDC hdcBitmap = ::CreateCompatibleDC( lpDrawItemStruct->hDC );

		HBITMAP hbmOld = (HBITMAP) ::SelectObject( hdcBitmap, m_hbmp );

		if ((lpDrawItemStruct->itemAction == ODA_SELECT) && 
			((lpDrawItemStruct->itemState & ODS_SELECTED) == ODS_SELECTED)) {
			if (m_selected) {
				m_selected = FALSE;
			}
			else {
				m_selected = TRUE;
				index = m_index + 1;  // selected state bitmap.
			}
		}
		FEU_TransBlt( lpDrawItemStruct->hDC, 
					  lpDrawItemStruct->rcItem.right - m_width, 
					  lpDrawItemStruct->rcItem.top, 
					  m_width, m_height,
					  hdcBitmap,
					  index * m_width, 0 ,WFE_GetUIPalette(GetParentFrame())
					 );

		::SelectObject( hdcBitmap, hbmOld );
		ShowWindow(SW_SHOW);
		VERIFY( ::DeleteDC( hdcBitmap ));
		CRgn pRgn;
		pRgn.CreateRectRgn( m_boundBox.left, m_boundBox.top, 
							m_boundBox.right, m_boundBox.bottom );
		GetParent()->ValidateRgn( &pRgn );
		pRgn.DeleteObject();
	}
}




unsigned int MCFSaveReady(NET_StreamClass *stream)	
{
	//	Get our save context out of the data object.
	CSelectorDropTarget* pDropTarget = (CSelectorDropTarget*)stream->data_object;	
	if (!pDropTarget->fd) {
		strcpy(pDropTarget->fileName, WH_TempFileName(xpTemporary, "mcf", ".mcf"));
		pDropTarget->fd = new CFile(pDropTarget->fileName, CFile::modeCreate | CFile::modeWrite );
	}
	return (MAX_WRITE_READY);
}

int MCFSaveWrite(NET_StreamClass *stream, const char *pWriteData, int32 iDataLength)	
{
	CSelectorDropTarget* pDropTarget = (CSelectorDropTarget*)stream->data_object;	
	if (!pDropTarget->fd) {
		strcpy(pDropTarget->fileName, WH_TempFileName(xpTemporary, "mcf", ".mcf"));
		pDropTarget->fd = new CFile(pDropTarget->fileName, CFile::modeCreate | CFile::modeWrite );
	}
	pDropTarget->fd->Write( pWriteData, (UINT)iDataLength );

	//	Return the amount written, or error.
	return((UINT)iDataLength);
}

void MCFSaveComplete(NET_StreamClass *stream)	
{
	CSelectorDropTarget* pDropTarget = (CSelectorDropTarget*)stream->data_object;	
	pDropTarget->fd->Close( );
/*
	HT_NewWorkspace(pDropTarget->fileName, pDropTarget->m_pParent, pDropTarget->m_pParent->GetHTPane());
*/
}
void MCFSaveAbort(NET_StreamClass *stream, int iStatus)	
{	
}

