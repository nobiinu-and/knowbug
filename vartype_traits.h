#ifndef IG_VARTYPE_TRAITS_H
#define IG_VARTYPE_TRAITS_H

#include "hsp3plugin_custom.h"

namespace hpimod
{

//------------------------------------------------
// �ϐ��^�̓���
//------------------------------------------------
namespace VtTraits
{
	namespace Impl
	{
		//------------------------------------------------
		// ���̌^
		//------------------------------------------------
		template<typename Tag> struct value_type;
		template<typename Tag> struct const_value_type;
		//{ using type = value_type<Tag>::type const; };

		//------------------------------------------------
		// ���̃|�C���^�^ (PDAT* �ƌ݊�)
		//------------------------------------------------
		template<typename Tag> struct valptr_type;
		template<typename Tag> struct const_valptr_type;

		//------------------------------------------------
		// �}�X�^�[�^ (PVal::master �̌^�ƌ݊�)
		//------------------------------------------------
		template<typename Tag> struct master_type {
			using type = void*;
		};

		//------------------------------------------------
		// �x�[�X�T�C�Y
		//------------------------------------------------
		template<typename Tag> struct basesize;

		//------------------------------------------------
		// �^�^�C�v�l (��Ԃ��֐�)
		//------------------------------------------------
		template<typename Tag> static vartype_t vartype();
	}

	//------------------------------------------------
	// alias
	//------------------------------------------------
	template<typename Tag> using value_t = typename Impl::value_type<Tag>::type;
	template<typename Tag> using const_value_t = typename Impl::const_value_type<Tag>::type;
	template<typename Tag> using valptr_t       = typename Impl::valptr_type<Tag>::type;
	template<typename Tag> using const_valptr_t = typename Impl::const_valptr_type<Tag>::type;
	template<typename Tag> using master_t       = typename Impl::master_type<Tag>::type;

	template<typename Tag> using basesize = Impl::basesize<Tag>;	// �ϐ��e���v���[�g���Ȃ��̂� ::value �͊O���Ȃ�

	//------------------------------------------------
	// value[] �̌^�A�̃C���^�[�t�F�[�X�I�Ȃ���
	//
	// ���̕��@�ł́A�^�O�͍��X1�̃C���^�[�t�F�[�X�������ĂȂ��B
	// �ꉞ�΍�͂��邪���^�v���O���~���O���C�u�������K�v�ɂȂ�̂ł߂�ǂ������B
	// using Int = VartypeTag<NativeVartypeTag<Int>, InternalTag<Int>>; �Ƃ��Ă����āA
	// �������ꉻ�� VartypeTag<Attrs...> �Ŏ󂯁A�Ή����鑮���� Attrs... �̒��ɂ��邩�ǂ����ɂ��� SFINAE �ŏꍇ�����B
	// �܂�����̂����� int �^�̃^�O�� int �ɂ���Ƃ������Ƃ��ł��Ȃ��B
	//------------------------------------------------
	template<typename TValue>
	struct NativeVartypeTag { };

	namespace Impl
	{
		template<typename T> struct value_type<NativeVartypeTag<T>> {
			using type = T;
		};
		template<typename T> struct const_value_type<NativeVartypeTag<T>> {
			using type = T const;
		};
		template<typename T> struct valptr_type<NativeVartypeTag<T>> {
			using type = T*;
		};
		template<typename T> struct const_valptr_type<NativeVartypeTag<T>> {
			using type = T const*;
		};
		template<typename T> struct basesize<NativeVartypeTag<T>> {
			static int const value = sizeof(T);
		};
	}

	//------------------------------------------------
	// �^�Ɋւ���e��֐�
	//------------------------------------------------

	// �Œ蒷�^���H
	template<typename Tag>
	struct isFixed { static bool const value = (basesize<Tag>::value >= 0); };
	
	// PDAT* �� ���̃|�C���^
	template<typename Tag>
	static const_valptr_t<Tag> asValptr(PDAT const* pdat) {
		return reinterpret_cast<const_valptr_t<Tag>>(pdat);
	}
	template<typename Tag>
	static valptr_t<Tag> asValptr(PDAT* pdat) { return const_cast<valptr_t<Tag>>(asValptr<Tag>(static_cast<PDAT const*>(pdat))); }

	// ���̃|�C���^ �� PDAT*
	template<typename Tag>
	static PDAT const* asPDAT(const_valptr_t<Tag> p) {
		return reinterpret_cast<PDAT const*>(p);
	}
	template<typename Tag>
	static PDAT* asPDAT(valptr_t<Tag> p) { return const_cast<PDAT*>(asPDAT<Tag>(static_cast<const_valptr_t<Tag>>(p))); }

	// PVal::master �̃L���X�g
	// todo: const_master_t ���K�v�H
	template<typename Tag>
	static master_t<Tag> const& getMaster(PVal const* pval) {
		return *reinterpret_cast<master_t<Tag> const*>(&pval->master);
	}
	template<typename Tag>
	static master_t<Tag>& getMaster(PVal* pval) { return const_cast<master_t<Tag>&>(getMaster<Tag>(static_cast<PVal const*>(pval))); }
	
	// ���̃|�C���^�̒E�Q�� (valptr_t = value_t* �ł���^�Ɍ���)
	template<typename Tag> static const_value_t<Tag>& derefValptr(PDAT const* pdat) {
		static_assert(std::is_same<valptr_t<Tag>, value_t<Tag>*>::value, "General 'derefValptr()' can be used for vartypes (valptr_t = value_t*).");
		return *asValptr<Tag>(pdat);
	}
	template<typename Tag>
	static value_t<Tag>& derefValptr(PDAT* pdat) { return const_cast<value_t<Tag>&>(derefValptr<Tag>(static_cast<PDAT const*>(pdat))); }

	//------------------------------------------------
	// �g�ݍ��݌^�̃^�O
	//------------------------------------------------
	namespace InternalVartypeTags
	{
		using vtLabel  = NativeVartypeTag<label_t>;
		using vtDouble = NativeVartypeTag<double>;
		using vtInt    = NativeVartypeTag<int>;
		using vtStruct = NativeVartypeTag<FlexValue>;

		struct vtStr { };
	}
	using namespace InternalVartypeTags;

	// �^�^�C�v�l
	namespace Impl
	{
		template<> static vartype_t vartype<vtLabel>()  { return HSPVAR_FLAG_LABEL; }
		template<> static vartype_t vartype<vtStr>()    { return HSPVAR_FLAG_STR; }
		template<> static vartype_t vartype<vtDouble>() { return HSPVAR_FLAG_DOUBLE; }
		template<> static vartype_t vartype<vtInt>()    { return HSPVAR_FLAG_INT; }
		template<> static vartype_t vartype<vtStruct>() { return HSPVAR_FLAG_STRUCT; }
	}

	// str �^�̓����̒�`
	namespace Impl
	{
		template<> struct value_type<vtStr>  { using type = char*; };
		template<> struct const_value_type<vtStr>  { using type = char const*; };
		template<> struct valptr_type<vtStr> { using type = char*; };
		template<> struct const_valptr_type<vtStr> { using type = char const*; };
		template<> struct master_type<vtStr> { using type = char**; };
		template<> struct basesize<vtStr> { static int const value = -1; };
	}

	// str �^�̃I�[�o�[���C�h

	// ���̃|�C���^�̒E�Q��: deref �͍��Ӓl��Ԃ����A���̃|�C���^���獶�Ӓl�𓾂��Ȃ��̂Œ�`�ł��Ȃ��B
	//template<> static inline const_value_t<vtStr>& derefValptr<vtStr>(PDAT const* pdat) { };
} // namespace VtTraits

//------------------------------------------------
// NativeVartype ��p�̊֐�
//------------------------------------------------
namespace VtTraits
{
	// NativeVartypeTag ���ǂ���
	template<typename Tag> struct isNativeVartype { static bool const value = false; };
	template<typename TVal> struct isNativeVartype<NativeVartypeTag<TVal>> { static bool const value = true; };

	// �ϐ�������̃|�C���^�𓾂�
	template<typename Tag> static const_valptr_t<Tag> getValptr(PVal const* pval) {
		static_assert(isNativeVartype<Tag>::value, "'getValptr' for non-NativeVartype types is undefined.");
		assert(pval->flag == Impl::vartype<Tag>());
		return asValptr<Tag>(pval->pt) + pval->offset;
	}
	template<typename Tag>
	static valptr_t<Tag> getValptr(PVal* pval) { return const_cast<valptr_t<Tag>>(getValptr<Tag>(static_cast<PVal const*>(pval))); }
} // namespace VtTraits

using namespace VtTraits::InternalVartypeTags;

} // namespace hpimod

#endif
