// call - SubCommand

#include <stack>
#include <map>

#include "hsp3plugin_custom.h"
#include "mod_varutil.h"

#include "cmd_sub.h"

#include "CCaller.h"
#include "CCall.h"
#include "CPrmInfo.h"
#include "CFunctor.h"

#include "CBound.h"

#include "vt_functor.h"

using namespace hpimod;

//##########################################################
//       ���������X�g
//##########################################################
// �������錾�f�[�^
static std::map<label_t, CPrmInfo> g_prmlistLabel;
static std::map<stdat_t, CPrmInfo> g_prmlistModcmd;

//------------------------------------------------
// ���������X�g�̐錾
//------------------------------------------------
void DeclarePrmInfo(label_t lb, CPrmInfo&& prminfo)
{
	using PrmInfoList_t = std::map<label_t, CPrmInfo>;

	if ( GetPrmInfo(lb) != CPrmInfo::undeclaredFunc ) {
		dbgout("���d��`�ł��B");
		puterror(HSPERR_ILLEGAL_FUNCTION);
	}

	g_prmlistLabel.insert({ lb, std::move(prminfo) });
	return;
}

//------------------------------------------------
// �������̎擾 (���x��)
//------------------------------------------------
CPrmInfo const& GetPrmInfo(label_t lb)
{
	auto const& iter = g_prmlistLabel.find(lb);
	return (iter != g_prmlistLabel.end())
		? iter->second
		: CPrmInfo::undeclaredFunc;		// �Ȃ�
}

//------------------------------------------------
// �������̎擾 (���[�U��`�֐�)
//------------------------------------------------
CPrmInfo const& GetPrmInfo(stdat_t pStDat)
{
	using PrmInfoList_t = decltype(g_prmlistModcmd);

	// STRUCTDAT -> CPrmInfo : �ϊ����ʂ̓L���b�V������
	auto const& iter = g_prmlistModcmd.find(pStDat);
	if ( iter != g_prmlistModcmd.end() ) {
		return iter->second;
	}

	return g_prmlistModcmd.insert({ pStDat, CPrmInfo::Create(pStDat) })
		.first->second;
}

//------------------------------------------------
// CPrmInfo <- ���ԃR�[�h
// 
// @prm: [ label, (prmlist) ]
//------------------------------------------------
CPrmInfo code_get_prminfo()
{
	CFunctor const&& functor = code_get_functor();

	if ( functor.getType() == FuncType_Label ) {
		CPrmInfo::prmlist_t&& prmlist = code_get_prmlist();
		return CPrmInfo(&prmlist);

	} else if ( functor.getType() == FuncType_Deffid ) {
		return CPrmInfo::Create(getSTRUCTDAT(AxCmd::getCode(functor.getAxCmd())));

	} else {
		return functor.getPrmInfo();
	}
}

//------------------------------------------------
// ����������擾����
//------------------------------------------------
CPrmInfo::prmlist_t code_get_prmlist()
{
	CPrmInfo::prmlist_t prmlist;

	// ���������X�g
	while ( code_isNextArg() ) {
		int const prmtype = code_get_prmtype(PRM_TYPE_NONE);
		if ( prmtype == PRM_TYPE_NONE ) continue;

		prmlist.push_back(prmtype);
	}

	return std::move(prmlist);
}

//##########################################################
//       �����擾
//##########################################################
//------------------------------------------------
// �������^�C�v���擾����
//------------------------------------------------
int code_get_prmtype( int deftype )
{
	int const prm = code_getprm();
	if ( prm <= PARAM_END ) {
		if ( prm == PARAM_DEFAULT ) return deftype;
		return PRM_TYPE_NONE;
	}

	switch ( mpval->flag ) {
		// ���l�Ȃ炻�̂܂ܕԂ�
		case HSPVAR_FLAG_INT:
			return *reinterpret_cast<int*>( mpval->pt );

		// ������ => ���ꕶ���� or �^��( HspVarProc ����擾 )
		case HSPVAR_FLAG_STR:
		{
			int const prmtype = GetPrmType( mpval->pt );

			if ( prmtype == PRM_TYPE_NONE ) puterror( HSPERR_ILLEGAL_FUNCTION );
			return prmtype;
		}
		default:
			puterror( HSPERR_TYPE_MISMATCH );
	}
	throw;	// �x���}��
}

//------------------------------------------------
// �֐��q���擾����
//------------------------------------------------
CFunctor code_get_functor()
{
	{
		int const chk = code_getprm();
		if ( chk <= PARAM_END ) puterror( HSPERR_NO_DEFAULT );
	}

	// label
	if ( mpval->flag == HSPVAR_FLAG_LABEL ) {
		return CFunctor( *reinterpret_cast<label_t*>( mpval->pt ) );

	// deffid
	} else if ( mpval->flag == HSPVAR_FLAG_INT ) {
		int const axcmd = *reinterpret_cast<int*>( mpval->pt );

		if ( AxCmd::getType(axcmd) != TYPE_MODCMD ) puterror( HSPERR_ILLEGAL_FUNCTION );

		return CFunctor(axcmd);

	// functor
	} else if ( mpval->flag == HSPVAR_FLAG_FUNCTOR ) {
		functor_t& functor = *reinterpret_cast<functor_t*>( mpval->pt );
		return functor;

	} else {
		puterror( HSPERR_LABEL_REQUIRED );
		throw;
	}
}

//##########################################################
//       ���̑�
//##########################################################

//------------------------------------------------
// prmtype ���擾���� (from ������)
// 
// @result: prmtype (���s => PRM_TYPE_NONE)
//------------------------------------------------
int GetPrmType( char const* s )
{
	{
		HspVarProc* const vp = seekHvp( s );
		if ( vp ) return vp->flag;
	}

	if ( !strcmp(s, "var"   ) ) return PRM_TYPE_VAR;
	if ( !strcmp(s, "array" ) ) return PRM_TYPE_ARRAY;
	if ( !strcmp(s, "any"   ) ) return PRM_TYPE_ANY;
	if ( !strcmp(s, "modvar") ) return PRM_TYPE_MODVAR;
	if ( !strcmp(s, "local" ) ) return PRM_TYPE_LOCAL;
	if ( !strcmp(s, "..."   )
	  || !strcmp(s, "flex"  ) ) return PRM_TYPE_FLEX;
	return PRM_TYPE_NONE;
}

//------------------------------------------------
// �\���̃p�����[�^���Q�Ƃ��Ă��� prmstk ��(���݂̏�񂩂�)�擾����
// 
// @result: prmstk �̈�ւ̃|�C���^ (���s => nullptr)
//------------------------------------------------
void* GetReferedPrmstk(stprm_t pStPrm)
{
	void* const cur_prmstk = ctx->prmstack;
	if ( !cur_prmstk ) return nullptr;

	if ( pStPrm->subid == STRUCTPRM_SUBID_STACK ) {		// ������
		return cur_prmstk;

	} else if ( pStPrm->subid >= 0 ) {	// �����o�ϐ�
		auto const thismod = reinterpret_cast<MPModVarData*>(cur_prmstk);
		return reinterpret_cast<FlexValue*>(PVal_getptr( thismod->pval, thismod->aptr ))->ptr;
	}

	return nullptr;
}

//##############################################################################
//                �������֐�
//##############################################################################
