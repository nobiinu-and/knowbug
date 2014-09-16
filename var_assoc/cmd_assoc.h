// assoc - Command header

#ifndef IG_ASSOC_COMMAND_H
#define IG_ASSOC_COMMAND_H

#include "vt_assoc.h"

extern void AssocTerm();

extern int SetReffuncResult( void** ppResult, CAssoc* const& pAssoc );

// ����
extern void AssocNew();		// �\�z
extern void AssocDelete();	// �j��
extern void AssocClear();	// ����
extern void AssocChain();	// �A��
extern void AssocCopy();	// ����

extern void AssocImport();	// �O���ϐ��̃C���|�[�g
extern void AssocInsert();	// �L�[��}������
extern void AssocRemove();	// �L�[����������

extern void AssocDim();		// �����ϐ���z��ɂ���
extern void AssocClone();	// �����ϐ��̃N���[�������

// �֐�
extern int AssocNewTemp(void** ppResult);
extern int AssocNewTempDup(void** ppResult);

extern int AssocVarinfo(void** ppResult);
extern int AssocSize(void** ppResult);
extern int AssocExists(void** ppResult);
extern int AssocIsNull(void** ppResult);
extern int AssocForeachNext(void** ppResult);

extern int AssocResult( void** ppResult );	// assoc �ԋp
extern int AssocExpr( void** ppResult );	// assoc ��

// �V�X�e���ϐ�

// �萔
enum VARINFO {
	VARINFO_NONE = 0,
	VARINFO_FLAG = VARINFO_NONE,
	VARINFO_MODE,
	VARINFO_LEN,
	VARINFO_SIZE,
	VARINFO_PT,
	VARINFO_MASTER,
	VARINFO_MAX
};

#endif
