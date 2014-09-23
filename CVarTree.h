// VarTree

// ���W���[�����m�[�h�A�ÓI�ϐ������[�t�Ƃ���؍\��

// ���̃N���X�Ƃ��ẮA���W���[�����͐擪�� '@' ���܂ނƂ���B
// (�������������\�[�g�̎�Ԃ��Ȃ���)
// �ϐ����̓X�R�[�v�������܂ނƂ���B(gvar �܂��� lvar@mod)

// �㐔�I�f�[�^�^�ɂ��悤�Ǝv�������A��ʉ��ł��Ȃ��܂ܕ��u

#ifndef IG_CLASS_VAR_TREE_H
#define IG_CLASS_VAR_TREE_H

#include "main.h"
#include <string>
#include <list>
#include <vector>
#include <map>
#include <iterator>
#include <memory>

/*
VarTree =
	| Var of string
	| Module of string * map<string, CStaticVarTree>;
*/
// Remark: don't inherit this class (except for its case classes).
class CStaticVarTree
{
private:
	string name_;

	CStaticVarTree(string const& name) : name_(name) { }

public:
	virtual ~CStaticVarTree() { }

	string const& getName() const { return name_; }

	static string const ModuleName_Global;

	// cases
	class VarNode;
	class ModuleNode;
	friend class VarNode;
	friend class ModuleNode;

	enum class CaseId { Var, Module };
	virtual CaseId getCaseId() const = 0;
	
	template<typename TCase> struct isCaseClass {
		using TCaseRaw = std::remove_const_t<TCase>;
		static bool const value = std::is_same<TCaseRaw, VarNode>::value || std::is_same<TCaseRaw, ModuleNode>::value;
	};
	template<typename TCase, typename = std::enable_if_t<isCaseClass<TCase>::value>>
	bool isCaseOf() const { return (this->getCaseId() == TCase::GetCaseId()); }

	template<typename TCase, typename = std::enable_if_t<isCaseClass<TCase>::value>>
	TCase const* asCaseOf() const { return (isCaseOf<TCase>() ? reinterpret_cast<TCase const*>(this) : nullptr); }

	template<typename TCase> TCase* asCaseOf() { return const_cast<CStaticVarTree*>(static_cast<CStaticVarTree const*>(this)->asCaseOf<TCase>()); }

	template<typename TResult, typename TFuncVar, typename TFuncModule>
	TResult match(TFuncVar&& fVar, TFuncModule&& fModule) const
	{
		switch ( getCaseId() ) {
			case CaseId::Var: return fVar(*asCaseOf<VarNode>());
			case CaseId::Module: return fModule(*asCaseOf<ModuleNode>());
		}
	}
	// non const �ł͏ȗ�
	// template<...> TResult match() { ���l }
};

// Var
class CStaticVarTree::VarNode
	: public CStaticVarTree
{
public:
	VarNode(string const& name)
		: CStaticVarTree(name)
	{ }

	// case
	static CaseId GetCaseId() { return CaseId::Var; }
	CaseId getCaseId() const override { return CaseId::Var; }
};

// Module
class CStaticVarTree::ModuleNode
	: public CStaticVarTree
{
	using map_t = std::map<string, std::unique_ptr<CStaticVarTree>>;
public:
	using iterator = map_t::iterator;
	using const_iterator = map_t::const_iterator;
private:
	std::unique_ptr<map_t> children_;
public:
	ModuleNode(string const& name)
		: CStaticVarTree(name)
		, children_(new map_t)
	{
		assert(name[0] == '@'
			&& std::strchr(name.c_str() + 1, '@') == nullptr);
	}
	/*
	ModuleNode(ModuleNode const& src)
		: CStaticVarTree(src.getName())
		, children_(new map_t(*src.children_))
	{ }
	ModuleNode(ModuleNode&& src)
		: CStaticVarTree(src)
		, children_(std::move(src.children_))
	{ }//*/

	void pushVar(char const* name);
	void pushModule(char const* name) { insertChildModule(name); }

private:
	ModuleNode& insertChildModule(char const* pModname);

	// �q�m�[�h����������B�Ȃ���Βǉ�����B
	template<typename TNode, typename ...TArgs>
	TNode& insertChildImpl(string const& name, TArgs&& ...args)
	{
		auto iter = children_->find(name);
		if ( iter == children_->end() ) {
			iter = children_->insert(
				{ name, std::make_unique<TNode>(name, std::forward<TArgs>(args)...) }
			).first;
		}
		return reinterpret_cast<TNode&>(*iter->second);
	}
	
public:
	// iterators
	const_iterator begin() const { return children_->begin(); }
	const_iterator end() const { return children_->end(); }

	// case
	static CaseId GetCaseId() { return CaseId::Module; }
	CaseId getCaseId() const override { return CaseId::Module; }
};

using CVarTree = CStaticVarTree;

#endif
