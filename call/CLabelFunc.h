// ���x���֐�

#ifndef IG_CLASS_LABEL_FUNC_H
#define IG_CLASS_LABEL_FUNC_H

#include "hsp3plugin_custom.h"

#include "axcmd.h"
#include "IFunctor.h"
#include "CCaller.h"

#include "cmd_sub.h"

using namespace hpimod;

class CLabelFunc
	: public IFunctor
{
	label_t lb_;

public:
	CLabelFunc(label_t lb) : lb_ { lb } {}
	CLabelFunc(int axcmd_) {
		assert(AxCmd::isOk(axcmd_));
		switch ( AxCmd::getType(axcmd_) ) {
			case TYPE_LABEL:  lb_ = hpimod::getLabel(AxCmd::getCode(axcmd_)); break;
			case TYPE_MODCMD: lb_ = hpimod::getLabel(getSTRUCTDAT(AxCmd::getCode(axcmd_))->otindex); break;
		}
		puterror(HSPERR_TYPE_MISMATCH);
	}

	// �擾
	label_t getLabel() const override { return lb_; }
	int getAxCmd() const override { return AxCmd::make(TYPE_LABEL, hpimod::getOTPtr(lb_)); }

	int getUsing() const override { return HspBool(lb_ != nullptr); }			// �g�p�� (0: ����, 1: �L��, 2: �N���[��)
	CPrmInfo const& getPrmInfo() const override {
		return GetPrmInfo(lb_);
	}

	// �L���X�g (IFunctor �p)
	/*
	template<typename T>       T     castTo()       { return dynamic_cast<T>(this); }
	template<typename T> const T     castTo() const { return dynamic_cast<T>(this); }
	template<typename T>       T safeCastTo()       { return safeCastTo_Impl<T>(); }
	template<typename T> const T safeCastTo() const { return safeCastTo_Impl<const T>(); }
	//*/

	// ����
	void call(CCaller& caller) override {
		return caller.getCall().callLabel(getLabel());
	}

	// �`���I��r
};

#endif
