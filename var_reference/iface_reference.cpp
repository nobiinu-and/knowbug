/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
 |
 *		hsp plugin interface (reference)
 |
 *				author uedai
 |
.*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

#include "iface_reference.h"
#include "vt_reference.h"
#include "cmd_reference.h"

#include "hsp3plugin_custom.h"
#include "mod_func_result.h"

// �֐��錾
static int  hpi_cmdfunc( int cmd );
static int hpi_termfunc( int option );

static int   ProcFuncCmd( int cmd, void** ppResult );
static int ProcSysvarCmd( int cmd, void** ppResult );

//##########################################################
//        HPI����
//##########################################################
//------------------------------------------------
// HPI�o�^�֐�
//------------------------------------------------
EXPORT void WINAPI hsp3typeinfo_reference( HSP3TYPEINFO* info )
{
	hsp3sdk_init( info );			// SDK�̏�����(�ŏ��ɍs�Ȃ��ĉ�����)
	
	HSPVAR_COREFUNC corefunc = HspVarReference_Init;
	registvar( -1, corefunc );		// �V�K�^��ǉ�
	
	info->cmdfunc  = hpi_cmdfunc;		// ���s�֐�(cmdfunc)�̓o�^
	info->reffunc  = hpi_reffunc<&ProcFuncCmd, &ProcSysvarCmd>;		// �Q�Ɗ֐�(reffunc)�̓o�^
	info->termfunc = hpi_termfunc;		// �I���֐�(termfunc)�̓o�^
	
	return;
}

//------------------------------------------------
// �I����
//------------------------------------------------
static int hpi_termfunc(int option)
{
	return 0;
}

//##########################################################
//        �R�}���h����
//##########################################################
//------------------------------------------------
// ����
//------------------------------------------------
static int hpi_cmdfunc( int cmd )
{
	code_next();
	
	switch ( cmd ) {
		case 0x000: ReferenceNew();    break;
		case 0x001: ReferenceDelete(); break;
		
		default:
			puterror( HSPERR_UNSUPPORTED_FUNCTION );
	}
	
	return RUNMODE_RUN;
}

//------------------------------------------------
// �֐�
//------------------------------------------------
static int ProcFuncCmd( int cmd, void** ppResult )
{
	switch ( cmd ) {
		case 0x000: ReferenceNew(); break;
		case 0x100: ReferenceMemberOf(); break;
			
		default:
			puterror( HSPERR_UNSUPPORTED_FUNCTION );
	}
	return SetReffuncResult( ppResult, VtReference::IdxDummy );
}

//------------------------------------------------
// �V�X�e���ϐ�
//------------------------------------------------
static int ProcSysvarCmd( int cmd, void** ppResult )
{
	switch ( cmd ) {
		case 0x000: return SetReffuncResult( ppResult, (int)g_vtReference );	// reference
		
		default:
			puterror( HSPERR_UNSUPPORTED_FUNCTION );
	}
	return 0;
}
