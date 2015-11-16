// visitor - �m�[�h�����q

#ifndef IG_CLASS_NODE_ITERATOR_H
#define IG_CLASS_NODE_ITERATOR_H

#include <iterator>
#include "Node.dec.h"

//#include <boost/shared_ptr.hpp>

#include "ITreeVisitor.h"

namespace DataTree
{

//##############################################################################
//                �錾�� : CNodeIterator
//##############################################################################
//------------------------------------------------
// �m�[�h�����q
// 
// @ �O�������q�B
// @ �u�Q�ƌ^�v�N���X�B�������Ă��������m���Q�Ƃ���B
// @	dup() �ŁA�������e������ (���낤) �ʂ̎��̂𐶐��ł���B
// @ �������ꂽ���_�ŁA�s����͌��肷��B
// @	���̂��߁ApRoot ���ύX���ꂽ���_�ŁA���̔����q�͖����ɂȂ�B
//------------------------------------------------
class CNodeIterator
	: public std::iterator<std::forward_iterator_tag, ITree const*, void>
	//, public IFlatVisitor<ITree>	// Mix-in
{
public:
	typedef CNodeIterator self_t;
	typedef ITree const*  elem_t;
	typedef self_t this_t;
	typedef elem_t Elem_t;

	typedef elem_t target_t;// visitor �� target
	
public:
	CNodeIterator( elem_t pRoot );
	CNodeIterator( self_t const& obj ) { opCopy( obj ); }
	virtual ~CNodeIterator();
	
	//******************************************************
	//    Visitor �Ƃ��Ă̊֐�
	//******************************************************
public:
	virtual void visit_impl(target_t);
	
private:
	// ����
	virtual void procPre(target_t);
	virtual bool requiresEach(target_t) const { return false; }
	virtual bool requiresPost(target_t) const { return false; }
	
	//******************************************************
	//    Iterator �Ƃ��Ă̊֐�
	//******************************************************
public:
	static self_t Begin( elem_t );
	static self_t End  ( elem_t );
	
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++
	//    �E�Q��
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++
	elem_t operator * () const { return mpElem; }
	elem_t operator ->() const { return mpElem; }
	
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++
	//    �ړ�
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// �O�u
	self_t& operator ++()
	{
		moveToNext();
		return *this;
	}
	
	// ��u
	self_t operator ++(int)
	{
		self_t obj_bak( dup() );
		moveToNext();
		return obj_bak;
	}
	
	// ���
	self_t& operator =(self_t const& obj)
	{
		return opCopy( obj );
	}
	
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++
	//    ��r
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++
	bool operator ==(self_t const& obj) const
	{
		return ( mpElem == obj.mpElem );
	}
	
	bool operator !=(self_t const& obj) const
	{
		return !( *this == obj );
	}
	
	bool operator !() const
	{
		return ( mpElem == nullptr );
	}
	
private:
	void initialize();
	void moveToBegin();
	void moveToNext();
	void moveToEnd();
	
	self_t& opCopy( self_t const& obj )
	{
		mpRoot = obj.mpRoot;
		mpElem = obj.mpElem;
		m      = obj.m;
		return *this;
	}
	
	self_t dup() const
	{
		return self_t( mpRoot );
	}
	
	//******************************************************
	//    �����o�ϐ�
	//******************************************************
private:
	elem_t mpRoot;
	elem_t mpElem;
	
	struct Impl;
	Impl* m;
};

}

#endif
