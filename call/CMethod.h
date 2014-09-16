// CMethod

#ifndef IG_CLASS_METHOD_H
#define IG_CLASS_METHOD_H

/**
@summary:
	�^1�̃��\�b�h�Q���Ǘ�����B
**/

#include <map>
#include <string>
#include "hsp3plugin_custom.h"

#include "CFunctor.h"
#include "cmd_call.h"

class CMethod
{
private:
	typedef std::map<std::string, CFunctor> methodlist_t;

	//############################################
	//    �����o�ϐ�
	//############################################
private:
	vartype_t mVartype;
	methodlist_t* mpMethodlist;

	//############################################
	//    �����o�֐�
	//############################################
public:
	explicit CMethod( vartype_t vt = HSPVAR_FLAG_NONE );
	virtual ~CMethod();

	int getVartype(void) const
	{
		return mVartype;
	}

	//--------------------------------------------
	// ���\�b�h��ǉ�
	//--------------------------------------------
	void add( const std::string& name, CFunctor const& functor );

	//--------------------------------------------
	// ���\�b�h���Ăяo��
	// @ ���ߌ`��
	//--------------------------------------------
	void call( const std::string& name, PVal* pvThis );

	//--------------------------------------------
	// 
	//--------------------------------------------

private:
	CMethod( CMethod const& src );
	CMethod& operator = (CMethod const& src );

};

#endif
