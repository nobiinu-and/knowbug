// ���\�b�h�Ǘ��N���X

#include "hsp3plugin_custom.h"

#include <map>
#include <string>

#include "CCaller.h"
#include "CCall.h"
#include "CMethod.h"

//------------------------------------------------
// �\�z
//------------------------------------------------
CMethod::CMethod(vartype_t vt)
	: mVartype( vt )
	, mpMethodlist( new methodlist_t )
{ }

//------------------------------------------------
// ���
//------------------------------------------------
CMethod::~CMethod()
{
	delete [] mpMethodlist; mpMethodlist = nullptr;
	return;
}

//--------------------------------------------
// ���\�b�h��ǉ�
//--------------------------------------------
void CMethod::add( const std::string& name, CFunctor const& functor )
{
	mpMethodlist->insert(
		methodlist_t::value_type( name, functor )
	);
	return;
}

//--------------------------------------------
// ���\�b�h���Ăяo��
// @ ���ߌ`��
//--------------------------------------------
void CMethod::call(const std::string& name, PVal* pvThis)
{
	auto iter = mpMethodlist->find( name );

	// ���\�b�h�Ȃ�
	if ( iter == mpMethodlist->end() ) {
		puterror( HSPERR_UNSUPPORTED_FUNCTION );
	}

	// �Ăяo��
	{
		CCaller caller;
		caller.setFunctor( iter->second );
		caller.addArgByRef( pvThis, pvThis->offset );	// this ����
		caller.setArgAll();		// �����̐ݒ�
		caller.call();			// �Ăяo��
	}

	return;
}
