// WrapCall

#include <vector>

#include "../main.h"
#include "../DebugInfo.h"

#include "WrapCall.h"
#include "ModcmdCallInfo.h"
#include "type_modcmd.h"

namespace WrapCall
{

static stkCallInfo_t g_stkCallInfo;

static int g_pluginType_WrapCall = -1;

/*
struct ValueRef
{
	PDAT*      p;
	vartype_t vt;
public:
	ValueRef()
		: p(nullptr)
		, vt(HSPVAR_FLAG_NONE)
	{ }

	void set(PDAT* p1, vartype_t vt1) {
		p = p1; vt = vt1;
	}
};
static ValueRef lastResult;	// �Ō�̕Ԓl�ւ̎Q�� (�����g���Ȃ��Ȃ�̂Œ���)
//*/

//------------------------------------------------
// �v���O�C���������֐�
//------------------------------------------------
EXPORT void WINAPI hsp3hpi_init_wrapcall(HSP3TYPEINFO* info)
{
	g_pluginType_WrapCall = info->type;
	auto const typeinfo = info - g_pluginType_WrapCall;

	hsp3sdk_init(info);

	// ������
	modcmd_init(&typeinfo[TYPE_MODCMD]);
	g_stkCallInfo.reserve(32);
	return;
}

/*
EXPORT void WINAPI WrapCallInitialize()
{
	//
}

// typeinfo ���������\��������̂Ō��ɖ߂��^�C�~���O�͂Ȃ�
EXPORT void WINAPI WrapCallTerminate()
{
	if ( g_typeinfo != nullptr ) {
		modcmd_term(&g_typeinfo[TYPE_MODCMD]);
	}
	return;
}
//*/

//------------------------------------------------
// �Ăяo���̊J�n
//------------------------------------------------
void bgnCall(stdat_t stdat)
{
	g_dbginfo->debug->dbg_curinf();

	// �Ăяo�����X�g�ɒǉ�
	size_t const idx = g_stkCallInfo.size();
	g_stkCallInfo.push_back(std::make_unique<ModcmdCallInfo>(
		stdat, ctx->prmstack, ctx->sublev, ctx->looplev,
		g_dbginfo->debug->fname, g_dbginfo->debug->line, idx
	));

	auto& callinfo = *g_stkCallInfo.back();
	
	// DebugWindow �ւ̒ʒm
	Knowbug::bgnCalling(callinfo);
	return;
}

//------------------------------------------------
// �Ăяo���̊���
//------------------------------------------------
void endCall()
{
	return endCall(nullptr, HSPVAR_FLAG_NONE);
}

void endCall(PDAT* p, vartype_t vt)
{
	if (g_stkCallInfo.empty()) return;
	
	auto const& callinfo = *g_stkCallInfo.back();
	
	// �x��
	if ( ctx->looplev != callinfo.looplev ) {
		Knowbug::logmesWarning( "�Ăяo�����ɓ����� loop ����A����ɒE�o�����A�Ăяo�����I�������B" );
	}
	
	if ( ctx->sublev != callinfo.sublev ) {
		Knowbug::logmesWarning("�Ăяo�����ɓ������T�u���[�`������A����ɒE�o�����A�Ăяo�����I�������B");
	}

	// DebugWindow �ւ̒ʒm
	Knowbug::endCalling(callinfo, p, vt);
	
	g_stkCallInfo.pop_back();
	return;
}

//------------------------------------------------
// callinfo �X�^�b�N�ւ̃A�N�Z�X
//------------------------------------------------
ModcmdCallInfo const* getCallInfoAt(size_t idx)
{
	return ( 0 <= idx && idx < g_stkCallInfo.size() )
		? g_stkCallInfo.at(idx).get()
		: nullptr;
}

std::pair<stkCallInfo_t::const_iterator, stkCallInfo_t::const_iterator> getCallInfoRange()
{
	return std::make_pair(g_stkCallInfo.begin(), g_stkCallInfo.end());
}

}
