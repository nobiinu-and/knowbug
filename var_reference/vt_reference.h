// reference - VarProc header

#ifndef IG_REFERENCE_VARPROC_H
#define IG_REFERENCE_VARPROC_H

#include "hsp3plugin_custom.h"

// �O���[�o���ϐ��̐錾
extern short g_vtReference;
extern HspVarProc* g_pHvpReference;

// �֐��̐錾
extern void HspVarReference_Init( HspVarProc* );

extern PVal* code_get_reference();

// �萔

// �}�N��

// ��`
namespace VtReference
{
	struct PValAfterMaster {		// PVal::master �ȍ~�̃����o
		void	*master;			// �Q�Ƃ��� PVal* �Ƃ��Ďg��
		unsigned short	support;
		short	arraycnt;
		int		offset;
		int		arraymul;			// �Q�Ƃ���ϐ��ɕR�t���� APTR �Ƃ��Ďg�� (�A�z�z��T�|�[�g�̏ꍇ�����͏��������Ȃ��݂����Ȃ̂�)
	};
	
	typedef PValAfterMaster value_t;
	typedef value_t* valptr_t;		// �� PDAT*
	const int basesize = sizeof(value_t);
	const int IdxDummy = 0x11235813;
	
	extern valptr_t GetPtr( PVal* pval );
}

#endif
