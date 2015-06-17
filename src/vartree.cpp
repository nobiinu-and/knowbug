
#include <Windows.h>
#include <CommCtrl.h>

#include "main.h"
#include "dialog.h"
#include "config_mng.h"

#include "vartree.h"
#include "CVarTree.h"
#include "CVarinfoText.h"
#include "CVardataString.h"

#include "SysvarData.h"
#include "DebugInfo.h"

#include "module/GuiUtility.h"

#define hwndVarTree (Dialog::getVarTreeHandle())

namespace VarTree
{

static CStaticVarTree const& getSttVarTree();
static vartype_t getVartypeOfNode(HTREEITEM hItem);
static void AddNodeModule(HTREEITEM hParent, CVarTree const& tree);
static HTREEITEM AddNodeSystem(char const* name, SystemNodeId id);
static void AddNodeSysvar();

static HTREEITEM g_hNodeScript, g_hNodeLog;
HTREEITEM getScriptNodeHandle() { return g_hNodeScript; }
HTREEITEM getLogNodeHandle() { return g_hNodeLog; }

#ifdef with_WrapCall
static HTREEITEM g_hNodeDynamic;
static void AddNodeDynamic();

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
// �c���[�r���[�̃m�[�h�� LPARAM �̌^��`
//------------------------------------------------
bool VarNode::isTypeOf(char const* s)
{
	return !(ModuleNode::isTypeOf(s) || SystemNode::isTypeOf(s) || SysvarNode::isTypeOf(s) || InvokeNode::isTypeOf(s) || ResultNode::isTypeOf(s));
}
namespace Detail
{
	template<> struct Verify < ModuleNode > { static bool apply(char const*, ModuleNode::lparam_t value) {
		return (value != nullptr);
	} };
	template<> struct Verify < SystemNode > { static bool apply(char const*, SystemNode::lparam_t value) {
		return true;
	} };
	template<> struct Verify < SysvarNode > { static bool apply(char const* s, SysvarNode::lparam_t value) {
		return (0 <= value && value < Sysvar::Count && Sysvar::trySeek(&s[1]) == value);
	} };
	template<> struct Verify < InvokeNode > { static bool apply(char const*, InvokeNode::lparam_t value) {
		return (value >= 0);
	} };
	template<> struct Verify < ResultNode > { static bool apply(char const*, ResultNode::lparam_t value) {
		return (value != nullptr);
	} };
	template<> struct Verify < VarNode > { static bool apply(char const*, PVal* pval) {
			return (pval && ctx->mem_var <= pval && pval < &ctx->mem_var[hpimod::cntSttVars()]);
	} };
}

template<typename Tag, typename lparam_t>
lparam_t TreeView_MyLParam(HWND hTree, HTREEITEM hItem, Tag*)
{
	auto&& v = (lparam_t)TreeView_GetItemLParam(hTree, hItem);
	DbgArea {
		string const&& s = TreeView_GetItemString(hTree, hItem);
		assert(Tag::isTypeOf(s.c_str()) && Detail::Verify<Tag>::apply(s.c_str(), v));
	}
	return v;
}

//------------------------------------------------
// �ϐ��c���[�̏�����
//------------------------------------------------
void init()
{
	AddNodeModule(TVI_ROOT, getSttVarTree());
#ifdef with_WrapCall
	AddNodeDynamic();
#endif
	AddNodeSysvar();
	g_hNodeScript = AddNodeSystem("+script", SystemNodeId::Script);
	g_hNodeLog    = AddNodeSystem("+log", SystemNodeId::Log);
	AddNodeSystem("+general", SystemNodeId::General);

	//@, +dynamic �͊J���Ă���
#ifdef with_WrapCall
	TreeView_Expand(hwndVarTree, g_hNodeDynamic, TVE_EXPAND);
#endif
	HTREEITEM const hRoot = TreeView_GetRoot(hwndVarTree);
	assert(TreeView_GetItemString(hwndVarTree, hRoot) == CStaticVarTree::ModuleName_Global);
	TreeView_Expand(hwndVarTree, hRoot, TVE_EXPAND);

	TreeView_EnsureVisible(hwndVarTree, hRoot); //�g�b�v�܂ŃX�N���[��
	TreeView_SelectItem(hwndVarTree, hRoot);
}

//------------------------------------------------
// �ϐ��c���[�I����
//------------------------------------------------
void term()
{
#ifdef with_WrapCall
	if ( auto const hVarTree = Dialog::getVarTreeHandle() ) {
		RemoveDependingResultNodes(g_hNodeDynamic);

		// dynamic �֘A�̃f�[�^���폜���� (�K�v�Ȃ�����)
		if ( usesResultNodes() ) {
			g_willAddResultNodeIndepend = nullptr;
			g_willAddResultNodes.clear();
		}
	}
#endif
}

//------------------------------------------------
// �ÓI�ϐ����X�g���擾����
//------------------------------------------------
static CStaticVarTree const& getSttVarTree()
{
	static std::unique_ptr<CStaticVarTree> stt_tree;
	if ( !stt_tree ) {
		stt_tree.reset(new CStaticVarTree(CStaticVarTree::ModuleName_Global));
		auto const&& names = g_dbginfo->fetchStaticVarNames();
		for ( auto const& name : names ) {
			stt_tree->pushVar(name.c_str());
		}
	}
	return *stt_tree;
}

//------------------------------------------------
// �c���[�r���[�ɗv�f��}������
//------------------------------------------------
template<typename Tag>
static HTREEITEM TreeView_MyInsertItem(HTREEITEM hParent, char const* name, bool sorts, typename Tag::lparam_t lp, Tag* = nullptr)
{
	TVINSERTSTRUCT tvis {};
	tvis.hParent = hParent;
	tvis.hInsertAfter = (sorts ? TVI_SORT : TVI_LAST);
	tvis.item.mask = TVIF_TEXT | TVIF_PARAM;
	tvis.item.pszText = const_cast<char*>(name);
	tvis.item.lParam = (LPARAM)(lp);
	return TreeView_InsertItem(hwndVarTree, &tvis);
}

//------------------------------------------------
// �ϐ��c���[�Ƀm�[�h��ǉ�����
//------------------------------------------------
void AddNodeModule(HTREEITEM hParent, CStaticVarTree const& tree)
{
	auto const hElem = TreeView_MyInsertItem<ModuleNode>(hParent, tree.getName().c_str(), true, &tree);
	tree.foreach(
		[&](CStaticVarTree const& module) {
			AddNodeModule(hElem, module);
		},
		[&](string const& varname) {
			PVal* const pval = hpimod::seekSttVar(varname.c_str());
			assert(!!pval);
			TreeView_MyInsertItem<VarNode>(hElem, varname.c_str(), true, pval);
		}
	);
}

//------------------------------------------------
// �ϐ��c���[�ɃV�X�e���m�[�h��ǉ�����
//------------------------------------------------
HTREEITEM AddNodeSystem(char const* name, SystemNodeId id) {
	assert(SystemNode::isTypeOf(name));
	return TreeView_MyInsertItem<SystemNode>(TVI_ROOT, name, false, id);
}

//------------------------------------------------
// �ϐ��c���[�ɃV�X�e���ϐ��m�[�h��ǉ�����
//------------------------------------------------
void AddNodeSysvar()
{
	HTREEITEM const hNodeSysvar = AddNodeSystem("+sysvar", SystemNodeId::Sysvar);

	// �V�X�e���ϐ��̃��X�g��ǉ�����
	for ( int i = 0; i < Sysvar::Count; ++ i ) {
		string const name = strf( "~%s", Sysvar::List[i].name );
		TreeView_MyInsertItem<SysvarNode>(hNodeSysvar, name.c_str(), false, static_cast<Sysvar::Id>(i));
	}
}

#ifdef with_WrapCall
//------------------------------------------------
// �ϐ��c���[�ɓ��I�ϐ��m�[�h��ǉ�����
//------------------------------------------------
void AddNodeDynamic()
{
	g_hNodeDynamic = AddNodeSystem("+dynamic", SystemNodeId::Dynamic);
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
		if ( InvokeNode::isTypeOf(name) ) {
			auto const idx = TreeView_MyLParam<InvokeNode>(hwndVarTree, hItem);
			if ( auto const pCallInfo = WrapCall::getCallInfoAt(idx) ) {
				auto const&& iter = g_config->clrTextExtra.find(
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
			if ( 0 < vtype && vtype < HSPVAR_FLAG_USERDEF ) {
				pnmcd->clrText = g_config->clrText[vtype];
				return CDRF_NEWFONT;

			} else if ( vtype >= HSPVAR_FLAG_USERDEF ) {
				auto const&& iter = g_config->clrTextExtra.find(hpimod::getHvp(vtype)->vartype_name);
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
	string const&& name = TreeView_GetItemString(hwndVarTree, hItem);

	if ( VarNode::isTypeOf(name.c_str()) ) {
		PVal* const pval = TreeView_MyLParam<VarNode>(hwndVarTree, hItem);
		return pval->flag;

	} else if ( SysvarNode::isTypeOf(name.c_str()) ) {
		Sysvar::Id const id = TreeView_MyLParam<SysvarNode>(hwndVarTree, hItem);
		return Sysvar::List[id].type;

	} else if ( ResultNode::isTypeOf(name.c_str()) ) {
		auto const&& iter = g_allResultData.find(hItem);
		if ( iter != g_allResultData.end() ) {
			return iter->second->vtype;
		}
	}
	return HSPVAR_FLAG_NONE;
}

//------------------------------------------------
// �ϐ����̃e�L�X�g���擾����
//------------------------------------------------
std::shared_ptr<string const> getItemVarText( HTREEITEM hItem )
{
	string const&& itemText = TreeView_GetItemString( hwndVarTree, hItem );
	char const* const name = itemText.c_str();

	CVarinfoText varinf;

	if ( ModuleNode::isTypeOf(name) ) {
		auto const pTree = TreeView_MyLParam<ModuleNode>(hwndVarTree, hItem);
		varinf.addModuleOverview(name, *pTree);

	} else if ( SystemNode::isTypeOf(name) ) {
		switch ( TreeView_MyLParam<SystemNode>(hwndVarTree, hItem) ) {
#ifdef with_WrapCall
			case SystemNodeId::Dynamic:
				varinf.addCallsOverview(FindLastIndependedResultData());
				break;
#endif
			case SystemNodeId::Sysvar:
				varinf.addSysvarsOverview();
				break;
			case SystemNodeId::Log:
				return shared_ptr_from_rawptr(&Dialog::LogBox::get());

			case SystemNodeId::Script:
				if ( auto const p = Dialog::tryGetCurrentScript() ) {
					return shared_ptr_from_rawptr(p);
				} else {
					return std::make_shared<string>(g_dbginfo->getCurInfString());
				}
			case SystemNodeId::General:
				varinf.addGeneralOverview();
				break;
			default: assert_sentinel;
		}
	} else {
		if ( SysvarNode::isTypeOf(name) ) {
			Sysvar::Id const id = static_cast<Sysvar::Id>(TreeView_GetItemLParam(hwndVarTree, hItem));
			varinf.addSysvar(id);

	#ifdef with_WrapCall
		} else if ( InvokeNode::isTypeOf(name) ) {
			auto const idx = TreeView_MyLParam<InvokeNode>( hwndVarTree, hItem );
			if ( auto const pCallInfo = WrapCall::getCallInfoAt(idx) ) {
				varinf.addCall(*pCallInfo);
			}

		} else if ( usesResultNodes() && ResultNode::isTypeOf(name) ) {
			auto const&& iter = g_allResultData.find(hItem);
			auto const pResult = (iter != g_allResultData.end() ? iter->second : nullptr);
			varinf.addResult( pResult->stdat, pResult->valueString, hpimod::STRUCTDAT_getName(pResult->stdat) );
	#endif
		} else {
			assert(VarNode::isTypeOf(name));
			PVal* const pval = TreeView_MyLParam<VarNode>(hwndVarTree, hItem);
			varinf.addVar( pval, name );
		}
	}
	return std::make_shared<string>(varinf.getStringMove());
}

#ifdef with_WrapCall
//------------------------------------------------
// �Ăяo���m�[�h��ǉ�
//------------------------------------------------
void AddCallNode(ModcmdCallInfo const& callinfo)
{
	// ��ˑ��ȕԒl�m�[�h������
	if ( usesResultNodes() ) {
		RemoveLastIndependedResultNode();
	}

	if ( !Knowbug::isStepRunning() ) {
		// ���ɒ�~�����Ƃ��ɂ܂Ƃ߂Ēǉ�����
		++g_cntWillAddCallNodes;
	} else {
		AddCallNodeImpl(callinfo);
	}
}

void AddCallNodeImpl(ModcmdCallInfo const& callinfo)
{
	char name[128] = "'";
	strcpy_s(&name[1], sizeof(name) - 1, hpimod::STRUCTDAT_getName(callinfo.stdat));
	HTREEITEM const hChild = TreeView_MyInsertItem<InvokeNode>(g_hNodeDynamic, name, false, callinfo.idx);

	// ���m�[�h�Ȃ玩���I�ɊJ��
	if ( TreeView_GetChild( hwndVarTree, g_hNodeDynamic ) == hChild ) {
		TreeView_Expand( hwndVarTree, g_hNodeDynamic, TVE_EXPAND );
	}
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
		if ( usesResultNodes() ) {
			RemoveLastIndependedResultNode();
		}

		HTREEITEM const hLast = TreeView_GetChildLast(hwndVarTree, g_hNodeDynamic);
		if ( !hLast ) return;
		assert( InvokeNode::isTypeOf(TreeView_GetItemString(hwndVarTree, hLast).c_str()) );

		TreeView_EscapeFocus(hwndVarTree, hLast);
		RemoveDependingResultNodes(hLast);
		TreeView_DeleteItem(hwndVarTree, hLast);
	}
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
	HTREEITEM const hChild = TreeView_MyInsertItem<ResultNode>(hParent, name, false, nullptr);

	// ���m�[�h�Ȃ玩���I�ɊJ��
	if ( TreeView_GetChild( hwndVarTree, hParent ) == hChild ) {
		TreeView_Expand( hwndVarTree, hParent, TVE_EXPAND );
	}

	// �Ԓl�m�[�h�f�[�^��ۑ����Ă���
	g_allResultData.emplace(hChild, pResult);

	if ( hParent == g_hNodeDynamic ) {
		g_lastIndependedResultNode = hChild;
	}
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
			int const idx = TreeView_MyLParam<InvokeNode>(hwndVarTree, hItem);
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
}

//------------------------------------------------
// �ˑ����Ă���Ԓl�m�[�h�����ׂč폜����
//------------------------------------------------
static void RemoveDependingResultNodes(HTREEITEM hItem)
{
	if ( !usesResultNodes() ) return;

	// +dynamic �����̕Ԓl�m�[�h�͔�ˑ��Ȃ��̂ł���A����͖��q�̍��X1�Ɍ�����
	if ( hItem == g_hNodeDynamic ) {
		RemoveLastIndependedResultNode();
		return;
	}

	DbgArea {
		string const nodeName = TreeView_GetItemString(hwndVarTree, hItem);
		assert(InvokeNode::isTypeOf(nodeName.c_str()) || ResultNode::isTypeOf(nodeName.c_str()));
	};

	for ( HTREEITEM hChild = TreeView_GetChild(hwndVarTree, hItem)
		; hChild != nullptr
		;
	) {
		HTREEITEM const hNext = TreeView_GetNextSibling(hwndVarTree, hChild);
		string const nodeName = TreeView_GetItemString(hwndVarTree, hChild);
		if ( ResultNode::isTypeOf(nodeName.c_str()) ) {
			RemoveResultNode(hChild);
		}
		hChild = hNext;
	}
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
}

//------------------------------------------------
// (�Ō��)��ˑ��ȕԒl�m�[�h�f�[�^��T��
//------------------------------------------------
ResultNodeData* FindLastIndependedResultData()
{
	if ( !g_lastIndependedResultNode ) return nullptr;
	auto const&& iter = g_allResultData.find(g_lastIndependedResultNode);
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
		auto const&& range = WrapCall::getCallInfoRange();
		size_t const lenStk = std::distance(range.first, range.second);
		for ( size_t i = lenStk - g_cntWillAddCallNodes; i < lenStk; ++i ) {
			AddCallNodeImpl(*(range.first[i]));
		}
		g_cntWillAddCallNodes = 0;
	}

	// �ǉ��\��̕Ԓl�m�[�h�����ۂɒǉ�����
	if ( usesResultNodes() ) {
		// ��ˑ��Ȃ���
		if ( g_willAddResultNodeIndepend ) {
			AddResultNodeImpl(g_willAddResultNodeIndepend);
			g_willAddResultNodeIndepend = nullptr;
		}

		// �ˑ�����Ă������
		if ( !g_willAddResultNodes.empty() ) {
			for ( auto const& pResult : g_willAddResultNodes ) {
				AddResultNodeImpl(pResult);
			}
			g_willAddResultNodes.clear();
		}
	}
}
#endif

} // namespace VarTree

//------------------------------------------------
// ResultNodeData �R���X�g���N�^
//------------------------------------------------
#include "module/CStrBuf.h"
namespace WrapCall
{
	ResultNodeData::ResultNodeData(ModcmdCallInfo const& callinfo, PDAT* ptr, vartype_t vt)
		: stdat(callinfo.stdat)
		, vtype(vt)
		, pCallInfoDepended(callinfo.getDependedCallInfo())
	{
		auto p = std::make_shared<CStrBuf>();
		CVardataStrWriter::create<CLineformedWriter>(p)
			.addResult(hpimod::STRUCTDAT_getName(stdat), ptr, vt);
		valueString = p->getMove();
	}
}
