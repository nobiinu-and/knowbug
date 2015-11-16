// visitor

// ������������

#ifndef IG_INTERFACE_VISITOR_H
#define IG_INTERFACE_VISITOR_H

//##############################################################################
//                �錾�� : IVisitor
//##############################################################################
//------------------------------------------------
// visitor
// 
// @ Mix-in �C���^�[�t�F�[�X�E�N���X(Interface-class)�B
// @ INode �̃c���[��K�� (visit() ��)�A�������s���B
// @ �Ăяo�����������Ȃ� => requires*() �� false ��Ԃ��B
// @	requires ���Ē�`���Ȃ� =>> �����p���� => ��������Ƃ݂Ȃ��B
// @ �񕪖؂ł͂Ȃ��̂ŁA�ʂ肪�����ł̏����ɂ͑Ή��ł��Ȃ��B
// @ 1. visit() �ŖK���ׂ��m�[�h���󂯎��B����������A�n���ꂽ�m�[�h�����A
// @	Visitor ���󂯓����֐� (acceptVisitor()) ���ĂԁB
// @ 2. �n�߂ɁA�m�[�h�́AIVisitor::procPre() ���ĂԁB���̊֐��� Visitor �́A
// @	�s���������̏ꍇ�ɍs�����������s����B
// @ 3. �m�[�h�́AVisitor �ɁA�q�m�[�h���ׂĂ�K���悤�Ɏw������B�܂�A
// @	procEach() ���ĂсA���̌�Avisit() �Ɏq�m�[�h��n�� ( �����ōċA�Ăяo��
// @	���������� )�B
// @ 4. �Ō�ɁA�m�[�h�́AIVisitor::procPost() ���ĂԁB���̊֐��ŁAVisitor �́A
// @	�A�肪�����̏ꍇ�ɍs�����������s����B
// @ 5. return;
//------------------------------------------------
template<class T>
class IVisitor
{
public:
	typedef const T* target_t;
	
public:
//	IVisitor();
	virtual ~IVisitor() { }
	
	//******************************************************
	//    �C���^�[�t�F�[�X
	//******************************************************
public:
	virtual void visit( target_t ) = 0;
	
	// ���� ( requires �`�F�b�N�̂� )
	virtual void procPre ( target_t p ) { if ( requiresPre (p) ) procImplPre (p); }
	virtual void procEach( target_t p ) { if ( requiresEach(p) ) procImplEach(p); }
	virtual void procPost( target_t p ) { if ( requiresPost(p) ) procImplPost(p); }
	
protected:
	// �������邩�ۂ� ( �������Ȃ��Ƃ� false ��Ԃ��悤�ɂ��� )
	virtual bool requiresPre ( target_t ) const { return true; }
	virtual bool requiresEach( target_t ) const { return true; }
	virtual bool requiresPost( target_t ) const { return true; }
	
	// ���� ( ���� )
	virtual void procImplPre ( target_t ) { }
	virtual void procImplEach( target_t ) { }
	virtual void procImplPost( target_t ) { }
};

//##############################################################################
//                �錾�� : IFlatVisitor
//##############################################################################
//------------------------------------------------
// flat-visitor
// 
// @ ����(traverse)�ł͂Ȃ�����(iterate)�B
// @ �ŏ��ɖK�ꂽ�m�[�h�̎q�m�[�h�ł̂ݏ���������B
// @	( �l�X�g 2 �ȉ��� requires* ���_�Œe�� )
//------------------------------------------------
template<class T>
class IFlatVisitor
	: public IVisitor<T>
{
public:
	IFlatVisitor()
		: mcntNest( 0 )
		, mbBlock( false )
	{ }
	virtual ~IFlatVisitor() { }
	
	//******************************************************
	//    �C���^�[�t�F�[�X
	//******************************************************
public:
	virtual void visit( target_t tar )
	{
		if ( !mbBlock ) {
			mcntNest = 0;		// �l�X�g��������
			mbBlock  = true;	// �u���b�N�J�n
				visit_impl( tar );
			mbBlock  = false;	// �u���b�N����
			
		} else if ( isProcable() ) {
			mcntNest ++;
				visit_impl( tar );
			mcntNest --;
		}
		return;
	}
	
	// ���� ( requires �`�F�b�N�̂� )
//	virtual void procPre ( target_t p ) { if ( requiresPre () ) procImplPre (p); }
//	virtual void procEach( target_t p ) { if ( requiresEach() ) procImplEach(p); }
//	virtual void procPost( target_t p ) { if ( requiresPost() ) procImplPost(p); }
	
protected:
	virtual void visit_impl( target_t pNode ) = 0;
	
	// �������邩�ۂ� ( �������Ȃ��Ƃ� false ��Ԃ��悤�ɂ��� )
//	virtual bool requiresPre ( target_t ) const { return true; }
//	virtual bool requiresEach( target_t ) const { return true; }
//	virtual bool requiresPost( target_t ) const { return true; }
	
	// ���� ( ���� )	
//	virtual void procImplPre ( target_t ) { }
//	virtual void procImplEach( target_t ) { }
//	virtual void procImplPost( target_t ) { }
	
	// �����ł��邩�ǂ���
	virtual bool isProcable(void) const
	{
		return ( mbBlock && mcntNest != 0 );
	}
	
private:
	bool mbBlock;
	int  mcntNest;
	
};

#endif
