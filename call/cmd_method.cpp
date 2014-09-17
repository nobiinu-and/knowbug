// Call(Method) - Command

#include <vector>
#include <map>
#include <string>

#include "cmd_method.h"
#include "cmd_sub.h"

#include "CCaller.h"
#include "CCall.h"
#include "CPrmInfo.h"

#include "CMethod.h"
#include "CMethodlist.h"

//################################################
//    �O���[�o���ϐ�
//################################################
static auto g_pMethodlist = (new CMethodlist);

//################################################
//    �O���[�o���֐�
//################################################
static void ObjectMethodCustom(PVal* pval);

static void Method_replace_proc(int vt);

//------------------------------------------------
// ���\�b�h�Ăяo���֐��̂���ւ�
// 
// @prm p1 = vt : �^�^�C�v�l
//------------------------------------------------
void Method_replace(void)
{
	int const vt = code_get_vartype();
	Method_replace_proc(vt);
	return;
}

static void Method_replace_proc(int vt)
{
	HspVarProc* const vp = getHvp( vt );

	// �����o�̎��֐��|�C���^������������
	vp->ObjectMethod = ObjectMethodCustom;

	// ��̃��\�b�h�N���X�����A�o�^���Ă���
	g_pMethodlist->set( vt );
	return;
}

//------------------------------------------------
// ���\�b�h�̒ǉ�
// 
// @prm p1 = vt  : �^�^�C�v�l
// @prm p2 = str : ���\�b�h���� (or default)
// @prm p3 = def : ��` (���x�� + ���������X�g, axcmd)
//------------------------------------------------
void Method_add()
{
	vartype_t const vtype = code_get_vartype();
	std::string const name = code_gets();

	// �Ăяo���� or ���x���֐��錾�̎擾
	functor_t&& functor = code_get_functor();

#if 0
	// ���x�� => ���������X�g���󂯎��
	if ( functor.getType() == FuncType_Label ) {
		CPrmInfo::prmlist_t&& prmlist = code_get_prmlist();
		prmlist.insert( prmlist.begin(), PRM_TYPE_VAR );		// �擪�� var this ��ǉ�
		DeclarePrmInfo( functor.getLabel(), std::move(CPrmInfo(&prmlist)) );
	}
#endif

	// CMethod �ɒǉ�
	CMethod* const pMethod = g_pMethodlist->get( vtype );

	if ( pMethod ) {
		pMethod->add( name, functor );

	} else {
		// �G���[�^�C�v����
		dbgout("Method_replace ����Ă��܂���I");
		puterror( HSPERR_UNSUPPORTED_FUNCTION );
	}

	return;
}

//------------------------------------------------
// ���\�b�h�Ăяo�����̕ϐ��̃N���[�������
//------------------------------------------------
void Method_cloneThis()
{
	CCall* const pCall     = TopCallStack();
	PVal*  const pvalClone = code_getpval();

	PVal_clone( pvalClone, pCall->getArgPVal(0), pCall->getArgAptr(0) );
	return;
}

//##############################################################################
//                method ������
//##############################################################################
//------------------------------------------------
// ���\�b�h�Ăяo���֐� ( method.hpi �� )
//------------------------------------------------
static void ObjectMethodCustom(PVal* pval)
{
	int const vt = pval->flag;
	std::string const name = code_gets();

	CMethod* const pMethod = g_pMethodlist->get( vt );

	if ( pMethod ) {
		pMethod->call( name, pval );
	} else {
		// Method_replace ���Ă��Ȃ��^�̃��\�b�h
		puterror( HSPERR_UNSUPPORTED_FUNCTION );
	}
	return;
}
