// vartype - functor

#ifndef IG_VARTYPE_FUNCTOR_H
#define IG_VARTYPE_FUNCTOR_H

#include "hsp3plugin_custom.h"
#include "mod_func_result.h"

#include "CFunctor.h"

// �ϐ��錾
extern vartype_t HSPVAR_FLAG_FUNCTOR;
extern HspVarProc* g_pHvpFunctor;

// �֐��錾
extern void HspVarFunctor_init(HspVarProc* vp);

// PVal::pt �ɔz���ɊǗ������I�u�W�F�N�g
using functor_t = CFunctor;

// �Ԓl�ݒ�֐�
extern functor_t g_resFunctor;		// �I�����A�ÓI�ϐ��Ȃǂ���ɉ�̂���
static int SetReffuncResult( void** ppResult, functor_t const& src )
{
	g_resFunctor = src;
	*ppResult = &g_resFunctor;
	return HSPVAR_FLAG_FUNCTOR;
}
//FTM_SetReffuncResult( functor_t, HSPVAR_FLAG_FUNCTOR );

#endif
