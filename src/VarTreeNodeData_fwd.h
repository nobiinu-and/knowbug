
#pragma once

#include <functional>

//class VTNodeModule;
class VTNodeVar;
class VTNodeSysvarList;
class VTNodeSysvar;
class VTNodeDynamic;
//class VTNodeInvoke;
//class VTNodeResult;
class VTNodeScript;
class VTNodeLog;
class VTNodeGeneral;

class StaticVarTree;
using VTNodeModule = StaticVarTree;

namespace WrapCall { struct ModcmdCallInfo; }
using VTNodeInvoke = WrapCall::ModcmdCallInfo;

struct ResultNodeData;
using VTNodeResult = ResultNodeData;

// �c���[�r���[�̃m�[�h�ɑΉ�����N���X�̃C���^�[�t�F�C�X
class VTNodeData
{
public:
	virtual ~VTNodeData() {}

	// visitor
	struct Visitor
	{
		virtual void fModule    (VTNodeModule     const&) {}
		virtual void fVar       (VTNodeVar        const&) {}
		virtual void fSysvarList(VTNodeSysvarList const&) {}
		virtual void fSysvar    (VTNodeSysvar     const&) {}
		virtual void fDynamic   (VTNodeDynamic    const&) {}
		virtual void fInvoke    (VTNodeInvoke     const&) {}
		virtual void fResult    (VTNodeResult     const&) {}
		virtual void fScript    (VTNodeScript     const&) {}
		virtual void fLog       (VTNodeLog        const&) {}
		virtual void fGeneral   (VTNodeGeneral    const&) {}
	};
	virtual void acceptVisitor(Visitor& visitor) const = 0;
};
