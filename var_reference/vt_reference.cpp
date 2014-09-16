// reference - VarProc code

#include "for_knowbug.var_reference.h"
#include "vt_reference.h"

#include "hsp3plugin_custom.h"
#include "mod_makepval.h"
#include "mod_argGetter.h"

// �ϐ��̒�`
short g_vtReference;
HspVarProc* g_pHvpReference;

// �֐��̐錾
extern PDAT* HspVarReference_GetPtr         ( PVal* pval) ;
extern int   HspVarReference_GetSize        ( const PDAT* pdat );
extern int   HspVarReference_GetUsing       ( const PDAT* pdat );
extern void* HspVarReference_GetBlockSize   ( PVal* pval, PDAT* pdat, int* size );
extern void  HspVarReference_AllocBlock     ( PVal* pval, PDAT* pdat, int  size );
extern void  HspVarReference_Alloc          ( PVal* pval, const PVal* pval2 );
extern void  HspVarReference_Free           ( PVal* pval);
extern void* HspVarReference_ArrayObjectRead( PVal* pval, int* mptype );
extern void  HspVarReference_ArrayObject    ( PVal* pval);
extern void  HspVarReference_ObjectWrite    ( PVal* pval, void* data, int vflag );
extern void  HspVarReference_ObjectMethod   ( PVal* pval);

//------------------------------------------------
// �A�N�Z�X�p
//------------------------------------------------
namespace VtReference {
	valptr_t GetPtr( PVal* pval )
	{
		assert( pval     != nullptr );
		assert( pval->pt != nullptr );
		return (valptr_t)pval->pt;
	}
	
	static PVal* GetPVal( const value_t& v ) {
		return reinterpret_cast<PVal*>( v.master );
	}
	
	static APTR GetAPTR( const value_t& v ) {
		return v.arraymul;
	}
	
	static PVal* GetPVal( PVal* pval ) { return GetPVal( *GetPtr(pval) ); }
	static APTR  GetAPTR( PVal* pval ) { return GetAPTR( *GetPtr(pval) ); }
}

//------------------------------------------------
// Core
//------------------------------------------------
static PDAT* HspVarReference_GetPtr( PVal* pval )
{
	return (PDAT*)( VtReference::GetPtr(pval) );
}

//------------------------------------------------
// Size
//------------------------------------------------
static int HspVarReference_GetSize( const PDAT* pdat )
{
	return VtReference::basesize;
}

//------------------------------------------------
// Using
//------------------------------------------------
static int HspVarReference_GetUsing( const PDAT* pdat )
{
	return VtReference::GetPVal(*reinterpret_cast<const VtReference::value_t*>(pdat)) != nullptr ? 1 : 0;
}

//------------------------------------------------
// �u���b�N������
//------------------------------------------------
static void* HspVarReference_GetBlockSize( PVal* pval, PDAT* pdat, int* size )
{
	*size = pval->size - ( ((char*)pdat) - ((char*)pval->pt) );
	return pdat;
}

static void HspVarReference_AllocBlock( PVal* pval, PDAT* pdat, int size )
{
	return;
}

//------------------------------------------------
// PVal�̕ϐ����������m�ۂ���
//
// @ pval �͖��m�� or ����ς݂̏�ԁB
// @ pval2 != NULL => pval2�̓��e���p������B
//------------------------------------------------
static void HspVarReference_Alloc( PVal* pval, const PVal* pval2 )
{
	// �z��Ƃ��ĎQ�Ƃł��邪�A�z��ɂ͂ł��Ȃ�
	pval->len[1] = 1;
	pval->len[2] = 0;
	
	// �p��
	if ( pval2 != NULL ) {
		pval->master   = pval2->master;
		pval->arraymul = pval2->arraymul;
		
	} else {
		pval->master   = nullptr;
		pval->arraymul = 0;		// aptr �Ƃ��Ďg��
	}
	
	// pval �֐ݒ�
	pval->flag = g_vtReference;		// reference �̌^�^�C�v�l
	pval->mode = HSPVAR_MODE_MALLOC;
	pval->size = VtReference::basesize;
	pval->pt   = (char*)&pval->master;		// �K�����g�� master �ւ̎Q�ƂƂȂ��Ă���
	return;
}

//------------------------------------------------
// PVal�̕ϐ����������������
//------------------------------------------------
static void HspVarReference_Free( PVal* pval )
{
	if ( pval->mode == HSPVAR_MODE_MALLOC ) {
		// ���\�[�X���g��Ȃ�
	}
	
	pval->pt     = nullptr;
	pval->mode   = HSPVAR_MODE_NONE;
	pval->master = nullptr;
	return;
}

//------------------------------------------------
// ��� (=)
// 
// @ �Q�Ƌ��L
//------------------------------------------------
static void HspVarReference_Set( PVal* pval, PDAT* pdat, const void* in )
{
	auto rhs = reinterpret_cast<const VtReference::value_t*>(in);
	
	pval->master   = rhs->master;
	pval->arraymul = rhs->arraymul;
	return;
}

//------------------------------------------------
// Reference �o�^�֐�
//------------------------------------------------
void HspVarReference_Init( HspVarProc* p )
{
	g_pHvpReference = p;
	g_vtReference   = p->flag;
	
	// �֐��|�C���^��o�^
	p->GetPtr       = HspVarReference_GetPtr;
	p->GetSize      = HspVarReference_GetSize;
	p->GetUsing     = HspVarReference_GetUsing;
	
	p->Alloc        = HspVarReference_Alloc;
	p->Free         = HspVarReference_Free;
	p->GetBlockSize = HspVarReference_GetBlockSize;
	p->AllocBlock   = HspVarReference_AllocBlock;
	
	// ���Z�֐�
	p->Set          = HspVarReference_Set;
//	p->AddI         = HspVarReference_AddI;
//	p->EqI          = HspVarReference_EqI;
//	p->NeI          = HspVarReference_NeI;
	
	// �A�z�z��p
	p->ArrayObjectRead = HspVarReference_ArrayObjectRead;	// �Q��(�E)
	p->ArrayObject     = HspVarReference_ArrayObject;		// �Q��(��)
	p->ObjectWrite     = HspVarReference_ObjectWrite;		// �i�[
//	p->ObjectMethod    = HspVarReference_ObjectMethod;		// ���\�b�h
	
	// �g���f�[�^
//	p->user         = (char*)HspVarReference_GetMapList;
	
	// ���̑��ݒ�
	p->vartype_name = "reference_k";	// �^�C�v�� (�Փ˂��Ȃ��悤�ɕςȖ��O�ɂ���)
	p->version      = 0x001;			// runtime ver(0x100 = 1.0)
	
	p->support							// �T�|�[�g�󋵃t���O(HSPVAR_SUPPORT_*)
		= HSPVAR_SUPPORT_STORAGE		// �Œ蒷�X�g���[�W
	//	| HSPVAR_SUPPORT_FLEXARRAY		// �ϒ��z��
		| HSPVAR_SUPPORT_ARRAYOBJ		// �A�z�z��T�|�[�g
		| HSPVAR_SUPPORT_NOCONVERT		// ObjectWrite�Ŋi�[
		| HSPVAR_SUPPORT_VARUSE			// varuse�֐���K�p
		;
	p->basesize = VtReference::basesize;	// size / �v�f (byte)
	return;
}

//#########################################################
//        �A�z�z��p�̊֐��Q
//#########################################################
//------------------------------------------------
// �A�z�z��::�Q�� (���Ӓl)
//------------------------------------------------
static void HspVarReference_ArrayObject( PVal* pval )
{
	if ( !pval->master ) puterror( HSPERR_VARIABLE_REQUIRED );
	
	PVal* const pvInner = VtReference::GetPVal( pval );
	APTR  const apRef   = VtReference::GetAPTR( pval );
	
	// �_�~�[�Y�����̂Ă�
	if ( code_geti() != VtReference::IdxDummy ) puterror( HSPERR_ARRAY_OVERFLOW );
	
	// �����ϐ��̓Y��������
	if ( code_isNextArg() ) {				// �Y��������ꍇ
		if ( apRef > 0 ) puterror( HSPERR_INVALID_ARRAY );	// �z��v�f�Q�� => �Y���ߏ�
		code_expand_index_impl_lhs( pvInner );
		
	} else {
		if ( PVal_supportArray(pvInner) && !(pval->support & HSPVAR_SUPPORT_ARRAYOBJ) ) {		// �W���z��T�|�[�g
			HspVarCoreReset( pvInner );								// �Y����Ԃ̏�����
			if ( apRef > 0 ) code_index_int_rhs( pvInner, apRef );	// �v�f�Y��
		}
	}
	
	/*
	// pval ������ϐ��̃N���[���ɂ���
	PVal_cloneVar( pval, pvInner,
		(apRef > 0)
			? apRef
			: ( pvInner->arraycnt > 0 ? pvInner->offset : -1 )
	);
	pvInner->support |= HSPVAR_SUPPORT_ARRAYOBJ;
	//*/
	
	return;
}

//------------------------------------------------
// �A�z�z��::�Q�� (�E�Ӓl)
//------------------------------------------------
static void* HspVarReference_ArrayObjectRead( PVal* pval, int* mptype )
{
	// �_�~�[�Y�����̂Ă�
	if ( code_geti() != VtReference::IdxDummy ) dbgout("!need dummy idx");//puterror( HSPERR_ARRAY_OVERFLOW );
	
	if ( !pval->master ) puterror( HSPERR_VARIABLE_REQUIRED );
	
	PVal* const pvInner = VtReference::GetPVal( pval );
	APTR  const apRef   = VtReference::GetAPTR( pval );
	
	// �����ϐ��̓Y��������
	if ( code_isNextArg() ) {				// �Y��������ꍇ
		if ( apRef > 0 ) puterror( HSPERR_INVALID_ARRAY );	// �z��v�f�Q�� => �Y���ߏ�
		return code_expand_index_impl_rhs( pvInner, mptype );
		
	} else {
		if ( PVal_supportArray(pvInner) && !(pval->support & HSPVAR_SUPPORT_ARRAYOBJ) ) {		// �W���z��T�|�[�g
			HspVarCoreReset( pvInner );								// �Y����Ԃ̏�����
			if ( apRef > 0 ) code_index_int_rhs( pvInner, apRef );	// �v�f�Y��
		}
		
		*mptype = pvInner->flag;
		return GetHvp( pvInner->flag )->GetPtr( pvInner );
	}
}

//------------------------------------------------
// �A�z�z��::�i�[
//------------------------------------------------
static void HspVarReference_ObjectWrite( PVal* pval, void* data, int vflag )
{
	PVal* const pvInner = VtReference::GetPVal( *VtReference::GetPtr(pval) );
	
	// reference �ւ̑��
	if ( !pvInner ) {
		if ( vflag != g_vtReference ) puterror( HSPERR_INVALID_ARRAYSTORE );	// �E�ӂ̌^���s��v
		
		HspVarReference_Set( pval, (PDAT*)HspVarReference_GetPtr(pval), data );
		
	// �����ϐ����Q�Ƃ��Ă���ꍇ
	} else {
		PVal_assign( pvInner, data, vflag );	// �����ϐ��ւ̑������
		code_assign_multi( pvInner );
	}
	
	return;
}

//------------------------------------------------
// ���\�b�h�Ăяo��
// 
// @ �����ϐ��̌^�Œ񋟂���Ă��郁�\�b�h���g��
//------------------------------------------------
static void HspVarReference_ObjectMethod(PVal* pval)
{
	PVal* const pvInner = VtReference::GetPVal( pval );
	if ( !pvInner ) puterror( HSPERR_UNSUPPORTED_FUNCTION );
	
	// �����ϐ��̏����ɓ]��
	GetHvp(pvInner->flag)->ObjectMethod( pvInner );
	
	return;
}
