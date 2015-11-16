// Visitor

#ifndef IG_INTERFACE_DATATREE_VISITOR_H
#define IG_INTERFACE_DATATREE_VISITOR_H

#include "Node.dec.h"

namespace DataTree
{;

	/*
	// �ȑO�̎����ł� iterator ������
#define ITreeVisitorDecProcMembersLeaf(T) \
	virtual void procPre(T);
#define ITreeVisitorDecProcMembersMono(T) \
	virtual void procPre(T); virtual void procPost(T);
#define ITreeVisitorDecProcMembersPoly(T) \
	virtual void procPre(T); virtual void procEach(T); virtual void procPost(T);
	
	// ���
//	ITreeVisitorDecProcMembers(target_t);
	virtual void procPre (target_t) { };	// �s��
	virtual void procEach(target_t) { };	// �e�q�m�[�h�� visit ���钼�O (�q�m�[�h�̃|�C���^���n�������������̂ł́H)
	virtual void procPost(target_t) { };	// �߂�
	//*/

class ITreeVisitor
{
public:
	virtual ~ITreeVisitor() { }

	virtual void visit0(ITree*) = 0;	// t->acceptVisitor(*this);
	virtual void visit(CLoopNode*) = 0;

	virtual void visit(CNodeModule*) = 0;
	virtual void visit(CNodeVarHolder*) = 0;
	virtual void visit(CNodeVarArray*) = 0;
	virtual void visit(CNodeVarElem*) = 0;

	virtual void visit(CNodeModInst*) = 0;
	virtual void visit(CNodeModInstNull*) = 0;
	virtual void visit(CNodePrmStk*) = 0;

	virtual void visit(CNodeLabel*) = 0;
	virtual void visit(CNodeString*) = 0;
	virtual void visit(CNodeDouble*) = 0;
	virtual void visit(CNodeInt*) = 0;
	virtual void visit(CNodeUnknown*) = 0;
};

}
#endif
