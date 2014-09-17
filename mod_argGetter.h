// �����擾���W���[��

#ifndef IG_MODULE_ARGUMENT_GETTER_H
#define IG_MODULE_ARGUMENT_GETTER_H

#include "hsp3plugin_custom.h"

namespace hpimod {

// �����̎擾
//extern bool code_getva_ex(PVal** pval, APTR* aptr);
extern int  code_getds_ex(char** ppStr, char const* defstr = "");
//extern int  code_get_int_or_str(char** ppStr);
extern int  code_get_vartype(int defType = HSPVAR_FLAG_NONE);
extern label_t code_getdlb(label_t defLabel = nullptr);
//extern label_t code_getlb2(void);
extern FlexValue* code_get_struct();

extern stdat_t code_get_modcls();
extern stprm_t code_get_stprm();

// �z��Y��
	// ���o�� (expand_index)
extern void  code_expand_index_int( PVal* pval, bool bRhs );	// �ʏ�z��̓Y���̏���
extern void  code_expand_index_lhs( PVal* pval );
extern PDAT* code_expand_index_rhs( PVal* pval, int& mptype );

extern void  code_checkarray( PVal* pval, int bRhs );
extern void  code_checkarray1( PVal* pval );	// �E
extern void  code_checkarray2( PVal* pval );	// ��
extern PDAT* code_checkarray_obj1( PVal* pval, int& mptype );
extern void  code_checkarray_obj2( PVal* pval );

	// �ݒ� (index)
extern void code_index_int( PVal* pval, int offset, bool bRhs );
extern void code_index_int_lhs( PVal* pval, int offset );
extern void code_index_int_rhs( PVal* pval, int offset );

extern void code_index_reset(PVal* pval);

// �z�����G�~�����[�g
extern void code_assign_multi( PVal* pval );

// ���̑�
extern bool code_isNextArg();
extern int  code_skipprm();

extern bool code_next_expect( int expect_type, int expect_val );

} // namespace hpimod

#endif
