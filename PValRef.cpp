// PValRef
#if 0
#include "hsp3plugin_custom.h"
#include "mod_makepval.h"

#include "PValRef.h"

namespace hpimod {

// �N���X�ϐ�
PVALREF_DBGCODE(
	int PValRef::stt_counter = 1;
)

PValRef* const PValRef::MagicNull = (PValRef*)HspTrue;

//------------------------------------------------
// �\�z(new & constructor)
//------------------------------------------------
PValRef* PValRef::New( int vflag )
{
	PValRef* self = (PValRef*)hspmalloc( sizeof(PValRef) );
	if ( vflag > 0 ) PVal_init( &self->pval, vflag );
	self->cntRefed = 0;

	PVALREF_DBGCODE(
		self->id = PValRef::stt_counter ++;		// �Q�ƃJ�E���^�\���p��ID�l������
		dbgout( "[%d] new!", self->id );
	)
	return self;
}

//------------------------------------------------
// ���(delete & destructor)
//------------------------------------------------
void PValRef::Delete( PValRef* pval )
{
	assert(!!pval);

	PVal_free( AsPVal(pval) );
	hspfree( pval );
	return;
}

//------------------------------------------------
// �Q�ƃJ�E���^�̑���
//------------------------------------------------
void PValRef::AddRef( PValRef* pval )
{
	assert(!!pval);

	pval->cntRefed ++;
	PVALREF_DBGCODE(
		dbgout( "[%d] ++ �� %d", pval->id, pval->cntRefed );
	)
	return;
}

void PValRef::Release( PValRef* pval )
{
	assert(!!pval);

	pval->cntRefed --;
	PVALREF_DBGCODE(
		dbgout( "[%d] -- �� %d", pval->id, pval->cntRefed );
	)
	// �J�E���^�� 0 �ȉ� => ���
	if ( pval->cntRefed == 0 ) PValRef::Delete( pval );
	return;
}

}	// namespace hpimod
#endif