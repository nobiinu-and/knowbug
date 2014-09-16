// Method - CMethod �̃��X�g

#ifndef IG_CLASS_METHOD_LIST_H
#define IG_CLASS_METHOD_LIST_H

/**
@summary:
	�^�� CMethod �̊֘A�Â����Ǘ�����N���X�B
	map �̃��b�p�B
@role:
	1. CMethod �̐���(new) (����щ��(delete))�B
	2. CMethod �ւ̃A�N�Z�X �B
		CMethodlist ������� CMethod �ւ̎Q�Ƃ��擾���邱�Ƃ͂ł��Ȃ��B
	3. �^�ւ� CMethod �̊���t���B
**/

#include "CMethod.h"

class CMethodlist
{
private:
	using value_t = std::map<int, CMethod*>;

	//############################################
	//    �����o�ϐ�
	//############################################
private:
	value_t* mplist;

	//############################################
	//    �����o�֐�
	//############################################
public:
	//--------------------------------------------
	// �\�z
	//--------------------------------------------
	CMethodlist()
		: mplist( new value_t )
	{ }

	//--------------------------------------------
	// ���
	//--------------------------------------------
	~CMethodlist()
	{
		dbgout("~CMethodlist");
		for ( auto& it : *mplist ) {
			delete it.second;
		}
		delete mplist; mplist = nullptr;
		return;
	}

	//--------------------------------------------
	// CMethod ���Z�b�g
	//--------------------------------------------
	void set( int vt )
	{
		if ( mplist->count(vt) > 0 ) {			// ���łɃ��\�b�h�����݂���
			return;
			/*
			value_t::iterator iter
				= mplist->find(vt);

			// ����{�폜
			delete iter->second; iter->second = nullptr;
			mplist->erase( iter );
			//*/
		}

		CMethod* pMethod = new CMethod(vt);
		mplist->insert( value_t::value_type(vt, pMethod) );
		return;
	}

	//--------------------------------------------
	// CMethod �����o��
	//--------------------------------------------
	CMethod* get( int vt ) const
	{
		auto iter = mplist->find(vt);
		return ( iter != mplist->end() )
			? iter->second
			: nullptr;
	}

//	operator std::map<int, CMethod*>*() { return mplist; }
};

#endif
