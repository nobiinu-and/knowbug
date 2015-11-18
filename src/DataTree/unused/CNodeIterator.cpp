// visitor - �m�[�h�O�������q

#include <vector>
#include "Node.h"
#include "CNodeIterator.h"

namespace DataTree
{

//##############################################################################
//                ��`�� : CNodeIterator
//##############################################################################

//**********************************************************
//    �����o�ϐ��Ȃ�
//**********************************************************
struct CNodeIterator::Impl
{
public:
	typedef CNodeIterator::elem_t elem_t;
	typedef std::vector<elem_t>   children_t;
	typedef children_t::iterator  childrenIter_t;
	
public:
	children_t     children;
	childrenIter_t iter;
	
public:
//	Impl() { }
	
public:
	elem_t get(void) const
	{
		return ( (iter == children.end()) ? NULL : *iter );
	}
};

//**********************************************************
//    �\�z�Ɖ��
//**********************************************************
//------------------------------------------------
// �\�z
//------------------------------------------------
CNodeIterator::CNodeIterator( elem_t pRoot )
	: m( new Impl )
	, mpRoot( pRoot )
	, mpElem( NULL )
{
	if ( pRoot ) {
		initialize();
		moveToBegin();
	} else {
		moveToEnd();
	}
	return;
}

//------------------------------------------------
// ���
//------------------------------------------------
CNodeIterator::~CNodeIterator()
{ }

//------------------------------------------------
// ������
//------------------------------------------------
void CNodeIterator::initialize()
{
	visit( mpRoot );
	return;
}

//**********************************************************
//    Visitor �Ƃ��Ă̊֐�
//**********************************************************
//------------------------------------------------
// �K�� (����)
//------------------------------------------------
void CNodeIterator::visit_impl( ITree const* pNode )
{
	pNode->acceptVisitor( const_cast<CNodeIterator*>(this) );
	return;
}

//------------------------------------------------
// �s��
//------------------------------------------------
void CNodeIterator::procPre( ITree const* pNode )
{
	if ( !isProcable() ) return;
	
	m->children.push_back( pNode );
	return;
}

//**********************************************************
//    Iterator �Ƃ��Ă̊֐�
//**********************************************************

//------------------------------------------------
// �擪�ֈړ�
//------------------------------------------------
void CNodeIterator::moveToBegin(void)
{
	m->iter = m->children.begin();
	mpElem = m->get();
	return;
}

//------------------------------------------------
// ���ֈړ�
// 
// @ End �� ++ ����̂�h�~
//------------------------------------------------
void CNodeIterator::moveToNext(void)
{
	if ( mpElem != NULL ) {
		++ m->iter;
		mpElem = m->get();
	}
	return;
}

//------------------------------------------------
// �I�[�ֈړ� ( �E�Q�Ƃł��Ȃ��Ȃ� )
//------------------------------------------------
void CNodeIterator::moveToEnd(void)
{
//	m->iter = m->children.end();
	mpElem = NULL;
	return;
}

//##############################################################################
//                �ÓI�����o�֐�
//##############################################################################
//------------------------------------------------
// �擪�̔����q�𓾂�
//------------------------------------------------
CNodeIterator CNodeIterator::Begin( ITree const* pNode )
{
	return self_t( pNode );
}

//------------------------------------------------
// �����̔����q�𓾂�
//------------------------------------------------
CNodeIterator CNodeIterator::End( ITree const* pNode )
{
	self_t obj( pNode );
	obj.moveToEnd();
	return obj;
}

}
