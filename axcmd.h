// AxCmd
// @ HSP���ԃR�[�h��1�P�ʂƓ������ʂ�����32bit�����l

#ifndef IG_AX_CMD_H
#define IG_AX_CMD_H

#include "hsp3plugin.h"

namespace AxCmd
{

static const int MagicCode = 0x20000000;

inline int make( int type, int code )
{
	return ( MagicCode | ((type & 0x0FFF) << 16) | (code & 0xFFFF) );
}

inline int getType( int id )
{
	return ( (id & MagicCode) ? ((id >> 16) & 0x0FFF) : 0 );
}

inline int getCode( int id )
{
	return ( (id & MagicCode) ? (id & 0xFFFF) : 0 );
}

inline bool isOk(int axcmd)
{
	return ((axcmd & MagicCode) != 0);
}

};

//------------------------------------------------
// ������ axcmd �����o��
//------------------------------------------------
inline int code_get_axcmd()
{
	int const axcmd = AxCmd::make(*type, *val);		// MagicCode ��
	code_next();

	// ���������⎮���ł͂Ȃ��A')' �ł��Ȃ� �� �^����ꂽ��������2����ȏ�łł��Ă���
	if ( !((*exinfo->npexflg & (EXFLG_1 | EXFLG_2)) != 0 || (*type == TYPE_MARK && *val == ')')) )
		puterror(HSPERR_INVALID_PARAMETER);
	return axcmd;
}

#endif
