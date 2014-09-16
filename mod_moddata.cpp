// #module �֌W

#include <cstring>

#include "mod_moddata.h"
#include "mod_makepval.h"
//#include "stack.h"			// hsp3/stack.h

namespace hpimod {

//------------------------------------------------
// ���[�U��`�R�}���h�̎�����������o��
// 
// @ ������ prmstk �Ɋi�[�B
//------------------------------------------------
void code_expandstruct(void* prmstk, stdat_t stdat, int option)
{
	stprm_t pStPrm = getSTRUCTPRM( stdat->prmindex );
	
	for ( int i = 0; i < stdat->prmmax; ++ i, ++ pStPrm ) {
		char* const out = reinterpret_cast<char*>(prmstk) + pStPrm->offset;	// �X�^�b�N�̎��̃|�C���^
		
		switch ( pStPrm->mptype ) {
			// int, double, label
			case MPTYPE_INUM:  *reinterpret_cast<int*>(out)     = code_getdi(0); break;
			case MPTYPE_DNUM:  *reinterpret_cast<double*>(out)  = code_getd();   break;
			case MPTYPE_LABEL: *reinterpret_cast<label_t*>(out) = code_getlb();  break;
			
			// str
			case MPTYPE_LOCALSTRING:
			{
				char* const str = code_gets();
				char* const ls  = hspmalloc( (std::strlen(str) + 1) * sizeof(char) );
				std::strcpy( ls, str );
				*reinterpret_cast<char**>(out) = ls;
				break;
			}
			// modvar
			case MPTYPE_MODULEVAR:
			{
				auto const var = reinterpret_cast<MPModVarData*>(out);
				
				var->magic = MODVAR_MAGICCODE;		// �}�W�b�N�R�[�h
				var->subid = pStPrm->subid;			// modvar �̎�� Id
				var->aptr  = code_getva( &var->pval );
				break;
			}
			// modinit, modterm => �G�~�����[�g�s�\
			case MPTYPE_IMODULEVAR:
			case MPTYPE_TMODULEVAR:
				puterror( HSPERR_UNSUPPORTED_FUNCTION );
			//	*(MPModVarData *)out = modvar_init;
				break;
				
			// �Q��(var, array)
			case MPTYPE_SINGLEVAR:
			case MPTYPE_ARRAYVAR:
			{
				auto const var = reinterpret_cast<MPVarData*>(out);
				var->aptr = code_getva( &var->pval );
				break;
			}
			// ���[�J���ϐ�(local)
			case MPTYPE_LOCALVAR:
			{
				PVal* const pval = reinterpret_cast<PVal*>(out);
				
				// ���[�J���ϐ��̏������s���ꍇ (������ variant ����)
				if ( option & CODE_EXPANDSTRUCT_OPT_LOCALVAR ) {
					int const prm = code_getprm();
					
					if ( prm > PARAM_END ) {	// ����
						PVal_init( pval, mpval->flag );		// �ŏ��T�C�Y���m��
						PVal_assign( pval, mpval->pt, mpval->flag );
						break;
					}
				//	else	// �ȗ���
				}
				
				PVal_init( pval, HSPVAR_FLAG_INT );	// ���̌^�ŏ�����
				break;
			}
			
			// �\���̃^�O
		//	case MPTYPE_STRUCTTAG: break;
				
			default:
				puterror( HSPERR_INVALID_STRUCT_SOURCE );
		}
	}
	return;
}

//------------------------------------------------
// prmstack ����̂���
// 
// @cf. openhsp: hsp3code.cpp/customstack_delete
//------------------------------------------------
void customstack_delete( stdat_t stdat, void* prmstk )
{
	stprm_t pStPrm = getSTRUCTPRM( stdat->prmindex );
	for ( int i = 0; i < stdat->prmmax; ++ i, ++ pStPrm ) {		// �p�����[�^�[���擾
		char* const out = reinterpret_cast<char*>(prmstk) + pStPrm->offset;
		
		switch ( pStPrm->mptype ) {
			case MPTYPE_LOCALSTRING:
			{
				char* const ls = *reinterpret_cast<char**>(out);
				hspfree(ls);
				break;
			}
			case MPTYPE_LOCALVAR:
				PVal_free( reinterpret_cast<PVal*>(out) );
				break;
		}
	}
	return;
}


} // namespace hpimod
