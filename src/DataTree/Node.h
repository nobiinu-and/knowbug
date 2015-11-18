#ifndef IG_CLASS_DATA_TREE_NODE_H
#define IG_CLASS_DATA_TREE_NODE_H

#include "main.h"

#include <string>
#include <vector>
#include <memory>

#include "Node.dec.h"
#include "ITreeVisitor.h"
#include "ITreeObserver.h"

namespace DataTree
{

// ���Ԃ�ς��Ă͂����Ȃ�
struct TreeObservers
{
	ITreeObserver* appendObserver;
	ITreeObserver* removeObserver;
};
extern std::vector<TreeObservers> stt_observers;
extern void registerObserver(TreeObservers obs);

//##############################################################################
//                �錾�� : ITree
//##############################################################################
//------------------------------------------------
// �f�[�^�c���[
// 
// HSP��̃f�[�^���ЂƂ܂Ƃ߂Ɉ����\���B
// ��{�I�� mutable (const �łȂ�)�B
// memo: �ړ��� I ������ interface �ł͂Ȃ����ۃN���X�ł́c�c
//------------------------------------------------
class ITree
{
public:
	virtual ~ITree() { }

	// visitor �ɂ��̃m�[�h�̎��ۂ̌^��������
	virtual void acceptVisitor(ITreeVisitor& visitor) = 0;

protected:
	template<typename TNode>
	void acceptVisitorTemplate(ITreeVisitor& visitor)
	{
		assert(dynamic_cast<TNode*>(this) != nullptr);
		visitor.visit(this_);
	}

public:
	// ����ɍ��킹�Ė؍\�����X�V����
	// �q�m�[�h child �� nullptr �ł͂Ȃ��ꍇ�A���̃m�[�h���X�V����q�m�[�h�ł���Ȃ� true ���A���ł����Ȃ� false ��Ԃ��B
	// nullptr �ł���ꍇ�A�e�q�m�[�h�ɍX�V��v�����Atrue ��Ԃ��B
	virtual bool updateState(tree_t childOrNull) = 0;

	void updateState() { updateState(nullptr); }

	// �X�V���
public:
	enum class UpdatedState { None = 0, Shallow = 1, Deep = 2 };
	void setUpdatedState(UpdatedState s) { updatedState_ = s; }
	UpdatedState getUpdatedState() const { return updatedState_; }
private:
	UpdatedState updatedState_;

};

//------------------------------------------------
// ���ۃR���e�i�m�[�h
// 
// ���ۂɂ� IMonoNode �� IPolyNode �ł���B
// ���O�������A���I�ω�����B
// �q�m�[�h�̃|�C���^�̏��L�������B
//------------------------------------------------
class INodeContainer
	: public ITree
{
protected:
	typedef std::vector<tree_t> children_t;

public:
	INodeContainer(tree_t parent, string const& name)
		: ITree(), parent_(parent), name_(name)
	{ }
	
	// �e
public:
	tree_t getParent() const { return parent_; }

private:
	tree_t parent_;

	// ���O
public:
	string const& getName() const { return name_; }
	
protected:
	void rename(string const& name) { name_ = std::move(name); }
	
private:
	string name_;

	// ���I�ω�
	/*
public:
	enum class CurrentStateType {
		NoChange = 0,
		Removed,	// ���̃m�[�h�����ɏ������Ă���
		Remaked,	// ���̃m�[�h��(���݂͂��Ă��邪)��蒼���ꂽ���ǂ���
		Extended,	// ���̃m�[�h�Ɏq�m�[�h���ǉ����ꂽ���ǂ��� (�폜�͂���ĂȂ�)
		Modified,	// ���̃m�[�h�̒l���ύX���ꂽ
	};

	struct CurrentState {
		virtual ~CurrentState() { }
	};
protected:
	typedef std::shared_ptr<CurrentState> change_t;

public:
	// ���݂̏�Ԃ𒲍�����
	// ��̓I�ȏ��͊e�m�[�h���Ƃ� ChangeStruct ���p���������̂�ԋp���ē`�B����B
	// TODO: �������z( = 0 )�ɂ���
	// change ��� modify �̕����炵����
	virtual change_t inspectCurrentState() const { return nullptr; }
	//*/
};

//*
//------------------------------------------------
// �P�̃R���e�i�m�[�h
// 
// @ �f�[�^������ł́A�u... =�v���\�z����B
//------------------------------------------------
class IMonoNode
	: public INodeContainer
{
public:
	IMonoNode(tree_t parent, string const& name, tree_t child)
		: INodeContainer(parent, name)
		, child_(child)
	{ }

	virtual ~IMonoNode() {
		removeChild();
	}

public:
	tree_t getChild() const { return child_; }

protected:
	void setChild(tree_t child);

private:
	void removeChild();

private:
	tree_t child_;
};
//*/

//------------------------------------------------
// �����R���e�i�m�[�h
// 
// @ ������(0�ȏ��)�m�[�h�����m�[�h�B
// @ �f�[�^������ł́A�u*: ...�v���\�z����B
// @ex: array����
//------------------------------------------------
class IPolyNode
	: public INodeContainer
{
public:
	IPolyNode(tree_t parent, string const& name)
		: INodeContainer(parent, name)
		, children_()
	{ }

	virtual ~IPolyNode() {
		removeChildAll();
	}
	
	children_t const& getChildren() { return children_; }
protected:
	tree_t addChild(tree_t child);
	tree_t replaceChild(children_t::iterator& pos, tree_t child);

private:
	void removeChild(children_t::iterator& pos);
	void removeChildAll() {
		for ( auto iter = children_.begin(); iter != getChildren().end(); ++iter ) {
			removeChild(iter);
		}
		children_.clear();
	}
	
private:
	children_t children_;
};

//------------------------------------------------
// ���[�v�m�[�h
// 
// @ �c��m�[�h�Ɠ���̃m�[�h���w�������B
//------------------------------------------------
class CLoopNode
	: public IMonoNode
{
public:
	CLoopNode(tree_t parent, tree_t ancestor)
		: IMonoNode(parent, "(loop)", ancestor)
	{ }

	void acceptVisitor(ITreeVisitor& visitor) override {
		visitor.visit(this);
	}
};

//------------------------------------------------
// ���[�t�m�[�h
// 
// �f�[�^������ł́A�u=�v�̉E�ӂ��\�z����B
//------------------------------------------------
class ILeaf
	: public ITree
{
public:
	ILeaf() { }
};

//##############################################################################
//                �錾�� : ��ۃm�[�h
//##############################################################################

//**********************************************************
//        �̈�
//**********************************************************
//------------------------------------------------
// ���W���[��
//------------------------------------------------
class CNodeModule
	: public IPolyNode
{
public:
	CNodeModule(tree_t parent, string const& name);

	void acceptVisitor(ITreeVisitor& visitor) override {
		acceptVisitorTemplate<CNodeModule>(visitor);
	}

	bool updateState(tree_t childOrNull) override {
		if ( !childOrNull ) {
			for ( auto child : getChildren() ) {
				child->updateState();
			}
		}
		return true;
	}

protected:
	void addVar(const char* fullName);
private:
	void addVarUnscoped(char const* fullName, string const& rawName);

	CNodeModule& findNestedModule(char const* scopeResolt);
	CNodeModule* addModule(string const& rawName);

	virtual bool contains(char const* name) const;
	virtual string unscope(string const& scopedName) const;
};

//------------------------------------------------
// �O���[�o���̈�
// 
// @ root �v�f�B
// @ �V���O���g���B
// @ ���O�C�����g�p���Ȃ��B
//------------------------------------------------
class CNodeGlobal
	: public CNodeModule
{
public:
	static CNodeGlobal& getInstance() {
		return *(instance ? instance : (instance = new CNodeGlobal()));
	}

private:
	static CNodeGlobal* instance;
	CNodeGlobal();

	// �m�[�h�Ƃ��Ă̐U�镑���͕ς��Ȃ��̂� acceptVisitor ���Ē�`���Ȃ��B
	// void acceptVisitor(ITreeVisitor&) override;

private:
	bool contains(char const* name) const override { return true; }
	string unscope(string const& scopedName) const override;
};

//**********************************************************
//        �ϐ�
//**********************************************************

// �z��A�v�f�̏�ʂ�(����)�ϐ��N���X�����ށH
// �ϐ��͒l��W�J����E���Ȃ��ɂ�����炸�z�񂩔ۂ��̔��f�����ׂ����H
// �z��^�v�f�A�ɂȂ�ׂāA�u���W�J�ϐ��v�N���X�����Ƃ��B

// inspectCurrentState ���̂��l�̗v���Ƃ��Ďq�m�[�h�̓W�J�����߂Ă���̂ł́H
// �Ƃ����� change �̈��Ƃ��āu�q�m�[�h��W�J����v������̂�
// �W�J���Ă��邩�ǂ����̃t���O�����ׂ����H

// inspectCurrentState 

//------------------------------------------------
// �ϐ�
//
// (���g�p)
// @ CNodeVarArray �� CNodeVarElem �����B
//------------------------------------------------
class CNodeVarHolder
	: public IMonoNode
{
public:
	CNodeVarHolder(tree_t parent, string const& name, PVal* pval)
		: IMonoNode(parent, name, nullptr)
		, pval_(pval)
	{ }

	void acceptVisitor(ITreeVisitor& visitor) override {
		acceptVisitorTemplate<CNodeVarHolder>(visitor);
	}
	
	bool updateState(tree_t childOrNull) override;
private:
	PVal* pval_;
};

//------------------------------------------------
// �z��
// 
// @ ��v�f�̕ϐ��͊܂܂Ȃ��B
// @ �v�f�̐������ϐ��v�f���܂ރR���e�i�Ƃ݂Ȃ��B
// TODO: �ÓI�ϐ��Ȃ�Ƃɂ����A���[�J���ϐ�(�⃁���o�ϐ�)�Ȃǂ͑��݂��Ă��邩������肩�łȂ��B
//	��������ȊO�� pval_ �ɐG��Ă͂Ȃ�Ȃ��B
//------------------------------------------------
class CNodeVarArray
	: public IPolyNode
{
public:
	CNodeVarArray(tree_t parent, string const& name, PVal* pval);

	void acceptVisitor(ITreeVisitor& visitor) override {
		acceptVisitorTemplate<CNodeVarArray>(visitor);
	}

	/*
	�Ԃ�������
		�E���̕ϐ��͊��ɑ��݂��Ă��Ȃ�
			�e(prmstk�A�܂��͊O���v���O�C����`�̃I�u�W�F�N�g)������ł�����ύX����Ă���ꍇ
			�ÓI�ϐ��Ȃ炠�肦�Ȃ��B
			�e��H�邽�߂̉������K�v
		�E���[�h���ω�����
		�E�ϐ��^���ω�����
			�ϐ����̕ʕ��Ȃ̂ŁA�q�m�[�h�𑍎���ւ�����
			���������΃_���O�����O�����N�ɂȂ��Ă���N���[���ϐ�������ƃf�o�b�K���Ɨ�����̂ł�
		�E�v�f���A���ω�����
			�����������������A�v�f�����������ꍇ�́A
				�ϐ����̂����ւ����Ă���̂Ō^���ω������̂Ɠ��������ł���
				���g���^�ɂ���Ă͓����ϐ��̗v�f�������������邩������Ȃ�
				�������A�ϐ��̈������Ă��̌^�ɑΉ����鏈�����s���ׂ��Ȃ̂�
			�������オ�����ꍇ�A
				�q�m�[�h�̖��O�⏇�Ԃ��ς�� ((a, b) �� (a, b, c) �ɂȂ�����)
			�����������ŁA�P�ɔz�񂪐L�т��ꍇ�ɂ́A
				�L�т�����ǉ����āA���X�������v�f�͍Čv�Z����B
				�Čv�Z���ׂ��ł���A�Ƃ�������`�B����K�v������B
		�E�v�f�̒��g���ω�����
			�v�f�̒l���Čv�Z����
	*/

	bool updateState(tree_t childOrNull);

	// �����f�[�^
	PVal* getPVal() const { return pval_; }
	size_t cntElems() const { return cntElems_; }
	size_t getMaxDim() const { return maxDim_; }
	size_t const* getLengths() const { return length_; }

private:
	void initialize();
	void addElem(size_t aptr, size_t const* idxes);
	
private:
	PVal* pval_;

	size_t cntElems_;
	size_t maxDim_;
	size_t length_[hpimod::ArrayDimMax];

	vartype_t vt_;
	varmode_t mode_;
};

//------------------------------------------------
// �ϐ��v�f
// 
// @ �v�f�ԍ������肳��Ă�����́B
// @ �z��̗v�f���Avar �����B
//------------------------------------------------
class CNodeVarElem
	: public IMonoNode
{
public:
	CNodeVarElem(tree_t parent, string name, PVal* pval, APTR aptr);
	
	void acceptVisitor(ITreeVisitor& visitor) override { acceptVisitorTemplate<CNodeVarElem>(visitor); }
	bool updateState(tree_t childOrNull) override;

private:
	void reset();

private:
	PVal* pval_;
	APTR  aptr_;
};

//**********************************************************
//        �^���Ƃ̃f�[�^
//**********************************************************
//------------------------------------------------
// �P���l
//------------------------------------------------
template<class TSelf, class TVal, vartype_t TVartype, class TNode>
class CNodeValue
	: public TNode
{
public:
	typedef TVal value_type;

public:
	CNodeValue(TVal const& val)
		: val_(val)
	{ }

	template<class = std::enable_if_t<std::is_convertible<TNode*, INodeContainer*>::value>>
	CNodeValue(tree_t parent, TVal const& val)
		: TNode(parent), val_(val)
	{ }

public:
	void acceptVisitor(ITreeVisitor& visitor) override {
		acceptVisitorTemplate<TSelf>(visitor);
	}

	virtual vartype_t getVartype() const { return TVartype; }
	TVal const& getValue() const { return val_; }

private:
	TVal const val_;
};

//------------------------------------------------
// label �^
//------------------------------------------------
class CNodeLabel
	: public CNodeValue<CNodeLabel, label_t, HSPVAR_FLAG_LABEL, ILeaf>
{
public:
	CNodeLabel(label_t val) : CNodeValue(val) { }
};

//------------------------------------------------
// str �^
//------------------------------------------------
class CNodeString
	: public CNodeValue<CNodeString, char const*, HSPVAR_FLAG_STR, ILeaf>
{
public:
	CNodeString(char const* val) : CNodeValue(val) { }
};

//------------------------------------------------
// double �^
//------------------------------------------------
class CNodeDouble
	: public CNodeValue<CNodeDouble, double, HSPVAR_FLAG_DOUBLE, ILeaf>
{
public:
	CNodeDouble(double val) : CNodeValue(val) { }
};

//------------------------------------------------
// int �^
//------------------------------------------------
class CNodeInt
	: public CNodeValue<CNodeInt, int, HSPVAR_FLAG_INT, ILeaf>
{
public:
	CNodeInt(int val) : CNodeValue(val) { }
};

//------------------------------------------------
// struct �^
//------------------------------------------------
class CNodeModInst
	: public CNodeValue<CNodeModInst, FlexValue const*, HSPVAR_FLAG_STRUCT, IMonoNode>
{
public:
	CNodeModInst(tree_t parent, FlexValue const* p);
};

// nullmod
// @ singleton
// (Node �̌^���̂𕪂��Ă����Ȃ��� Visitor ���ŏꍇ������ procPre, procPost ��2��s�����ƂɂȂ��Ă��܂�)
class CNodeModInstNull
	: public CNodeValue<CNodeModInstNull, FlexValue const*, HSPVAR_FLAG_STRUCT, ILeaf>
{
public:
	static CNodeModInstNull& getInstance() { return const_cast<CNodeModInstNull&>(instance); }
private:
	CNodeModInstNull()
		: CNodeValue(nullptr)
	{ }

	static CNodeModInstNull const instance;
};

//------------------------------------------------
// �s���Ȍ^
//------------------------------------------------
class CNodeUnknown
	: public CNodeValue<CNodeUnknown, void const*, 0, ILeaf>
{
public:
	CNodeUnknown(void const* p, vartype_t vt)
		: CNodeValue(p), type_(vt)
	{ }

	vartype_t getVartype() const override { return type_; }
	
private:
	vartype_t type_;
};

//**********************************************************
//        ���̑��̃f�[�^
//**********************************************************
//------------------------------------------------
// prmstk
//------------------------------------------------
class CNodePrmStk
	: public IPolyNode
{
public:
	CNodePrmStk(tree_t parent, string name, void* prmstk, stdat_t stdat);
	CNodePrmStk(tree_t parent, string name, void* prmstk, stprm_t stprm);

public:
	void acceptVisitor(ITreeVisitor& visitor) override {
		acceptVisitorTemplate<CNodePrmStk>(visitor);
	}

private:
	void initialize();
	void add( size_t idx, void* member, stprm_t stprm );
	
private:
	void* prmstk_;
	stdat_t stdat_;
	stprm_t stprm_;
};

/*
//------------------------------------------------
// �P���ȕ�����
//------------------------------------------------
class CNodeSimpleStr
	: public ILeaf
{
public:
	CNodeSimpleStr(string s) : str_(s) { }
	
	string getDataString() const { return str_; }
	
private:
	string str_;
};
//*/

}

#endif
