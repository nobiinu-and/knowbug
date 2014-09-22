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
// Remark: ���̃N���X���̂� Managed<> �ł͂Ȃ����A���|�C���^�̌`�Ŏg���Ȃ��B
class ManagedVarData
{
private:
	ManagedPVal pval_;
	APTR aptr_;

public:
	ManagedVarData()
		: pval_ { }, aptr_ { 0 }
	{ }

	ManagedVarData(PVal* pval, APTR aptr)
		: pval_ { ManagedPVal::ofValptr(pval) }, aptr_ { aptr }
	{ }
	ManagedVarData(MPVarData const& vardata)
		: ManagedVarData(vardata.pval, vardata.aptr)
	{ }

	ManagedVarData(PDAT const* pdat, vartype_t vtype)
		: ManagedVarData()
	{ PVal_assign(getPVal(), pdat, vtype); }

public:
	PVal* getPVal() const { return pval_.valuePtr(); }
	APTR  getAptr() const { return aptr_; }

	PVal* getVar() const {
		auto const pval = getPVal();
		if ( getAptr() > 0 ) {
			pval->arraycnt = 1;
			pval->offset = getAptr();
		} else {
			HspVarCoreReset(pval);
		}
		return pval;
	}
};

} // namespace hpimod

// �̈���m�ۂ��鎞�_�ł��ꂪ���LPVal�Ȃ̂� view PVal �Ȃ̂��킩��̂ŁA����Ŏg��������Ƃ������@������B

#endif
