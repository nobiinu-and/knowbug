// �����擾���W���[��

#include "hsp3plugin_custom.h"
#include "mod_argGetter.h"
#include "mod_makepval.h"

namespace hpimod {

//##########################################################
//    �錾
//##########################################################
//static void code_checkarray2( PVal* pval );

//##########################################################
//    �����̎擾
//##########################################################
/*
//------------------------------------------------
// �ϐ����擾����
//------------------------------------------------
bool code_getva_ex(PVal** pval, APTR* aptr)
{
	*pval = &ctx->mem_var[*val];					// �O���[�o���ϐ��̃f�[�^���� PVal �����o��
	if ( code_getprm() <= PARAM_END ) return false;	// �������U
	*aptr = (*pval)->offset;
	return true;
}
//*/

//------------------------------------------------
// ��������擾����( hspmalloc �Ŋm�ۂ��� )
// 
// @ ����`���͌Ăяo�����ɂ���B
//------------------------------------------------
int code_getds_ex(char** ppStr, char const* defstr)
{
	char* pStr = code_getds(defstr);
	size_t len = strlen(pStr);

	*ppStr = hspmalloc( (len + 1) * sizeof(char) );
	strncpy( *ppStr, pStr, len );
	(*ppStr)[len] = '\0';		// �I�[
	return len;
}

#if 0
//------------------------------------------------
// �����񂩐��l���擾����
// 
// @ ������Ȃ� sbAlloc �Ŋm�ہA��������R�s�[����B
// @result = int
//		*ppStr �� nullptr �Ȃ�A�Ԓl���L���B
//		�����łȂ��ꍇ�A*ppStr ���L���B
//------------------------------------------------
int code_get_int_or_str( char** ppStr )
{
	*ppStr = nullptr;

	if ( code_getprm() <= PARAM_END ) return 0;

	switch ( mpval->flag ) {
		case HSPVAR_FLAG_INT:
			return *(int*)( mpval->pt );

		case HSPVAR_FLAG_DOUBLE:
			return static_cast<int>( *(double*)(mpval->pt) );

		case HSPVAR_FLAG_STR:
		{
			*ppStr = hspmalloc( getHvp(mpval->flag)->GetSize((PDAT*)mpval->pt) /*PVal_size(mpval)*/ + 1 );
			strcpy( *ppStr, (char*)(mpval->pt) );
			return 0;
		}
		default:
			puterror( HSPERR_TYPE_MISMATCH );
	}
	return 0;
}
#endif

//------------------------------------------------
// �^�^�C�v�l���擾����
// 
// @ ������ or ���l
// @error ������Ŕ�^��        => HSPERR_ILLEGAL_FUNCTION
// @error ������ł����l�ł��Ȃ� => HSPERR_TYPE_MISMATCH
//------------------------------------------------
int code_get_vartype( int deftype )
{
	int const prm = code_getprm();
	if ( prm == PARAM_DEFAULT ) return deftype;
	if ( prm <= PARAM_END ) return HSPVAR_FLAG_NONE;

	switch ( mpval->flag ) {
		// �^�^�C�v�l
		case HSPVAR_FLAG_INT:
			return *(int*)(mpval->pt);

		// �^��
		case HSPVAR_FLAG_STR:
		{
			auto const vp = seekHvp( mpval->pt );
			if ( !vp ) puterror( HSPERR_ILLEGAL_FUNCTION );

			return vp->flag;
		}
		default:
			puterror( HSPERR_TYPE_MISMATCH );
	}
	throw;	// �x���}��
}

//------------------------------------------------
// ���s�|�C���^���擾����( ���x���A�ȗ��\ )
//------------------------------------------------
label_t code_getdlb( label_t defLabel )
{
	try {
		return code_getlb();

	} catch( HSPERROR err ) {
		if ( err == HSPERR_LABEL_REQUIRED ) {
			return defLabel;
		}

		puterror( err );
		throw;		// �x���}��
	}

	/*
	label_t lb = nullptr;

	// ���e����( *lb )�̏ꍇ
	// @ *val �ɂ̓��x��ID ( ctx->mem_ot �̗v�f�ԍ� )�������Ă���B
	// @ code_getlb() �œ�����̂̓��x�����w�����s�|�C���^�Ȃ���́B
	if ( *type == TYPE_LABEL ) {	// ���x���萔
		lb = code_getlb();

	// ���x���^�ϐ��̏ꍇ
	// @ code_getlb() �Ɠ�������, mpval ���X�V����B
	// @ ���� ( *type == TYPE_VAR ) �ł́A���x���^��Ԃ��֐���V�X�e���ϐ��ɑΉ��ł��Ȃ��B
	} else {
		// ���x���̎w�����s�|�C���^���擾
		if ( code_getprm() <= PARAM_END )         return nullptr;
		if ( mpval->flag   != HSPVAR_FLAG_LABEL ) return nullptr;

		// ���x���̃|�C���^���擾����( �ϐ��̎��̂�����o�� )
		lb = *(label_t*)( mpval->pt );
	}

	return lb;
	//*/
}

//------------------------------------------------
// ���x�����s�|�C���^���擾����
// @ ???
//------------------------------------------------
//pExec_t code_getlb2(void)
//{
//	pExec_t pLbExec = code_getlb();
//	code_next();
//	*exinfo->npexflg &= ~EXFLG_2;
//	return pLbExec;
//}

//------------------------------------------------
// �C���X�^���X���擾����
//------------------------------------------------
extern FlexValue* code_get_struct()
{
	PVal* const pval = code_get_var();
	if ( pval->flag != HSPVAR_FLAG_STRUCT ) puterror(HSPERR_TYPE_MISMATCH);

	return reinterpret_cast<FlexValue*>(getHvp(HSPVAR_FLAG_STRUCT)->GetPtr(pval));
}

//------------------------------------------------
// ���W���[���N���X���ʎq���擾
//------------------------------------------------
stdat_t code_get_modcls()
{
	stprm_t const stprm = code_get_stprm();
	if ( stprm->mptype != MPTYPE_STRUCTTAG ) puterror( HSPERR_STRUCT_REQUIRED );
	return STRUCTPRM_getStDat(stprm);
}

//------------------------------------------------
// �\���̃p�����[�^���擾
//------------------------------------------------
stprm_t code_get_stprm()
{
	if ( *type != TYPE_STRUCT ) puterror( HSPERR_STRUCT_REQUIRED );

	stprm_t const pStPrm = &ctx->mem_minfo[ *val ];
	code_next();
	*exinfo->npexflg &= ~EXFLG_2;

	return pStPrm;
}

//##########################################################
//    �z��Y���̉���
//##########################################################
//------------------------------------------------
// �Y���̎��o�� (�ʏ�z��)
// 
// @ '(' �����o��������̏�ԂŌĂ�
//------------------------------------------------
void code_expand_index_int( PVal* pval, bool bRhs )
{
	HspVarCoreReset(pval);	// �z��Y���̏�������������

	int n = 0;
	PVal tmpPVal;

	for (;;) {
		// �Y���̏�Ԃ�ۑ�
		HspVarCoreCopyArrayInfo( &tmpPVal, pval );

		int prm = code_getprm();	// �p�����[�^�[�����o��

		// �G���[�`�F�b�N
		if ( prm == PARAM_DEFAULT ) {
			n = 0;

		} else if ( prm <= PARAM_END ) {
			puterror( HSPERR_BAD_ARRAY_EXPRESSION );

		} else if ( mpval->flag != HSPVAR_FLAG_INT ) {
			puterror( HSPERR_TYPE_MISMATCH );
		}

		// �Y���̏�Ԃ�߂�
		HspVarCoreCopyArrayInfo( pval, &tmpPVal );

		if ( prm != PARAM_DEFAULT ) {
			n = *(int*)(mpval->pt);
		}

		code_index_int( pval, n, bRhs );	// �z��v�f�w�� (int)
		if ( prm == PARAM_SPLIT ) break;
	}
	return;
}

//------------------------------------------------
// �Y�����ʂ̎��o�� (�ʏ�z��)
//------------------------------------------------
static void code_checkarray( PVal* pval, bool bRhs )
{
	if ( *type == TYPE_MARK && *val == '(' ) {
		code_next();

		code_expand_index_int(pval, bRhs);

		if ( !(*type == TYPE_MARK && *val == ')') ) {
			puterror( HSPERR_BAD_ARRAY_EXPRESSION );
		}
		code_next();

	// �Y�����Ȃ���Ώ�������������
	} else {
		HspVarCoreReset( pval );
	}
	return;
}

void code_checkarray2( PVal* pval ) { code_expand_index_int( pval, false ); }
void code_checkarray1( PVal* pval ) { code_expand_index_int( pval, true  ); }

//------------------------------------------------
// �Y�����ʂ̎��o�� (�A�z�z��, ��)
//------------------------------------------------
void code_checkarray_obj2( PVal* pval )
{
	HspVarCoreReset( pval );

	if ( *type == TYPE_MARK && *val == '(' ) {
		code_next();

		getHvp(pval->flag)->ArrayObject( pval );	// �Y���Q��

		if ( !(*type == TYPE_MARK && *val == ')') ) {
			puterror( HSPERR_BAD_ARRAY_EXPRESSION );
		}
		code_next();
	}
	return;
}

//------------------------------------------------
// �Y�����ʂ̎��o�� (�A�z�z��, �E)
// 
// @prm pval   : �Y���w�肳���z��ϐ�
// @prm mptype : �ėp�f�[�^�̌^�^�C�v�l��Ԃ�
// @result     : �ėp�f�[�^�ւ̃|�C���^
//------------------------------------------------
void* code_checkarray_obj( PVal* pval, int* mptype )
{
	HspVarCoreReset( pval );

	if ( *type == TYPE_MARK && *val == '(' ) {
		code_next();

		void* const pResult = getHvp( pval->flag )->ArrayObjectRead( pval, mptype );

		if ( !(*type == TYPE_MARK && *val == ')') ) {
			puterror( HSPERR_BAD_ARRAY_EXPRESSION );
		}
		code_next();
		return pResult;
	}

	*mptype = pval->flag;
	return PVal_getptr(pval);
}

//------------------------------------------------
// �Y���̎��o�� (�ʏ�/�A�z, ���g����)
// 
// @ '(' �����o��������ɌĂ΂��B
//------------------------------------------------
void code_expand_index_lhs( PVal* pval )
{
	// �A�z�z��^ => ArrayObject() ���Ă�
	if ( pval->support & HSPVAR_SUPPORT_ARRAYOBJ ) {
		getHvp( pval->flag )->ArrayObject( pval );

	// �ʏ�z��^ => �����̐������v�f�����o��
	} else {
		PVal pvalTemp;
		HspVarCoreReset( pval );

		for ( int i = 0; i < ArrayDimMax && !(*type == TYPE_MARK && *val == ')'); ++ i ) {
			HspVarCoreCopyArrayInfo( &pvalTemp, pval );
			int const idx = code_geti();
			HspVarCoreCopyArrayInfo( pval, &pvalTemp );

			code_index_int_lhs( pval, idx );
		}
	}

	return;
}

void* code_expand_index_rhs( PVal* pval, int* mptype )
{
	// �A�z�z��^ => ArrayObjectRead() ���Ă�
	if ( pval->support & HSPVAR_SUPPORT_ARRAYOBJ ) {
		return getHvp( pval->flag )->ArrayObjectRead( pval, mptype );

	// �ʏ�z��^ => �����̐������v�f�����o��
	} else {
		PVal pvalTemp;
		HspVarCoreReset( pval );

		for ( int i = 0; i < ArrayDimMax && !(*type == TYPE_MARK && *val == ')'); ++ i ) {
			HspVarCoreCopyArrayInfo( &pvalTemp, pval );
			int const idx = code_geti();
			HspVarCoreCopyArrayInfo( pval, &pvalTemp );

			code_index_int_rhs( pval, idx );
		}

		*mptype = pval->flag;
		return getHvp( pval->flag )->GetPtr( pval );
	}
}

//------------------------------------------------
// �z��v�f�̐ݒ� (�ʏ�z��, 1����, ���E)
// 
// @ �ʏ�^ (int) �̂݁B
// @ Reset ��Ɏ����������A���ŌĂ΂��B
//------------------------------------------------
void code_index_int( PVal* pval, int offset, bool bRhs )
{
	if ( !bRhs ) {
		code_index_int_lhs( pval, offset );		// �����g������
	} else {
		code_index_int_rhs( pval, offset );		// �����g�����Ȃ�
	}
	return;
}

// ���Ӓl�Ƃ��ĎQ��
void code_index_int_lhs( PVal* pval, int offset )
{
	if ( pval->arraycnt >= 5 ) puterror( HSPVAR_ERROR_ARRAYOVER );
	if ( pval->arraycnt == 0 ) {
		pval->arraymul = 1;		// �{�������l
	} else {
		pval->arraymul *= pval->len[pval->arraycnt];
	}
	++pval->arraycnt;
	if ( offset < 0 ) puterror( HSPVAR_ERROR_ARRAYOVER );
	if ( offset >= pval->len[pval->arraycnt] ) {							// �z��g�����K�v
		if ( (pval->arraycnt >= 4 || pval->len[pval->arraycnt + 1] == 0)	// �z��g�����\
			&& (pval->support & HSPVAR_SUPPORT_FLEXARRAY)					// �ϒ��z��T�|�[�g => �z����g������
		) {
			exinfo->HspFunc_redim( pval, pval->arraycnt, offset + 1 );
			pval->offset += offset * pval->arraymul;
			return;
		}
		puterror( HSPVAR_ERROR_ARRAYOVER );
	}
	pval->offset += offset * pval->arraymul;
	return;
}

// �E�Ӓl�Ƃ��ĎQ��
extern void code_index_int_rhs( PVal* pval, int offset )
{
	exinfo->HspFunc_array( pval, offset );
#if 0
	if ( pval->arraycnt >= 5 ) puterror(HSPVAR_ERROR_ARRAYOVER);
	if ( pval->arraycnt == 0 ) {
		pval->arraymul = 1;
	} else {
		pval->arraymul *= pval->len[pval->arraycnt];
	}
	++pval->arraycnt;
	if ( offset < 0 ) puterror(HSPVAR_ERROR_ARRAYOVER);
	if ( offset >= (pval->len[pval->arraycnt]) ) {
		puterror(HSPVAR_ERROR_ARRAYOVER);
	}
	pval->offset += offset * pval->arraymul;
#endif
	return;
}

// �Y���̏�����
void code_index_reset(PVal* pval)
{
	// �W���z��̏ꍇ�̂ݏ���������
	if ( PVal_supportArray(pval) && !(pval->support & HSPVAR_SUPPORT_ARRAYOBJ) ) {	
		HspVarCoreReset(pval);
	}
	return;
}

//------------------------------------------------
// 
//------------------------------------------------

//##########################################################
//    ��������G�~�����[�g
//##########################################################

//------------------------------------------------
// �A����� (�ʏ�z��)
// 
// @ 1�ڂ̑���͏I�����Ă���Ƃ���
// @ �������l���Ȃ� => do nothing
//------------------------------------------------
void code_assign_multi( PVal* pval )
{
	if ( !code_isNextArg() ) return;

	int const len1 = pval->len[1];
	assert(len1 > 0);

	// aptr = �ꎟ���ڂ̓Y�� + baseaptr ������
	APTR baseaptr = pval->offset % len1;
	APTR aptr = pval->offset - baseaptr;

	do {
		int const prm = code_getprm();				// ���ɑ������l���擾
		if ( prm <= PARAM_END ) puterror( HSPERR_SYNTAX );
	//	if ( !(pval->support & HSPVAR_SUPPORT_ARRAYOBJ) && pval->flag != mpval->flag ) {
	//		puterror( HSPERR_INVALID_ARRAYSTORE );	// �^�ύX�͂ł��Ȃ�
	//	}

		baseaptr ++;

		pval->arraycnt = 0;							// �z��w��J�E���^�����Z�b�g
		pval->offset   = aptr;
		code_index_int_lhs( pval, baseaptr );		// �z��`�F�b�N

		// ���
		PVal_assign( pval, mpval->pt, mpval->flag );
	} while ( code_isNextArg() );

	return;
}

//##########################################################
//    ���̑�
//##########################################################
//------------------------------------------------
// �������������ǂ���
// 
// @ ���ߌ`���A�֐��`���ǂ���ł��n�j
//------------------------------------------------
bool code_isNextArg(void)
{
	return !( *exinfo->npexflg & EXFLG_1 || ( *type == TYPE_MARK && *val == ')' ) );
}

//------------------------------------------------
// ���̈�����ǂݔ�΂�
// 
// @ exflg ���Ȃ�Ƃ����悤�Ƃ��Ă݂�B
// @ result : PARAM_* (code_getprm �Ɠ���)
//------------------------------------------------
int code_skipprm(void)
{
	int& exflg = *exinfo->npexflg;

	// �I��, or �ȗ�
	if ( exflg & EXFLG_1 ) return PARAM_END;	// �����A�����p�����[�^�[�I�[
	if ( exflg & EXFLG_2 ) {					// �p�����[�^�[��؂�(�f�t�H���g��)
		exflg &= ~EXFLG_2;
		return PARAM_DEFAULT;
	}

	if ( *type == TYPE_MARK ) {
		// �p�����[�^�[�ȗ���('?')
		if ( *val == 63 ) {
			code_next();
			exflg &= ~EXFLG_2;
			return PARAM_DEFAULT;

		// �֐����̃p�����[�^�[�ȗ���
		} else if ( *val == ')' ) {
			exflg &= ~EXFLG_2;
			return PARAM_ENDSPLIT;
		}
	}

	// �����̎��̓ǂݔ�΂�����
	for ( int lvBracket = 0; ; ) {			// �������[�v
		if ( *type == TYPE_MARK ) {
			if ( *val == '(' ) lvBracket ++;
			if ( *val == ')' ) lvBracket --;
		}
		code_next();

		if ( lvBracket == 0
			&& ( exflg & EXFLG_1 || exflg & EXFLG_2 || (*type == TYPE_MARK && *val == ')') )
		) {
			break;
		}
	}

	if ( exflg ) exflg &= ~EXFLG_2;

	// �I��
	if ( *type == TYPE_MARK && *val == ')' ) {
		return PARAM_SPLIT;

	} else {
		return PARAM_OK;
	}
}

//------------------------------------------------
// ���̓���̃R�[�h�𖳎�����
//------------------------------------------------
bool code_next_expect( int expect_type, int expect_val )
{
	if ( !(*type == expect_type && *val == expect_val) ) return false;
	code_next();
	return true;
}

} // namespace hpimod
