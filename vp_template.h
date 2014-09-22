// HspVarProc �̂ЂȌ`

#ifndef IG_HSPVARPROC_TEMPLATE_H
#define IG_HSPVARPROC_TEMPLATE_H

#include "hsp3plugin_custom.h"

#if _DEBUG
 #include "mod_makepval.h"
#endif
#include <functional>

namespace hpimod
{

//------------------------------------------------
// ���̃|�C���^�𓾂�
//
// @ pt �� value[] �ł���ꍇ
//------------------------------------------------
template<typename vtTag>
static PDAT* HspVarTemplate_GetPtr(PVal* pval)
{
	static_assert(VtTraits::isNativeVartype<vtTag>::value, "valptr_t = value_t[] �ł���ϐ��^�ł̂ݎg�p�ł���B");
	assert(PVal_supportArray(pval));
	return VtTraits::asPDAT<vtTag>( VtTraits::getValptr<vtTag>(pval) );
}

//------------------------------------------------
// �ϒ��^�ɓ��L�̊֐��́A�Œ蒷�^�̏ꍇ
//------------------------------------------------
template<typename vtTag>
static int HspVarTemplate_GetSize(PDAT const* pdat)
{
	static_assert(VtTraits::isFixed<vtTag>::value, "�Œ蒷�̕ϐ��^�ł̂ݎg�p�ł���B");
	return VtTraits::basesize<vtTag>::value;
}

template<typename vtTag>
static void* HspVarTemplate_GetBlockSize(PVal* pval, PDAT* pdat, int* size)
{
	static_assert(VtTraits::isFixed<vtTag>::value, "�Œ蒷�̕ϐ��^�ł̂ݎg�p�ł���B");
	*size = VtTraits::basesize<vtTag>::value * PVal_cntElems(pval);
	return pdat;
}

template<typename vtTag>
static void HspVarTemplate_AllocBlock(PVal* pval, PDAT* pdat, int size)
{
	static_assert(VtTraits::isFixed<vtTag>::value, "�Œ蒷�̕ϐ��^�ł̂ݎg�p�ł���B");
	// do nothing
	return;
}

//------------------------------------------------
// ���Z�p�֐��̌^�ϊ�
//------------------------------------------------
using typeRedefinedOperator_t = void(*)(PDAT*, PDAT const*);

static operator_t HspVarProcOperatorCast(typeRedefinedOperator_t op)
{
	return reinterpret_cast<operator_t>(op);
}

//------------------------------------------------
// ��r�֐��̋�̉�
//
// @ HspVar**_CmpI �Ƃ���1�̊֐����牉�Z�֐��𐶐�����B
// @ aftertype �͂��̒��Őݒ肷��K�v������B
//------------------------------------------------
using compare_func_t = int(*)(PDAT* pdat, PDAT const* val);

namespace detail
{
template<compare_func_t CmpI, typename TCmpFunctor>
static void HspVarTemplate_CmpI(PDAT* pdat, PDAT const* val)
{
	static TCmpFunctor comparer {};

	VtTraits::derefValptr<vtInt>(pdat) = HspBool( comparer(CmpI(pdat, val), 0) );

//	myhvp->aftertype = HSPVAR_FLAG_INT;
	return;
}
}

// ���l���̂ݒ�`����
template<compare_func_t CmpI>
static void HspVarTemplate_InitCmpI_Equality(HspVarProc* hvp)
{
	hvp->EqI = HspVarProcOperatorCast( detail::HspVarTemplate_CmpI< CmpI, std::equal_to<int> > );
	hvp->NeI = HspVarProcOperatorCast( detail::HspVarTemplate_CmpI< CmpI, std::not_equal_to<int> > );
	return;
}

// ��r�֐������ׂĒ�`����
template<compare_func_t CmpI>
static void HspVarTemplate_InitCmpI_Full(HspVarProc* hvp)
{
	HspVarTemplate_InitCmpI_Equality< CmpI >(hvp);

	hvp->LtI   = HspVarProcOperatorCast(detail::HspVarTemplate_CmpI< CmpI, std::less<int> > );
	hvp->GtI   = HspVarProcOperatorCast(detail::HspVarTemplate_CmpI< CmpI, std::greater<int> > );
	hvp->LtEqI = HspVarProcOperatorCast(detail::HspVarTemplate_CmpI< CmpI, std::less_equal<int> > );
	hvp->GtEqI = HspVarProcOperatorCast(detail::HspVarTemplate_CmpI< CmpI, std::greater_equal<int> > );
	return;
}

} // namespace hpimod

#endif
