// visitor - Treeview-Node ������

#ifndef IG_CLASS_TREEVIEW_NODE_CREATOR_H
#define IG_CLASS_TREEVIEW_NODE_CREATOR_H

#include <memory>
#include <cstdarg>
#include <windows.h>
#include <CommCtrl.h>

#include "ITreeVisitor.h"

namespace DataTree
{

//##############################################################################
//                �錾�� : CTvCreator
//##############################################################################
//------------------------------------------------
// �c���[�r���[�E�m�[�h�̐�����
//------------------------------------------------
class CTvCreator
	: public ITreeVisitor
{
public:
	CTvCreator( HWND hTreeView );
	
	//******************************************************
	//    �C���^�[�t�F�[�X
	//******************************************************
public:
	void visit(tree_t);
	
	//******************************************************
	//    �m�[�h�����̎���
	//******************************************************
private:
	// ���
//	virtual void procPre (ITree*) { }
//	virtual void procEach(ITree*) { }
//	virtual void procPost(ITree*) { }
	
private:
	// �R���e�i
	virtual void     procPre (IPolyNode*);
	virtual bool requiresEach(IPolyNode*) const { return false; }
	virtual void     procPost(IPolyNode*);
	
	// ���[�t
	virtual void     procPre (ILeaf*);
	virtual bool requiresEach(ILeaf*) const { return false; }
	virtual bool requiresPost(ILeaf*) const { return false; }
	
	//******************************************************
	//    ���������o�֐�
	//******************************************************
private:
	void CTvCreator::insertItem(tree_t, string name, HTREEITEM hInsertAfter);

	//******************************************************
	//    �����o�ϐ�
	//******************************************************
private:
	struct Impl;
	std::unique_ptr<Impl> m;
};

}

#endif
