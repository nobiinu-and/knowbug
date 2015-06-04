// WrapCall - type modcmd

// TYPE_MODCMD �̏��������b�v���A���[�U��`�֐��̌Ăяo���E�I����ʒm����B

#include "type_modcmd.h"
#include "WrapCall.h"

static int   modcmd_cmdfunc(int cmd);
static void* modcmd_reffunc(int* type_res, int cmd);

// �ϐ�
static int   (*g_modcmd_cmdfunc_impl)(int)       = nullptr;
static void* (*g_modcmd_reffunc_impl)(int*, int) = nullptr;

//------------------------------------------------
// TYPE_MODCMD �̏�����������
//------------------------------------------------
void modcmd_init(HSP3TYPEINFO* info)
{
	g_modcmd_cmdfunc_impl = info->cmdfunc;
	g_modcmd_reffunc_impl = info->reffunc;

	info->cmdfunc = modcmd_cmdfunc;
	info->reffunc = modcmd_reffunc;
}

//------------------------------------------------
// TYPE_MODCMD �̏��������ɖ߂�
//------------------------------------------------
void modcmd_term(HSP3TYPEINFO* info)
{
	if ( g_modcmd_cmdfunc_impl ) {
		info->cmdfunc = g_modcmd_cmdfunc_impl;
		info->reffunc = g_modcmd_reffunc_impl;

		g_modcmd_cmdfunc_impl = nullptr;
		g_modcmd_reffunc_impl = nullptr;
	}
}

//------------------------------------------------
// [modcmd] ���߃R�}���h
//------------------------------------------------
int modcmd_cmdfunc(int cmdid)
{
	stdat_t const stdat = hpimod::getSTRUCTDAT(cmdid);

	WrapCall::bgnCall(stdat);
	int const result = g_modcmd_cmdfunc_impl(cmdid);
	WrapCall::endCall();
	return result;
}

//------------------------------------------------
// [modcmd] �֐��R�}���h
//------------------------------------------------
void* modcmd_reffunc(int* type_res, int cmdid)
{
	stdat_t const stdat = hpimod::getSTRUCTDAT(cmdid);

	WrapCall::bgnCall(stdat);
	PDAT* const result = static_cast<PDAT*>(g_modcmd_reffunc_impl(type_res, cmdid));
	WrapCall::endCall(result, *type_res);
	return result;
}
