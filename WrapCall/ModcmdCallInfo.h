// struct ModcmdCallInfo

#ifndef IG_STRUCT_MODCMD_CALL_INFO_H
#define IG_STRUCT_MODCMD_CALL_INFO_H

#include "../main.h"
#include "WrapCall.h"

#include "../SysvarData.h"

namespace WrapCall
{

// �Ăяo�����O�̏��
// Remark: Don't rearrange the members.
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
	int const line;

	// g_stkCallInfo �ɂ�����ʒu
	size_t const idx;

public:
	ModcmdCallInfo(stdat_t stdat, void* prmstk_bak, int sublev, int looplev, char const* fname, int line, size_t idx)
		: stdat(stdat), prmstk_bak(prmstk_bak), sublev(sublev), looplev(looplev), fname(fname), line(line), idx(idx)
	{ }

	// �����W�J���I�����Ă��邩
/*
�����ɂ��āF
1. ���̃��[�U��`�R�}���h���N������Ȃ�����A����̈����W�J�̊Ԃ� knowbug �͓��삵�Ȃ��B
���̂��߁A���̃��[�U��`�R�}���h�����݂��Ȃ���΁A�����W�J�͏I�����Ă���B
������ (call.hpi �̂悤��) �g���v���O�C���̃R�}���h�ɂ��A���[�U��`�R�}���h�ȊO�̕��@�ň����������� gosub ��������ƁA
���ꂪ�ŐV�̌Ăяo���ł���ɂ�������炸�A�����W�J���I�����Ă��Ȃ���Ԃ� knowbug �������A�Ƃ������Ƃ����肤��B
�܂��Amodinit �ɂ��Ăяo���̊Ԃ� ctx->prmstack ���ς�邽�ߎQ�Ƃł��Ȃ��B
2. ����̃��[�U��`�R�}���h�����̌Ăяo���̈������̒��ɂ���Ȃ�A����̈����W�J�͏I�����Ă��Ȃ��B
�t�ɁA����̃��[�U��`�R�}���h���������̒��ɂȂ��Ȃ�A���̌Ăяo���̎��s�͊J�n����Ă���\�\���Ȃ킿�A����̈����W�J�͏I�����Ă���B
�������A1. �Ɠ��l�ɁA��҂͊g���v���O�C�����l�����Ă��Ȃ��B
3. �g���v���O�C���̃R�}���h�ɂ�鎮�� gosub ���l��������ŁA�����W�J���I�����Ă��邽�߂̏\�������������邱�Ƃ͂ł��Ȃ��B
//*/
	bool isMaybeRunning() const {
		return (!getNext() || sublev < getNext()->sublev);
	}

	// result is optional (none: nullptr)
	ModcmdCallInfo const* getPrev() const {
		return getCallInfoAt(idx - 1);
	}
	ModcmdCallInfo const* getNext() const {
		return getCallInfoAt(idx + 1);
	}

	// ���̌Ăяo���̎������������� prmstack �𓾂�B(failure: nullptr)
	// �Ȃ��Actx->prmstack ������ɕύX����邱�Ƃ͍l�����Ă��Ȃ��B
	void* getPrmstk() const
	{
		// �ŐV�̌Ăяo��
		if ( !getNext() ) {
			// modinit �����s���̉\��������Ƃ�
			if ( ctx->sublev > sublev + 1  && Sysvar::getThismod() ) {
				return nullptr;
			}

			return ctx->prmstack;

		// �����W�J���������Ă���
		} else if ( isMaybeRunning() ) {
			return getNext()->prmstk_bak;

		// �����W�J�� (�������� prmstack �͖��쐬)
		} else {
			return nullptr;
		}
	}

	// ���̌Ăяo�������ڈˑ�����Ă���Ăяo���𓾂�B(failure: nullptr)
	// �����ɂ��āF���O�̌Ăяo���ŁA����� sublev ����������Έˑ��֌W�ɂ���A�����łȂ���΂Ȃ�
	// �Ȃ��A������g���v���O�C���� gosub ���l�����Ȃ��B
	ModcmdCallInfo const* getDependedCallInfo() const
	{
		auto const prev = getPrev();
		return (prev && prev->sublev == sublev)
			? prev
			: nullptr;
	}
};

}

#endif
