// vartype - functor

#include "vt_functor.h"
#include "cmd_call.h"
#include "cmd_sub.h"
#include "iface_call.h"

#include "CCaller.h"
#include "CBound.h"
#include "axcmd.h"

#include "hsp3plugin_custom.h"
#include "mod_argGetter.h"

#include <new>

using namespace hpimod;

// �ϐ��̐錾
vartype_t HSPVAR_FLAG_FUNCTOR;
HspVarProc* g_pHvpFunctor;

functor_t g_resFunctor;

// �֐��錾 ( ���ׂĎ�������Ă����ł͂Ȃ� )
static PDAT* HspVarFunctor_getptr    (PVal* pval);
static int   HspVarFunctor_getSize (PDAT const* pdat);
static int   HspVarFunctor_getUsing(PDAT const* pdat);
static void* HspVarFunctor_getBlockSize(PVal* pval, PDAT* pdat, int* size);
static void  HspVarFunctor_allocBlock  (PVal* pval, PDAT* pdat, int  size);
static void  HspVarFunctor_alloc(PVal* pval, PVal const* pval2);
static void  HspVarFunctor_free (PVal* pval);
static void* HspVarFunctor_cnv      (void const* buffer, int flag);
static void* HspVarFunctor_cnvCustom(void const* buffer, int flag);

static void* HspVarFunctor_arrayObjectRead(PVal* pval, int* mptype);
static void  HspVarFunctor_arrayObject(PVal* pval);
static void  HspVarFunctor_objectWrite(PVal* pval, void* data, int vflag);
static void  HspVarFunctor_method     (PVal* pval);

static void  HspVarFunctor_set(PVal* pval, PDAT* pdat, void const* in);
static void  HspVarFunctor_eqI  (PDAT* pdat, void const* val);
static void  HspVarFunctor_neI  (PDAT* pdat, void const* val);
static void  HspVarFunctor_gtI  (PDAT* pdat, void const* val);
static void  HspVarFunctor_ltI  (PDAT* pdat, void const* val);
static void  HspVarFunctor_gtEqI(PDAT* pdat, void const* val);
static void  HspVarFunctor_ltEqI(PDAT* pdat, void const* val);
static void  HspVarFunctor_rrI  (PDAT* pdat, void const* val);
static void  HspVarFunctor_lrI  (PDAT* pdat, void const* val);

// �T�|�[�g�֐�
static functor_t* GetFunctorPtr( PVal* pval );
static functor_t* GetFunctorPtr( PDAT* pdat );
static void Call_functorVar(PVal* pval, CCaller& caller);
static void Call_functorVar(functor_t* pstFunctor, CCaller& caller);

//------------------------------------------------
// ���̃|�C���^�𓾂�
//------------------------------------------------
static PDAT* HspVarFunctor_getptr( PVal* pval )
{
	return (PDAT*)GetFunctorPtr(pval);
}

//------------------------------------------------
// �v�f���m�ۂ��Ă���T�C�Y���擾
//------------------------------------------------
static int HspVarFunctor_getSize( PDAT const* pdat )
{
	return sizeof(functor_t);
}

//------------------------------------------------
// �g�p��(varuse)
//------------------------------------------------
static int HspVarFunctor_getUsing( PDAT const* pdat )
{
	functor_t* pstFunctor = GetFunctorPtr( (PDAT*)pdat );
	return pstFunctor->getUsing();
}

//------------------------------------------------
// �u���b�N�T�C�Y
//------------------------------------------------
static void* HspVarFunctor_getBlockSize(PVal* pval, PDAT* pdat, int* size)
{
	*size = pval->size - ( (char*)pdat - (char*)pval->pt );
	return (pdat);
}

static void HspVarFunctor_allocBlock(PVal* pval, PDAT* pdat, int size)
{
	return;
}

//------------------------------------------------
// PVal�̕ϐ����������m�ۂ���
// 
// @ pval �͖��m�� or ����ς݁B
// @ pval2 != NULL => pval2 ���p���B
// @ �z��͈ꎟ���̂݁B
//------------------------------------------------
static void HspVarFunctor_alloc(PVal* pval, PVal const* pval2)
{
	if ( pval->len[1] < 1 ) pval->len[1] = 1;		// �z����Œ� 1 �͊m�ۂ���
	pval->len[2] = 0;
	pval->len[3] = 0;
	pval->len[4] = 0;

	int cntElems = pval->len[1];					// PVal_cntElems( pval );
	int size     = cntElems * sizeof(functor_t);
	int offset   = 0;

	functor_t* pt = (functor_t*)hspmalloc( size );

	// �p��
	if ( pval2 ) {
		offset = ( pval2->size / sizeof(functor_t) );
		memcpy( pt, pval2->pt, pval2->size );		// �����Ă����f�[�^���R�s�[
		hspfree( pval2->pt );						// ���̃o�b�t�@�����
	}

	// functor_t[] �̏�����
	for ( int i = offset; i < cntElems; ++ i ) {
		functor_t* p = new( &pt[i] ) functor_t;
		if ( p != &pt[i] ) dbgout("(%p, %p)", p, &pt[i] );
	}

	pval->flag   = HSPVAR_FLAG_FUNCTOR;
	pval->mode   = HSPVAR_MODE_MALLOC;
	pval->size   = size;
	pval->pt     = (char*)pt;
	pval->master = nullptr;
	return;
}

//------------------------------------------------
// PVal�̕ϐ����������������
//------------------------------------------------
static void HspVarFunctor_free(PVal* pval)
{
	if ( pval->mode == HSPVAR_MODE_MALLOC ) {
		// �f�X�g���N�^�N��
		functor_t* const pt = (functor_t*)pval->pt;
		for ( int i = 0; i < pval->len[1]; ++ i ) {
			pt[i].~CFunctor();
		}

		hspfree( pval->pt );
	}

	pval->mode = HSPVAR_MODE_NONE;
	pval->pt   = nullptr;
	return;
}

//------------------------------------------------
// �^�ϊ�����
// 
// @ �� -> functor
// @ g_resFunctor �� SetReffuncResult �Ƌ��p�B
//------------------------------------------------
static void* HspVarFunctor_cnv(void const* buffer, int flag)
{
	static CFunctor& stt_cnv = g_resFunctor;

	switch ( flag ) {
		case HSPVAR_FLAG_LABEL:
			stt_cnv = CFunctor( *(label_t*)buffer );
			break;

		case HSPVAR_FLAG_INT:
		{
			stt_cnv = CFunctor( *(int*)buffer );
			break;
		}
		default:
			if ( flag == HSPVAR_FLAG_FUNCTOR ) {
				stt_cnv = *(functor_t*)buffer;

			} else {
				puterror( HSPERR_TYPE_MISMATCH );
			}
			break;
	}

	return &stt_cnv;
}

//------------------------------------------------
// �^�ϊ�����
// 
// @ functor -> ��
//------------------------------------------------
static void* HspVarFunctor_cnvCustom(void const* buffer, int flag)
{
	functor_t* pstFunctor = (functor_t*)buffer;

	switch ( flag ) {
		case HSPVAR_FLAG_LABEL:
		{
			static label_t stt_label;

			stt_label = pstFunctor->getLabel();
			if ( stt_label ) return &stt_label;

			break;
		}
		case HSPVAR_FLAG_INT:
		{
			static int stt_int;

			stt_int = pstFunctor->getAxCmd();
			if ( AxCmd::getType(stt_int) == TYPE_MODCMD ) return &stt_int;

			break;
		}

		default:
			if ( flag == HSPVAR_FLAG_FUNCTOR ) {
				return (void*)buffer;
			}

			break;
	}

	puterror( HSPERR_TYPE_MISMATCH );
	throw;	// (�x���}��)
}

//*
//------------------------------------------------
// �A�z�z�� : �Q�� (�E)
// 
// @ �P�� => �֐��`���Ăяo��
// @ �z�� => �v�f�擾
//------------------------------------------------
static void* HspVarFunctor_arrayObjectRead( PVal* pval, int* mptype )
{
	// �z�� => �Y���ɑΉ�����v�f�� functor �l�����o��
	if ( pval->len[1] != 1 ) {
		int idx = code_geti();
		code_index_int_rhs( pval, idx );

		*mptype = HSPVAR_FLAG_FUNCTOR;
		return GetFunctorPtr( pval );
	}

	// �Ăяo��
	functor_t* pstFunctor = GetFunctorPtr( pval );

	// ��Ăяo���Y�� (�o�O�ւ̑΍�)
	if ( *type == g_pluginType_call && *val == CallCmdId::NoCall ) {
		code_next();
		if ( code_isNextArg() ) puterror( HSPERR_SYNTAX );
		*mptype = HSPVAR_FLAG_FUNCTOR;
		return pstFunctor;
	}
	if ( pstFunctor->getUsing() == 0 ) puterror( HSPERR_ILLEGAL_FUNCTION );	// ����

	// �Ăяo��
	{
		CCaller caller;

		Call_functorVar( pstFunctor, caller );

		// �Ԓl���󂯎��
		void* result = nullptr;
		*mptype = caller.getCallResult( &result );

		if ( *mptype == HSPVAR_FLAG_NONE || !result ) puterror( HSPERR_NORETVAL );
		return result;
	}
}
//*/

//*
//------------------------------------------------
// �A�z�z�� : �Q�� (��)
//------------------------------------------------
static void HspVarFunctor_arrayObject( PVal* pval )
{
	int idx = code_geti();
	code_index_int_lhs( pval, idx );

	return;
}
//*/

//*
//------------------------------------------------
// �i�[����
//------------------------------------------------
static void HspVarFunctor_objectWrite( PVal* pval, void* data, int vflag )
{
	functor_t* pstFunctor = GetFunctorPtr( pval );
	*pstFunctor = *(functor_t*)HspVarFunctor_cnv(data, vflag);

	// �A�����
	code_assign_multi( pval );
	return;
}
//*/

//------------------------------------------------
// ���\�b�h����
//------------------------------------------------
static void HspVarFunctor_method(PVal* pval)
{
	char const* const psMethod = code_gets();

	if ( !strcmp(psMethod, "call") ) {
		CCaller caller;

		Call_functorVar( pval, caller );

	} else {
		puterror( HSPERR_ILLEGAL_FUNCTION );
	}
	return;
}

//------------------------------------------------
// ����֐�
//------------------------------------------------
static void  HspVarFunctor_set(PVal* pval, PDAT* pdat, void const* in)
{
	*(functor_t*)pdat = *(functor_t*)in;
	return;
}

//------------------------------------------------
// ��r�֐�
//------------------------------------------------
static void HspVarFunctor_eqI(PDAT* pdat, void const* val)
{
	functor_t* pstFunctor[2] = {
		((functor_t*)pdat), 
		((functor_t*)val)
	};

	bool bEq = pstFunctor[0] == pstFunctor[1]				// ����
		|| (   pstFunctor[0] != nullptr
			&& pstFunctor[1] != nullptr
			&& pstFunctor[0]->compare( *pstFunctor[1] ) == 0
		)
	;

	*((int*)pdat)            = bEq ? 1 : 0;
	g_pHvpFunctor->aftertype = HSPVAR_FLAG_INT;
	return;
}

// !=
static void HspVarFunctor_neI(PDAT* pdat, void const* val)
{
	HspVarFunctor_eqI(pdat, val);

	*((int*)pdat) ^= 1;
	return;
}

//------------------------------------------------
// HspVarProc�������֐�
//------------------------------------------------
void HspVarFunctor_init(HspVarProc* p)
{
	HSPVAR_FLAG_FUNCTOR = p->flag;
	g_pHvpFunctor       = p;

	p->GetPtr       = HspVarFunctor_getptr;
	p->GetSize      = HspVarFunctor_getSize;
	p->GetUsing     = HspVarFunctor_getUsing;

	p->GetBlockSize = HspVarFunctor_getBlockSize;
	p->AllocBlock   = HspVarFunctor_allocBlock;

	p->Cnv          = HspVarFunctor_cnv;
	p->CnvCustom    = HspVarFunctor_cnvCustom;

	p->Alloc        = HspVarFunctor_alloc;
	p->Free         = HspVarFunctor_free;

	p->ArrayObjectRead = HspVarFunctor_arrayObjectRead;
	p->ArrayObject  = HspVarFunctor_arrayObject;
	p->ObjectWrite  = HspVarFunctor_objectWrite;
	p->ObjectMethod = HspVarFunctor_method;

	p->Set          = HspVarFunctor_set;

//	p->AddI         = HspVarFunctor_addI;
//	p->SubI         = HspVarFunctor_subI;
//	p->MulI         = HspVarFunctor_mulI;
//	p->DivI         = HspVarFunctor_divI;
//	p->ModI         = HspVarFunctor_modI;

//	p->AndI         = HspVarFunctor_andI;
//	p->OrI          = HspVarFunctor_orI;
//	p->XorI         = HspVarFunctor_xorI;

	p->EqI          = HspVarFunctor_eqI;
	p->NeI          = HspVarFunctor_neI;
//	p->GtI          = HspVarFunctor_gtI;
//	p->LtI          = HspVarFunctor_ltI;
//	p->GtEqI        = HspVarFunctor_gtEqI;
//	p->LtEqI        = HspVarFunctor_ltEqI;

//	p->RrI          = HspVarFunctor_rrI;
//	p->LrI          = HspVarFunctor_lrI;

	p->vartype_name	= "functor";			// �^��
	p->version      = 0x001;				// VarType RuntimeVersion(0x100 = 1.0)
	p->support      = HSPVAR_SUPPORT_STORAGE
					| HSPVAR_SUPPORT_FLEXARRAY
					| HSPVAR_SUPPORT_ARRAYOBJ
					| HSPVAR_SUPPORT_NOCONVERT
	                | HSPVAR_SUPPORT_VARUSE
					;						// �T�|�[�g�󋵃t���O(HSPVAR_SUPPORT_*)
	p->basesize     = sizeof(functor_t);	// 1�̃f�[�^��bytes / �ϒ��̎���-1
	return;
}

//##############################################################################
//                �������֐�
//##############################################################################
//------------------------------------------------
// PVal, PDAT ���� functor_t �|�C���^�𓾂�
// 
// @ �^�s���S
// @ �L���X�g����
//------------------------------------------------
static functor_t* GetFunctorPtr( PVal* pval )
{
	if ( pval->flag != HSPVAR_FLAG_FUNCTOR ) {
		return nullptr;
	} else {
		return ((functor_t*)pval->pt) + pval->offset;
	}
}

static functor_t* GetFunctorPtr( PDAT* pdat )
{
	return (functor_t*)pdat;
}

//------------------------------------------------
// functor �I�u�W�F�N�g�� call ����
// 
// @ �������̓R�[�h������o��
//------------------------------------------------
static void Call_functorVar( PVal* pval, CCaller& caller )
{
	Call_functorVar( GetFunctorPtr(pval), caller );
	return;
}

static void Call_functorVar( functor_t* pstFunctor, CCaller& caller )
{
	caller.setFunctor( *pstFunctor );
	caller.setArgAll();
	caller.call();
	return;
}
