// ����֐��q�C���^�[�t�F�[�X
// ���蓮��̒�`

#include "IFunctorEx.h"
#include <vector>

static std::vector<exfunctor_t> g_allExFunctors;

//------------------------------------------------
// �\�z
//------------------------------------------------
IFunctorEx::IFunctorEx()
	: mRefer( 0 )
{
#ifdef DBGOUT_FUNCTOR_EX_ADDREF_OR_RELEASE
	static int stt_counter = 0; mId = ++ stt_counter;
#endif
//	dbgout( "new!" );
//	g_allExFunctors.push_back( this ); AddRef();	// �S���̃R���e�i�ɓo�^

	return;
}

//------------------------------------------------
// ���
//------------------------------------------------
IFunctorEx::~IFunctorEx()
{
	// �S���̃R���e�i����o�^����
	/*
	auto iter = std::find( g_allExFunctors.begin(), g_allExFunctors.end(), this );
	if ( iter != g_allExFunctors.end() ) {
		*iter = nullptr;
		dbgout( "unregister!" );
	}
	//*/

//	dbgout( "delete!" );
}

void IFunctorEx::Delete( exfunctor_t& self )
{
	delete self; self = nullptr;
	return;
}

void IFunctorEx::ReleaseAllInstance()
{
	/*
	// �S���̃R���e�i�̉��
	for each ( auto iter in g_allExFunctors ) {
		FunctorEx_Release( iter );
	}

	g_allExFunctors.clear();
	//*/
	return;
}

