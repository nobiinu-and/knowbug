
#pragma once

#include "Node.h"
#include "ITreeVisitor.h"

namespace DataTree
{;

// updated state �����ׂ� None �ɓ\�蒼�� iterator
class CTreeUpdatedStateInitializer
	: public ITreeVisitor
{
public:
	void set(ITree* t) { t->setUpdatedState(ITree::UpdatedState::None); }

	void visit1(IMonoNode* t) { set(t); t->getChild(); }
	void visit1(IPolyNode* t) { set(t); for ( auto const child : t->getChildren() ) { visit0(child); } }
	void visit1(ILeaf* t) { set(t); }

	void visit0(ITree* t) override { t->acceptVisitor(*this); }

	void visit(CLoopNode* t)        override { visit1(t); }
	void visit(CNodeModule* t)      override { visit1(t); }
	void visit(CNodeVarArray* t)    override { visit1(t); }
	void visit(CNodeVarElem* t)     override { visit1(t); }
	void visit(CNodeVarHolder* t)   override { visit1(t); }
	void visit(CNodeModInst* t)     override { visit1(t); }
	void visit(CNodeModInstNull* t) override { visit1(t); }
	void visit(CNodePrmStk* t)      override { visit1(t); }
	void visit(CNodeLabel* t)       override { visit1(t); }
	void visit(CNodeString* t)      override { visit1(t); }
	void visit(CNodeDouble* t)      override { visit1(t); }
	void visit(CNodeInt* t)         override { visit1(t); }
	void visit(CNodeUnknown* t)     override { visit1(t); }
};

// ����m�[�h�̒��ڂ̑c��Ǝq���m�[�h���X�V����
// �c��� Shallow ��(���̃m�[�h����)�A�q���� Deep ��(���ׂĂ̎q�����܂�)�X�V
class CTreeUpdater
	: public ITreeVisitor
{
public:
	CTreeUpdater(ITree* base)
		: base_(base), bNowDescendant_(true)
	{ }

	void visit0(ITree* t) override { t->acceptVisitor(*this); }

	void visit(CLoopNode*)        override;
	void visit(CNodeModule*)      override;
	void visit(CNodeVarArray*)    override;
	void visit(CNodeVarElem*)     override;
	void visit(CNodeVarHolder*)   override;
	void visit(CNodeModInst*)     override;
	void visit(CNodeModInstNull*) override;
	void visit(CNodePrmStk*)      override;
	void visit(CNodeLabel*)       override;
	void visit(CNodeString*)      override;
	void visit(CNodeDouble*)      override;
	void visit(CNodeInt*)         override;
	void visit(CNodeUnknown*)     override;

private:
	void visitParent(INodeContainer* t)  {
		bool bak = bNowDescendant_; bNowDescendant_ = bNowDescendant_ && (t != base_);
		visit0(t->getParent());
		bNowDescendant_ = bak;
	}

	void visitAsChild(ITree* child) {
		bool bak = bNowDescendant_; bNowDescendant_ |= (child == base_);
		visit0(child);
		bNowDescendant_ = bak;
	}

	// �� visit ���Ă���ꏊ���N�_�m�[�h�����̎q���Ȃ�^�A�c��Ȃ�U�B
	// �Ȃ��c��ł��q���ł��Ȃ��m�[�h�� visit ���邱�Ƃ͂Ȃ�
	bool isNowDescendant() const { return bNowDescendant_; }

private:
	// �X�V�̋N�_
	ITree* base_;

	bool bNowDescendant_;
};

}

