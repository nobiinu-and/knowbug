// hsp3plugin �g���w�b�_ (for ue_dai)

#ifndef IG_HSP3PLUGIN_CUSTOM_H
#define IG_HSP3PLUGIN_CUSTOM_H

#include <windows.h>
#undef max
#undef min

#include "../hspsdk/hsp3plugin.h"
#undef stat	// �������̕W�����C�u�����ƏՓ˂��Ă��܂��̂�

#include "./basis.h"

namespace hpimod
{

// var ���������o�� (code_getva() �Ɠ������� aptr �l�ł͂Ȃ� PVal* ��Ԃ�)
static PVal* code_get_var()
{
	PVal* pval;
	code_getva(&pval);
	return pval;
}

// �v���O�C���E�C���^�[�t�F�[�X�p�̊֐��e���v���[�g
//------------------------------------------------
// ���߃R�}���h�Ăяo���֐�
//------------------------------------------------
template<int(*ProcSttmCmd)(int)>
static int cmdfunc(int cmd)
{
	code_next();
	return ProcSttmCmd(cmd);
}

//------------------------------------------------
// �֐��R�}���h�Ăяo���֐�
//------------------------------------------------
template< int(*ProcFunc  )(int, void**),
          int(*ProcSysvar)(int, void**) >
static void* reffunc( int* type_res, int cmd )
{
	void* pResult = nullptr;

	if ( !(*type == TYPE_MARK && *val == '(') ) {

		*type_res = ProcSysvar( cmd, &pResult );

	} else {
	//	if ( !(*type == TYPE_MARK && *val == '(') ) puterror( HSPERR_INVALID_FUNCPARAM );
		code_next();

		*type_res = ProcFunc( cmd, &pResult );	// �R�}���h����

		if ( !(*type == TYPE_MARK && *val == ')') ) puterror( HSPERR_INVALID_FUNCPARAM );
		code_next();
	}

	if ( !pResult ) puterror( HSPERR_NORETVAL );
	return pResult;
}

} // namespace hpimod

#endif
