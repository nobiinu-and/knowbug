// AxCmd
// @ HSP���ԃR�[�h��1�P�ʂƓ������ʂ�����32bit�����l

#ifndef IG_AX_CMD_H
#define IG_AX_CMD_H

#include "hsp3plugin_custom.h"
#include "mod_argGetter.h"

namespace AxCmd
{

static int const MagicCode = 0x20000000;

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
static inline int code_get_axcmd()
{
	int const axcmd = AxCmd::make(*type, *val);		// MagicCode ��
	hpimod::code_get_singleToken();
	return axcmd;
}

#endif
