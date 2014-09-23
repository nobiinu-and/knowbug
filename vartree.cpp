
#include <Windows.h>
#include <CommCtrl.h>

#include "main.h"
#include "dialog.h"
#include "config_mng.h"

#include "vartree.h"
#include "CVarinfoText.h"
#include "CVardataString.h"

#include "SysvarData.h"
#include "DebugInfo.h"

extern CVarTree* getSttVarTree();	// at main.cpp

#define hwndVarTree (Dialog::getVarTreeHandle())

namespace VarTree
{

static vartype_t getVartypeOfNode(HTREEITEM hItem);
static void AddNode(HTREEITEM hParent, CVarTree const& tree);
static void AddNodeSysvar(HTREEITEM hParent);

#ifdef with_WrapCall
HTREEITEM g_hNodeDynamic;
static void AddNodeDynamic(HTREEITEM hParent);

// �c���[�r���[�Ɋ܂܂��Ԓl�m�[�h�̃f�[�^
using resultDataPtr_t = std::shared_ptr<ResultNodeData>;
static std::map<HTREEITEM, resultDataPtr_t> g_allResultData;
static HTREEITEM g_lastIndependedResultNode;	// ��ˑ��ȕԒl�m�[�h

// ���I�m�[�h�̒ǉ��E�����̒x���̊Ǘ�
static size_t g_cntWillAddCallNodes = 0;						// ���̍X�V�Œǉ����ׂ��m�[�h��
static std::vector<resultDataPtr_t> g_willAddResultNodes;		// ���̍X�V�Œǉ����ׂ��Ԓl�m�[�h
static resultDataPtr_t g_willAddResultNodeIndepend = nullptr;	// ���̍X�V�Œǉ����ׂ���ˑ��ȕԒl�m�[�h

static void AddCallNodeImpl(ModcmdCallInfo const& callinfo);
static void AddResultNodeImpl(std::shared_ptr<ResultNodeData> pResult);
static HTREEITEM FindDependedCallNode(ResultNodeData* pResult);
static void RemoveDependingResultNodes(HTREEITEM hItem);
static void RemoveLastIndependedResultNode();
static ResultNodeData* FindLastIndependedResultData();
#endif

//------------------------------------------------
// �ϐ��c���[�̏�����
//------------------------------------------------
void init()
{
//	TreeView_DeleteAllItems( hwndVarTree );
	
	AddNode(TVI_ROOT, *getSttVarTree());
#ifdef with_WrapCall
	AddNodeDynamic(TVI_ROOT);
#endif
	AddNodeSysvar(TVI_ROOT);
	
	// ���ׂẴ��[�g�m�[�h���J��
	HTREEITEM const hRoot = TreeView_GetRoot(hwndVarTree);
	
	for ( HTREEITEM hNode = hRoot
		; hNode != nullptr
		; hNode = TreeView_GetNextSibling(hwndVarTree, hNode)
	) {
		TreeView_Expand(hwndVarTree, hNode, TVE_EXPAND);
	}
	
	// �g�b�v��\������悤�Ɏd������
	TreeView_EnsureVisible(hwndVarTree, hRoot);
	return;
}

//------------------------------------------------
// �ϐ��c���[�I����
//------------------------------------------------
void term()
{
#ifdef with_WrapCall
	if ( auto hVarTree = Dialog::getVarTreeHandle() ) {
		RemoveDependingResultNodes(g_hNodeDynamic);

		// dynamic �֘A�̃f�[�^���폜���� (�K�v�Ȃ�����)
		if ( utilizeResultNodes() ) {
			g_willAddResultNodeIndepend = nullptr;
			g_willAddResultNodes.clear();
		}
	}
#endif
}

//------------------------------------------------
// �ϐ��c���[�Ƀm�[�h��ǉ�����
//------------------------------------------------
void AddNode(HTREEITEM hParent, CVarTree const& tree)
{
	TVINSERTSTRUCT tvis;
	tvis.hParent      = hParent;
	tvis.hInsertAfter = TVI_SORT;
	tvis.item.mask    = TVIF_TEXT | TVIF_PARAM;
	tvis.item.pszText = const_cast<char*>( tree.getName().c_str() );
	// �ÓI�ϐ��⃂�W���[���� lParam �l�́ACVarTree �̑Ή�����m�[�h�ւ̃|�C���^
	tvis.item.lParam  = (LPARAM)&tree;

	auto const hElem = TreeView_InsertItem( hwndVarTree, &tvis );
	if ( auto const modnode = tree.asCaseOf<CStaticVarTree::ModuleNode>() ) {
		for ( auto const& iter : *modnode ) {
			//for ( CVarTree::const_iterator iter = tree.begin(); iter != tree.end(); ++iter ) {
			AddNode(hElem, *iter.second);
		}
	}
	return;
}

//------------------------------------------------
// �ϐ��c���[�ɃV�X�e���ϐ��m�[�h��ǉ�����
//------------------------------------------------
void AddNodeSysvar( HTREEITEM hParent )
{
	TVINSERTSTRUCT tvis;
	tvis.hParent      = hParent;
	tvis.hInsertAfter = TVI_LAST;
	tvis.item.mask    = TVIF_TEXT;
	tvis.item.pszText = "+sysvar";
	
	HTREEITEM const hNodeSysvar = TreeView_InsertItem( hwndVarTree, &tvis );
	
	tvis.hParent      = hNodeSysvar;
	tvis.hInsertAfter = TVI_LAST;		// ���Ԃ����
	
	// �V�X�e���ϐ��̃��X�g��ǉ�����
	for ( int i = 0; i < SysvarCount; ++ i ) {
		string const name = strf( "~%s", SysvarData[i].name );
		tvis.item.pszText = const_cast<char*>( name.c_str() );
		TreeView_InsertItem( hwndVarTree, &tvis );
	}
	
	return;
}

#ifdef with_WrapCall
//------------------------------------------------
// �ϐ��c���[�ɓ��I�ϐ��m�[�h��ǉ�����
//------------------------------------------------
void AddNodeDynamic( HTREEITEM hParent )
{
	TVINSERTSTRUCT tvis;
	tvis.hParent      = hParent;
	tvis.hInsertAfter = TVI_LAST;
	tvis.item.mask    = TVIF_TEXT;
	tvis.item.pszText = "+dynamic";
	
	g_hNodeDynamic = TreeView_InsertItem( hwndVarTree, &tvis );
	return;
}
#endif

//------------------------------------------------
// �ϐ��c���[�� NM_CUSTOMDRAW ����������
//------------------------------------------------
LRESULT customDraw( LPNMTVCUSTOMDRAW pnmcd )
{
	if ( pnmcd->nmcd.dwDrawStage == CDDS_PREPAINT ) {
		return CDRF_NOTIFYITEMDRAW;
		
	} else if ( pnmcd->nmcd.dwDrawStage == CDDS_ITEMPREPAINT ) {
		auto const hItem = reinterpret_cast<HTREEITEM>(pnmcd->nmcd.dwItemSpec);

		// �I����ԂȂ�F�������Ȃ�
		if ( TreeView_GetItemState(hwndVarTree, hItem, 0) & TVIS_SELECTED ) {
			return 0;
		}

		string const sItem = TreeView_GetItemString( hwndVarTree, hItem );
		char const* const name = sItem.c_str();

		// �Ăяo���m�[�h
		// __sttm__, __func__ �Ɏw�肳�ꂽ�F�ɂ���
		if ( isCallNode(name) ) {
			auto const idx = static_cast<int>(TreeView_GetItemLParam(hwndVarTree, hItem));
			assert(idx >= 0);
			if ( auto const* pCallInfo = WrapCall::getCallInfoAt(idx) ) {
				auto const iter = g_config->clrTextExtra.find(
					(pCallInfo->stdat->index == STRUCTDAT_INDEX_CFUNC)
					? "__func__" : "__sttm__"
				);
				if ( iter != g_config->clrTextExtra.end() ) {
					pnmcd->clrText = iter->second;
					return CDRF_NEWFONT;
				}
			}

		// ���̑�
		} else {
			vartype_t const vtype = getVartypeOfNode(hItem);

			// �g�ݍ��݌^
			if ( 0 < vtype && vtype < HSPVAR_FLAG_USERDEF ) {
				pnmcd->clrText = g_config->clrText[vtype];
				return CDRF_NEWFONT;

			// �g���^
			} else if ( vtype >= HSPVAR_FLAG_USERDEF ) {
				auto const iter = g_config->clrTextExtra.find(hpimod::getHvp(vtype)->vartype_name);
				pnmcd->clrText = (iter != g_config->clrTextExtra.end())
					? iter->second
					: g_config->clrText[HSPVAR_FLAG_NONE];
				return CDRF_NEWFONT;
			}
		}
	}
	return 0;
}

//------------------------------------------------
// �ϐ��c���[�̗v�f�̌^���擾����
//------------------------------------------------
vartype_t getVartypeOfNode( HTREEITEM hItem )
{
	auto const name = TreeView_GetItemString(hwndVarTree, hItem);

	if ( isVarNode(name.c_str()) ) {
		PVal* const pval = hpimod::seekSttVar( name.c_str() );
		if ( pval ) return pval->flag;
		
	} else if ( isSysvarNode(name.c_str()) ) {
		auto const iSysvar = Sysvar_seek( &name[1] );
		if ( iSysvar != SysvarId_MAX ) {
			return SysvarData[iSysvar].type;
		}

	} else if ( isResultNode(name.c_str()) ) {
		auto iter = g_allResultData.find(hItem);
		if ( iter != g_allResultData.end() ) {
			return iter->second->vtype;
		}
	}
	return HSPVAR_FLAG_NONE;
}

//------------------------------------------------
// �ϐ����̃e�L�X�g���擾����
//------------------------------------------------
string getItemVarText( HTREEITEM hItem )
{
	string const itemText = TreeView_GetItemString( hwndVarTree, hItem );
	char const* const name = itemText.c_str();
	
	CVarinfoText varinf;
	
	// �m�[�h
	if ( isModuleNode(name) || isSystemNode(name) ) {
		//auto const varinf = std::make_unique<CVarinfoLine>(g_config->maxlenVarinfo);
#ifdef with_WrapCall
		if ( strcmp(name, "+dynamic") == 0 ) {
			varinf.addCallsOverview(FindLastIndependedResultData());
		} else
#endif
		if ( strcmp(name, "+sysvar") == 0 ) {
			varinf.addSysvarsOverview();

		// ���W���[�� (@...)
		} else {
			auto const* pTree = reinterpret_cast<CVarTree::ModuleNode*>(
				TreeView_GetItemLParam(hwndVarTree, hItem)
			);
			assert(pTree != nullptr);
			varinf.addModuleOverview(name, *pTree);
		}
		
	// ���[�t
	} else {
		if ( isSysvarNode(name) ) {
			varinf.addSysvar( &name[1] );
			
	#ifdef with_WrapCall
		} else if ( isCallNode(name) ) {
			auto const idx = static_cast<int>(
				TreeView_GetItemLParam( hwndVarTree, hItem )
			);
			assert(idx >= 0);
			if ( auto const pCallInfo = WrapCall::getCallInfoAt(idx) ) {
				varinf.addCall(*pCallInfo);
			}
			
		// �Ԓl�f�[�^
		} else if ( utilizeResultNodes() && isResultNode(name) ) {
			auto const iter = g_allResultData.find(hItem);
			auto const pResult = (iter != g_allResultData.end() ? iter->second : nullptr);
			varinf.addResult( pResult->stdat, pResult->valueString, hpimod::STRUCTDAT_getName(pResult->stdat) );
	#endif
		// �ÓI�ϐ�
		} else {
		/*
			// HSP���ɖ₢���킹
			char* p = g_debug->get_varinf( name, GetTabVarsOption() );
			SetWindowText( g_dialog.hVarEdit, p );
			g_debug->dbg_close( p );
		//*/
			PVal* const pval = hpimod::seekSttVar( name );
			if ( !pval ) {
				return strf("[Error] \"%s\"�͐ÓI�ϐ��̖��̂ł͂Ȃ��B\n�Q�ƁF�ÓI�ϐ������݂��Ȃ��Ƃ��ɂ��̃G���[�������邱�Ƃ�����B", name);
			}
			varinf.addVar( pval, name );
		}
	}
	return varinf.getStringMove();
}

#ifdef with_WrapCall
//------------------------------------------------
// �Ăяo���m�[�h��ǉ�
//------------------------------------------------
void AddCallNode(ModcmdCallInfo const& callinfo)
{
	// ��ˑ��ȕԒl�m�[�h������
	if ( utilizeResultNodes() ) {
		RemoveLastIndependedResultNode();
	}

	if ( !Knowbug::isStepRunning() ) {
		// ���ɒ�~�����Ƃ��ɂ܂Ƃ߂Ēǉ�����
		++g_cntWillAddCallNodes;
	} else {
		AddCallNodeImpl(callinfo);
	}
	return;
}

void AddCallNodeImpl(ModcmdCallInfo const& callinfo)
{
	char name[0x80] = "'";
	strcpy_s( &name[1], sizeof(name) - 1, hpimod::STRUCTDAT_getName(callinfo.stdat) );
	
	TVINSERTSTRUCT tvis = { 0 };
	tvis.hParent      = g_hNodeDynamic;
	tvis.hInsertAfter = TVI_LAST;
	tvis.item.mask    = TVIF_TEXT | TVIF_PARAM;
	tvis.item.pszText = name;
	tvis.item.lParam  = (LPARAM)(callinfo.idx);		// lparam �� ModcmdCallInfo �̓Y����ݒ肷��
	
	HTREEITEM const hChild = TreeView_InsertItem( hwndVarTree, &tvis );
	
	// ���m�[�h�Ȃ玩���I�ɊJ��
	if ( TreeView_GetChild( hwndVarTree, g_hNodeDynamic ) == hChild ) {
		TreeView_Expand( hwndVarTree, g_hNodeDynamic, TVE_EXPAND );
	}
	return;
}

//------------------------------------------------
// �Ō�̌Ăяo���m�[�h���폜
//------------------------------------------------
void RemoveLastCallNode()
{
	if ( g_cntWillAddCallNodes > 0 ) {
		--g_cntWillAddCallNodes;		// ����ς�ǉ����Ȃ�

	} else {
		// ���q�ɕԒl�m�[�h������΍폜����
		if ( utilizeResultNodes() ) {
			RemoveLastIndependedResultNode();
		}

		HTREEITEM const hLast = TreeView_GetChildLast(hwndVarTree, g_hNodeDynamic);
		if ( !hLast ) return;
		assert( isCallNode(TreeView_GetItemString(hwndVarTree, hLast).c_str()) );

		TreeView_EscapeFocus(hwndVarTree, hLast);
		RemoveDependingResultNodes(hLast);
		TreeView_DeleteItem(hwndVarTree, hLast);
	}
	return;
}

//------------------------------------------------
// �Ԓl�m�[�h��ǉ�
/*
�Ԓl�f�[�^ ptr �̐������Ԃ͍������Ȃ̂ŁA���̂����ɕ����񉻂��Ȃ���΂����Ȃ��B
�Ԓl�m�[�h���A�Ăяo���m�[�h�Ɠ��l�ɁA���Ɏ��s����~�����Ƃ��ɂ܂Ƃ߂Ēǉ�����B

�uA( B() )�v�̂悤�ɁA���[�U��`�R�}���h�̈������̒��Ń��[�U��`�֐����Ă΂�Ă����Ԃ��A
�uA �� B �Ɉˑ�����v�ƕ\�����邱�Ƃɂ���BA �����[�U��`�֐��ł���ꍇ�̂ݍl����B
���̂Ƃ� B �̎��s���I�����Ă��� A �̎��s���n�܂�B
B �̕Ԓl�m�[�h�́AA �̌Ăяo���m�[�h�̎q�m�[�h�Ƃ��Ēǉ������B

�\���������Ⴒ���Ⴕ�Ȃ��悤�ɁA�Ԓl�m�[�h�͋߂������ɍ폜�����B
��̓I�ɂ́A�ȉ��̒ʂ�F
1. ��ˑ��ȕԒl�m�[�h�́A���ɌĂяo���m�[�h����ˑ��ȕԒl�m�[�h���ǉ�����钼�O�A
	�܂��͌Ăяo���m�[�h���폜����钼�O�Ɏ�菜�����B
2. �ˑ�����Ԓl�m�[�h�́A���̈ˑ���̌Ăяo���m�[�h���폜�����Ƃ��Ɏ�菜�����B
3. ���s���I�������Ƃ��A���ׂĂ̕Ԓl�m�[�h����菜�����B
*/
//------------------------------------------------
void AddResultNode(ModcmdCallInfo const& callinfo, std::shared_ptr<ResultNodeData> pResult)
{
	assert(!!pResult);

	// ���s�� => ���ɒ�~�����Ƃ��ɒǉ�����
	if ( !Knowbug::isStepRunning() ) {
		if ( pResult->pCallInfoDepended ) {
			g_willAddResultNodes.push_back(pResult);
		} else {
			g_willAddResultNodeIndepend = pResult;
		}
	} else {
		AddResultNodeImpl(pResult);
	}
	return;
}

void AddResultNodeImpl(std::shared_ptr<ResultNodeData> pResult)
{
	HTREEITEM const hParent = FindDependedCallNode(pResult.get());
	if ( !hParent ) return;

	// ��ˑ��ȕԒl�m�[�h�͍��X1�Ɍ�����
	if ( hParent == g_hNodeDynamic ) {
		RemoveLastIndependedResultNode();
	}
	
	// �}��
	char name[128] = "\"";
	strcpy_s( &name[1], sizeof(name) - 1, hpimod::STRUCTDAT_getName(pResult->stdat) );
	
	TVINSERTSTRUCT tvis = { 0 };
		tvis.hParent      = hParent;
		tvis.hInsertAfter = TVI_LAST;
		tvis.item.mask    = TVIF_TEXT;
		tvis.item.pszText = name;
	
	HTREEITEM const hChild = TreeView_InsertItem( hwndVarTree, &tvis );
	
	// ���m�[�h�Ȃ玩���I�ɊJ��
	if ( TreeView_GetChild( hwndVarTree, hParent ) == hChild ) {
		TreeView_Expand( hwndVarTree, hParent, TVE_EXPAND );
	}

	// �Ԓl�m�[�h�f�[�^��ۑ����Ă���
	g_allResultData.insert({ hChild, pResult });

	if ( hParent == g_hNodeDynamic ) {
		g_lastIndependedResultNode = hChild;
	}
	return;
}

//------------------------------------------------
// �ˑ����̌Ăяo���m�[�h��T�� (failure: nullptr)
//
// @ �ˑ������c���[�r���[�ɂȂ���Ύ��s�Ƃ���B
//------------------------------------------------
HTREEITEM FindDependedCallNode(ResultNodeData* pResult)
{
	// �ˑ�����Ă���Ȃ�A���̌Ăяo���m�[�h����������
	if ( pResult->pCallInfoDepended ) {
		HTREEITEM hItem = nullptr;
		for ( hItem = TreeView_GetChild(hwndVarTree, g_hNodeDynamic)
			; hItem != nullptr
			; hItem = TreeView_GetNextSibling(hwndVarTree, hItem)
		) {
			int const idx = static_cast<int>(TreeView_GetItemLParam(hwndVarTree, hItem));
			assert(idx >= 0);
			if ( WrapCall::getCallInfoAt(idx) == pResult->pCallInfoDepended ) break;
		}
		return hItem;

	// ��ˑ��Ȃ�A+dynamic �����ɒǉ�����
	} else {
		return g_hNodeDynamic;
	}
}

//------------------------------------------------
// �Ԓl�m�[�h���폜
//------------------------------------------------
void RemoveResultNode(HTREEITEM hResult)
{
	// ����Ԓl�m�[�h�Ɉˑ�����Ԓl�m�[�h�͑}������Ȃ�
	//RemoveDependingResultNodes(hResult);

	TreeView_EscapeFocus(hwndVarTree, hResult);

	// �֘A���Ă����Ԓl�m�[�h�f�[�^��j��
	{
		size_t const cnt = g_allResultData.erase(hResult);
		assert(cnt == 1);
	}

	TreeView_DeleteItem(hwndVarTree, hResult);
	return;
}

//------------------------------------------------
// �ˑ����Ă���Ԓl�m�[�h�����ׂč폜����
//------------------------------------------------
static void RemoveDependingResultNodes(HTREEITEM hItem)
{
	if ( !utilizeResultNodes() ) return;

	// +dynamic �����̕Ԓl�m�[�h�͔�ˑ��Ȃ��̂ł���A����͖��q�̍��X1�Ɍ�����
	if ( hItem == g_hNodeDynamic ) {
		RemoveLastIndependedResultNode();
		return;
	}

	DbgArea {
		string const nodeName = TreeView_GetItemString(hwndVarTree, hItem);
		assert(isCallNode(nodeName.c_str()) || isResultNode(nodeName.c_str()));
	};

	for ( HTREEITEM hChild = TreeView_GetChild(hwndVarTree, hItem)
		; hChild != nullptr
		;
	) {
		HTREEITEM const hNext = TreeView_GetNextSibling(hwndVarTree, hChild);
		string const nodeName = TreeView_GetItemString(hwndVarTree, hChild);
		if ( isResultNode(nodeName.c_str()) ) {
			RemoveResultNode(hChild);
		}
		hChild = hNext;
	}
	return;
}

//------------------------------------------------
// (�Ō��)��ˑ��ȕԒl�m�[�h���폜����
//------------------------------------------------
void RemoveLastIndependedResultNode()
{
	if ( g_lastIndependedResultNode ) {
		RemoveResultNode(g_lastIndependedResultNode);
		g_lastIndependedResultNode = nullptr;
	}
	g_willAddResultNodeIndepend = nullptr;
	return;
}

//------------------------------------------------
// (�Ō��)��ˑ��ȕԒl�m�[�h�f�[�^��T��
//------------------------------------------------
ResultNodeData* FindLastIndependedResultData()
{
	if ( !g_lastIndependedResultNode ) return nullptr;
	auto const iter = g_allResultData.find(g_lastIndependedResultNode);
	return (iter != g_allResultData.end())
		? iter->second.get()
		: nullptr;
}

//------------------------------------------------
// �Ăяo���m�[�h�X�V
//------------------------------------------------
void UpdateCallNode()
{
	// �ǉ��\��̌Ăяo���m�[�h�����ۂɒǉ�����
	if ( g_cntWillAddCallNodes > 0 ) {
		auto const range = WrapCall::getCallInfoRange();
		size_t const lenStk = std::distance(range.first, range.second);
		for ( size_t i = lenStk - g_cntWillAddCallNodes; i < lenStk; ++i ) {
			AddCallNodeImpl(*(range.first[i]));
		}
		g_cntWillAddCallNodes = 0;
	}

	// �ǉ��\��̕Ԓl�m�[�h�����ۂɒǉ�����
	if ( utilizeResultNodes() ) {
		// ��ˑ��Ȃ���
		if ( g_willAddResultNodeIndepend ) {
			AddResultNodeImpl(g_willAddResultNodeIndepend);
			g_willAddResultNodeIndepend = nullptr;
		}

		// �ˑ�����Ă������
		if ( !g_willAddResultNodes.empty() ) {
			for ( auto const pResult : g_willAddResultNodes ) {
				AddResultNodeImpl(pResult);
			}
			g_willAddResultNodes.clear();
		}
	}
	return;
}
#endif

} // namespace VarTree
