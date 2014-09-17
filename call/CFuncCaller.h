#ifndef IG_CLASS_FUNCTION_CALLER_H
#define IG_CLASS_FUNCTION_CALLER_H

// �֐��Ăяo���̃I�u�W�F�N�g
// �������璼�� prmstack �̐������s���B
// CCaller, CCall �̑���ɂ������B

// CStreamCaller �Ɏ��Ă��邪�A�Ăяo�����ɒǉ��̈�����^�����Ȃ��B

#include "hsp3plugin_custom.h"
#include "Functor.h"
#include "CPrmStk.h"
#include "ManagedPVal.h"

class CFuncCaller;

using funcCaller_t = CFuncCaller*;

class CFuncCaller
	: public IFunctor
{
	// �����o�ϐ�
private:
	functor_t functor_;
	CPrmStk prmstk_;

	// �\�z
private:
	CFuncCaller() = delete;
	CFuncCaller(functor_t f)
		: functor_ { std::move(f) }
		, prmstk_(f->getPrmInfo())
	{ }
	~CFuncCaller() { }

	CFuncCaller(CFuncCaller const&) = delete;

public:
	// IFunctor �̎���
	void call(CCaller& caller) override
	{
		functor_->call(caller);
		return;
	}

	label_t   getLabel() const override { return functor_->getLabel(); }
	int       getAxCmd() const override { return functor_->getAxCmd(); }
	int       getUsing() const override { return functor_->getUsing(); }
	CPrmInfo const& getPrmInfo() const override { return functor_->getPrmInfo(); }

	CPrmStk& getPrmStk() { return prmstk_; }

	// ���b�p�[
	static functor_t New(functor_t f) { return functor_t::make<CFuncCaller>(std::move(f)); }
};

#endif
