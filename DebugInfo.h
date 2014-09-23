// debug info

// �O���[�o���ϐ��̐������炷���߂̂���
// ctx, exinfo ���������݂��č������邾���Ɏv����

#ifndef IG_STRUCT_DEBUG_INFO_H
#define IG_STRUCT_DEBUG_INFO_H

#include <memory>

#include "hsp3debug.h"
#include "hsp3struct.h"
#include "hspvar_core.h"

#include "module/strf.h"

#include "CAx.h"

using hpimod::CAx;

struct DebugInfo
{
public:
	HSP3DEBUG* const debug;
//	HSPCTX*    const ctx;
//	HSPEXINFO* const exinfo;

	std::unique_ptr<CAx> const ax;

public:
	DebugInfo(HSP3DEBUG* debug)
		: debug(debug)
		, ax(new CAx())
	{ }

	// ���ݎ��s�̎��s�ʒu��\�������� (�X�V�͂��Ȃ�)
	std::string getCurInfString() const {
		auto const fname = (debug->fname ? debug->fname : "(�t�@�C�����Ȃ�)");
	//	return strf("%s\n( line : %d )", fname, debug->line);
		return strf("#%d \"%s\"", debug->line, fname);
	}
};

#endif
