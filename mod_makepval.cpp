// PVal �̓Ǝ��Ǘ�

#include "mod_makepval.h"

namespace hpimod {

static PVal* PVal_initDefault(vartype_t vt);

//------------------------------------------------
// PVal�\���̂̏�����
// 
// @ (pval == nullptr) => �������Ȃ��B
// @prm pval: �s��l�ł���
//------------------------------------------------
void PVal_init(PVal* pval, vartype_t vtype)
{
	if ( !pval ) return;

	pval->flag = vtype;
	pval->mode = HSPVAR_MODE_NONE;
	PVal_alloc( pval );
	return;
}

//------------------------------------------------
// �����Ƃ��ȒP�ŗL����PVal�\���̂ɂ���
// 
// @ (pval == nullptr) => �������Ȃ��B
// @ (vtype == ����) => pval->flag �̌^�ɏ���������B
// @ HspVarCoreDim �̑��� (�z��Y���͎w��ł��Ȃ���)�B
//------------------------------------------------
void PVal_alloc(PVal* pval, PVal* pval2, vartype_t vtype)
{
	if ( !pval ) return;
	if ( vtype <= HSPVAR_FLAG_NONE ) vtype = pval->flag;
	if ( vtype <= HSPVAR_FLAG_NONE ) return;

	HspVarProc* const vp = getHvp( vtype );

	// pt ���m�ۂ���Ă���ꍇ�A�������
	if ( pval->flag != HSPVAR_FLAG_NONE && pval->mode == HSPVAR_MODE_MALLOC ) {
		PVal_free( pval );
	}

	// �m�ۏ���
	memset( pval, 0x00, sizeof(PVal) );
	pval->flag     = vtype;
	pval->mode     = HSPVAR_MODE_NONE;
	pval->support |= vp->support;
	vp->Alloc( pval, pval2 );
	return;
}

//------------------------------------------------
// PVal�\���̂��ȒP�ɏ���������
// 
// @ �ł��ȒP�Ȍ`�Ŋm�ۂ����B
// @ HspVarCoreClear �̑���B
//------------------------------------------------
void PVal_clear(PVal* pval, vartype_t vtype)
{
	PVal_alloc( pval, nullptr, vtype );
}

//------------------------------------------------
// PVal �\���̂̒��g���������
// 
// @ (pval == nullptr) => �������Ȃ��B
// @ pval �|�C���^���͔̂j�󂳂�Ȃ��B
//------------------------------------------------
void PVal_free(PVal* pval)
{
	if ( !pval ) return;

	HspVarProc* const vp = getHvp( pval->flag );
	vp->Free( pval );

	pval->support &= ~vp->support;		// meta �łȂ� support �t���O����菜��
	return;
}

//------------------------------------------------
// ����l��\�� PVal �\���̂�������
// @private
// @ vt �͕K���L���Ȓl (str �` int)�B
//------------------------------------------------
static PVal* PVal_initDefault(vartype_t vt)
{
	static PVal** stt_pDefPVal   = nullptr;
	static int    stt_cntDefPVal = 0;

	// stt_pDefPVal �̊g��
	if ( stt_cntDefPVal <= vt ) {
		int cntNew = vt + 1;

		if ( !stt_pDefPVal ) {
			stt_pDefPVal = reinterpret_cast<PVal**>(hspmalloc( cntNew * sizeof(PVal*) ));

		} else {
			stt_pDefPVal = reinterpret_cast<PVal**>(hspexpand(
				reinterpret_cast<char*>( stt_pDefPVal ),
				cntNew * sizeof(PVal*)
			));
		}

		// �g������ nullptr �ŏ���������
		for( int i = stt_cntDefPVal; i < cntNew; ++ i ) {
			stt_pDefPVal[i] = nullptr;
		}

		stt_cntDefPVal = cntNew;
	}

	// ���������̏ꍇ�́APVal �̃��������m�ۂ��A����������
	if ( !stt_pDefPVal[vt] ) {
		stt_pDefPVal[vt] = reinterpret_cast<PVal*>(hspmalloc( sizeof(PVal) ));
		PVal_init( stt_pDefPVal[vt], vt );
	}
	return stt_pDefPVal[vt];
}

//------------------------------------------------
// ����l��\�� PVal �\���̂ւ̃|�C���^�𓾂�
// 
// @ vt ���s���ȏꍇ�Anullptr ��Ԃ��B
//------------------------------------------------
PVal* PVal_getDefault( vartype_t vt )
{
	if ( vt <= HSPVAR_FLAG_NONE ) {
		return nullptr;

	} else {
		return PVal_initDefault( vt );
	}
}

//##########################################################
//        �ϐ����̎擾
//##########################################################
#if 0
// basis �ɂ���
//------------------------------------------------
// �ϐ��̗v�f�̑�����Ԃ�
//------------------------------------------------
size_t PVal_cntElems( PVal const* pval )
{
	int cntElems = 1;

	// �v�f���𒲂ׂ�
	for ( unsigned int i = 0; i < ArrayDimMax; ++ i ) {
		if ( pval->len[i + 1] ) {
			cntElems *= pval->len[i + 1];
		}
	}

	return cntElems;
}
#endif

//------------------------------------------------
// �ϐ��̃T�C�Y��Ԃ�
// 
// @ pval->offset ������B
// @ �Œ蒷�^�Ȃ� HspVarProc::basesize ���A
// @	�ϒ��^�Ȃ�A�w��v�f�̃T�C�Y���B
//------------------------------------------------
size_t PVal_size( PVal const* pval )
{
	auto const vp = getHvp( pval->flag );

	if ( vp->basesize < 0 ) {
		return vp->GetSize( reinterpret_cast<PDAT const*>(pval->pt) );
	} else {
		return vp->basesize;
	}
}

//------------------------------------------------
// �ϐ�������̃|�C���^�𓾂�
// 
// @ pval->offset ������B
//------------------------------------------------
PDAT* PVal_getptr( PVal* pval )
{
	return getHvp(pval->flag)->GetPtr( pval );
}

PDAT* PVal_getptr( PVal* pval, APTR aptr )
{
	pval->offset = aptr;
	if ( pval->arraycnt == 0 ) pval->arraycnt = 1;
	return PVal_getptr(pval);
}

//##########################################################
//        �ϐ��ɑ΂��鑀��
//##########################################################
//------------------------------------------------
// PVal�֒l���i�[���� (�ėp)
// 
// @ pval �̓Y����Ԃ��Q�Ƃ���B
//------------------------------------------------
void PVal_assign( PVal* pval, PDAT const* data, vartype_t vtype )
{
	// �Y������ => ObjectWrite
	if ( (pval->support & HSPVAR_SUPPORT_NOCONVERT) && (pval->arraycnt != 0) ) {
		getHvp(pval->flag)->ObjectWrite(pval, data, vtype);

	// �ʏ�̑��
	} else {
		code_setva(pval, pval->offset, vtype, data);
	}
	return;
}

//------------------------------------------------
// ���ݑ��
//------------------------------------------------
static void PVal_assign_mutual_impl( PVal* pvLhs, PVal* pvRhs, PVal* pvTmp );

void PVal_assign_mutual( PVal* pvLhs, PVal* pvRhs )
{
	PVal vTmp = {0};
	PVal_assign_mutual( pvLhs, pvRhs, &vTmp );
}

void PVal_assign_mutual( PVal* pvLhs, PVal* pvRhs, APTR apLhs, APTR apRhs )
{
	PVal vTmp = {0};
	PVal_assign_mutual( pvLhs, pvRhs, apLhs, apRhs, &vTmp );
}

void PVal_assign_mutual( PVal* pvLhs, PVal* pvRhs, PVal* pvTmp )
{
	if ( pvLhs == pvRhs ) return;
	PVal_assign_mutual( pvLhs, pvRhs, pvLhs->offset, pvRhs->offset, pvTmp );
	return;
}

void PVal_assign_mutual( PVal* pvLhs, PVal* pvRhs, APTR apLhs, APTR apRhs, PVal* pvTmp )
{
	if ( pvLhs == pvRhs ) {
		PVal vLhs = *pvLhs; vLhs.offset = apLhs;		// �Y����Ԃ�ۑ����邽��
		PVal vRhs = *pvRhs; vRhs.offset = apRhs;
		PVal_assign_mutual_impl( &vLhs, &vRhs, pvTmp );
	} else {
		pvLhs->offset = apLhs;
		pvRhs->offset = apRhs;
		PVal_assign_mutual_impl( pvLhs, pvRhs, pvTmp );
	}
	return;
}

void PVal_assign_mutual_impl( PVal* pvLhs, PVal* pvRhs, PVal* pvTmp )
{
//	assert( pvLhs != pvRhs );
	PVal_init( pvTmp, pvLhs->flag );
	{
		PVal_assign( pvTmp, PVal_getptr(pvLhs), pvLhs->flag );
		PVal_assign( pvLhs, PVal_getptr(pvRhs), pvRhs->flag );
		PVal_assign( pvRhs, pvTmp->pt, pvTmp->flag );
	}
	PVal_free( pvTmp );
	return;
}

//------------------------------------------------
// PVal�̕���
// 
// @ �S�v�f�𕡎ʂ���B
//------------------------------------------------
void PVal_copy(PVal* pvDst, PVal* pvSrc)
{
	if ( pvDst == pvSrc ) return;

	size_t const cntElems = PVal_cntElems(pvSrc);

	// pvDst ���m�ۂ���
	exinfo->HspFunc_dim(
		pvDst, pvSrc->flag, 0, pvSrc->len[1], pvSrc->len[2], pvSrc->len[3], pvSrc->len[4]
	);

	// �P���ϐ� => ���v�f��������̂�
	if ( cntElems == 1 ) {
		PVal_assign( pvDst, pvSrc->pt, pvSrc->flag );

	// �A��������� => ���ׂĂ̗v�f��������
	} else {
		auto const hvpSrc = getHvp( pvSrc->flag );

		pvDst->arraycnt = 1;
		for ( size_t i = 0; i < cntElems; ++ i ) {
			pvDst->offset = i;
			pvSrc->offset = i;

			PVal_assign(pvDst, hvpSrc->GetPtr(pvSrc), pvSrc->flag);
		}
	}
	return;
}

//------------------------------------------------
// �ϐ��l�̌���
//------------------------------------------------
void PVal_swap( PVal* pvLhs, PVal* pvRhs )
{
	PVal_swap( pvLhs, pvRhs, pvLhs->offset, pvRhs->offset );
}

void PVal_swap( PVal* pvLhs, PVal* pvRhs, APTR apLhs, APTR apRhs )
{
	PVal   vTmp = {0};
	PVal* pvTmp = &vTmp;

	if ( pvLhs->arraycnt != 0 || pvRhs->arraycnt != 0 ) {
		// @ ������z��v�f => ����ɂ�����

		PVal_assign_mutual( pvLhs, pvRhs, apLhs, apRhs, pvTmp );

	} else {
		// @ �����Ƃ��ϐ� => PVal ���̂̌��� (meta �� support �t���O�͈ێ�����)
		int const supportMetaLhs = PVal_supportMeta( pvLhs );
		int const supportMetaRhs = PVal_supportMeta( pvRhs );

		*pvTmp = *pvLhs;
		*pvLhs = *pvRhs;
		*pvRhs = *pvTmp;

		pvRhs->support |= supportMetaLhs;
		pvLhs->support |= supportMetaRhs;
	}

	return;
}

//------------------------------------------------
// �ϐ����N���[���ɂ���
// 
// @ HspVarCoreDup, HspVarCoreDupPtr
//------------------------------------------------
void PVal_clone( PVal* pvDst, PVal* pvSrc, APTR aptrSrc )
{
	HspVarProc* const vp = getHvp( pvSrc->flag );

	if ( aptrSrc >= 0 ) pvSrc->offset = aptrSrc;
	PDAT* const pSrc = vp->GetPtr(pvSrc);		// ���̃|�C���^

	int size;								// �N���[���ɂ���T�C�Y
	vp->GetBlockSize( pvSrc, pSrc, &size );

	// ���̃|�C���^����N���[�������
	PVal_clone( pvDst, pSrc, pvSrc->flag, size );
	return;
}

void PVal_clone( PVal* pval, PDAT* ptr, int vtype, int size )
{
	PVal_free( pval );

	HspVarProc* const vp = getHvp(vtype);

	pval->pt   = ptr;
	pval->flag = vtype;
	pval->size = size;
	pval->mode = HSPVAR_MODE_CLONE;
	pval->len[0] = 1;

	if ( vp->basesize < 0 ) {
		pval->len[1] = 1;
	} else {
		pval->len[1] = size / vp->basesize;
	}
	pval->len[2] = 0;
	pval->len[3] = 0;
	pval->len[4] = 0;
	pval->offset = 0;
	pval->arraycnt = 0;
	pval->support |= HSPVAR_SUPPORT_STORAGE;
	return;
}

//------------------------------------------------
// �ϐ����N���[���ɂ���
// 
// @ �ϐ��ɑ΂����苭�͂ȃN���[���B
//------------------------------------------------
void PVal_cloneVar( PVal* pvDst, PVal* pvSrc, APTR aptrSrc )
{
	PVal_free( pvDst );
	int const supportMeta = pvDst->support;	// meta �� support �t���O�͈ێ����� (PVal_free �ɂ���� meta �łȂ� support �t���O�͎�菜����Ă���)

	// �z��ւ̃N���[��
	if ( pvSrc->arraycnt == 0 && aptrSrc < 0 ) {
		*pvDst = *pvSrc;					// �܂邲�ƃR�s�[

	// �P��v�f�ւ̃N���[��
	} else {
		if ( aptrSrc >= 0 ) pvSrc->offset = aptrSrc;
		HspVarProc const* const vp  = getHvp(pvSrc->flag);
		PDAT const*       const ptr = vp->GetPtr( pvSrc );

		std::memset( pvDst, 0x00, sizeof(PVal) );
		pvDst->flag    = pvSrc->flag;
		pvDst->len[1]  = 1;
		pvDst->pt      = const_cast<PDAT*>(ptr);
		pvDst->size    = vp->GetSize( ptr );
		pvDst->master  = pvSrc->master;
	}

	pvDst->mode    = HSPVAR_MODE_CLONE;		// �N���[���Ƃ������Ƃɂ���
	pvDst->support = PVal_supportNotmeta(pvSrc) | supportMeta;
	return;
}

//------------------------------------------------
// �l���V�X�e���ϐ��ɑ������
//------------------------------------------------
void SetResultSysvar(PDAT const* data, vartype_t vtype)
{
	if ( !data ) return;

	ctx->retval_level = ctx->sublev;

	switch ( vtype ) {
		case HSPVAR_FLAG_INT:
			ctx->stat = VtTraits<int>::derefValptr(data);
			break;

		case HSPVAR_FLAG_STR:
			strncpy(
				ctx->refstr,
				VtTraits<str_tag>::asValptr(data),
				HSPCTX_REFSTR_MAX - 1
			);
			break;

		case HSPVAR_FLAG_DOUBLE:
			ctx->refdval = VtTraits<double>::derefValptr(data);
			break;

		default:
			puterror( HSPERR_TYPE_MISMATCH );
	}
	throw;	// �x���}��
}

//------------------------------------------------
// ���̃|�C���^���^�ϊ�����
//------------------------------------------------
PDAT const* Valptr_cnvTo( PDAT const* pValue, vartype_t vtSrc, vartype_t vtDst )
{
	return (PDAT const*)(
		( vtSrc < HSPVAR_FLAG_USERDEF )
			? getHvp(vtDst)->Cnv( pValue, vtSrc )
			: getHvp(vtSrc)->CnvCustom( pValue, vtDst )
	);
}

//------------------------------------------------
// meta �� support �t���O�����o��
// 
// @ ���̕ϐ��̌^�� HspVarProc::support �������Ă���t���O��
// @	���̕ϐ��ɌŗL�̃t���O�ł���A
// @	�����łȂ��t���O�͂��� PVal �ɌŗL�̃t���O�ł���A�Ɖ��߂��A
// @	��҂̃t���O���umeta �� support �t���O�v�ƌĂԁB
// @ex: HSPVAR_SUPPORT_TEMPVAR
//------------------------------------------------
int PVal_supportMeta( PVal* pval )
{
	return pval->support &~ getHvp(pval->flag)->support;
}

int PVal_supportNotmeta( PVal* pval )
{
	return pval->support & getHvp(pval->flag)->support;
}

//------------------------------------------------
// �z��T�|�[�g���ۂ�
//------------------------------------------------
bool PVal_supportArray( PVal* pval )
{
	return (pval->support & ( HSPVAR_SUPPORT_FIXEDARRAY | HSPVAR_SUPPORT_FLEXARRAY | HSPVAR_SUPPORT_ARRAYOBJ )) != 0;
}

} // namespace hpimod
