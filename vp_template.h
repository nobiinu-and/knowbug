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
// traits
//------------------------------------------------
// pt = TValue[] �ł���悤�ȑf�p�Ȍ^�� tag
template<typename TValue, typename TMaster = void*>
struct NativeVartypeTag
{
	using value_t = TValue;
	using valptr_t = TValue*;
	using const_valptr_t = TValue const*;
	using master_t = TMaster;

	static int const basesize = sizeof(TValue);
};

// str �^�̂悤�Ȍ^�� tag
// master = TValue[], pt = master[0] �ƂȂ�B
template<typename TValue, int TBasesize>
struct StrLikeVartypeTag
{
	using value_t  = TValue;
	using valptr_t = TValue;
	using const_valptr_t = TValue const;

	using master_t = TValue*;

	static int const basesize = TBasesize;
};

// �g�ݍ��݌^�̕ϐ��^�^�O

using label_tag  = NativeVartypeTag<label_t>;
using double_tag = NativeVartypeTag<double>;
using int_tag    = NativeVartypeTag<int>;
using struct_tag = NativeVartypeTag<FlexValue>;

using str_tag = StrLikeVartypeTag<char*, (-1)>;

//------------------------------------------------
// VartypeTag �̒��g���Q�Ƃ���C���^�[�t�F�[�X
//
// (�utypename�v�n��������邽��)
//------------------------------------------------
// �x�[�X
template<typename VartypeTag>
struct VtTraitsBase
	: public VartypeTag
{
	using value_t        = typename VartypeTag::value_t;
	using master_t       = typename VartypeTag::master_t;
	using valptr_t       = typename VartypeTag::valptr_t;
	using const_valptr_t = typename VartypeTag::const_valptr_t;

	static int const basesize = VartypeTag::basesize;

	// �Œ蒷�^���H
	static bool const isFixed_v = (VartypeTag::basesize >= 0);

	// ���̃|�C���^�̃L���X�g
	static inline const_valptr_t asValptr(void const* pdat) {
		return reinterpret_cast<const_valptr_t>(pdat);
	}
	static inline valptr_t asValptr(void* pdat) { return const_cast<valptr_t>(asValptr(static_cast<void const*>(pdat))); }

	static inline PDAT const* asPDAT(const_valptr_t p) {
		return reinterpret_cast<PDAT const*>(p);
	}
	static inline PDAT* asPDAT(valptr_t p) { return const_cast<PDAT*>(asPDAT(static_cast<const_valptr_t>(p))); }

	// ���̃|�C���^�̒E�Q��
	static inline value_t const& derefValptr(void const* pdat)
	{
		return *asValptr(pdat);
	}
	static inline value_t& derefValptr(void* pdat)
	{ return const_cast<value_t&>(derefValptr(static_cast<void const*>(pdat))); }

	static void derefValptr(PVal const* pval) { static_assert(false, "derefValptr(PVal*)...really?"); }

	// master �|�C���^�̃L���X�g
	static inline master_t& getMaster(PVal* pval) {
		return *reinterpret_cast<master_t*>(&pval->master);
	}
};

// ���ۂɎg����
// ���ꉻ����̂��y�ɂȂ�悤�ɁA���̂� VtTraitsBase �ɏo���Ă���
template<typename VartypeTag>
struct VtTraits
	: public VtTraitsBase<VartypeTag>
{ };

// ���L�̂��߂̓��ꉻ

template<> struct VtTraits<double>
	: public VtTraits<double_tag> { };
template<> struct VtTraits<int>
	: VtTraits<int_tag> { };

//------------------------------------------------
// ���̃|�C���^�𓾂�
//
// @ pt �� valptr_array �ł���ꍇ
//------------------------------------------------
template<typename VartypeTag>
static PDAT* HspVarTemplate_GetPtr(PVal* pval)
{
	assert(PVal_supportArray(pval));
	return reinterpret_cast<PDAT*>(VtTraits<VartypeTag>::asValptr(pval) + pval->offset);
}

//------------------------------------------------
// �ϒ��^�ɓ��L�̊֐��́A�Œ蒷�^�̏ꍇ
//------------------------------------------------
template<typename VartypeTag>
static int HspVarTemplate_GetSize(PDAT const* pdat)
{
	static_assert(VtTraits<VartypeTag>::isFixed_v, "");
	return VtTraits<VartypeTag>::basesize;
}

template<typename VartypeTag>
static void* HspVarTemplate_GetBlockSize(PVal* pval, PDAT* pdat, int* size)
{
	static_assert(VtTraits<VartypeTag>::isFixed_v, "");
	*size = VtTraits<VartypeTag>::basesize * PVal_cntElems(pval);
	return pdat;
}

template<typename VartypeTag>
static void HspVarTemplate_AllocBlock(PVal* pval, PDAT* pdat, int size)
{
	static_assert(VtTraits<VartypeTag>::isFixed_v, "");
}

//------------------------------------------------
// ��r�֐��̋�̉�
//
// @ HspVar**_CmpI �Ƃ���1�̊֐����牉�Z�֐��𐶐�����B
// @ aftertype �͂��̒��Őݒ肷��K�v������B
//------------------------------------------------
using compare_func_t = int(*)(PDAT* pdat, void const* val);

namespace detail
{
template<compare_func_t CmpI, typename TCmpFunctor>
static void HspVarTemplate_CmpI(PDAT* pdat, void const* val)
{
	static TCmpFunctor comparer {};

	VtTraits<int>::derefValptr(pdat) = HspBool( comparer(CmpI(pdat, val), 0) );

//	myhvp->aftertype = HSPVAR_FLAG_INT;
	return;
}
}

// ���l���̂ݒ�`����
template<compare_func_t CmpI>
static void HspVarTemplate_InitCmpI_Equality(HspVarProc* hvp)
{
	hvp->EqI = detail::HspVarTemplate_CmpI< CmpI, std::equal_to<int> >;
	hvp->NeI = detail::HspVarTemplate_CmpI< CmpI, std::not_equal_to<int> >;
	return;
}

// ��r�֐������ׂĒ�`����
template<compare_func_t CmpI>
static void HspVarTemplate_InitCmpI_Full(HspVarProc* hvp)
{
	HspVarTemplate_InitCmpI_Equality< CmpI >(hvp);

	hvp->LtI   = detail::HspVarTemplate_CmpI< CmpI, std::less<int> >;
	hvp->GtI   = detail::HspVarTemplate_CmpI< CmpI, std::greater<int> >;
	hvp->LtEqI = detail::HspVarTemplate_CmpI< CmpI, std::less_equal<int> >;
	hvp->GtEqI = detail::HspVarTemplate_CmpI< CmpI, std::greater_equal<int> >;
	return;
}


} // namespace hpimod

#endif
