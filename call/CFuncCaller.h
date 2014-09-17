#ifndef IG_CLASS_FUNCTION_CALLER_H
#define IG_CLASS_FUNCTION_CALLER_H

// �֐��Ăяo���̃I�u�W�F�N�g
// �������璼�� prmstack �̐������s���B
// CCaller, CCall �̑���ɂ������B

// CStreamCaller �Ɏ��Ă��邪�A�Ăяo�����ɒǉ��̈�����^�����Ȃ��B

#include "hsp3plugin_custom.h"
#include "Functor.h"
#include "CPrmStkCreator.h"

class CFuncCaller;

using funcCaller_t = CFuncCaller*;

class CFuncCaller
	: public IFunctor
{
	// �����o�ϐ�
private:
	functor_t functor_;
	CPrmInfo& prminfo_;
	CPrmStkCreatorWithBuffer prmstk_;

	// �\�z
private:
	CFuncCaller() = delete;
	CFuncCaller(functor_t f)
		: functor_ { std::move(f) }
		, prminfo_ { f->getPrmInfo() }
		, prmstk_(f->getPrmInfo().getStackSize() + sizeof(void*))
	{ }
	~CFuncCaller() { }

	CFuncCaller(CFuncCaller const&) = delete;

public:
	void call(CCaller& caller) {
		functor_->call();
		return;
	}

	label_t   getLabel() const override { return functor_.getLabel(); }
	int       getAxCmd() const override { return functor_.getAxCmd(); }
	int       getUsing() const override { return functor_.getUsing(); }

	CPrmInfo const& getPrmInfo() const override { return prminfo_; }

	// ���b�p�[
	static funcCaller_t New(functor_t f) { return new CFuncCaller(f); }
};

#endif
