#ifndef IG_MANAGED_PVAL_H
#define IG_MANAGED_PVAL_H

#include "hsp3plugin_custom.h"
#include "mod_makepval.h"
#include "Managed.h"

namespace hpimod {

namespace detail {
	struct PValDefaultCtorDtor {
		static inline void defaultCtor(PVal* p) {
			PVal_init(p, HSPVAR_FLAG_INT);
		}
		static inline void defaultDtor(PVal& self) {
			PVal_free(&self);
		}
	};
}

using ManagedPVal = Managed<PVal, false, detail::PValDefaultCtorDtor>;

// PVal with APTR
// ��{�I�ɂ͎��O�� PVal �𐶐����ď��L����B
// �^����ꂽ PVal* �̎Q��(var, array �����Ȃǂ̂���)�Ƃ��Ă��g����B
class ManagedVarData
{
private:
	ManagedPVal pval_;
	APTR aptr_;

public:
	ManagedVarData()
		: pval_ { }, aptr_ { AptrInvalid }
	{ }

	ManagedVarData(PVal* pval, APTR aptr)
		: pval_ { ManagedPVal::ofValptr(pval) }, aptr_ { aptr }
	{ }

	PVal* getPVal() const { return pval_.valuePtr(); }
	APTR  getAPTR() const { return aptr_; }

	static APTR const AptrInvalid = -1;
};

} // namespace hpimod

// �̈���m�ۂ��鎞�_�ł��ꂪ���LPVal�Ȃ̂� view PVal �Ȃ̂��킩��̂ŁA����Ŏg��������Ƃ������Ƃ��ł���B

#endif
