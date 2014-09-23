
#pragma comment(lib, "comctl32.lib")

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>

#include "main.h"
#include "hspwnd.h"
#include "module/supio/supio.h"
//#include "module/hspdll.h"
//#include "module/SortNote.h"
#include "WrapCall/WrapCall.h"

#include "dialog.h"
#include "vartree.h"
#include "config_mng.h"
#include "DebugInfo.h"

#include <fstream>
#include <map>
#include <vector>
#include <algorithm>
#include <array>

namespace Dialog
{
	
static HWND hDlgWnd;

static HWND hTabCtrl;
static HWND hGenList;
static HWND hTabSheet[TABDLGMAX];

static HWND hBtn1;
static HWND hBtn2;
static HWND hBtn3;
static HWND hBtn4;
static HWND hBtn5;
static HWND hSttCtrl;

static HWND hVarPage;
static HWND hVarTree;
static HWND hVarEdit;

static HWND hLogPage;
static HWND hLogEdit;
static HWND hLogChkUpdate;
static HWND hLogChkCalog;

static HWND hSrcPage;
static HWND hSrcEdit;
static HWND hSrcBox;

static HMENU hPopup;
static HMENU hPopupOfVar;

HWND getKnowbugHandle() { return hDlgWnd; }
HWND getSttCtrlHandle() { return hSttCtrl; }
HWND getVarTreeHandle() { return hVarTree; }

//------------------------------------------------
// �E�B���h�E�E�I�u�W�F�N�g�̐���
//------------------------------------------------
static HWND GenerateObj( HWND parent, char const* name, char const* ttl, int x, int y, int sx, int sy, int menu, HFONT font )
{
	HWND const h = CreateWindow(
		name, ttl,
		(WS_CHILD | WS_VISIBLE),
		x, y, sx, sy,
		parent,
		(HMENU)menu,
		Knowbug::getInstance(),
		nullptr
	);

	if ( font ) SendMessage( h, WM_SETFONT, (WPARAM)font, TRUE );
	return h;
}

//------------------------------------------------
// �S�ʃ^�u�̑O����
//------------------------------------------------
static void TabGeneralInit( void )
{
	LVCOLUMN col;
	col.mask     = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	col.fmt      = LVCFMT_LEFT;
	col.cx       = 120;
	col.pszText  = "����";
	col.iSubItem = 0;
	ListView_InsertColumn( hGenList , 0 , &col);
	
	col.cx       = 400;
	col.iSubItem = 1;
	col.pszText  = "���e";
	ListView_InsertColumn( hGenList , 1 , &col);
	return;
}

//------------------------------------------------
// �S�ʃ^�u�̍X�V
//------------------------------------------------
static void SrcSync( char const* filepath, int line_num, bool bUpdateEdit, bool bUpdateBox );
static void TabGeneral_AddItem( char const* sItem, char const* sValue, int iItem );

static void TabGeneralReset()
{
	ListView_DeleteAllItems( hGenList );

	int tgmax = 0;
	int chk;
	char name[256];
	char val[512];

	char* const p = g_dbginfo->debug->get_value( DEBUGINFO_GENERAL );		// HSP���ɖ₢���킹
	strsp_ini();
	for (;;) {
		chk = strsp_get( p, name, 0, sizeof(name) - 1 );
		if ( chk == 0 ) break;
		
		chk = strsp_get( p, val, 0, sizeof(val) - 1 );
		if ( chk == 0 ) break;
		
		TabGeneral_AddItem( name, val, tgmax ); ++tgmax;
	}
	
	// �g�����e�̒ǉ�
	if ( exinfo->actscr ) {
		auto const pBmscr = reinterpret_cast<BMSCR*>(exinfo->HspFunc_getbmscr(*exinfo->actscr));
		if ( pBmscr ) {
			// color
			{
				COLORREF const cref = pBmscr->color;
				sprintf_s(val, "(%d, %d, %d)",
					GetRValue(cref), GetGValue(cref), GetBValue(cref)
				);
			}
			TabGeneral_AddItem("color", val, tgmax); ++tgmax;

			// pos
			sprintf_s(val, "(%d, %d)", pBmscr->cx, pBmscr->cy);
			TabGeneral_AddItem("pos", val, tgmax); ++tgmax;
		}
	}
	
	g_dbginfo->debug->dbg_close( p );

	SrcSync( g_dbginfo->debug->fname, g_dbginfo->debug->line, false, true );
	return;
}

static void TabGeneral_AddItem( char const* sItem, char const* sValue, int iItem )
{
	LV_ITEM item;
	
	item.mask     = LVIF_TEXT;
	item.iItem    = iItem;
	
	item.iSubItem = 0;
	item.pszText  = const_cast<char*>(sItem);
	ListView_InsertItem( hGenList, &item );
	
	item.iSubItem = 1;
	item.pszText  = const_cast<char*>(sValue);
	ListView_SetItem( hGenList, &item );
	return;
}

//------------------------------------------------
// ���s���̈ʒu�\�����X�V���� (line, file)
//------------------------------------------------
static void CurrentUpdate()
{
	SetWindowText( hSttCtrl, g_dbginfo->getCurInfString().c_str() );
	
#ifdef with_khad
	if ( hKhad != nullptr ) {
		SendMessage( hKhad, UWM_KHAD_CURPOS, g_dbginfo->debug->line, (LPARAM)g_dbginfo->debug->fname );
	}
#endif
	return;
}

//------------------------------------------------
// �ϐ��^�u�̏�����
//------------------------------------------------
void TabVarInit( HWND hDlg )
{
	hVarPage = hDlg;
	hVarTree = GetDlgItem( hDlg, IDC_VARTREE );
	hVarEdit = GetDlgItem( hDlg, IDC_VARINFO );;
	setEditStyle( hVarEdit );
	
	VarTree::init();
	
	// �|�b�v�A�b�v���j���[�̒ǉ�
	hPopupOfVar = CreatePopupMenu();
		AppendMenu( hPopupOfVar, MF_STRING, IDM_VAR_LOGGING, "���O(&L)");	// ���͕\�����ɏ㏑�������
		AppendMenu( hPopupOfVar, MF_STRING, IDM_VAR_UPDATE,  "�X�V(&U)" );
		AppendMenu( hPopupOfVar, MF_SEPARATOR, 0, 0 );
		AppendMenu( hPopupOfVar, MF_STRING, IDM_VAR_STEPOUT, "�E�o(&O)" );
	return;
}

//------------------------------------------------
// �ϐ��^�u::�ϐ����̍X�V
//------------------------------------------------
void TabVarsUpdate()
{
	HTREEITEM const hItem = TreeView_GetSelection( hVarTree );
	if ( !hItem ) return;
	
	string const varinfoText = VarTree::getItemVarText(hItem);
	if ( !varinfoText.empty() ) {
		Edit_UpdateText(hVarEdit, varinfoText.c_str());
	}
	return;
}

//------------------------------------------------
// ���O�̃`�F�b�N�{�b�N�X
//------------------------------------------------
bool isLogAutomaticallyUpdated()
{
	return IsDlgButtonChecked( hLogPage, IDC_CHK_LOG_UPDATE ) != FALSE;
}

bool isLogCallings()
{
	return IsDlgButtonChecked( hLogChkCalog, IDC_CHK_LOG_CALOG ) != FALSE;
}

//------------------------------------------------
// ���O���b�Z�[�W������������
//------------------------------------------------
static string stt_logmsg;

void logClear()
{
	stt_logmsg.clear();
	
	Edit_SetSel( hLogEdit, 0, -1 );
	Edit_ReplaceSel( hLogEdit, "" );		// ����ۂɂ���
	return;
}

//------------------------------------------------
// ���O���b�Z�[�W��ǉ��E�X�V����
//------------------------------------------------
void logUpdate( char const* textAdd )
{
	int caret[2];
	SendMessage( hLogEdit, EM_GETSEL,
		(WPARAM)( &caret[0] ),
		(LPARAM)( &caret[1] )
	);
	
	int const size = Edit_GetTextLength( hLogEdit );
	Edit_SetSel( hLogEdit, size, size );		// �Ō���ɃL�����b�g��u��
	Edit_ReplaceSel( hLogEdit, textAdd );		// �������ǉ�����
	Edit_ScrollCaret( hLogEdit );				// ��ʂ�K�v�Ȃ����X�N���[��
	
	// �I����Ԃ����ɖ߂�
	Edit_SetSel( hLogEdit, caret[0], caret[1] );
	return;
}

//------------------------------------------------
// ���O���b�Z�[�W���X�V���� (commit)
//------------------------------------------------
void TabLogCommit()
{
	if ( stt_logmsg.empty() ) return;
	
	logUpdate( stt_logmsg.c_str() );
	stt_logmsg.clear();
	return;
}

//------------------------------------------------
// ���O���b�Z�[�W�ɒǉ�����
//------------------------------------------------
void logAdd( char const* str )
{
	// �����X�V
	if ( isLogAutomaticallyUpdated() ) {
//	if ( IsDlgButtonChecked( hLogPage, IDC_CHK_LOG_UPDATE ) ) {
		logUpdate( str );
		
	} else {
		stt_logmsg.append( str );
	}
	return;
}

void logAddCrlf()
{
	logAdd( "\r\n" );
}

// ���݈ʒu���X�V���āA���݈ʒu�����O�ɒǉ�����B
void logAddCurInf()
{
	g_dbginfo->debug->dbg_curinf();
	logAdd(("CurInf:" + g_dbginfo->getCurInfString()).c_str());
	logAddCrlf();
}

//------------------------------------------------
// ���O���b�Z�[�W��ۑ�����
//------------------------------------------------
void logSave()
{
	char filename[MAX_PATH + 1] = "";
	char fullname[MAX_PATH + 1] = "hspdbg.log";
	OPENFILENAME ofn = { 0 };
		ofn.lStructSize    = sizeof(ofn);			// �\���̂̃T�C�Y
		ofn.hwndOwner      = hDlgWnd;				// �R�����_�C�A���O�̐e�E�B���h�E�n���h��
		ofn.lpstrFilter    = "log text(*.txt;*.log)\0*.txt;*.log\0All files(*.*)\0*.*\0\0";	// �t�@�C���̎��
		ofn.lpstrFile      = fullname;				// �I�����ꂽ�t�@�C����(�t���p�X)���󂯎��ϐ��̃A�h���X
		ofn.lpstrFileTitle = filename;				// �I�����ꂽ�t�@�C�������󂯎��ϐ��̃A�h���X
		ofn.nMaxFile       = sizeof(fullname);		// lpstrFile�Ɏw�肵���ϐ��̃T�C�Y
		ofn.nMaxFileTitle  = sizeof(filename);		// lpstrFileTitle�Ɏw�肵���ϐ��̃T�C�Y
		ofn.Flags          = OFN_OVERWRITEPROMPT;	// �t���O�w��
		ofn.lpstrTitle     = "���O��t���ĕۑ�";	// �R�����_�C�A���O�̃L���v�V����
		ofn.lpstrDefExt    = "log";					// �f�t�H���g�̃t�@�C���̎��

	if ( GetSaveFileName(&ofn) ) {
		logSave(fullname);
	}
	return;
}

void logSave( char const* filepath )
{
	// ���O���b�Z�[�W�����o��
	int const size = Edit_GetTextLength( hLogEdit );
	char* const buf = new char[size + 2];
	GetWindowText( hLogEdit, buf, size + 1 );
	
	// �ۑ�
	HANDLE const hFile =
		CreateFile( filepath, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr );
	if ( hFile != INVALID_HANDLE_VALUE ) {
		DWORD writesize;
		WriteFile( hFile, buf, size, &writesize, nullptr );
		CloseHandle( hFile );
	}

	delete buf;
	return;
}

//------------------------------------------------
// �\�[�X�t�@�C�����J��
//
// @ �G�f�B�^��ŕҏW���̏ꍇ�A�t�@�C���̓��e�����ۂƈقȂ邱�Ƃ�����B�s�ԍ��̃A�E�g�����W�ɒ��ӁB
//------------------------------------------------
/*
// �s���Ƃɕ������ꂽ�e�L�X�g
first: �X�N���v�g�S�́A�������e�s�̎������󔒂͏�������Ă���B
second: �s�ԍ�(0�x�[�X)�̓Y���ɑ΂��āAfirst �ɂ����邻�̍s�̐擪�ւ̃I�t�Z�b�g�l�B
	�ŏ��̗v�f�� 0�A�Ō�̗v�f�� first �̒����B
*/
using script_t = std::pair<string const, std::vector<size_t>>;

// ���݃\�[�X�^�u�ŕ\������Ă���t�@�C���̃p�X
static string stt_viewingFilepath;

// �ǂݍ��ݏ��� (failure: nullptr)
static script_t const* ReadFromSourceFile( char const* _filepath )
{
	static std::map<string const, script_t> stt_src_cache;
	
	string const filepath = _filepath;
	
	// �L���b�V�����猟��
	{
		auto const iter = stt_src_cache.find(filepath);
		if ( iter != stt_src_cache.end() ) return &iter->second;
	}

	// �t�@�C������ǂݍ���
	string code;
	std::vector<size_t> idxlist;
	{
		std::ifstream ifs { filepath };
		if ( !ifs.is_open() ) {			// ������Ȃ����� => common ���������
			char path[MAX_PATH];
			if ( SearchPath( g_config->commonPath.c_str(), _filepath, nullptr, sizeof(path), path, nullptr ) == 0 ) {
				return nullptr;
			}
			
			ifs.open( path );
			if ( !ifs.is_open() ) return nullptr;
		}
		
		char linebuf[0x400];
		size_t idx = 0;
		idxlist.push_back( 0 );
		do {
			ifs.getline( linebuf, sizeof(linebuf) );
			int cntIndents = 0; {
				for( int& i = cntIndents; linebuf[i] == '\t' || linebuf[i] == ' '; ++ i );
			}
			char const* const p = &linebuf[cntIndents];
			size_t const len = std::strlen(p);
			code.append( p, p + len ).append("\r\n");
			idx += len + 2;
			idxlist.push_back( idx );
		} while ( ifs.good() );
	}

	auto const res =
		stt_src_cache.insert({ std::move(filepath), { std::move(code), std::move(idxlist) } });
	return &res.first->second;
}

// �\�[�X�^�u�𓯊�����
static void SrcSyncImpl( HWND hEdit, char const* p )
{
	//Edit_SetSel( hEdit, 0, -1 );	// �S�̂�u��������
	//Edit_ReplaceSel( hEdit, p );
	Edit_UpdateText(hEdit, p);
	return;
}

static void SrcSync( char const* filepath, int line_num, bool bUpdateEdit, bool bUpdateBox )
{
	if ( !filepath || line_num < 0 ) return;

	if ( auto const p = ReadFromSourceFile( filepath ) ) {
		assert(line_num >= 1);	// �s�ԍ� line_num �� 1-based
		size_t const iLine = static_cast<size_t>(line_num) - 1;

		size_t idxlist[2];
		if ( iLine + 1 < p->second.size() ) {
			idxlist[0] = p->second[iLine]; idxlist[1] = p->second[iLine + 1];
		} else {
			idxlist[0] = idxlist[1] = 0;
		}
		if ( bUpdateEdit ) {
			if ( stt_viewingFilepath.empty() || stt_viewingFilepath != filepath ) {
				SrcSyncImpl(hSrcEdit, p->first.c_str());
				stt_viewingFilepath = filepath;
			}
			Edit_SetSel( hSrcEdit, idxlist[0], idxlist[1] );	// �Y���s��I��
			Edit_Scroll( hSrcEdit, iLine, 0 );
		}
		if ( bUpdateBox ) {
			auto const text = p->first.substr( idxlist[0], idxlist[1] - idxlist[0] );
			SrcSyncImpl( hSrcBox, text.c_str() );
		}
	} else {
		auto const text = strf("(#%d \"%s\")", line_num, filepath);
		if ( bUpdateEdit ) SrcSyncImpl( hSrcEdit, text.c_str() );
		if ( bUpdateBox  ) SrcSyncImpl( hSrcBox,  text.c_str() );
	}
	return;
}

//------------------------------------------------
// �\�[�X�^�u�̍X�V
//------------------------------------------------
static void TabSrcUpdate()
{
	SrcSync( g_dbginfo->debug->fname, g_dbginfo->debug->line, true, false );
}

//------------------------------------------------
// �S�ʃ^�u::�v���V�[�W��
//------------------------------------------------
LRESULT CALLBACK TabGeneralProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp)
{
	switch ( msg ) {
		
		case WM_INITDIALOG:
			hGenList = GetDlgItem( hDlg, IDC_LV_GENERAL );
			hSrcBox  = GetDlgItem( hDlg, IDC_SRC_BOX );
			TabGeneralInit();
			TabGeneralReset();
			return TRUE;
			
		case WM_COMMAND:
			switch ( LOWORD(wp) ) {
				case IDC_BTN_UPDATE:
					TabGeneralReset();
					break;
			}
			return FALSE;
	}
	
	return FALSE;
}

//------------------------------------------------
// �ϐ��^�u::�v���V�[�W��
//------------------------------------------------
LRESULT CALLBACK TabVarsProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp)
{
	switch ( msg ) {
		case WM_INITDIALOG:
			TabVarInit( hDlg );
			return TRUE;
			
		case WM_CONTEXTMENU:
		{
			// �c���[��ŋt�N���b�N
			if ( wp == (WPARAM)hVarTree ) {
				TV_HITTESTINFO tvHitTestInfo;
					tvHitTestInfo.pt.x = LOWORD(lp);
					tvHitTestInfo.pt.y = HIWORD(lp);
				ScreenToClient( hVarTree, &tvHitTestInfo.pt );
				auto const hItem = TreeView_HitTest( hVarTree, &tvHitTestInfo );	// �Ώۂ��m��
				if ( !hItem )  break;
				
				if ( tvHitTestInfo.flags & TVHT_ONITEMLABEL ) {		// ������A�C�e���̏ꍇ
					auto const varname = TreeView_GetItemString(hVarTree, hItem);
					{
						auto const menuText = strf( "�u%s�v�����O(&L)", varname.c_str() );
						MENUITEMINFO menuInfo;
							menuInfo.cbSize = sizeof(menuInfo);
							menuInfo.fMask  = MIIM_STRING;
							menuInfo.dwTypeData = const_cast<LPSTR>( menuText.c_str() );
						SetMenuItemInfo( hPopupOfVar, IDM_VAR_LOGGING, FALSE, &menuInfo );
					}
					
					// �u�E�o�v�͌Ăяo���m�[�h�ɑ΂��Ă̂ݗL��
					EnableMenuItem( hPopupOfVar, IDM_VAR_STEPOUT,
						(VarTree::isCallNode(varname.c_str()) ? MFS_ENABLED : MFS_DISABLED) );
					
					// �|�b�v�A�b�v���j���[��\������
					int const idSelected = TrackPopupMenuEx(
						hPopupOfVar, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_NONOTIFY | TPM_RETURNCMD,
						(int)LOWORD(lp), (int)HIWORD(lp), hDlgWnd, nullptr
					);
					
					switch ( idSelected ) {
						case IDM_VAR_LOGGING:
						{
							string const varinfoText = VarTree::getItemVarText(hItem);
							Knowbug::logmes( varinfoText.c_str() );		// logmes ���M
							return TRUE;
						}
						case IDM_VAR_UPDATE:
							TabVarsUpdate();
							break;
						case IDM_VAR_STEPOUT:		// �Ăяo���m�[�h�Ɖ��肵�Ă悢
						{
							auto const idx = static_cast<int>( TreeView_GetItemLParam(hVarTree, hItem) );
							assert(idx >= 0);
							if ( auto const pCallInfo = WrapCall::getCallInfoAt(idx) ) {
								// �Ώۂ��Ăяo���ꂽ�K�w�܂Ői��
								Knowbug::runStepReturn(pCallInfo->sublev);
							}
							break;
						}
						default: break;
					}
					return TRUE;
				}
			}
			break;
		}
		case WM_NOTIFY:
		{
			auto const nmhdr = reinterpret_cast<LPNMHDR>(lp);
			
			if ( nmhdr->hwndFrom == hVarTree ) {
				switch ( nmhdr->code ) {
					
					// �I�����ڂ��ω�����
					case TVN_SELCHANGED:
					case NM_DBLCLK:
					case NM_RETURN:
						TabVarsUpdate();
						break;
						
					// �J�X�^���h���[
					case NM_CUSTOMDRAW:
					{
						if ( !g_config->bCustomDraw ) break;
						
						LRESULT const res = VarTree::customDraw(reinterpret_cast<LPNMTVCUSTOMDRAW>(nmhdr));
						SetWindowLong( hDlg, DWL_MSGRESULT, res );
						return TRUE;
					}
				}
			}
			break;
		}
	}
	return FALSE;
}

//------------------------------------------------
// ���O�^�u::�v���V�[�W��
//------------------------------------------------
LRESULT CALLBACK TabLogProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp)
{
	switch ( msg ) {
		case WM_INITDIALOG:
			hLogPage = hDlg;
			hLogEdit = GetDlgItem( hDlg, IDC_LOG );
			hLogChkUpdate = GetDlgItem( hDlg, IDC_CHK_LOG_UPDATE );
			hLogChkCalog  = GetDlgItem( hDlg, IDC_CHK_LOG_CALOG );
			
			CheckDlgButton( hLogPage, IDC_CHK_LOG_UPDATE, BST_CHECKED );
#ifdef with_WrapCall
		//	CheckDlgButton( hLogPage, IDC_CHK_LOG_CALOG,  BST_CHECKED );
#else
			EnableWindow( hLogChkCalog, false );
#endif
			
			setEditStyle( hLogEdit );
			SendMessage( hLogEdit, EM_SETLIMITTEXT, (WPARAM) g_config->logMaxlen, 0 );

			stt_logmsg.reserve( 0x400 + 1 );
			return TRUE;
			
		case WM_COMMAND:
			switch ( LOWORD(wp) ) {
				
				case IDC_CHK_LOG_UPDATE:
					// �`�F�b�N���t����ꂽ�Ƃ�
					if ( IsDlgButtonChecked( hLogPage, IDC_CHK_LOG_UPDATE ) ) goto LUpdate;
					break;
					
				case IDC_BTN_LOG_UPDATE:
				LUpdate:
					TabLogCommit();
					break;
					
				case IDC_BTN_LOG_SAVE:
					logSave();
					break;
					
				case IDC_BTN_LOG_CLEAR:
					logClear();
					break;
			}
			return FALSE;
	}
	return FALSE;
}

//------------------------------------------------
// �\�[�X�^�u::�v���V�[�W��
//------------------------------------------------
LRESULT CALLBACK TabSrcProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp)
{
	switch ( msg ) {
		case WM_INITDIALOG:
			hSrcPage = hDlg;
			hSrcEdit = GetDlgItem( hDlg, IDC_SRC );
			
			setEditStyle( hSrcEdit );
			SendMessage( hSrcEdit, EM_SETLIMITTEXT, (WPARAM) g_config->logMaxlen, 0 );	// ���O�̍ő咷�𗬗p
			return TRUE;
			
		case WM_COMMAND:
			switch ( LOWORD(wp) ) {
				case IDC_BTN_SRC_UPDATE:
					TabSrcUpdate();
					break;
			}
	}
	return FALSE;
}

//------------------------------------------------
// �e�_�C�A���O�̃R�[���o�b�N�֐�
//------------------------------------------------
LRESULT CALLBACK DlgProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp)
{
	switch ( msg ) {

	// ������
	case WM_CREATE:
	{
		// �|�b�v�A�b�v���j���[�𐶐�
		hPopup = CreatePopupMenu();
			AppendMenu( hPopup, (g_config->bTopMost ? MFS_CHECKED : MFS_UNCHECKED), IDM_TOPMOST, "��ɍőO�ʂɕ\������(&T)" );
		
		// �_�C�A���O�I�u�W�F�N�g�𐶐�
		auto const hFont = (HFONT)GetStockObject( DEFAULT_GUI_FONT );
		hTabCtrl = GenerateObj( hDlg, WC_TABCONTROL, "", DIALOG_X0, DIALOG_Y0, DIALOG_X1, DIALOG_Y2, IDU_TAB, hFont );
		hSttCtrl = GenerateObj( hDlg, "static",    "",  DIALOG_X0 + 210, DIALOG_Y1 + 12, DIALOG_X1 - 210, 48, 0, hFont );
		hBtn1    = GenerateObj( hDlg, "button", "���s", DIALOG_X0 +   8, DIALOG_Y1 + 12, 40, 24, ID_BTN1, hFont );
		hBtn2    = GenerateObj( hDlg, "button", "���s", DIALOG_X0 +  48, DIALOG_Y1 + 12, 40, 24, ID_BTN2, hFont );	
		hBtn3    = GenerateObj( hDlg, "button", "��~", DIALOG_X0 +  88, DIALOG_Y1 + 12, 40, 24, ID_BTN3, hFont );
		hBtn4    = GenerateObj( hDlg, "button", "����", DIALOG_X0 + 128, DIALOG_Y1 + 12, 40, 24, ID_BTN4, hFont );		// �ǉ�
		hBtn5    = GenerateObj( hDlg, "button", "�E�o", DIALOG_X0 + 168, DIALOG_Y1 + 12, 40, 24, ID_BTN5, hFont );		// �ǉ�
		
		// �S�ʃ^�u�A�ϐ��^�u�A���O�^�u�A�X�N���v�g�^�u��ǉ�
		{
			struct tabinfo {
				char const* const label;
				char const* const name;
				DLGPROC proc;
			};
			static std::array<tabinfo, TABDLGMAX> stc_tabinfo = { {
				{ "�S��", "T_GENERAL", (DLGPROC)TabGeneralProc },
				{ "�ϐ�", "T_VAR", (DLGPROC)TabVarsProc },
				{ "���O", "T_LOG", (DLGPROC)TabLogProc },
				{ "�X�N���v�g", "T_SRC", (DLGPROC)TabSrcProc }
			} };

			TCITEM tc;
			tc.mask = TCIF_TEXT;
			for ( int i = 0; i < TABDLGMAX; ++i ) {
				tc.pszText = const_cast<char*>( stc_tabinfo[i].label );
				TabCtrl_InsertItem(hTabCtrl, i, &tc);
				hTabSheet[i] = CreateDialog(Knowbug::getInstance(), stc_tabinfo[i].name, hDlg, stc_tabinfo[i].proc);
			}
		}

		RECT rt;
		//auto pt = (LPPOINT) &rt;
		//GetClientRect(hTabCtrl, &rt);
		SetRect( &rt, 8, DIALOG_Y2 + 4, DIALOG_X1 + 8, DIALOG_Y1 + 4 );
		//TabCtrl_AdjustRect(hTabCtrl, FALSE, &rt);
		//MapWindowPoints(hTabCtrl, hDlg, pt, 2);

		// ���������q�_�C�A���O���^�u�V�[�g�̏�ɓ\��t����
		for ( int i = 0; i < TABDLGMAX; ++ i ) {
			MoveWindow(
				hTabSheet[i],
				rt.left, rt.top,
				(rt.right  - rt.left), (rt.bottom - rt.top),
				FALSE
			);
		}

		// �f�t�H���g�ō��[�̃^�u��\��
		ShowWindow( hTabSheet[0], SW_SHOW );
		return TRUE;
	}
	case WM_NOTIFY:
	{
		auto const nm = reinterpret_cast<NMHDR*>(lp);		// �^�u�R���g���[���̃V�[�g�؂�ւ��ʒm
		int const cur = TabCtrl_GetCurSel(hTabCtrl);
		for ( int i = 0; i < TABDLGMAX; ++ i ) {
			ShowWindow( hTabSheet[i], (i == cur ? SW_SHOW : SW_HIDE) );
		}
		break;
	}
	case WM_COMMAND:
		switch ( LOWORD(wp) ) {
			case ID_BTN1: Knowbug::run();         break;
			case ID_BTN2: Knowbug::runStepIn();   break;
			case ID_BTN3: Knowbug::runStop();     break;
			case ID_BTN4: Knowbug::runStepOver(); break;
			case ID_BTN5: Knowbug::runStepOut();  break;
		}
		if ( LOWORD(wp) != ID_BTN3 ) SetWindowText( Dialog::hSttCtrl, "" );
		break;
		
	case WM_CONTEXTMENU:		// �|�b�v�A�b�v���j���[�\��
	{
		POINT pt;
		GetCursorPos( &pt );	// �J�[�\���ʒu (�X�N���[�����W)
		
		// �|�b�v�A�b�v���j���[��\������
		int const idSelected = TrackPopupMenuEx(
			hPopup, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_NONOTIFY | TPM_RETURNCMD, pt.x, pt.y, hDlg, nullptr
		);
		switch ( idSelected ) {
			case IDM_TOPMOST:
			{
				g_config->bTopMost = !g_config->bTopMost;		// ���]
				
				MENUITEMINFO menuInfo;
					menuInfo.cbSize = sizeof(menuInfo);
					menuInfo.fMask  = MIIM_STATE;
					menuInfo.fState = ( g_config->bTopMost ? MFS_CHECKED : MFS_UNCHECKED );
				SetMenuItemInfo( hPopup, IDM_TOPMOST, FALSE, &menuInfo );
					
				SetWindowPos(	// �őO��
					hDlgWnd, (g_config->bTopMost ? HWND_TOPMOST : HWND_NOTOPMOST),
					0, 0, 0, 0, (SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE)
				);
				break;
			}
			default: break;
		}
		return TRUE;
	}
	
	case WM_CLOSE:
		return FALSE;
		
	case WM_DESTROY:
		DestroyMenu( hPopup );      hPopup      = nullptr;
		DestroyMenu( hPopupOfVar ); hPopupOfVar = nullptr;
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hDlg, msg, wp, lp) ;
}

//------------------------------------------------
// ���C���_�C�A���O�𐶐�����
//------------------------------------------------
HWND Dialog::createMain()
{
	int const dispx = GetSystemMetrics( SM_CXSCREEN );
	int const dispy = GetSystemMetrics( SM_CYSCREEN );
	
	WNDCLASS wndclass;
	wndclass.style         = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc   = DlgProc;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = 0;
	wndclass.hInstance     = Knowbug::getInstance();
	wndclass.hIcon         = nullptr;
	wndclass.hCursor       = LoadCursor(nullptr, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wndclass.lpszMenuName  = nullptr;
	wndclass.lpszClassName = myClass;
	RegisterClass( &wndclass );
	
	hDlgWnd = CreateWindow(
		myClass,
		"Debug Window",
		(WS_CAPTION | WS_OVERLAPPED | WS_BORDER | WS_VISIBLE),
		dispx - WND_SX, 0,
		WND_SX, WND_SY,
		nullptr,
		nullptr,
		Knowbug::getInstance(),
		nullptr
	);
	ShowWindow( hDlgWnd, SW_SHOW );
	UpdateWindow( hDlgWnd );
	
	// hDlgWnd = CreateDialog( myinst, "HSP3DEBUG", nullptr, (DLGPROC)DlgProc );
	if ( !hDlgWnd ) {
		MessageBox( nullptr, "Debug window initalizing failed.", "Error", 0 );
	}
	
	SetWindowPos(	// �őO��
		hDlgWnd, (g_config->bTopMost ? HWND_TOPMOST : HWND_NOTOPMOST),
		0, 0, 0, 0, (SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE)
	);
	
	ShowWindow( hDlgWnd, SW_SHOW );
    UpdateWindow( hDlgWnd );
	
	return hDlgWnd;
}

void Dialog::destroyMain()
{
	if ( hDlgWnd != nullptr ) {
		DestroyWindow( hDlgWnd );
		hDlgWnd = nullptr;
	}
	return;
}

//------------------------------------------------
// �X�V
// 
// @ dbgnotice (stop) ����Ă΂��B
//------------------------------------------------
void update()
{
	CurrentUpdate();

	int const idxTab = TabCtrl_GetCurSel( hTabCtrl );
	switch( idxTab ) {
		case 0:
			TabGeneralReset();
			break;
		case 1:
			TabVarsUpdate();
			break;
		case 3:
			TabSrcUpdate();
			break;
	}
}

//------------------------------------------------
// �G�f�B�b�g�R���g���[���̕W���X�^�C��
// 
// @ �ݒ�Ɉˑ�
//------------------------------------------------
void setEditStyle( HWND hEdit )
{
	Edit_SetTabLength( hEdit, g_config->tabwidth );
	//Edit_EnableWordwrap( hEdit, g_config->bWordwrap );
	return;
}

} // namespace Dialog

//##############################################################################
//                ���JAPI
//##############################################################################
EXPORT HWND WINAPI knowbug_hwnd()
{
	return Dialog::getKnowbugHandle();
}

//##############################################################################
//                ����������
//##############################################################################
//------------------------------------------------
// EditControl �̃^�u��������ύX����
//------------------------------------------------
void Edit_SetTabLength( HWND hEdit, const int tabwidth )
{
	HDC const hdc = GetDC(hEdit);
	{
		TEXTMETRIC tm;
		if ( GetTextMetrics(hdc, &tm) ) {
			int const tabstops = tm.tmAveCharWidth / 4 * tabwidth * 2;
			SendMessage( hEdit, EM_SETTABSTOPS, 1, (LPARAM)(&tabstops) );
		}
	}
	ReleaseDC( hEdit, hdc );
	return;
}

//------------------------------------------------
// EditControl �́u�E�[�Ő܂�Ԃ��v���ۂ�
//
// �X�^�C�� ES_AUTOHSCROLL �͓��I�ɕύX�ł��Ȃ��炵���B
//------------------------------------------------
/*
void Edit_EnableWordwrap( HWND hEdit, bool bEnable )
{
	static LONG const Style_HorzScroll = WS_HSCROLL | ES_AUTOHSCROLL;
	LONG const style = GetWindowLongPtr(hEdit, GWL_STYLE);
	
	SetWindowLongPtr( hEdit, GWL_STYLE,
		g_config->bWordwrap ? (style &~ Style_HorzScroll) : (style | Style_HorzScroll)
	);
	SetWindowPos( hEdit, nullptr, 0, 0, 0, 0, (SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED) );
	return;
}
//*/

//------------------------------------------------
// EditControl �̕�����̒u������
//------------------------------------------------
void Edit_UpdateText(HWND hwnd, char const* s)
{
	int const vscrollBak = Edit_GetFirstVisibleLine(hwnd);
	SetWindowText(hwnd, s);
	Edit_Scroll(hwnd, vscrollBak, 0);
	return;
}

//------------------------------------------------
// �c���[�r���[�̍��ڃ��x�����擾����
//------------------------------------------------
string TreeView_GetItemString( HWND hwndTree, HTREEITEM hItem )
{
	char stmp[256];
	
	TVITEM ti;
	ti.hItem      = hItem;
	ti.mask       = TVIF_TEXT;
	ti.pszText    = stmp;
	ti.cchTextMax = sizeof(stmp) - 1;
	
	return ( TreeView_GetItem( hwndTree, &ti ) )
		? stmp
		: "";
}

//------------------------------------------------
// �c���[�r���[�̃m�[�h�Ɋ֘A���� lparam �l���擾����
//------------------------------------------------
LPARAM TreeView_GetItemLParam( HWND hwndTree, HTREEITEM hItem )
{
	TVITEM ti;
	ti.hItem = hItem;
	ti.mask  = TVIF_PARAM;
	
	TreeView_GetItem( hwndTree, &ti );
	return ti.lParam;
}

//------------------------------------------------
// �c���[�r���[�̃t�H�[�J�X���������
// 
// @ �Ώۂ̃m�[�h���I����ԂȂ�A���̌Z�m�[�h���e�m�[�h��I������B
//------------------------------------------------
void TreeView_EscapeFocus( HWND hwndTree, HTREEITEM hItem )
{
	if ( TreeView_GetSelection(hwndTree) == hItem ) {
		HTREEITEM hUpper = TreeView_GetPrevSibling( hwndTree, hItem );
		if ( !hUpper ) hUpper = TreeView_GetParent(hwndTree, hItem);
		
		TreeView_SelectItem( hwndTree, hUpper );
	}
	return;
}

//------------------------------------------------
// ���q�m�[�h���擾���� (failure: nullptr)
//------------------------------------------------
HTREEITEM TreeView_GetChildLast(HWND hwndTree, HTREEITEM hItem)
{
	HTREEITEM hLast = TreeView_GetChild(hwndTree, hItem);
	if ( !hLast ) return nullptr;	// error

	for ( HTREEITEM hNext = hLast
		; hNext != nullptr
		; hNext = TreeView_GetNextSibling(hwndTree, hLast)
		) {
		hLast = hNext;
	}
	return hLast;
}
