// �ϐ��f�[�^�e�L�X�g�����N���X

#ifndef IG_CLASS_VARINFO_TEXT_H
#define IG_CLASS_VARINFO_TEXT_H

#include "main.h"
#include "DebugInfo.h"
#include "module/strf.h"
#include "module/CStrWriter.h"

#include "CVarTree.h"

#ifdef with_WrapCall
namespace WrapCall
{
	struct ModcmdCallInfo;
	struct ResultNodeData;
}
#endif

class CVarinfoText
{
	// �����o�ϐ�
private:
	string buf_;
	CStrWriter writer_;

	// �����o�֐�
public:
	CVarinfoText()
		: writer_ { getBuf() }
	{ }
	
	void addVar(PVal* pval, char const* name);
	void addSysvar(char const* name);
#ifdef with_WrapCall
	void addCall(WrapCall::ModcmdCallInfo const& callinfo);
	void addResult(stdat_t stdat, string const& text, char const* name);
#endif

	void addModuleOverview(char const* name, CVarTree::ModuleNode const& tree);
	void addSysvarsOverview();
#ifdef with_WrapCall
	void addCallsOverview(WrapCall::ResultNodeData const* pLastResult);
#endif

	string const& getString() const { return buf_; }
	string&& getStringMove() { return std::move(buf_); }

private:
	CStrWriter& getWriter() { return writer_; }
	string* getBuf() { return &buf_; }
};

extern char const* getMPTypeString(int mptype);
extern string getPrmlistString(stdat_t stdat);
extern string getVartypeString(PVal const* pval);

#endif
