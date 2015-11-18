// �c���[�r���[�\����

#include "Node.h"
#include "CTvRepresenter.h"

#include "module/ptr_cast.h"
#include "module/strf.h"

#include <stack>
#include <vector>
#include <map>
#include <cstring>
#include <CommCtrl.h>
#include <windows.h>

#include "dialog.h"		// for TreeView_*

static TVINSERTSTRUCT& getTvIs(HTREEITEM hParent);

namespace DataTree
{
	//**********************************************************
	// �����o�ϐ�
	//**********************************************************
	struct CTvRepresenter::Impl
	{
		HWND hTreeView;

		typedef std::map<tree_t, HTREEITEM> mapNodeToTvItem_t;
		mapNodeToTvItem_t mapNodeToTvItem;

		CTvAppendObserver appendObserver;
		CTvRemoveObserver removeObserver;
	public:
		Impl(HWND hTv, CTvAppendObserver _append, CTvRemoveObserver _remove)
			: hTreeView(hTv), appendObserver(_append), removeObserver(_remove)
		{ }
	};

	//**********************************************************
	// �C���^�[�t�F�[�X
	//**********************************************************
	CTvRepresenter::CTvRepresenter(HWND hTreeView)
		: m(new Impl(hTreeView, CTvAppendObserver(this), CTvRemoveObserver(this)))
	{
		registerObserver({&m->appendObserver, &m->removeObserver});
		return;
	}

	CTvRepresenter::~CTvRepresenter()
	{
		delete m;
	}
	
	//**********************************************************
	// append
	//**********************************************************
	void CTvRepresenter::CTvAppendObserver::visit1(INodeContainer* newChild)
	{
		auto const parent = newChild->getParent();
		getOwner()->insertItem(newChild, newChild->getName(), getOwner()->findTvItem(parent), TVI_SORT);
		return;
	}

	//**********************************************************
	// remove
	//**********************************************************
	void CTvRepresenter::CTvRemoveObserver::visit1(INodeContainer* removed)
	{
		if ( auto const hItem = getOwner()->findTvItem(removed) ) {
			TreeView_DeleteItem(getOwner()->m->hTreeView, hItem);
		}
		return;
	}

	//**********************************************************
	// �������\�b�h
	//**********************************************************
	HTREEITEM CTvRepresenter::findTvItem(tree_t node) const {
		auto const iter = m->mapNodeToTvItem.find(node);
		return (iter != m->mapNodeToTvItem.end() ? iter->second : nullptr);
	}

	tree_t CTvRepresenter::findNode(HTREEITEM hItem) const {
		return reinterpret_cast<tree_t>(TreeView_GetItemLParam(m->hTreeView, hItem));
	}

	//------------------------------------------------
	// �A�C�e���̑}��
	//------------------------------------------------
	void CTvRepresenter::insertItem(tree_t node, string name, HTREEITEM hParent, HTREEITEM hInsertAfter)
	{
		static TVINSERTSTRUCT tvis;
		//TVINSERTSTRUCT& tvis = getTvIs(hParent);
		std::memset(&tvis, 0x00, sizeof(tvis));
		tvis.hParent = hParent;
		tvis.hInsertAfter = hInsertAfter;
		tvis.item.mask = TVIF_TEXT;

		tvis.item.mask |= TVIF_PARAM;
		tvis.item.lParam = ctype_cast<LPARAM>(address_cast(node));
		tvis.item.pszText = const_cast<char*>(name.c_str());

		TreeView_InsertItem(m->hTreeView, &tvis);
		return;
	}


	/*
	//**********************************************************
	//    �����o�ϐ��̎���
	//**********************************************************
	struct CTvRepresenter::Impl
	{
	public:
		HWND hTreeView;

		std::stack<HTREEITEM> stkParent;
		std::vector<string> modnamelist;

	private:
		HTREEITEM hParent;

		TVINSERTSTRUCT tvis;

	public:
		//--------------------------------------------
		// �\�z
		//--------------------------------------------
		Impl(HWND _hTreeView)
			: hTreeView(_hTreeView)
		{
			modnamelist.reserve(1);
			pushParent(TVI_ROOT);		// �ԕ�
			pushParent(TVI_ROOT);
			return;
		}

		//--------------------------------------------
		// stkParent �̑���
		//--------------------------------------------
		void pushParent(HTREEITEM hParent_)
		{
			stkParent.push(hParent_);
			hParent = hParent_;
			return;
		}

		void popParent()
		{
			stkParent.pop();
			hParent = stkParent.top();
			return;
		}

		//--------------------------------------------
		// tvis �̎擾
		// 
		// @ �ē��s�\���ɒ���
		//--------------------------------------------
		TVINSERTSTRUCT& getTvIs()
		{
			initializeTvIs();
			return tvis;
		}

	private:
		//--------------------------------------------
		// tvis �̏�����
		//--------------------------------------------
		void initializeTvIs()
		{
			std::memset(&tvis, 0x00, sizeof(tvis));
			tvis.hParent = hParent;
			tvis.hInsertAfter = TVI_SORT;	// TODO: �Ăяo���m�[�h�Ȃǂ̓\�[�g���Ă͂����Ȃ�
			tvis.item.mask = TVIF_TEXT;
			return;
		}
	};

	//**********************************************************
	//    �\�z�Ɖ��
	//**********************************************************
	//------------------------------------------------
	// �\�z
	//------------------------------------------------
	CTvRepresenter::CTvRepresenter(HWND hTreeView)
		: m(new Impl(hTreeView))
	{ }

	//------------------------------------------------
	// �A�C�e���̑}��
	//------------------------------------------------
	void CTvRepresenter::insertItem(tree_t node, string name, HTREEITEM hInsertAfter)
	{
		TVINSERTSTRUCT& tvis = m->getTvIs();
		tvis.hInsertAfter = hInsertAfter;
		tvis.item.mask |= TVIF_PARAM;
		tvis.item.lParam = ctype_cast<LPARAM>(address_cast(node));
		tvis.item.pszText = const_cast<char*>(name.c_str());

		m->pushParent(TreeView_InsertItem(m->hTreeView, &tvis));
		return;
	}

	//**********************************************************
	//    �C���^�[�t�F�[�X
	//**********************************************************
	//------------------------------------------------
	// �K��
	//------------------------------------------------
	void CTvRepresenter::visit(tree_t node)
	{
		node->acceptVisitor(*this);
		return;
	}

	//**********************************************************
	//    �m�[�h�̏��� (callback)
	//**********************************************************
	//++++++++++++++++++++++++++++++++++++++++++++++++
	//    �R���e�i
	//++++++++++++++++++++++++++++++++++++++++++++++++
	//------------------------------------------------
	// �s��
	//------------------------------------------------
	void CTvRepresenter::procPre(IPolyNode* node)
	{
		string name(node->getName());

		// �ϐ� => �X�R�[�v������t������
		if ( typeid(node) == typeid(CNodeVarArray*)
			|| typeid(node) == typeid(CNodeVarElem*)
			) {
			for each (string modname in m->modnamelist) {
				name.append("@");
				name.append(modname);
			}

			// ���W���[�� => �擪�� '@' ��t������
		} else if ( typeid(node) == typeid(CNodeModule*) ) {
			m->modnamelist.push_back(name);

			name = string("@") + name;

			// �V�X�e���ϐ� => �擪�� ` ��t������
		}
		//	else if ( typeid(node) == typeid(CNodeSysvar*) ) { }

		insertItem(node, std::move(name), TVI_SORT);
		return;
	}

	//------------------------------------------------
	// �߂�
	//------------------------------------------------
	void CTvRepresenter::procPost(IPolyNode* pNode)
	{
		if ( typeid(pNode) == typeid(CNodeModule const*) ) {
			m->modnamelist.pop_back();
		}

		// �e�m�[�h��߂�
		m->popParent();
		return;
	}

	//++++++++++++++++++++++++++++++++++++++++++++++++
	//    ���[�t
	//++++++++++++++++++++++++++++++++++++++++++++++++
	//------------------------------------------------
	// �s��
	//------------------------------------------------
	void CTvRepresenter::procPre(ILeaf* pNode)
	{
		/*
		TVINSERTSTRUCT& tvis = m->getTvIs();
		tvis.item.mask   |= TVIF_PARAM;
		tvis.item.lParam  = forcible_cast<LPARAM>( address_cast(pNode) );
		tvis.item.pszText = const_cast<char*>( pNode->getName().c_str() );

		// �A�C�e����}�� (�e�ɂȂ�Ȃ��̂ŁA�߂�l�͕ۑ����Ȃ��Ă��悢)
		TreeView_InsertItem( m->hTreeView, &tvis )
		//* /
		return;
	}
	//*/
}
