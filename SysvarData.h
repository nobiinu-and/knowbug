// �V�X�e���ϐ��f�[�^

#ifndef IG_SYSVAR_DATA_H
#define IG_SYSVAR_DATA_H

#include "hsp3struct.h"

struct StSysvarData
{
	char const* name;
	int type;
};

//**********************************************************
//        �V�X�e���ϐ��f�[�^
//**********************************************************
// SysvarId �ɑΉ�����
static StSysvarData const SysvarData[]
= {
	{ "stat",    HSPVAR_FLAG_INT },
	{ "refstr",  HSPVAR_FLAG_STR },
	{ "refdval", HSPVAR_FLAG_DOUBLE },
	{ "thismod", HSPVAR_FLAG_STRUCT },
	{ "cnt",     HSPVAR_FLAG_INT },
	{ "iparam",  HSPVAR_FLAG_INT },
	{ "wparam",  HSPVAR_FLAG_INT },
	{ "lparam",  HSPVAR_FLAG_INT },
	{ "strsize", HSPVAR_FLAG_INT },
	{ "looplev", HSPVAR_FLAG_INT },
	{ "sublev",  HSPVAR_FLAG_INT },
	{ "err",     HSPVAR_FLAG_INT },
	{ "notebuf", HSPVAR_FLAG_STR },
	
//	{ "mousex",  HSPVAR_FLAG_INT },
//	{ "mousey",  HSPVAR_FLAG_INT },
//	{ "mousew",  HSPVAR_FLAG_INT },
//	{ "ginfo",   HSPVAR_FLAG_INT }
//	{ "dirinfo", HSPVAR_FLAG_STR }
};

static size_t const SysvarCount = ( sizeof(SysvarData) / sizeof(SysvarData[0]) );

// SysvarData �ɑΉ�����
enum SysvarId
{
	SysvarId_Stat = 0,		// stat
	SysvarId_Refstr,		// refstr
	SysvarId_Refdval,		// refdval
	SysvarId_Thismod,		// thismod
	SysvarId_Cnt,			// cnt
	SysvarId_IParam,		// iparam
	SysvarId_WParam,		// wparam
	SysvarId_LParam,		// lparam
	SysvarId_StrSize,		// strsize
	SysvarId_Looplev,		// looplev
	SysvarId_Sublev,		// sublev
	SysvarId_Err,			// err
	SysvarId_NoteBuf,		// notebuf
	
	// GUI
//	SysvarId_MouseX,		// mousex
//	SysvarId_MouseY,		// mousey
//	SysvarId_MouseW,		// mousew
//	SysvarId_GInfo,			// ginfo
//	SysvarId_DirInfo,		// dirinfo
	SysvarId_MAX,
};

//**********************************************************
//        �֐��錾
//**********************************************************
extern SysvarId Sysvar_seek( char const* name );

extern int* Sysvar_getPtrOfInt(int id);
extern FlexValue* Sysvar_getThismod();

extern void Sysvar_getDumpInfo(int id,  void const*& out_data, size_t& out_size);

#endif
