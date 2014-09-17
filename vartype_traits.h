#ifndef IG_VARTYPE_TRAITS_H
#define IG_VARTYPE_TRAITS_H

#include "hsp3plugin_custom.h"

namespace hpimod
{

//------------------------------------------------
// traits
//------------------------------------------------

// pt = TValue[] �ł���悤�ȑf�p�Ȍ^�� tag
template<typename TValue>
struct NativeVartypeTag
{
	using value_t = TValue;
	using valptr_t = TValue*;
	using const_valptr_t = TValue const*;
	using master_t = void*;

	static int const basesize = sizeof(TValue);
	static vartype_t vartype();

	// �ϐ�������̃|�C���^�𓾂�
	static const_valptr_t& getValptr(PVal const* pval) {
		assert(pval->flag == vartype());
		return &reinterpret_cast<const_valptr_t>(pval->pt)[pval->offset];
	}
	static valptr_t& getValptr(PVal* pval) { return const_cast<valptr_t&>(getValptr(static_cast<PVal const*>(pval))); }
};

// str �^�̂悤�Ȍ^�� tag
// master = TValue[], pt = master[0] �ƂȂ�B
template<typename TValue, typename TConstValue, int TBasesize>
struct StrLikeVartypeTag
{
	using value_t = TValue;
	using valptr_t = TValue;
	using const_valptr_t = TConstValue;

	using master_t = TValue*;

	static int const basesize = TBasesize;
};

// �g�ݍ��݌^�̕ϐ��^�^�O
namespace detail
{
	// �^�^�C�v�l�͒萔���ŗ^������
	template<vartype_t NVartype, typename TagSuper>
	struct InternalVartypeTag : public TagSuper {
		static vartype_t vartype() { return NVartype; }
	};
}

using label_tag  = detail::InternalVartypeTag< HSPVAR_FLAG_LABEL,  NativeVartypeTag<label_t> >;
using double_tag = detail::InternalVartypeTag< HSPVAR_FLAG_DOUBLE, NativeVartypeTag<double> >;
using int_tag    = detail::InternalVartypeTag< HSPVAR_FLAG_INT,    NativeVartypeTag<int> >;
using struct_tag = detail::InternalVartypeTag< HSPVAR_FLAG_STRUCT, NativeVartypeTag<FlexValue> >;

using str_tag    = detail::InternalVartypeTag< HSPVAR_FLAG_STR, StrLikeVartypeTag<char*, char const*, (-1)> >;

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
	using value_t = typename VartypeTag::value_t;
	using valptr_t = typename VartypeTag::valptr_t;
	using const_valptr_t = typename VartypeTag::const_valptr_t;
	using master_t = typename VartypeTag::master_t;

	static int const basesize = VartypeTag::basesize;

	// �Œ蒷�^���H
	static bool const isFixed_v = (VartypeTag::basesize >= 0);

	// ���̃|�C���^�� PDAT* �̑��݃L���X�g
	static inline const_valptr_t asValptr(PDAT const* pdat) {
		return reinterpret_cast<const_valptr_t>(pdat);
	}
	static inline valptr_t asValptr(PDAT* pdat) { return const_cast<valptr_t>(asValptr(static_cast<PDAT const*>(pdat))); }

	static inline PDAT const* asPDAT(const_valptr_t p) {
		return reinterpret_cast<PDAT const*>(p);
	}
	static inline PDAT* asPDAT(valptr_t p) { return const_cast<PDAT*>(asPDAT(static_cast<const_valptr_t>(p))); }

	// ���̃|�C���^�̒E�Q��
	static inline value_t const& derefValptr(PDAT const* pdat)
	{
		static_assert(std::is_same<value_t*, valptr_t>::value, "����� derefValptr �� valptr_t = value_t* �ƂȂ�ϐ��^�ɂ̂ݎg�p�ł���B");
		return *asValptr(pdat);
	}
	static inline value_t& derefValptr(PDAT* pdat)
	{ return const_cast<value_t&>(derefValptr(static_cast<PDAT const*>(pdat))); }

	// PVal::master �̃L���X�g
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

template<> struct VtTraits<label_t> : public VtTraits<label_tag> {};
template<> struct VtTraits<double>  : public VtTraits<double_tag> {};
template<> struct VtTraits<int>     : public VtTraits<int_tag> {};

} // namespace hpimod

#endif
