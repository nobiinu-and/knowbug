// ����֐��q�C���^�[�t�F�[�X

// ����ȁu�֐��v��\�����߂ɁA�v���O�C�����ō쐬����N���X�����̊��N���X�B
// CFunctor ���� exfuntor_t �^�Ƃ��ĎQ�Ƃ���邽�߂ɑ��݂���B
// �Ⴆ�΁ACBound, CCoRoutine, CLambda �ȂǂɌp������Ă���B

// �Q�ƃJ�E���^�Ƃ��Ă̋@�\�������Ă���B

#ifndef IG_INTERFACE_FUNCTOR_EXTRA_H
#define IG_INTERFACE_FUNCTOR_EXTRA_H

#include "hsp3plugin_custom.h"
using namespace hpimod;

//	#define DBGOUT_FUNCTOR_EX_ADDREF_OR_RELEASE	// AddRef, Release �� dbgout �ŕ񍐂���

class CPrmInfo;
class CCaller;
class CFunctor;

class IFunctorEx;
using exfunctor_t = IFunctorEx*;

//------------------------------------------------
// ����Ȋ֐��q��\���N���X
// 
// @ �p�����Ďg���B
// @ �����ł͊��蓮����`���Ă���B
//------------------------------------------------
class IFunctorEx
{
protected:
	IFunctorEx();

public:
	virtual ~IFunctorEx();

	// �擾
	virtual label_t getLabel() const { return nullptr; }
	virtual int getAxCmd() const { return 0; }

	virtual int getUsing() const { return 0; }	// �g�p�� (0: ����, 1: �L��, 2: �N���[��)
	virtual CPrmInfo const& getPrmInfo() const = 0;				// ������

	// ����
	virtual void call( CCaller& caller ) = 0;

	// ���̑�
	virtual int compare( exfunctor_t const& obj ) const { return this - obj; }	// obj �͏�� this �Ɠ��������̔h���̌^

private:
	// ����
	IFunctorEx( IFunctorEx const& ) = delete;
	IFunctorEx& operator =( IFunctorEx const& ) = delete;

	// �Q�ƃJ�E���^�֘A
private:
	mutable int mRefer;

public:
#ifdef DBGOUT_FUNCTOR_EX_ADDREF_OR_RELEASE
	int mId;
	void AddRef()  const { mRefer ++; dbgout("[%d] ++ �� %d", mId, mRefer); }
	bool Release() const { mRefer --; dbgout("[%d] -- �� %d", mId, mRefer); return (mRefer <= 0); }
#else
	void AddRef()  const { mRefer ++; }
	bool Release() const { return ( (-- mRefer) <= 0 ); }
#endif

//	static exfunctor_t New();	// �����֐�
	static void Delete( exfunctor_t& self );

	static void ReleaseAllInstance();
};

inline void FunctorEx_AddRef( exfunctor_t self ) {
	if ( self ) self->AddRef();
	return;
}

inline void FunctorEx_Release( exfunctor_t& self ) {
	if ( self ) {
		if ( self->Release() ) IFunctorEx::Delete(self);
	}
	return;
}
#endif
