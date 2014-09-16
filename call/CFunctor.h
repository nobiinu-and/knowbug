// �֐��q�I�u�W�F�N�g

// ��ʂɁA�u�֐��v�Ƃ��ČĂяo������̂��������߂̃I�u�W�F�N�g�B
// ��{�֐��q { ���x���A���[�U��`�֐� } ���A����֐��q (IFunctorEx) �����B
// �����̋�̓I�ȈႢ���ɏՂ��邽�߂̃C���^�[�t�F�[�X�ł���B

// �������郁���o�ϐ��͂��ׂĒP���R�s�[�\�Ȃ��̂ł���A�T�C�Y�����������߁A
// �Q�Ƃł͂Ȃ����̂Ƃ��Ďg�p���Ă悢�B

#ifndef IG_CLASS_FUNCTOR_H
#define IG_CLASS_FUNCTOR_H

#include "hsp3plugin_custom.h"
#include "axcmd.h"

using namespace hpimod;

class CPrmInfo;
class CCaller;

class CFunctor;
typedef       CFunctor functor_t;
typedef const CFunctor cfunctor_t;

class IFunctorEx;
using exfunctor_t = IFunctorEx*;

//################################################
//    ��`��
//################################################
//------------------------------------------------
// �֐��q�^�C�v
//------------------------------------------------
enum FuncType
{
	FuncType_None = 0,	// �Ȃ� (����)
	FuncType_Label,		// ���x��
	FuncType_Deffid,	// ���[�U��`�֐�
	FuncType_Ex,		// ����֐��q
	FuncType_MAX
};

//------------------------------------------------
// �֐��q��\���N���X
//------------------------------------------------
class CFunctor
{
private:
	FuncType type;
	union {
		label_t  lb;		// ���x���֐�
		int      deffid;	// ���[�U��`�֐�
		exfunctor_t ex;		// ����֐��q
	};

public:
	CFunctor() : type( FuncType_None ) { }
	CFunctor( label_t  _lb    ) : type( FuncType_Label ), lb( _lb ) { }
	CFunctor( int      _axcmd );
	CFunctor( exfunctor_t _ex );

	CFunctor( CFunctor const& src )
		: type( FuncType_None )
	{ this->copy( src ); }

	~CFunctor();

	void clear();

	// �擾
	FuncType getType() const { return type; }

	int             getUsing()   const;	// �g�p�� (0: ����, 1: �L��, 2: �N���[��)
	CPrmInfo const& getPrmInfo() const;	// not nullptr

	label_t   getLabel() const;		// or nullptr
	int       getAxCmd() const;		// or 0
	exfunctor_t getEx()  const { return getType() == FuncType_Ex ? ex : nullptr; }

	// �L���X�g (IFunctorEx �p)
	template<class T>       T     castTo()       { return dynamic_cast<T>( getEx() ); }
	template<class T> const T     castTo() const { return dynamic_cast<T>( getEx() ); }
	template<class T>       T safeCastTo()       { return safeCastTo_Impl<T>(); }
	template<class T> const T safeCastTo() const { return safeCastTo_Impl<const T>(); }

	template<class T> T safeCastTo_Impl() const {
		auto result = castTo<T>();
		if ( !result ) puterror( HSPERR_TYPE_MISMATCH );
		return result;
	}

	// ����
	void call( CCaller& caller );

	// ���Z
	CFunctor& operator =( CFunctor const& src ) { return this->copy( src ); }

	int compare( CFunctor const& src ) const;

private:
	CFunctor& copy( CFunctor const& );

};

#endif
