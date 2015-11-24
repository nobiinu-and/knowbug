
#include <unordered_set>
#include <unordered_map>
#include "main.h"
#include "VarTreeNodeData.h"
#include "config_mng.h"
#include "module/LineDelimitedString.h"

struct VTNodeScript::Impl
{
	std::unordered_set<string> userDirs_;
	std::unordered_map<string, shared_ptr<string const>> fullPathFromRefName_;
	std::map<string const, LineDelimitedString> cache_;

public:
	auto searchFile(char const* fileName) -> shared_ptr<string const>;
	auto searchFile(char const* fileName, char const* dir) -> shared_ptr<string const>;
	auto fetchScript(char const* fileName) -> optional_ref<LineDelimitedString>;
};

VTNodeScript::VTNodeScript()
	: p_(new Impl {})
{}

auto VTNodeScript::parent() const -> shared_ptr<VTNodeData>
{
	return VTRoot::make_shared();
}

auto VTNodeScript::Impl::searchFile(char const* fileRefName, char const* dir)
	-> shared_ptr<string const>
{
	char* fileName = nullptr;
	std::array<char, MAX_PATH> fullPath {};
	bool const succeeded =
		SearchPath
			( dir, fileRefName, /* lpExtenson = */ nullptr
			, fullPath.size(), fullPath.data(), &fileName)
		!= 0;
	if ( succeeded ) {
		// �������ꂽ�f�B���N�g���������Ώۂɒǉ�����
		userDirs_.emplace(string(fullPath.data(), fileName));

		auto&& p = std::make_shared<string const>(fullPath.data());

		// ������
		fullPathFromRefName_.emplace(fileRefName, p);
		return p;
	} else {
		return nullptr;
	}
}

auto VTNodeScript::Impl::searchFile(char const* fileRefName)
	-> shared_ptr<string const>
{
	// ��������ǂ�
	auto&& iter = fullPathFromRefName_.find(fileRefName);
	if ( iter != fullPathFromRefName_.end() ) {
		return iter->second;
	}

	// ���[�U�f�B���N�g���A�J�����g�f�B���N�g���Acommon�A�̏��ŒT��
	for ( string const& dir : userDirs_ ) {
		if ( auto&& p = searchFile(fileRefName, dir.c_str()) ) {
			return std::move(p);
		}
	}
	if ( auto&& p = searchFile(fileRefName, nullptr) ) {
		return std::move(p);
	}
	return searchFile(fileRefName, g_config->commonPath().c_str());
}

auto VTNodeScript::searchFile(char const* fileRefName) const
	-> shared_ptr<string const>
{
	return p_->searchFile(fileRefName);
}

auto VTNodeScript::Impl::fetchScript(char const* fileRefName)
	-> optional_ref<LineDelimitedString>
{
	if ( auto&& p = VTRoot::script()->searchFile(fileRefName) ) {
		string const& filePath = *p;

		auto& lds = map_find_or_insert(cache_, filePath, [&filePath] () {
			std::ifstream ifs { filePath };
			assert(ifs.is_open());
			return LineDelimitedString(ifs);
		});
		return &lds;
	} else {
		return nullptr;
	}
}

auto VTNodeScript::fetchScriptAll(char const* fileRefName) const
	-> optional_ref<string const>
{
	if ( auto&& p = p_->fetchScript(fileRefName) ) {
		return &p->get();
	} else {
		return nullptr;
	}
}

auto VTNodeScript::fetchScriptLine(char const* fileRefName, size_t lineIndex) const
	-> unique_ptr<string const>
{
	if ( auto&& p = p_->fetchScript(fileRefName) ) {
		/**
		�ҏW���̃t�@�C�������s����Ă���ꍇ�A�t�@�C���̓��e�����ۂƈقȂ邱�Ƃ�����B
		�s�ԍ��̃A�E�g�����W�ɒ��ӁB
		//*/
		return std::make_unique<string>(p->line(lineIndex));
	} else {
		return nullptr;
	}
}
