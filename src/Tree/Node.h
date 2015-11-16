﻿
#pragma once

#include <functional>
#include "../main.h"
#include "../module/utility.h"
#include "../module/Singleton.h"
#include "Visitor.h"
#include "Observer.h"

namespace DataTree {

struct TreeObservers
{
	std::function<void(NodeGlobal*)> spawnRoot;
	IVisitor* appendObserver;
	IVisitor* removeObserver;
};
extern std::vector<TreeObservers> stt_observers;
extern void registerObserver(TreeObservers obs);

/**
データツリー

HSPの値を木構造として取り扱うインターフェイス
//*/
class ITree
{
public:
	virtual ~ITree() { }

	virtual void acceptVisitor(IVisitor& visitor) = 0;

protected:
	template<typename TNode>
	void acceptVisitorTemplate(IVisitor& visitor)
	{
		auto const p = dynamic_cast<TNode*>(this);
		assert(p != nullptr);
		visitor.visit(p);
	}

public:
	/**
	状況に合わせて木構造を更新する

	child != nullptr の場合は、child は必ず直接の子ノードである。
	このときは、子ノードの更新を行わない。child が生存している場合にかぎり true を返す。
	//*/
	virtual bool updateState(tree_t child_opt) = 0;

	void updateStateAll() { updateState(nullptr); }

	// 更新状態
public:
	enum class UpdatedState { None, Shallow, Deep };
	void setUpdatedState(UpdatedState s) { updatedState_ = s; }
	UpdatedState getUpdatedState() const { return updatedState_; }
private:
	UpdatedState updatedState_;
};

class INode
	: public ITree
{
protected:
	typedef std::vector<tree_t> children_t;

public:
	INode(tree_t parent, string const& name)
		: ITree(), parent_(parent), name_(name)
	{ }

	virtual ~INode()
	{
		removeChildAll();
	}
	
public:
	tree_t getParent() const { return parent_; }
	string const& getName() const { return name_; }
	children_t const& getChildren() { return children_; }

protected:
	void rename(string const& name) { name_ = std::move(name); }
	
	tree_t addChild(tree_t child);
	tree_t replaceChild(children_t::iterator& pos, tree_t child);
	void removeChild(children_t::iterator& pos);
	void removeChildAll();

private:
	tree_t parent_;
	string name_;
	children_t children_;
};

class NodeLoop
	: public INode
{
public:
	NodeLoop(tree_t parent, tree_t ancestor)
		: INode(parent, string("(loop)"))
	{ }

	void acceptVisitor(IVisitor& visitor) override {
		visitor.visit(this);
	}
};

class NodeModule
	: public INode
{
public:
	NodeModule(tree_t parent, string const& name);

	void acceptVisitor(IVisitor& visitor) override
	{
		acceptVisitorTemplate<NodeModule>(visitor);
	}

	bool updateState(tree_t childOrNull) override;

protected:
	void addVar(const char* fullName);
private:
	void addVarUnscoped(char const* fullName, string const& rawName);

	NodeModule& findModule(char const* scopeResolt);
	NodeModule* addModule(string const& rawName);

	virtual bool contains(char const* name) const;
	virtual string unscope(string const& scopedName) const;
};

class NodeGlobal
	: public NodeModule
	, public Singleton<NodeGlobal>
{
	friend class Singleton<NodeGlobal>;
	NodeGlobal();

private:
	void spawnRoot();
	bool contains(char const* name) const override { return true; }
	string unscope(string const& scopedName) const override;
};

class NodeArray
	: public INode
{
public:
	NodeArray(tree_t parent, string const& name, PVal* pval);

	void acceptVisitor(IVisitor& visitor) override
	{
		acceptVisitorTemplate<NodeArray>(visitor);
	}
	bool updateState(tree_t childOrNull) override;

	PVal* getPVal() const { return pval_; }

private:
	void addElem(size_t aptr);
	void updateElem(size_t aptr);
	
private:
	PVal* pval_;
	PVal cur_;
};

class NodeValue
	: public INode
{
public:
	NodeValue(tree_t parent, string const& name, PDAT const* pdat, vartype_t vt)
		: INode(parent, name)
		, pdat_(pdat)
		, vt_(vt)
	{}

	void acceptVisitor(IVisitor& visitor) override
	{
		acceptVisitorTemplate<NodeValue>(visitor);
	}

	// Default implementation for scalar types
	bool updateState(tree_t chlidOrNull) override
	{
		return getParent()->updateState(this);
	}

	vartype_t getVartype() const { return vt_; }
	PDAT const* getValptr() const { return pdat_; }
	void setValptr(PDAT const* pdat) { pdat_ = pdat; }

private:
	PDAT const* pdat_;
	vartype_t vt_;
};

/*
class NodePrmStk
	: public IPolyNode
{
public:
	NodePrmStk(tree_t parent, string name, void* prmstk, stdat_t stdat);
	NodePrmStk(tree_t parent, string name, void* prmstk, stprm_t stprm);

public:
	void acceptVisitor(IVisitor& visitor) override {
		acceptVisitorTemplate<NodePrmStk>(visitor);
	}

private:
	void initialize();
	void add( size_t idx, void* member, stprm_t stprm );
	
private:
	void* prmstk_;
	stdat_t stdat_;
	stprm_t stprm_;
};
//*/

}
