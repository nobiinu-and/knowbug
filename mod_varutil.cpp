// �ϐ��֘A utility

#include "mod_varutil.h"
#include "mod_argGetter.h"

namespace hpimod {

//##########################################################
//        HspVarProc �֌W
//##########################################################

//##########################################################
//        �ϐ����̎擾
//##########################################################

//------------------------------------------------
// �ϐ��̏��𓾂�
//------------------------------------------------
int code_varinfo(PVal* pval)
{
	int const varinfo = code_geti();
	switch ( varinfo ) {
		case VarInfo_Flag:   return static_cast<int>(pval->flag);
		case VarInfo_Mode:   return static_cast<int>(pval->mode);
		case VarInfo_Size:   return pval->size;
		case VarInfo_Ptr:    return reinterpret_cast<UINT_PTR>(pval->pt);
		case VarInfo_Master: return reinterpret_cast<UINT_PTR>(pval->master);
		default:
			if ( VarInfo_Len0 <= varinfo && varinfo <= VarInfo_Len4 ) {
				return pval->len[varinfo - VarInfo_Len0];
			}
			puterror(HSPERR_ILLEGAL_FUNCTION); throw;
	}
}



//##########################################################
//        �ϐ��̍쐬
//##########################################################
//------------------------------------------------
// dimtype
//------------------------------------------------
void code_dimtype(PVal* pval)
{
	vartype_t const vtype = code_geti();
	int len[4] {};

	for ( int i = 0; i < ArrayDimMax; ++i ) {
		len[i] = code_getdi(0);
		if ( len[i] < 0 ) puterror(HSPERR_ILLEGAL_FUNCTION);
		if ( len[i] == 0 ) break;
	}
	exinfo->HspFunc_dim(pval, vtype, 0, len[0], len[1], len[2], len[3]);
	return;
}

//------------------------------------------------
// pval �� dimtype ����
// 
// @ x(a), y(b), z(c) ... �ɂ��ꎟ���z��̘A���������B
// @ dim �I�p�@�B
//------------------------------------------------
int dimtypeEx( vartype_t vflag, DimFunc_t fDim )
{
	// �Ђ����烋�[�v����
	for ( int i = 0; code_isNextArg(); ++ i ) {

		// �ϐ���PVal�|�C���^�Ɨv�f�����擾
		PVal* pval;
		APTR const aptr = code_getva( &pval );

		if ( i == 0 && aptr <= 0 ) {	// �擪�ŁA���� () �Ȃ�
			// dimtype ����
			int idx[4];
			for ( int i = 0; i < 4; ++ i ) {
				idx[i] = code_getdi(0);
			}
			if ( fDim ) {
				(*fDim)(pval, vflag, 0, idx[0], idx[1], idx[2], idx[3]);
			} else {
				exinfo->HspFunc_dim(pval, vflag, 0, idx[0], idx[1], idx[2], idx[3]);
			}
			break;
		}

		if ( fDim ) {
			(*fDim)(pval, vflag, 0, aptr, 0, 0, 0);
		} else {
			exinfo->HspFunc_dim(pval, vflag, 0, aptr, 0, 0, 0);
		}
	}

	return 0;
}

//------------------------------------------------
// �Y������ aptr �l���쐬����
//------------------------------------------------
APTR CreateAptrFromIndex(PVal const* pval, int idx[4])
{
	int multiple[4];

	multiple[0] = 1;

	// 1 ������ʂ��Z�o
	for ( int i = 1; i < 4; ++ i ) {
		multiple[i] = multiple[i - 1] * pval->len[i];
	}

	// �|�����킹�邾��
	return (
		idx[0] * multiple[0] +
		idx[1] * multiple[1] +
		idx[2] * multiple[2] +
		idx[3] * multiple[3]
	);
}

//------------------------------------------------
// aptr ����Y�������߂�
//------------------------------------------------
void GetIndexFromAptr(PVal const* pval, APTR aptr, int ret[])
{
	APTR tmpAptr = aptr;

	for ( int i = 0; i < ArrayDimMax; ++ i ) ret[i] = 0;

	for ( int i = 0; i < pval->arraycnt; ++ i ) {
		if ( pval->len[i + 1] ) {
			ret[i] = tmpAptr %  pval->len[i + 1];
			         tmpAptr /= pval->len[i + 1];
			if ( tmpAptr == 0 ) break;
		}
	}
	return;
}

} // namespace hpimod
