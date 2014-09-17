// call - command.cpp

#include <stack>
#include <set>

#include "hsp3plugin_custom.h"
#include "mod_func_result.h"

#include "CCall.h"
#include "CCaller.h"

#include "Functor.h"
#include "CBound.h"
#include "CStreamCaller.h"
#include "CLambda.h"
#include "CCoRoutine.h"

#include "cmd_call.h"
#include "cmd_sub.h"

#include "vt_functor.h"

using namespace hpimod;

//------------------------------------------------
// ���x�����߁E�֐��̌Ăяo��
//------------------------------------------------
int Call( void** ppResult )
{
	vartype_t restype;

	// �Ăяo������
	{
		CCaller caller;

		// �Ăяo���O�̏���
		caller.setFunctor();
		caller.setArgAll();

		// �Ăяo��
		caller.call();

		// �Ăяo����̏���
		restype = caller.getCallResult( ppResult );
	}

	return restype;
}

//------------------------------------------------
// ���x�����߁E�֐��̉������錾
//------------------------------------------------
void Call_declare()
{
	label_t const lb = code_getlb();
	if ( !lb ) puterror( HSPERR_ILLEGAL_FUNCTION );

	CPrmInfo::prmlist_t&& prmlist = code_get_prmlist();		// ��������

	// �o�^
	DeclarePrmInfo( lb, CPrmInfo(&prmlist) );
	return;
}

#if 0
//##########################################################
//        �������擾
//##########################################################
//------------------------------------------------
// �Ăяo�����������擾����֐�
//------------------------------------------------
int Call_arginfo(void** ppResult)
{
	CCall* const pCall = TopCallStack();

	auto const id = static_cast<ARGINFOID>( code_geti() );	// �f�[�^�̎��
	int const iArg = code_getdi(-1);							// �����̔ԍ�

	return SetReffuncResult( ppResult, pCall->getArgInfo(id, iArg) );
}

//------------------------------------------------
// �����̒l���擾����
//------------------------------------------------
int Call_argv(void** ppResult)
{
	CCall* const pCall = TopCallStack();

	int const iArg = code_getdi( 0 );		// �����̔ԍ�

	// �Ԓl��ݒ肷��
	PVal* const pvalArg = pCall->getArgv( iArg );
	vartype_t const rettype = pvalArg->flag;		// �Ԓl�̌^��ݒ�
	*ppResult = PVal_getptr(pvalArg);				// ���̃|�C���^�����߂�

	return rettype;
}

//------------------------------------------------
// ���x���֐��̕Ԓl��ݒ肷��
//------------------------------------------------
void Call_retval()
{
	CCall* const pCall = TopCallStack();

	if ( code_getprm() <= PARAM_END ) return;		// �ȗ���

	vartype_t const vt = mpval->flag;
	void* const pValue = PVal_getptr( mpval );
	pCall->setResult( pValue, vt );
	return;
}

//------------------------------------------------
// ���x���֐��̕Ԓl�ƂȂ����l�����o��
// 
// @ call �I����ɌĂяo�����B
//------------------------------------------------
int Call_result(void** ppResult)
{
	PVal* const pvResult = CCaller::getLastRetVal();

	*ppResult = pvResult->pt;
	return pvResult->flag;
}

//------------------------------------------------
// local �ϐ��̒l���擾����
//------------------------------------------------
int Call_getLocal( void** ppResult )
{
	CCall* const pCall = TopCallStack();

	// �O���琔�������[�J���ϐ��̈ʒu
	int const idxLocal = code_geti();

	PVal* const pvLocal = pCall->getLocal( idxLocal );
	if ( !pvLocal ) puterror( HSPERR_ILLEGAL_FUNCTION );

	*ppResult = pvLocal->pt;
	return pvLocal->flag;
}

//------------------------------------------------
// �G�C���A�X�ɂ���
//------------------------------------------------
void Call_alias()
{
	CCall* const pCall = TopCallStack();

	PVal* const pval = code_get_var();
	int const iArg = code_getdi( 0 );

	pCall->alias( pval, iArg );
	return;
}


//------------------------------------------------
// �G�C���A�X�����ꊇ���ĕt����
//------------------------------------------------
void Call_aliasAll()
{
	CCall* const pCall = TopCallStack();

	// �񋓂��ꂽ�ϐ����G�C���A�X�ɂ���
	for( size_t i = 0
		; code_isNextArg() && ( i < pCall->getCntArg() )
		; ++ i
	) {
		// �ϐ��łȂ���Ζ�������
		if ( *type != TYPE_VAR ) {
			code_getprm();

		// �ϐ����擾
		} else {
			PVal* const pval = code_get_var();
			pCall->alias( pval, i );		// pval�������̃N���[���ɂ���
		}
	}
	return;
}


//------------------------------------------------
// �Ăяo���ꂽ���x��
//------------------------------------------------
int Call_thislb(void** ppResult)
{
	return SetReffuncResult( ppResult, TopCallStack()->getFunctor()->getLabel() );
}


//##########################################################
//        �����X�g���[���Ăяo��
//##########################################################
static std::stack<CCaller*> g_stkStream;

//------------------------------------------------
// �����X�g���[���Ăяo��::�J�n
//------------------------------------------------
void Call_StreamBegin()
{
	// �Ăяo���O�̏���
	g_stkStream.push( new CCaller() );
	CCaller* const pCaller = g_stkStream.top();

	// ���x���̐ݒ�
	if ( code_isNextArg() ) {
		Call_StreamLabel();
	}
	return;
}


//------------------------------------------------
// �����X�g���[���Ăяo��::���x���ݒ�
//------------------------------------------------
void Call_StreamLabel()
{
	if ( g_stkStream.empty() ) puterror( HSPERR_NO_DEFAULT );

	CCaller* const pCaller = g_stkStream.top();

	// �W�����v��̌���
	pCaller->setFunctor();
	return;
}


//------------------------------------------------
// �����X�g���[���Ăяo��::�ǉ�
//------------------------------------------------
void Call_StreamAdd()
{
	if ( g_stkStream.empty() ) puterror( HSPERR_NO_DEFAULT );

	CCaller* const pCaller = g_stkStream.top();

	// ������ǉ�����
	pCaller->setArgAll();

	return;
}


//------------------------------------------------
// �����X�g���[���Ăяo��::����
// 
// @ ���ߌ`���̏ꍇ�� ppResult == nullptr �B
//------------------------------------------------
int Call_StreamEnd(void** ppResult)
{
	if ( g_stkStream.empty() ) puterror( HSPERR_NO_DEFAULT );

	CCaller* const pCaller = g_stkStream.top();

	// �Ăяo��
	pCaller->call();

	// �㏈��
	g_stkStream.pop();

	vartype_t const restype = pCaller->getCallResult( ppResult );

	delete pCaller;
	return restype;
}


//------------------------------------------------
// �X�g���[���Ăяo���I�u�W�F�N�g::����
//------------------------------------------------
int Call_NewStreamCaller( void** ppResult )
{
	stream_t const stream = CStreamCaller::New();

	// ��������
	CCaller* const caller = stream->getCaller();
	{
		caller->setFunctor();
	}

	// functor �^�Ƃ��ĕԋp����
	return SetReffuncResult( ppResult, functor_t::make(stream) );
}


//------------------------------------------------
// �X�g���[���Ăяo���I�u�W�F�N�g::�ǉ�
//------------------------------------------------
void Call_StreamCallerAdd()
{
	functor_t&& functor = code_get_functor();
	stream_t const stream = functor->safeCastTo<stream_t>();

	stream->getCaller()->setArgAll();		// �S�Ă̈�����ǉ�����
	return;
}

//##########################################################
//    ���[�U��`���ߊ֌W
//##########################################################
//------------------------------------------------
// �R�}���h�𐔒l�����Ď擾����
//------------------------------------------------
int AxCmdOf(void** ppResult)
{
	int const axcmd = code_get_axcmd();
	if ( axcmd == 0 ) puterror( HSPERR_TYPE_MISMATCH );
	return SetReffuncResult( ppResult, axcmd );
}

//------------------------------------------------
// ���[�U��`���߁E�֐����烉�x�����擾����
//------------------------------------------------
int LabelOf(void** ppResult)
{
	int axcmd  = 0;
	label_t lb = nullptr;

	// ���[�U��`�R�}���h
	if ( *type == TYPE_MODCMD ) {
		axcmd = code_get_axcmd();

	// ���̑�
	} else {
		if ( code_getprm() <= PARAM_END ) puterror( HSPERR_NO_DEFAULT );

		// axcmd
		if ( mpval->flag == HSPVAR_FLAG_INT ) {
			axcmd = VtTraits<int>::derefValptr(mpval->pt);

		// label
		} else if ( mpval->flag == HSPVAR_FLAG_LABEL ) {
			lb = VtTraits<label_t>::derefValptr( mpval->pt );

		// functor
		} else if ( mpval->flag == g_vtFunctor ) {
			lb = FunctorTraits::derefValptr(mpval->pt)->getLabel();
		}
	}

	// ���[�U��`�R�}���h �� ���x��
	if ( !lb ) {
		if ( AxCmd::getType(axcmd) == TYPE_MODCMD ) {
			lb = getLabel( getSTRUCTDAT( AxCmd::getCode(axcmd) )->otindex );

		} else {
			puterror( HSPERR_ILLEGAL_FUNCTION );
		}
	}

	return SetReffuncResult( ppResult, lb );
}

//##########################################################
//    functor �^�֌W
//##########################################################
//------------------------------------------------
// �^�ϊ��֐�
//------------------------------------------------
int Functor_cnv(void** ppResult)
{
	HspVarProc* const vp = g_hvpFunctor;

	int prm = code_getprm();
	if ( prm <= PARAM_END ) puterror( HSPERR_NO_DEFAULT );

	// �ϊ����Ċi�[
	*ppResult = vp->Cnv( mpval->pt, mpval->flag );

	return g_vtFunctor;
}

//------------------------------------------------
// ���������
//------------------------------------------------
int Functor_argc( void** ppResult )
{
	functor_t&& f = code_get_functor();
	return SetReffuncResult( ppResult, static_cast<int>( f->getPrmInfo().cntPrms() ) );
}

int Functor_isFlex( void** ppResult )
{
	functor_t&& f = code_get_functor();
	return SetReffuncResult( ppResult, HspBool(f->getPrmInfo().isFlex()) );
}

//##########################################################
//    ���̑�
//##########################################################
//------------------------------------------------
// �R�}���h�̌Ăяo�� (���ߌ`��)
//------------------------------------------------
void CallCmd_sttm()
{
	int const id = code_geti();

	if ( !(id & AxCmd::MagicCode) ) puterror( HSPERR_ILLEGAL_FUNCTION );

	{
		*type = AxCmd::getType( id );		// �w�肵���R�}���h�����邱�Ƃɂ��� (����ɂ��1�̃R�[�h�ׂ��̂ŁA�_�~�[��z�u���ׂ�)
		*val  = AxCmd::getCode( id );
		*exinfo->npexflg = EXFLG_1;
	}
	return;
}

//------------------------------------------------
// �R�}���h�̌Ăяo�� (�֐��`��)
//------------------------------------------------
int CallCmd_func( void** ppResult )
{
	int const id = code_geti();
	if ( !(id & AxCmd::MagicCode) ) puterror( HSPERR_ILLEGAL_FUNCTION );

	// �֐����Ăяo�� (���̕Ԓl�� call_func ���̂̕Ԓl�Ƃ���)
	{
		*type = AxCmd::getType( id );
		*val  = AxCmd::getCode( id );
		*exinfo->npexflg = 0;

		if ( code_getprm() <= PARAM_END ) puterror( HSPERR_NO_DEFAULT );
	}

	*ppResult = mpval->pt;
	return mpval->flag;
}

//------------------------------------------------
// ��������
//------------------------------------------------
int ArgBind( void** ppResult )
{
	bound_t const bound = CBound::New();

	// ��������
	CCaller* const caller = bound->getCaller();
	{
		caller->setFunctor();	// �X�N���v�g����푩���֐������o��
		caller->setArgAll();	// �X�N���v�g����^����ꂽ������S�Ď󂯎�� (�s�����������󂯕t����)
	}
	bound->bind();

	// functor �^�Ƃ��ĕԋp����
	return SetReffuncResult( ppResult, functor_t::make(bound) );
}

//------------------------------------------------
// ��������
//------------------------------------------------
int UnBind( void** ppResult )
{
	if ( code_getprm() <= PARAM_END ) puterror( HSPERR_NO_DEFAULT );
	if ( mpval->flag != g_vtFunctor ) puterror( HSPERR_TYPE_MISMATCH );

	auto const functor = FunctorTraits::derefValptr(mpval->pt);
	auto const bound   = functor->safeCastTo<bound_t>();

	return SetReffuncResult( ppResult, bound->unbind() );
}

//------------------------------------------------
// �����֐��̈ꊇ���
//------------------------------------------------
void ReleaseBounds()
{
	g_resFunctor.clear();
	return;
}

//------------------------------------------------
// �����_��
// 
// @ funcexpr(args...) �� function() { lambdaBody args... : return }
//------------------------------------------------
int Call_Lambda( void** ppResult )
{
	auto const lambda = CLambda::New();

	lambda->code_get();

	return SetReffuncResult( ppResult, functor_t::make(lambda) );
}

//------------------------------------------------
// lambda �������ŗp����R�}���h
// 
// @ LambdaBody:
// @	���������Ɏ��o���A���ꂼ��� local �ϐ��ɑ�����Ă����B
// @	�Ō�̈����͕Ԓl�Ƃ��Ď󂯎��B
// @	�䂦�ɁA���̖��߂̈����͕K�� (local �ϐ��̐� + 1) ���݂���B
// @ LambdaValue:
// @	idx �Ԗڂ� local �ϐ������o���B
//------------------------------------------------
void Call_LambdaBody()
{
	CCall* const pCall     = TopCallStack();
	size_t const cntLocals = pCall->getPrmInfo().cntLocals();

	for ( size_t i = 0; i < cntLocals; ++ i ) {
		code_getprm();
		PVal* const pvLocal = pCall->getLocal( i );
		PVal_assign( pvLocal, mpval->pt, mpval->flag );
	}

	Call_retval();
	return;
}

//------------------------------------------------
// �R���[�`��::����
//------------------------------------------------
int Call_CoCreate( void** ppResult )
{
	auto coroutine = CCoRoutine::New();

	CCaller* const caller = coroutine->getCaller();
	caller->setFunctor();		// functor ���󂯂�
	caller->setArgAll();

	return SetReffuncResult( ppResult, functor_t::make(coroutine) );
}

//------------------------------------------------
// �R���[�`��::���f����
//------------------------------------------------
void Call_CoYieldImpl()
{
	Call_retval();		// �Ԓl���󂯎��

	// newlab �����ϐ����R���[�`���ɓn��
	PVal* const pvNextLab = code_get_var();
	CCoRoutine::setNextVar( pvNextLab );	// static �ϐ��Ɋi�[����

	return;
}
#endif

//------------------------------------------------
// �I������
//------------------------------------------------
void Call_Term()
{
	CCaller::releaseLastRetVal();
	return;
}

//##########################################################
//    �e�X�g�R�[�h
//##########################################################
#ifdef _DEBUG

void CallHpi_test(void)
{
	//

	return;
}

#endif
