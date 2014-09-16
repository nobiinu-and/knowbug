// �֐��q�I�u�W�F�N�g

#include "CCaller.h"
#include "CBound.h"
#include "CStreamCaller.h"
#include "CLambda.h"

#include "CFunctor.h"

#include "cmd_sub.h"

#include <vector>
#include <algorithm>
#include <typeinfo>

using namespace hpimod;

//------------------------------------------------
// �\�z (AxCmd)
// 
// @ axcmd
//------------------------------------------------
CFunctor::CFunctor( int _axcmd )
	 : type( FuncType_Deffid )
	 , deffid( 0 )
{
	assert( AxCmd::isOk(_axcmd) );

	switch ( AxCmd::getType(_axcmd) ) {
		case TYPE_LABEL:
		{
			type = FuncType_Label;
			lb   = hpimod::getLabel( AxCmd::getCode(_axcmd) );
			break;
		}
		case TYPE_MODCMD:
			deffid = AxCmd::getCode( _axcmd );
			break;
		default: puterror( HSPERR_TYPE_MISMATCH );
	}

	return;
}

//------------------------------------------------
// �\�z (����֐��q)
//------------------------------------------------
CFunctor::CFunctor( exfunctor_t _ex )
	: type( FuncType_Ex )
	, ex( _ex )
{
	if ( !ex ) type = FuncType_None;
	ex->AddRef();
	return;
}

//------------------------------------------------
// �j��
//------------------------------------------------
CFunctor::~CFunctor()
{
	clear();
	return;
}

//------------------------------------------------
// ������
//------------------------------------------------
void CFunctor::clear()
{
	switch ( type ) {
		case FuncType_None: return;
		case FuncType_Label:
		case FuncType_Deffid:
			break;
		case FuncType_Ex:
			FunctorEx_Release( ex );
			break;
		default: throw;
	}

	type = FuncType_None;
	return;
}

//------------------------------------------------
// �g�p��
//------------------------------------------------
int CFunctor::getUsing() const
{
	switch ( type ) {
		case FuncType_None:   return 0;
		case FuncType_Label:  return ( lb ? 1 : 0 );
		case FuncType_Deffid: return 1;
		case FuncType_Ex:     return ( ex ? ex->getUsing() : 0 );
		default:
			throw;
	}
}

//------------------------------------------------
// ���������X�g�𓾂�
//------------------------------------------------
CPrmInfo const& CFunctor::getPrmInfo() const
{
	switch ( type ) {
		case FuncType_Label:  return GetPrmInfo( lb );
		case FuncType_Deffid: return GetPrmInfo( getSTRUCTDAT(deffid) );
		case FuncType_Ex:     return ex->getPrmInfo();
		default: throw;
	}
}

//------------------------------------------------
// ���x���擾
//------------------------------------------------
label_t CFunctor::getLabel() const
{
	switch ( type ) {
		case FuncType_Label:  return lb;
		case FuncType_Deffid: return hpimod::getLabel( getSTRUCTDAT(deffid)->otindex );
		case FuncType_Ex:     return ex->getLabel();
		default: throw;
	}
//	return nullptr;
}

//------------------------------------------------
// ��`ID�𓾂�
//------------------------------------------------
int CFunctor::getAxCmd() const
{
	switch ( type ) {
		case FuncType_Label:  break;
		case FuncType_Deffid: return AxCmd::make( TYPE_MODCMD, deffid );
		case FuncType_Ex:     return ex->getAxCmd();
		default: throw;
	}
	return 0;
}

//------------------------------------------------
// 
//------------------------------------------------

//------------------------------------------------
// �Ăяo��
//------------------------------------------------
void CFunctor::call( CCaller& caller )
{
	switch ( type ) {
		case FuncType_Label:
		case FuncType_Deffid:
			return caller.getCall().callLabel( getLabel() );

		case FuncType_Ex:
			return ex->call( caller );

		default:
			return puterror( HSPERR_UNSUPPORTED_FUNCTION );
	}
}

//------------------------------------------------
// 
//------------------------------------------------


//------------------------------------------------
// ��r
// 
// @ �召�֌W����ӂɌ���ł���΂悢�B
//------------------------------------------------
int CFunctor::compare( CFunctor const& obj ) const
{
	if ( type != obj.type ) return type - obj.type;

	switch ( type ) {
		case FuncType_Label:  return ( lb     - obj.lb );
		case FuncType_Deffid: return ( deffid - obj.deffid );
		case FuncType_Ex:
			{
				// ���ۂ̌^�̈Ⴂ��D��
				auto& t0 = typeid(*ex);
				auto& t1 = typeid(*obj.ex);
				if ( t0 != t1 ) return t0.hash_code() - t1.hash_code();
			}
			return ex->compare( obj.ex );
		default:
			puterror( HSPERR_UNSUPPORTED_FUNCTION );
			throw;
	}
}

//------------------------------------------------
// ����
//------------------------------------------------
CFunctor& CFunctor::copy( CFunctor const& src )
{
	clear();

	if ( src.getUsing() == 0 ) {
		return *this;
	}

	type = src.type;

	switch ( type ) {
		case FuncType_Label:  lb     = src.lb;     break;
		case FuncType_Deffid: deffid = src.deffid; break;
		case FuncType_Ex:
			ex = src.ex;
			FunctorEx_AddRef( ex );
			break;
		default: throw;
	}

	return *this;
}
