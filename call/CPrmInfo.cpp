// ���������N���X

#include "mod_makepval.h"

#include "CPrmInfo.h"

using namespace hpimod;

CPrmInfo const CPrmInfo::undeclaredFunc = CPrmInfo( nullptr, true );
CPrmInfo const CPrmInfo::noprmFunc     = CPrmInfo( nullptr, false );

//##############################################################################
//                ��`�� : CPrmInfo
//##############################################################################
//------------------------------------------------
// �W���\�z
//------------------------------------------------
CPrmInfo::CPrmInfo(prmlist_t const* pPrmlist, bool bFlex)
	: mcntPrms( 0 )
	, mcntLocals( 0 )
	, mbFlex  ( bFlex )
{
	if ( pPrmlist ) {
		setPrmlist( *pPrmlist );	// prmlist �̕���
	}
	return;
}

//###############################################
//    �ݒ�n
//###############################################
//-----------------------------------------------
// �ϒ�������
//-----------------------------------------------
void CPrmInfo::setFlex( bool bFlex )
{
	mbFlex = bFlex;
	return;
}

//------------------------------------------------
// prmlist �̕���
//------------------------------------------------
void CPrmInfo::setPrmlist( prmlist_t const& prmlist )
{
	if ( prmlist.empty() ) return;

	mcntPrms   = 0;
	mcntLocals = 0;
	mprmlist.clear();
	mprmlist.reserve( mcntPrms );

	for ( auto& it : prmlist ) {
		if ( it == PRM_TYPE_FLEX ) {
			mbFlex = true;

		} else if ( it == PRM_TYPE_LOCAL ) {
			mcntLocals ++;

		} else {
			mprmlist.push_back( it );
			mcntPrms ++;
		}
	}

	return;
}

//###############################################
//    �擾�n
//###############################################
//-----------------------------------------------
// �����̐�
//-----------------------------------------------
size_t CPrmInfo::cntPrms() const
{
	return mcntPrms;
}

//-----------------------------------------------
// ���[�J���ϐ��̐�
//-----------------------------------------------
size_t CPrmInfo::cntLocals() const
{
	return mcntLocals;
}

//-----------------------------------------------
// �ϒ�������
//-----------------------------------------------
bool CPrmInfo::isFlex() const
{
	return mbFlex;
}

//-----------------------------------------------
// �������^�C�v
// 
// @ ���s����� PRM_TYPE_NONE ��Ԃ��B
//-----------------------------------------------
int CPrmInfo::getPrmType( size_t index ) const
{
	if ( index < 0 ) return PRM_TYPE_NONE;
	if ( index >= mcntPrms ) {
		return ( isFlex() ) ? PRM_TYPE_ANY : PRM_TYPE_NONE;	// �ϕ��� or �ߏ�
	}

	return mprmlist[index];
}

//-----------------------------------------------
// �X�^�b�N�T�C�Y�v�Z
// 
// @ �ϒ������͖�������B
//-----------------------------------------------
int CPrmInfo::getStackSize() const
{
	int sum = 0;

	for ( size_t i = 0; i < mcntPrms; ++ i ) {
		sum += PrmType_Size( getPrmType(i) );
	}

	sum += PrmType_Size( PRM_TYPE_LOCAL ) * mcntLocals;
	return sum;
}

// �ϒ��������� (cntFlex: �ϒ������̌�)
int CPrmInfo::getStackSizeWithFlex( size_t cntFlex ) const
{
	return getStackSize() + (PrmType_Size(PRM_TYPE_ANY) * cntFlex);
}

//-----------------------------------------------
// �������������ۂ�
//-----------------------------------------------
void CPrmInfo::checkCorrectArg( PVal const* pvArg, size_t iArg, bool bByRef ) const
{
	int const prmtype = getPrmType(iArg);

	// �ϒ�����
	if ( iArg >= cntPrms() ) {
		if ( isFlex() ) {
			// �K�����������Ƃɂ���
		} else {
			puterror( HSPERR_TOO_MANY_PARAMETERS );
		}

	// any
	} else if ( prmtype == PRM_TYPE_ANY ) {
		// OK

	// �Q�Ɠn���v��
	} else if ( prmtype == PRM_TYPE_VAR || prmtype == PRM_TYPE_ARRAY ) {
		if ( !bByRef ) {
			puterror( HSPERR_VARIABLE_REQUIRED );
		}

	// �^�s��v
	} else if ( prmtype != pvArg->flag ) {
		puterror( HSPERR_TYPE_MISMATCH );
	}

	return;
}

//-----------------------------------------------
// �ȗ��l���擾
// 
// @ �ȗ��ł��Ȃ� => �G���[
//-----------------------------------------------
PVal* CPrmInfo::getDefaultArg( size_t iArg ) const
{
	// �ϒ�����
	if ( iArg >= cntPrms() ) {
		if ( isFlex() ) {
			return PVal_getDefault();
		} else {
			puterror( HSPERR_TOO_MANY_PARAMETERS );
		}
	}

	int const prmtype = getPrmType(iArg);

	switch ( prmtype ) {
		// �ʏ퉼�����ŁA����l�̂���^
		case HSPVAR_FLAG_STR:
		case HSPVAR_FLAG_DOUBLE:
		case HSPVAR_FLAG_INT:
			return PVal_getDefault( prmtype );

		// �ʏ퉼�����ŁA�ȗ��s��
		case HSPVAR_FLAG_LABEL:  puterror( HSPERR_LABEL_REQUIRED );
		case HSPVAR_FLAG_STRUCT: puterror( HSPERR_STRUCT_REQUIRED );

		// any
		case PRM_TYPE_ANY:
			return PVal_getDefault();

		default:
			// �Q�Ɠn���v��
			if ( PrmType_IsRef(prmtype) ) puterror( HSPERR_VARIABLE_REQUIRED );

			// ���̑�
			puterror( HSPERR_NO_DEFAULT );
	}

	throw;	// �x���}��
}

//################################################
//    ���Z�q
//################################################
//------------------------------------------------
// ��r
//------------------------------------------------
int CPrmInfo::compare( CPrmInfo const& rhs ) const
{
	if ( mbFlex   != rhs.mbFlex   ) return (mbFlex   ? 1 : -1);
	if ( mcntPrms != rhs.mcntPrms ) return mcntPrms - rhs.mcntPrms;

	for ( size_t i = 0; i < mcntPrms; ++ i ) {
		int const diff = getPrmType(i) - rhs.getPrmType(i);
		if ( diff ) return diff;
	}
	return 0;
}

//################################################
//    ���������o�֐�
//################################################
//------------------------------------------------
// ����
//------------------------------------------------
CPrmInfo& CPrmInfo::copy( CPrmInfo const& src )
{
	mcntPrms   = src.mcntPrms;
	mcntLocals = src.mcntLocals;
	mbFlex     = src.mbFlex;
	mprmlist   = src.mprmlist;
	return *this;
}

//------------------------------------------------
// CPrmInfo <- STRUCTDAT
//------------------------------------------------
CPrmInfo CPrmInfo::Create(stdat_t stdat)
{
	CPrmInfo::prmlist_t prmlist;
	prmlist.reserve(stdat->prmmax);

	stprm_t const stprm = hpimod::STRUCTDAT_getStPrm(stdat);

	for ( int i = 0; i < stdat->prmmax; ++i ) {
		int const prmtype = PrmType_FromMPType(stprm[i].mptype);
		if ( prmtype != PRM_TYPE_NONE ) {
			prmlist.push_back(prmtype);
		}
	}
	return CPrmInfo(&prmlist, false);
}

//##############################################################################
//                ���̑��̊֐�
//##############################################################################
//------------------------------------------------
// �Q�Ɠn���̉������^�C�v��
//------------------------------------------------
bool PrmType_IsRef(int prmtype)
{
	return prmtype == PRM_TYPE_VAR
		|| prmtype == PRM_TYPE_ARRAY
		|| prmtype == PRM_TYPE_MODVAR
		;
}

//------------------------------------------------
// �������^�C�v�� prmstack �ɗv������T�C�Y
//------------------------------------------------
int PrmType_Size(int prmtype)
{
	switch ( prmtype ) {
		case HSPVAR_FLAG_LABEL:  return sizeof(label_t);
		case HSPVAR_FLAG_STR:    return sizeof(char*);
		case HSPVAR_FLAG_DOUBLE: return sizeof(double);
		case HSPVAR_FLAG_INT:    return sizeof(int);
		case PRM_TYPE_VAR:
		case PRM_TYPE_ARRAY:
		case PRM_TYPE_ANY:    return sizeof(MPVarData);
		case PRM_TYPE_MODVAR: return sizeof(MPModVarData);
		case PRM_TYPE_LOCAL:  return sizeof(PVal);
		default:
			// ���̑��̌^�^�C�v�l
			if ( HSPVAR_FLAG_INT < prmtype && prmtype < (HSPVAR_FLAG_USERDEF + ctx->hsphed->max_varhpi) ) {
				return sizeof(MPVarData);
			}
			return 0;
	}
}

//------------------------------------------------
// prmtype <- mptype
//------------------------------------------------
int PrmType_FromMPType(int mptype)
{
	switch ( mptype ) {
		case MPTYPE_LABEL:       return HSPVAR_FLAG_LABEL;
		case MPTYPE_LOCALSTRING: return HSPVAR_FLAG_STR;
		case MPTYPE_DNUM:        return HSPVAR_FLAG_DOUBLE;
		case MPTYPE_INUM:        return HSPVAR_FLAG_INT;
		case MPTYPE_SINGLEVAR:   return PRM_TYPE_VAR;
		case MPTYPE_ARRAYVAR:    return PRM_TYPE_ARRAY;
		case MPTYPE_TMODULEVAR:  return PRM_TYPE_FLEX; // destructor �͉ϒ������Ƃ������Ƃɂ��� (modcls �Ŏg������)
		case MPTYPE_IMODULEVAR:	//
		case MPTYPE_MODULEVAR:   return PRM_TYPE_MODVAR;
		case MPTYPE_LOCALVAR:    return PRM_TYPE_LOCAL;

		default:
			return PRM_TYPE_NONE;
	}
}
