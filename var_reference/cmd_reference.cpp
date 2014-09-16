// reference - Command code

#include "vt_reference.h"
#include "cmd_reference.h"

#include "mod_makepval.h"
#include "mod_argGetter.h"
#include "mod_func_result.h"

//#########################################################
//        ����
//#########################################################
//------------------------------------------------
// reference ����
//------------------------------------------------
static void ReferenceNew( PVal* pvDst, PVal* pvSrc )
{
	exinfo->HspFunc_dim( pvDst, g_vtReference, 0, 1, 0, 0, 0 );
	pvDst->master   = pvSrc;
	pvDst->arraymul = ( pvSrc->arraycnt != 0 ? pvSrc->offset : 0 );
	return;
}

//------------------------------------------------
// �\�z (dim)
//------------------------------------------------
void ReferenceNew()
{
	PVal* const pvDst = code_getpval();
	PVal* const pvSrc = code_get_var();
	return ReferenceNew( pvDst, pvSrc );
}

//------------------------------------------------
// �j��
//------------------------------------------------
void ReferenceDelete()
{
	PVal* pval = code_getpval();
	if ( pval->flag != g_vtReference ) puterror( HSPERR_TYPE_MISMATCH );
	
	PVal_free( pval );
	PVal_init( pval, HSPVAR_FLAG_INT );
	return;
}

//------------------------------------------------
// member_of
//------------------------------------------------
void ReferenceMemberOf()
{
	PVal* const pvDst = code_getpval();
	
	// ���o�����̃C���X�^���X
	PVal* const pvInst = code_get_var();
	if ( pvInst->flag != HSPVAR_FLAG_STRUCT ) puterror( HSPERR_TYPE_MISMATCH );
	
	const auto* const fv     = reinterpret_cast<FlexValue*>( GetHvp(HSPVAR_FLAG_STRUCT)->GetPtr(pvInst) );
	const void* const member = fv->ptr;
	
	// �Ή����郁���o�ϐ�
	const STRUCTPRM* const pStPrm = code_get_stprm();
//	const STRUCTDAT* const modcls = &ctx->mem_finfo[ pStPrm->subid ];
	if ( pStPrm->subid != ctx->mem_minfo[ fv->customid ].subid ) puterror( HSPERR_INVALID_STRUCT_SOURCE );	// �N���X���Ⴄ
	
	assert( pStPrm->mptype == MPTYPE_LOCALVAR );		// modcls �� stprm �͂��ׂă��[�J���ϐ�
	PVal* const pvMember = reinterpret_cast<PVal*>( &((char*)member)[ pStPrm->offset ] );
	
	// �����o�ϐ��̓Y������
	if ( *type == TYPE_MARK && *val == '(' ) {
		code_next();
		code_expand_index_impl_lhs( pvMember );
		code_next_expect( TYPE_MARK, ')' );
	} else {
		if ( PVal_supportArray(pvMember) && !(pvMember->support & HSPVAR_SUPPORT_ARRAYOBJ) ) {
			HspVarCoreReset(pvMember);		//�@�Y��������
		}
	}
	
	// ���t�@�����X��
	ReferenceNew( pvDst, pvMember );
	
	return;
}
