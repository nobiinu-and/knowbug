// �O���֐��̌Ăяo�����[�`��

#include "call_extfunc.h"

//------------------------------------------------
// �O���֐��Ăяo��
// 
// @prm proc : �֐��|�C���^
// @prm prm  : �������̓����� int �z��
// @prm prms : �������X�g
// @result   : �֐��̖߂�l
//------------------------------------------------
#ifdef _MSC_VER

// @compiler: VC++
__declspec( naked ) int __cdecl call_extfunc( void *proc, int *prm, int prms )
{
	__asm {
		push	ebp
		mov		ebp,esp

		;# ebp+8	: �֐��̃|�C���^
		;# ebp+12	: ������������INT�̔z��
		;# ebp+16	: �����̐��ipush����񐔁j

		;# �p�����[�^��np��push����
		mov		eax, dword ptr [ebp+12]
		mov		ecx, dword ptr [ebp+16]
		jmp		_$push_chk

	_$push:
		push	dword ptr [eax+ecx*4]

	_$push_chk:
		dec		ecx
		jge		_$push

		;# �֐��Ăяo��
		call	dword ptr [ebp+8]

		;# �߂�l�� eax �ɓ���̂ł��̂܂܃��^�[��
		leave
		ret
	}
}

#elif defined( __GNUC__ )

// @ compiler: gcc
int __cdecl call_extfunc(void * proc, int * prm, int prms)
{
    int ret = 0;
    __asm__ volatile (
		"pushl  %%ebp;"
		"movl   %%esp, %%ebp;"
		"jmp    _push_chk;"
		
		// �p�����[�^��prms��push����
	"_push:"
		"pushl  ( %2, %3, 4 );"
		
	"_push_chk:"
		"decl   %3;"
		"jge    _push;"
		
		"calll  *%1;"
		"leave;"
		
		: "=a" ( ret )
        : "r" ( proc ) , "r" ( prm ), "r" ( prms )
    );
    return ret;
}

