// vartype - functor

#ifndef IG_VARTYPE_FUNCTOR_H
#define IG_VARTYPE_FUNCTOR_H

#include "hsp3plugin_custom.h"
#include "mod_func_result.h"
#include "vp_template.h"

#include "Functor.h"

// �ϐ�
extern vartype_t g_vtFunctor;
extern HspVarProc* g_hvpFunctor;

// �֐�
extern void HspVarFunctor_init(HspVarProc* vp);

// vartype tag
struct functor_tag
	: public NativeVartypeTag<functor_t>
{
	static vartype_t vartype() { return g_vtFunctor; }
};

// VtTraits<> �̓��ꉻ
namespace hpimod
{
	template<> struct VtTraits<functor_tag> : public VtTraitsBase<functor_tag>
	{
	};
}

using FunctorTraits = hpimod::VtTraits<functor_tag>;

// �Ԓl�ݒ�֐�
extern functor_t g_resFunctor;		// �I�����A�ÓI�ϐ��Ȃǂ���ɉ�̂���
static int SetReffuncResult( void** ppResult, functor_t const& src );

#endif
