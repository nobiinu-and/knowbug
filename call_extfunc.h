// �O���֐��̌Ăяo�����[�`��

// dllfunc �Ȃǂ̌Ăяo���Ɏg���Ă���A��

#ifndef __CALL_EXTENDED_FUNCTION_H__
#define __CALL_EXTENDED_FUNCTION_H__

extern
#ifdef _MSC_VER
	__declspec(naked)
#endif
	int __cdecl call_extfunc(void *proc, int *prm, int prms);

#endif
