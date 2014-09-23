// �ϐ��f�[�^�e�L�X�g�����N���X

#include <numeric>	// for accumulate
#include <algorithm>

#include "module/ptr_cast.h"

#include "main.h"
#include "SysvarData.h"
#include "CVarinfoText.h"
#include "CVardataString.h"

#ifdef with_WrapCall
# include "WrapCall/ResultNodeData.h"
using namespace WrapCall;
#endif

//------------------------------------------------
// �ϐ��f�[�^���琶��
//------------------------------------------------
void CVarinfoText::addVar( PVal* pval, char const* name )
{
	auto const hvp = hpimod::getHvp(pval->flag);
	int bufsize;
	void const* const pMemBlock =
		hvp->GetBlockSize(pval, ptr_cast<PDAT*>(pval->pt), ptr_cast<int*>(&bufsize));

	// �ϐ��Ɋւ�����
	getWriter().catln(strf("�ϐ����F%s", name));
	getWriter().catln(strf("�ϐ��^�F%s", getVartypeString(pval).c_str()));
	getWriter().catln(strf("�A�h���X�F0x%08X, 0x%08X", address_cast(pval->pt), address_cast(pval->master)));
	getWriter().catln(strf("�T�C�Y�Fusing %d of %d [byte]", pval->size, bufsize));
	getWriter().catCrlf();

	// �ϐ��̓��e�Ɋւ�����
	{
		auto vardat = CVardataStrWriter::create<CTreeformedWriter>(getBuf());
		vardat.addVar(name, pval);
	}
	getWriter().catCrlf();

	// �������_���v
	getWriter().catDump(pMemBlock, static_cast<size_t>(bufsize));
	return;
}

//------------------------------------------------
// �V�X�e���ϐ��f�[�^���琶��
//------------------------------------------------
void CVarinfoText::addSysvar(char const* name)
{
	auto const id = Sysvar_seek(name);
	if ( id == SysvarId_MAX ) return;

	getWriter().catln(strf("�ϐ����F%s\t(�V�X�e���ϐ�)", name));
	getWriter().catln(strf("�ϐ��^�F%s", hpimod::getHvp(SysvarData[id].type)->vartype_name));
	getWriter().catCrlf();

	{
		CVardataStrWriter::create<CTreeformedWriter>(getBuf())
			.addSysvar(id);
	}
	getWriter().catCrlf();

	// �������_���v
	{
		void const* data; size_t size;
		Sysvar_getDumpInfo(id, data, size);
		getWriter().catDump(data, size);
	}
	return;
}

#if with_WrapCall
//------------------------------------------------
// �Ăяo���f�[�^���琶��
// 
// @prm prmstk: nullptr => �������m��
//------------------------------------------------
void CVarinfoText::addCall(ModcmdCallInfo const& callinfo)
{
	auto const stdat = callinfo.stdat;
	auto const name = hpimod::STRUCTDAT_getName(stdat);
	getWriter().catln(
		(callinfo.fname == nullptr)
			? strf("�֐����F%s", name)
			: strf("�֐����F%s (#%d of %s)", name, callinfo.line, callinfo.fname)
	);

	// �V�O�l�`��
	getWriter().catln(strf("�������F(%s)", getPrmlistString(stdat).c_str()));
	getWriter().catCrlf();

	auto const prmstk = callinfo.getPrmstk();
	
	CVardataStrWriter::create<CTreeformedWriter>(getBuf())
			.addCall( stdat, prmstk );

	if ( prmstk ) {
		getWriter().catCrlf();
		getWriter().catDump(prmstk, static_cast<size_t>(stdat->size));
	}
	return;
}

//------------------------------------------------
// �Ԓl�f�[�^���琶��
//------------------------------------------------
void CVarinfoText::addResult( stdat_t stdat, string const& text, char const* name )
{
	getWriter().catln(strf("�֐����F%s", name));
//	getWriter().catln(strf("�������F(%s)", getPrmlistString(stdat).c_str()));
	getWriter().catCrlf();

	// �ϐ��̓��e�Ɋւ�����
	getWriter().cat(text);
//	getWriter().catCrlf();
	
	// �������_���v
//	getWriter().catDump( ptr, static_cast<size_t>(bufsize) );
	return;
}

#endif

//**********************************************************
//        �T�ς̒ǉ�
//**********************************************************
//------------------------------------------------
// [add] ���W���[���T��
//------------------------------------------------
void CVarinfoText::addModuleOverview(char const* name, CVarTree::ModuleNode const& tree)
{
	getWriter().catln(strf("[%s]", name));

	for ( auto const& iter : tree ) {
		//for ( auto iter = tree.begin(); iter != tree.end(); ++iter ) {
		CVarTree const& child = *iter.second;

		child.match<void>([&](CStaticVarTree::VarNode const& child) {
			auto const varname = child.getName();
			auto const varname_raw = removeScopeResolution(varname);

			getWriter().cat(varname_raw + "\t= ");
			{
				CVardataStrWriter::create<CLineformedWriter>(getBuf())
					.addVar(varname.c_str(), hpimod::seekSttVar(varname.c_str()));
			}
			getWriter().catCrlf();

		}, [&](CStaticVarTree::ModuleNode const& child) {
			// (����q��)���W���[���͖��O�����\�����Ă���
			getWriter().catln(child.getName());
		});
	}
	return;
}

//------------------------------------------------
// [add] �V�X�e���ϐ��T��
//------------------------------------------------
void CVarinfoText::addSysvarsOverview()
{
	getWriter().catln("[�V�X�e���ϐ�]");

	for ( int i = 0; i < SysvarCount; ++i ) {
		getWriter().cat(SysvarData[i].name);
		getWriter().cat("\t= ");
		{
			CVardataStrWriter::create<CLineformedWriter>(getBuf())
				.addSysvar(static_cast<SysvarId>(i));
		}
		getWriter().catCrlf();
	}
	return;
}

#ifdef with_WrapCall
//------------------------------------------------
// [add] �Ăяo���T��
// 
// depends on WrapCall
//------------------------------------------------
void CVarinfoText::addCallsOverview(ResultNodeData const* pLastResult)
{
	getWriter().catln("[�Ăяo������]");

	auto const range = WrapCall::getCallInfoRange();
	std::for_each(range.first, range.second, [&](stkCallInfo_t::value_type const& pCallInfo) {
		{
			CVardataStrWriter::create<CLineformedWriter>(getBuf())
				.addCall(pCallInfo->stdat, pCallInfo->getPrmstk());
		}
		getWriter().catCrlf();
	});

	// �Ō�̕Ԓl
	if ( pLastResult ) {
		getWriter().cat("-> ");
		getWriter().catln(pLastResult->valueString);
	}
	return;
}
#endif

//**********************************************************
//        �������֐�
//**********************************************************
/*
//------------------------------------------------
// ���s��A������
//------------------------------------------------
void CVarinfoText::cat_crlf( void )
{
	if ( mlenLimit < 2 ) return;
	
	mpBuf->append( "\r\n" );
	mlenLimit -= 2;
	return;
}

//------------------------------------------------
// �������A������
//------------------------------------------------
void CVarinfoText::cat( char const* string )
{
	if ( mlenLimit <= 0 ) return;
	
	size_t len( strlen( string ) + 2 );		// 2 �� crlf �̕�
	
	if ( static_cast<int>(len) > mlenLimit ) {
		mpBuf->append( string, mlenLimit );
		mpBuf->append( "(���������̂ŏȗ����܂����B)" );
		mlenLimit = 2;
	} else {
		mpBuf->append( string );
		mlenLimit -= len;
	}
	
	cat_crlf();
	return;
}

//------------------------------------------------
// �����t���������A������
//------------------------------------------------
void CVarinfoText::catf( char const* format, ... )
{
	va_list   arglist;
	va_start( arglist, format );
	
	string s = vstrf( format, arglist );
	cat( s.c_str() );
	
	va_end( arglist );
	return;
}
//*/

//------------------------------------------------
// mptype �̕�����𓾂�
// todo: hpimod �Ɉړ��H
//------------------------------------------------
char const* getMPTypeString(int mptype)
{
	switch ( mptype ) {
		case MPTYPE_NONE:        return "none";
		case MPTYPE_STRUCTTAG:   return "structtag";

		case MPTYPE_LABEL:       return "label";
		case MPTYPE_DNUM:        return "double";
		case MPTYPE_STRING:
		case MPTYPE_LOCALSTRING: return "str";
		case MPTYPE_INUM:        return "int";
		case MPTYPE_VAR:
		case MPTYPE_PVARPTR:				// #dllfunc
		case MPTYPE_SINGLEVAR:   return "var";
		case MPTYPE_ARRAYVAR:    return "array";
		case MPTYPE_LOCALVAR:    return "local";
		case MPTYPE_MODULEVAR:   return "thismod";//"modvar";
		case MPTYPE_IMODULEVAR:  return "modinit";
		case MPTYPE_TMODULEVAR:  return "modterm";

#if 0
		case MPTYPE_IOBJECTVAR:  return "comobj";
	//	case MPTYPE_LOCALWSTR:   return "";
	//	case MPTYPE_FLEXSPTR:    return "";
	//	case MPTYPE_FLEXWPTR:    return "";
		case MPTYPE_FLOAT:       return "float";
		case MPTYPE_PPVAL:       return "pval";
		case MPTYPE_PBMSCR:      return "bmscr";
		case MPTYPE_PTR_REFSTR:  return "prefstr";
		case MPTYPE_PTR_EXINFO:  return "pexinfo";
		case MPTYPE_PTR_DPMINFO: return "pdpminfo";
		case MPTYPE_NULLPTR:     return "nullptr";
#endif
		default: return "unknown";
	}
}

#include "module/map_iterator.h"
//------------------------------------------------
// ���������X�g�̕�����
//------------------------------------------------
string getPrmlistString(stdat_t stdat)
{
#if 0
	string s = "";

	//if ( stdat->prmmax == 0 ) s = "void";
	std::for_each(hpimod::STRUCTDAT_getStPrm(stdat), hpimod::STRUCTDAT_getStPrmEnd(stdat), [&](STRUCTPRM const& stprm) {
		if ( !s.empty() ) s += ", ";
		s += getMPTypeString(stprm.mptype);
	});
	return s;
#else
	auto const range = make_mapped_range(hpimod::STRUCTDAT_getStPrm(stdat), hpimod::STRUCTDAT_getStPrmEnd(stdat),
		[&](STRUCTPRM const& stprm) { return getMPTypeString(stprm.mptype); });
	return join(range.begin(), range.end(), ", ");
#endif
}

#if 0 // ���g�p
static char const* getModeString(varmode_t mode)
{
	return	(mode == HSPVAR_MODE_NONE) ? "����" :
		(mode == HSPVAR_MODE_MALLOC) ? "����" :
		(mode == HSPVAR_MODE_CLONE) ? "�N���[��" : "???";
}
#endif
static char const* getTypeQualifierFromMode(varmode_t mode)
{
	return (mode == HSPVAR_MODE_NONE) ? "!" :
		(mode == HSPVAR_MODE_MALLOC) ? "" :
		(mode == HSPVAR_MODE_CLONE) ? "&" : "<err>";
}

// �ϐ��̌^��\��������
string getVartypeString(PVal const* pval)
{
	size_t const maxDim = hpimod::PVal_maxDim(pval);

	string const arrayType =
		(maxDim == 0) ? "(empty)" :
		(maxDim == 1) ? makeArrayIndexString(1, &pval->len[1]) :
		strf("%s (%d in total)", makeArrayIndexString(maxDim, &pval->len[1]).c_str(), hpimod::PVal_cntElems(pval))
	;

	return strf("%s%s %s",
		hpimod::getHvp(pval->flag)->vartype_name,
		getTypeQualifierFromMode(pval->mode),
		arrayType.c_str()
	);
}
