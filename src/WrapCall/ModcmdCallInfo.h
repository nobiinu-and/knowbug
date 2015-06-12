// struct ModcmdCallInfo

#ifndef IG_STRUCT_MODCMD_CALL_INFO_H
#define IG_STRUCT_MODCMD_CALL_INFO_H

//unhookable invocation �c�c modinit/modterm/deffunc_onexit commands, and call by call.hpi
// �����̌Ăяo����prmstk��ύX����ɂ�������炸���m�ł��Ȃ��̂Ŋ댯�B

#include "../main.h"
#include "WrapCall.h"

#include "../SysvarData.h"

namespace WrapCall
{

// �Ăяo�����O�̏��
struct ModcmdCallInfo
{
	// �Ăяo���ꂽ�R�}���h
	stdat_t const stdat;

	// �Ăяo�����O�ł� prmstk
	void* const prmstk_bak;

	// �Ăяo�����O�ł̃l�X�g���x��
	int const sublev;
	int const looplev;

	// �Ăяo���ꂽ�ʒu
	char const* const fname;
	int const line; //0-based

	// g_stkCallInfo �ɂ�����ʒu
	size_t const idx;

public:
	ModcmdCallInfo(stdat_t stdat, void* prmstk_bak, int sublev, int looplev, char const* fname, int line, size_t idx)
		: stdat(stdat), prmstk_bak(prmstk_bak), sublev(sublev), looplev(looplev), fname(fname), line(line), idx(idx)
	{ }

	optional_ref<ModcmdCallInfo const> getPrev() const {
		return getCallInfoAt(idx - 1);
	}
	optional_ref<ModcmdCallInfo const> getNext() const {
		return getCallInfoAt(idx + 1);
	}

	//prmstk: ���̌Ăяo���̎�������� (failure: nullptr)
	//safety: ����prmstk���m���Ɉ��S�ł��邩�B
	// prmstk��hsp�̃X�^�b�N���������v�[����Ɋm�ۂ����̂ŁA�������A�N�Z�X�͏�Ɉ��S�B
	std::pair<void*, bool> tryGetPrmstk() const
	{
		//���ꂪ�ŐV�̌Ăяo��
		if ( !getNext() ) {
			assert(sublev <= ctx->sublev);
			//�{�̂��炳��ɑ��̃T�u���[�`�������s���Ȃ�A�����unhookable invocation�̉\��������
			bool const safe = ( ctx->sublev == sublev + 1 );

			return { ctx->prmstack, safe };

		//�Ăяo�������s��
		//�� ���̌Ăяo��������A����͂���̎�����������̌Ăяo���ł͂Ȃ�
		//�� ���̌Ăяo��������A����͂���̖{��(�܂��͂�����[���ʒu)����Ăяo����Ă���
		} else if ( sublev < getNext()->sublev ) {
			assert(sublev + 1 <= getNext()->sublev);
			//�{�̂��炳��ɑ��̃T�u���[�`�������s���Ȃ�A�����unhookable invocation�̉\��������
			bool const safe = (sublev + 1 == getNext()->sublev);

			return { getNext()->prmstk_bak, safe };

		// �����W�J��
		//��prmstack �͖��쐬
		} else {
			return { nullptr, false };
		}
	}

	// ���̌Ăяo�������ڈˑ�����Ă���Ăяo���𓾂�B(failure: nullptr)
	// �����ɂ��āF���O�̌Ăяo���ŁA����� sublev ����������Έˑ��֌W�ɂ���A�����łȂ���΂Ȃ�
	// �Ȃ��A������g���v���O�C���� gosub ���l�����Ȃ��B
	optional_ref<ModcmdCallInfo const> getDependedCallInfo() const
	{
		auto const prev = getPrev();
		return (prev && prev->sublev == sublev)
			? prev
			: nullptr;
	}
};

}

#endif
