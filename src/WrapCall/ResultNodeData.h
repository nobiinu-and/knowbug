#ifndef IG_RESULT_NODE_DATA_H
#define IG_RESULT_NODE_DATA_H

// todo: WrapCall �{�̂Ƃ͂��܂�֌W���Ȃ��̂œK�؂Ȉʒu�Ɉړ�������

#include <memory>

#include "../main.h"
#include "../config_mng.h"
#include "../CVardataString.h"
#include "WrapCall/ModcmdCallInfo.h"

namespace WrapCall
{
	// Remark: Don't rearrange the members.
	struct ResultNodeData
	{
		stdat_t stdat;

		// �Ԓl�̌^
		vartype_t vtype;

		// �l�̕�����
		string valueString;

		// ����Ɉˑ�����Ăяo�� (���݂���ꍇ�͂���̎q�m�[�h�ɂȂ�)
		optional_ref<ModcmdCallInfo const> pCallInfoDepended;

	public:
		ResultNodeData(ModcmdCallInfo const& callinfo, PDAT* ptr, vartype_t vt);
		ResultNodeData(ModcmdCallInfo const& callinfo, PVal* pvResult)
			: ResultNodeData(callinfo, pvResult->pt, pvResult->flag)
		{ }
	};

} // namespace WrapCall

#endif
