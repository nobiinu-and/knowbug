// visitor - Treeview-Node ������

#include "Node.h"
#include "CTvCreator.h"

#include "module/ptr_cast.h"
#include "module/strf.h"

#include <stack>
#include <vector>
#include <cstring>
#include <CommCtrl.h>
#include <windows.h>

namespace DataTree
{

//**********************************************************
//    �����o�ϐ��̎���
//**********************************************************
struct CTvCreator::Impl
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
	Impl( HWND _hTreeView )
		: hTreeView( _hTreeView )
	{
		modnamelist.reserve( 1 );
		pushParent( TVI_ROOT );		// �ԕ�
		pushParent( TVI_ROOT );
		return;
	}
	
	//--------------------------------------------
	// stkParent �̑���
	//--------------------------------------------
	void pushParent( HTREEITEM hParent_ )
	{
		stkParent.push( hParent_ );
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
		std::memset( &tvis, 0x00, sizeof(tvis) );
		tvis.hParent      = hParent;
		tvis.hInsertAfter = TVI_SORT;	// TODO: �Ăяo���m�[�h�Ȃǂ̓\�[�g���Ă͂����Ȃ�
		tvis.item.mask    = TVIF_TEXT;
		return;
	}
};

//**********************************************************
//    �\�z�Ɖ��
//**********************************************************
//------------------------------------------------
// �\�z
//------------------------------------------------
CTvCreator::CTvCreator( HWND hTreeView )
	: m( new Impl( hTreeView ) )
{ }

//------------------------------------------------
// �A�C�e���̑}��
//------------------------------------------------
void CTvCreator::insertItem(tree_t node, string name, HTREEITEM hInsertAfter)
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
void CTvCreator::visit( tree_t node )
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
void CTvCreator::procPre( IPolyNode* node )
{
	string name(node->getName());
	
	// �ϐ� => �X�R�[�v������t������
	if (   typeid(node) == typeid(CNodeVarArray*)
		|| typeid(node) == typeid(CNodeVarElem*)
	) {
		for each ( string modname in m->modnamelist ) {
			name.append( "@" );
			name.append( modname );
		}
		
	// ���W���[�� => �擪�� '@' ��t������
	} else if ( typeid(node) == typeid(CNodeModule*) ) {
		m->modnamelist.push_back( name );
		
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
void CTvCreator::procPost( IPolyNode* pNode )
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
void CTvCreator::procPre( ILeaf* pNode )
{
	/*
	TVINSERTSTRUCT& tvis = m->getTvIs();
	tvis.item.mask   |= TVIF_PARAM;
	tvis.item.lParam  = forcible_cast<LPARAM>( address_cast(pNode) );
	tvis.item.pszText = const_cast<char*>( pNode->getName().c_str() );
	
	// �A�C�e����}�� (�e�ɂȂ�Ȃ��̂ŁA�߂�l�͕ۑ����Ȃ��Ă��悢)
	TreeView_InsertItem( m->hTreeView, &tvis )
	//*/
	return;
}

}
