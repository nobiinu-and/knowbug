// �ϐ��f�[�^�̃c���[�`��������

#include <algorithm>
#include "module/ptr_cast.h"

#include "main.h"

#include "CVardataString.h"
#include "SysvarData.h"

#include "with_ModPtr.h"
#include "WrapCall/ResultNodeData.h"

extern string getVartypeString(PVal const* pval);

static string stringizeSimpleValue(vartype_t type, void const* ptr, bool bShort);
static char const* findVarName(PVal const* pval);

//##########################################################
//        �v�f���Ƃ̏����֐�
//##########################################################
//------------------------------------------------
// [add][item] �ϐ�
//------------------------------------------------
void CVardataStrWriter::addVar(char const* name, PVal const* pval)
{
	assert(!!pval);
	auto const hvp = hpimod::getHvp(pval->flag);

	// �g���^
	if ( pval->flag >= HSPVAR_FLAG_USERDEF ) {
		auto const iter = g_config->vswInfo.find(hvp->vartype_name);
		if ( iter != g_config->vswInfo.end() ) {
			if ( auto const addVar = iter->second.addVarUserdef ) {
				addVar(this, name, pval);
				return;
			}
		}
	}

	// �W���z��
	if ( !(pval->len[1] == 1 && pval->len[2] == 0)
		&& (hvp->support & (HSPVAR_SUPPORT_FIXEDARRAY | HSPVAR_SUPPORT_FLEXARRAY))
	) {
		addVarArray(name, pval);

	// �P��
	} else {
		addVarScalar(name, pval, 0);
	}
	return;
}

//------------------------------------------------
// [add][item] �P�̕ϐ�
// 
// @ �v�f�̒l���o�͂���B
//------------------------------------------------
void CVardataStrWriter::addVarScalar(char const* name, PVal const* pval)
{
	addValue(name, pval->flag, hpimod::PVal_getPtr(pval));
	return;
}

void CVardataStrWriter::addVarScalar(char const* name, PVal const* pval, APTR aptr)
{
	addValue(name, pval->flag, hpimod::PVal_getPtr(pval, aptr));
	return;
}

//------------------------------------------------
// [add][item] �W���z��ϐ�
//------------------------------------------------
void CVardataStrWriter::addVarArray(char const* name, PVal const* pval)
{
	auto const hvp = hpimod::getHvp(pval->flag);

	getWriter().catNodeBegin(name, strf("<%s>[", hvp->vartype_name).c_str());

	size_t const dim = hpimod::PVal_maxDim(pval);

	// �c���[�󕶎���̏ꍇ
	if ( !getWriter().isLineformed() ) {
		getWriter().catAttribute("type", getVartypeString(pval).c_str());

		// �e�v�f��ǉ�����
		size_t const cntElems = hpimod::PVal_cntElems(pval);
		int indexes[hpimod::ArrayDimMax] = { 0 };

		for ( size_t i = 0; i < cntElems; ++i ) {
			calcIndexesFromAptr(indexes, i, &pval->len[1], cntElems, dim);

			// �v�f�̒l��ǉ�
			string const nameChild = makeArrayIndexString(dim, indexes);
			addVarScalar(nameChild.c_str(), pval, i);
		}
		
	// ��s������̏ꍇ
	} else if ( dim > 0 ) {
		// cntElems[1 + i] = ����i�����z��̗v�f��
		// (�Ⴆ�Δz�� int(2, 3, 4) ���ƁAcntElems = {1, 2, 2*3, 2*3*4, 0})
		size_t cntElems[1 + hpimod::ArrayDimMax] = { 1 };
		for ( size_t i = 0; i < dim && pval->len[i + 1] > 0; ++i ) {
			cntElems[i + 1] = pval->len[i + 1] * cntElems[i];
		}

		addVarArrayRec(pval, cntElems, dim - 1, 0);
	}

	getWriter().catNodeEnd("]");
	return;
}

void CVardataStrWriter::addVarArrayRec(PVal const* pval, size_t const cntElems[], size_t idxDim, APTR aptr_offset)
{
	assert(getWriter().isLineformed());
	for ( int i = 0; i < pval->len[idxDim + 1]; ++i ) {
		//if ( i != 0 ) getWriter().cat(", ");

		// 2���ȏ� => �z����o��
		if ( idxDim >= 1 ) {
			getWriter().catNodeBegin(CStructedStrWriter::stc_strUnused, "[");
			addVarArrayRec(pval, cntElems, idxDim - 1, aptr_offset + (i * cntElems[idxDim]));
			getWriter().catNodeEnd("]");

		// 1�� => �e�v�f���o��
		} else {
			addVarScalar(CStructedStrWriter::stc_strUnused, pval, aptr_offset + i);
		}
	}
	return;
}

//------------------------------------------------
// ��ʂ̒l
//------------------------------------------------
void CVardataStrWriter::addValue(char const* name, vartype_t type, void const* ptr)
{
	// �����l�X�g�΍�
	if ( getWriter().inifiniteNesting() ) {
		getWriter().catLeafExtra(name, "too_many_nesting");
		return;
	}

	// �g���^
	if ( type >= HSPVAR_FLAG_USERDEF ) {
		auto const iter = g_config->vswInfo.find(hpimod::getHvp(type)->vartype_name);
		if ( iter != g_config->vswInfo.end() ) {
			if ( auto const addValue = iter->second.addValueUserdef ) {
				addValue(this, name, ptr);
				return;
			}
		}
	}

	if ( type == HSPVAR_FLAG_STRUCT ) {
		addValueStruct(name, cptr_cast<FlexValue*>(ptr));

#ifdef with_ModPtr
	} else if ( type == HSPVAR_FLAG_INT && ModPtr::isValid(*cptr_cast<int*>(ptr)) ) {
		auto const modptr = *cptr_cast<int*>(ptr);
		string const name2 = strf("%s = mp#%d", name, ModPtr::getIdx(modptr));
		addValueStruct(name2.c_str(), ModPtr::getValue(modptr));
#endif
	} else {
		auto const dbgstr = stringizeSimpleValue(type, ptr, getWriter().isLineformed());
		getWriter().catLeaf(name, dbgstr.c_str());
	}
	return;
}

//------------------------------------------------
// [add][item] flex-value
//------------------------------------------------
void CVardataStrWriter::addValueStruct(char const* name, FlexValue const* fv)
{
	assert(fv);

	if ( !fv->ptr || fv->type == FLEXVAL_TYPE_NONE ) {
		getWriter().catLeafExtra(name, "nullmod");

	} else {
		//	cat(strf("%s.myid = %d", getIndent().c_str(), fv->myid ));
		auto const stdat = hpimod::FlexValue_getModule(fv);
		auto const modclsNameString =
			makeModuleClassNameString(stdat, hpimod::FlexValue_isClone(fv));

		getWriter().catNodeBegin(name, (modclsNameString + "{").c_str());
		getWriter().catAttribute("modcls", modclsNameString.c_str());
			
		addPrmstack(stdat, fv->ptr);

		getWriter().catNodeEnd("}");
	}
	return;
}

//------------------------------------------------
// [add][item] prmstack
// 
// @ ���g�����o�͂���B
//------------------------------------------------
void CVardataStrWriter::addPrmstack(stdat_t stdat, void const* prmstack)
{
	/*
	getWriter().catAttribute( "id_finfo", strf("%d", stdat->subid).c_str() );
	getWriter().catAttribute( "id_minfo", strf("%d", stdat->prmindex).c_str() );
	//*/

	int prev_mptype = MPTYPE_NONE;
	int i = 0;

	std::for_each(hpimod::STRUCTDAT_getStPrm(stdat), hpimod::STRUCTDAT_getStPrmEnd(stdat), [&](STRUCTPRM const& stprm) {
		auto const member = hpimod::Prmstack_getMemberPtr(prmstack, &stprm);

		// ��s������F�ŏ��� local �̑O�ɂ͋󔒂�1���߂ɒu��
		if ( !getWriter().isLineformed()
			&& i > 0
			&& (prev_mptype != MPTYPE_LOCALVAR && stprm.mptype == MPTYPE_LOCALVAR)
			) {
			getWriter().cat(" ");
		}

		addParameter(getStPrmName(&stprm, i).c_str(), stdat, &stprm, member);

		// structtag => �����o�ł͂Ȃ��̂ŁA�����Ȃ�
		if ( stprm.mptype != MPTYPE_STRUCTTAG ) { ++i; }
	});
	return;
}

//------------------------------------------------
// [add][item] �����o (in prmstack)
//------------------------------------------------
void CVardataStrWriter::addParameter(char const* name, stdat_t stdat, stprm_t stprm, void const* member)
{
	switch ( stprm->mptype ) {
		case MPTYPE_STRUCTTAG: break;

		// �ϐ� (PVal*)
		//	case MPTYPE_VAR:
		case MPTYPE_SINGLEVAR:
		case MPTYPE_ARRAYVAR:
		{
			// ��s������F�Q�Ƃł��邱�Ƃ����� (var �� array ���͖��炩)
			// delimiter �Ƃ̈ʒu��肪���
		//	if ( getWriter().isLineformed() ) getWriter().cat("& ");

			auto const vardata = cptr_cast<MPVarData*>(member);

			if ( stprm->mptype == MPTYPE_SINGLEVAR ) {
				addVarScalar(name, vardata->pval, vardata->aptr);
			} else {
				addVar(name, vardata->pval);
			}
			break;
		}
		// �ϐ� (PVal)
		case MPTYPE_LOCALVAR:
		{
			auto const pval = cptr_cast<PVal*>(member);
			addVar(name, pval);
			break;
		}
		// thismod
		case MPTYPE_MODULEVAR:
		case MPTYPE_IMODULEVAR:
		case MPTYPE_TMODULEVAR:
		{
			auto const thismod = cptr_cast<MPModVarData*>(member);
			auto const fv = cptr_cast<FlexValue*>(hpimod::PVal_getPtr(thismod->pval, thismod->aptr));
			addValueStruct(name, fv);
			break;
		}
		// ������ (char**)
		//	case MPTYPE_STRING:
		case MPTYPE_LOCALSTRING:
			addValue(name, HSPVAR_FLAG_STR, *cptr_cast<char**>(member));
			break;

		// ���̑�
		case MPTYPE_DNUM:
			addValue(name, HSPVAR_FLAG_DOUBLE, cptr_cast<double*>(member));
			break;

		case MPTYPE_INUM:
			addValue(name, HSPVAR_FLAG_INT, cptr_cast<int*>(member));
			break;

		case MPTYPE_LABEL:
			addValue(name, HSPVAR_FLAG_LABEL, cptr_cast<label_t*>(member));
			break;

		// �� => ����
		default:
			getWriter().catLeaf(name,
				strf("ignored (mptype = %d)", stprm->mptype).c_str()
			);
			break;
	}
	return;
}

//------------------------------------------------
// [add] �V�X�e���ϐ�
// 
// @result: �������_���v����o�b�t�@�ƃT�C�Y
//------------------------------------------------
void CVardataStrWriter::addSysvar(SysvarId id)
{
	char const* const name = SysvarData[id].name;

	switch ( id ) {
		case SysvarId_Refstr:
			addValue(name, HSPVAR_FLAG_STR, ctx->refstr);
			break;

		case SysvarId_Refdval:
			addValue(name, HSPVAR_FLAG_DOUBLE, &ctx->refdval);
			break;

		case SysvarId_Cnt:
			if ( ctx->looplev ) {
				// int �z��Ɠ����\���ɂ���
				getWriter().catNodeBegin(name, "<int>[");
				for ( int i = 0; i < ctx->looplev; ++ i ) {
					auto const lvLoop = ctx->looplev - i;
					addValue(strf("#%d", lvLoop).c_str(), HSPVAR_FLAG_INT, &ctx->mem_loop[lvLoop].cnt);
					//getWriter().catLeaf(strf("#%d", lvLoop), strf("%d", cnt).c_str());
				}
				getWriter().catNodeEnd("]");
			} else {
				getWriter().catLeafExtra(name, "out_of_loop");
			}
			break;

		case SysvarId_NoteBuf:
		{
			if ( PVal const* const pval = ctx->note_pval ) {
				APTR const aptr = ctx->note_aptr;
				string name2;
				if ( auto const nameOfVar = findVarName(pval) ) {
					size_t const maxDim = hpimod::PVal_maxDim(pval);
					int indexes[hpimod::ArrayDimMax];
					calcIndexesFromAptr(indexes, aptr, &pval->len[1], hpimod::PVal_cntElems(pval), maxDim);
					name2 = strf("%s (%s%s)", name, nameOfVar, makeArrayIndexString(maxDim, indexes).c_str());
				} else {
					name2 = strf("%s (0x%08X[%d])", name, address_cast(pval), aptr);
				}
				addVarScalar(name2.c_str(), pval, aptr);
			} else {
				getWriter().catLeafExtra(name, "not_exist");
			}
			break;
		}
		case SysvarId_Thismod:
			if ( auto const fv = Sysvar_getThismod() ) {
				addValueStruct(name, fv);
			} else {
				getWriter().catLeafExtra(name, "nullmod");
			}
			break;
		default:
			// �����l
			if ( SysvarData[id].type == HSPVAR_FLAG_INT ) {
				addValue(name, HSPVAR_FLAG_INT, Sysvar_getPtrOfInt(id));
				break;
			} else throw;
	};
}

//------------------------------------------------
// [add] �Ăяo��
//
// @prm prmstk can be nullptr
//------------------------------------------------
void CVardataStrWriter::addCall(stdat_t stdat, void const* prmstk)
{
	char const* const name = hpimod::STRUCTDAT_getName(stdat);
#if 0
	string const name2 =
		(stdat->index == STRUCTDAT_INDEX_CFUNC && !getWriter().isLineformed())
		? strf("%s()", name)			// �c���[�󂩂֐��Ȃ疼�O�� () ������
		: name;
#endif
	addCall(name, stdat, prmstk);
	return;
}

void CVardataStrWriter::addCall(char const* name, stdat_t stdat, void const* prmstk)
{
	getWriter().catNodeBegin(name, strf("%s(", name).c_str());
	if ( !prmstk ) {
		if ( getWriter().isLineformed() ) {
			getWriter().catLeafExtra(CStructedStrWriter::stc_strUnused, "not_available");
		} else {
			getWriter().catLeafExtra("arguments", "not_available");
		}
	} else {
		addPrmstack(stdat, prmstk);
	}
	getWriter().catNodeEnd(")");
	return;
}

//------------------------------------------------
// [add] �Ԓl
//------------------------------------------------
void CVardataStrWriter::addResult(char const* name, void const* ptr, vartype_t type)
{
	assert(getWriter().isLineformed());	// ���݂̎����ł�

	addValue(name, type, ptr);
	return;
}

//**********************************************************
//        �������֐�
//**********************************************************

//------------------------------------------------
// �P���Ȓl�𕶎��񉻂���
//
// @ addItem_value �Ńt�b�N�����^�͂����ɗ��Ȃ��B
//------------------------------------------------
string stringizeSimpleValue(vartype_t type, void const* ptr, bool bShort)
{
	assert(type != HSPVAR_FLAG_STRUCT);

	switch ( type ) {
		case HSPVAR_FLAG_STR:
		{
			auto const val = cptr_cast<char*>(ptr);
			return (bShort
				? toStringLiteralFormat(val)
				: string(val));
		}
		case HSPVAR_FLAG_COMOBJ:  return strf("comobj(0x%08X)", address_cast(*cptr_cast<void**>(ptr)));
		case HSPVAR_FLAG_VARIANT: return strf("variant(0x%08X)", address_cast(*cptr_cast<void**>(ptr)));
		case HSPVAR_FLAG_DOUBLE:  return strf((bShort ? "%f" : "%.16f"), *cptr_cast<double*>(ptr));
		case HSPVAR_FLAG_INT:
		{
			int const val = *cptr_cast<int*>(ptr);
#ifdef with_ModPtr
			assert(!ModPtr::isValid(val));	// addItem_value �ŏ������ꂽ�͂�
#endif
			return (bShort
				? strf("%d", val)
				: strf("%-10d (0x%08X)", val, val));
		}
		case HSPVAR_FLAG_LABEL:
		{
			auto const lb = *cptr_cast<label_t*>(ptr);
			int const idx = hpimod::findOTIndex(lb);
			auto const name =
				(idx >= 0 ? g_dbginfo->ax->getLabelName(idx) : nullptr);
			return (name ? strf("*%s", name) : strf("label(0x%08X)", address_cast(lb)));
		}
#if 0
		case HSPVAR_FLAG_STRUCT:
			{
				auto const fv = cptr_cast<FlexValue*>(ptr);
				if ( fv->type == FLEXVAL_TYPE_NONE ) {
					return string("(empty struct)");
				} else {
					return strf("struct%s(0x%08X; #%d, size:%d)",
						(fv->type == FLEXVAL_TYPE_CLONE ? "&" : ""),
						fv->customid, address_cast(fv->ptr), fv->size,
						);
				}
			}
#endif
		default:
		{
			auto const vtname = hpimod::getHvp(type)->vartype_name;

#ifdef with_ExtraBasics
			// �g����{�^
			bool bSigned = false;

			if ( strcmp(vtname, "bool") == 0 ) {
				static char const* const bool_name[2] = { "false", "true" };
				return bool_name[*cptr_cast<bool*>(ptr) ? 1 : 0];

				// char (signed char �Ƃ���)
			} else if (
				(strcmp(vtname, "char") == 0 || strcmp(vtname, "signed_char") == 0) && (bSigned = true)
				|| (strcmp(vtname, "uchar") == 0 || strcmp(vtname, "unsigned_char") == 0)
				) {
				int const val = bSigned ? *cptr_cast<signed char*>(ptr) : *cptr_cast<unsigned char*>(ptr);
				return (val == 0) ? string("0 ('\\0')") : strf("%-3d '%c'", static_cast<int>(val), static_cast<char>(val));

				// short
			} else if (
				(strcmp(vtname, "short") == 0 || strcmp(vtname, "signed_short") == 0) && (bSigned = true)
				|| (strcmp(vtname, "ushort") == 0 || strcmp(vtname, "unsigned_short") == 0)
				) {
				int const val = static_cast<int>(bSigned ? *cptr_cast<signed short*>(ptr) : *cptr_cast<unsigned short*>(ptr));
				return (bShort ? strf("%d", val) : strf("%-6d (0x%04X)", val, static_cast<short>(val)));

				// unsigned int
			} else if ( strcmp(vtname, "uint") == 0 || strcmp(vtname, "unsigned_int") == 0 ) {
				auto const val = *cptr_cast<unsigned int*>(ptr);
				return (bShort ? strf("%d", val) : strf("%-10d (0x%08X)", val, val));

				// long
			} else if (
				(strcmp(vtname, "long") == 0 || strcmp(vtname, "signed_long") == 0) && (bSigned = true)
				|| (strcmp(vtname, "ulong") == 0 || strcmp(vtname, "unsigned_long") == 0)
				) {
				auto const   signed_val = *cptr_cast<long long*>(ptr);
				auto const unsigned_val = *cptr_cast<unsigned long long*>(ptr);
				return (bShort
					? strf("%d", (bSigned ? signed_val : unsigned_val))
					: strf("%d (0x%16X)", (bSigned ? signed_val : unsigned_val), signed_val)
					);
				// ���܂�
			} else if ( strcmp(vtname, "tribyte") == 0 ) {
				auto const bytes = cptr_cast<char*>(ptr);
				int const val = bytes[0] << 16 | bytes[1] << 8 | bytes[2];
				return (bShort
					? strf("%d", val)
					: strf("%-8d (0x%06X)", val, val)
					);
			}
#endif
			return strf("unknown<%s>(0x%08X)", vtname, address_cast(ptr));
		}
	}
}

//------------------------------------------------
// ������𕶎��񃊃e�����̌`���ɕϊ�����
//------------------------------------------------
string toStringLiteralFormat(char const* src)
{
	size_t const maxlen = (std::strlen(src) * 2) + 2;
	char* const buf = exinfo->HspFunc_malloc(maxlen + 1);
	size_t idx = 0;

	buf[idx++] = '\"';

	for ( int i = 0;; ++i ) {
		char c = src[i];

		// �G�X�P�[�v�E�V�[�P���X����������
		if ( c == '\0' ) {
			break;

		} else if ( c == '\\' || c == '\"' ) {
			buf[idx++] = '\\';
			buf[idx++] = c;

		} else if ( c == '\t' ) {
			buf[idx++] = '\\';
			buf[idx++] = 't';

		} else if ( c == '\r' || c == '\n' ) {
			buf[idx++] = '\\';

			if ( c == '\r' && src[i + 1] == '\n' ) {	// CR + LF
				buf[idx++] = 'n';
				i++;
			} else {
				buf[idx++] = 'r';
			}

		} else {
			buf[idx++] = c;
		}
	}

	buf[idx++] = '\"';
	buf[idx++] = '\0';

	string const sResult = buf;
	exinfo->HspFunc_free(buf);
	return std::move(sResult);
}

//------------------------------------------------
// ���W���[���N���X����\��������
//------------------------------------------------
string makeModuleClassNameString(stdat_t stdat, bool bClone)
{
	auto const modclsName = hpimod::STRUCTDAT_getName(stdat);
	return (bClone
		? strf("%s&", modclsName)
		: string(modclsName));
}

//------------------------------------------------
// �z��Y���̕�����
//------------------------------------------------
string makeArrayIndexString(size_t dim, int const indexes[])
{
	switch ( dim ) {
		case 0: return BracketIdxL BracketIdxR;
		case 1: return strf(BracketIdxL "%d"             BracketIdxR, indexes[0]);
		case 2: return strf(BracketIdxL "%d, %d"         BracketIdxR, indexes[0], indexes[1]);
		case 3: return strf(BracketIdxL "%d, %d, %d"     BracketIdxR, indexes[0], indexes[1], indexes[2]);
		case 4: return strf(BracketIdxL "%d, %d, %d, %d" BracketIdxR, indexes[0], indexes[1], indexes[2], indexes[3]);
		default:
			return "(invalid index)";
	}
}

//------------------------------------------------
// APTR�l����Y�����v�Z����
// 
// @prm indexes: dim �ȏ�̗v�f�����Bdim ����̗v�f�͕ύX����Ȃ��B
//------------------------------------------------
void calcIndexesFromAptr(int* indexes, APTR aptr, int const* lengths, size_t cntElems, size_t dim)
{
	for ( size_t idxDim = 0; idxDim < dim; ++idxDim ) {
		indexes[idxDim] = aptr % lengths[idxDim];
		aptr /= lengths[idxDim];
	}
	return;
}

//------------------------------------------------
// �X�R�[�v��������菜�������O
//------------------------------------------------
string removeScopeResolution(string const& name)
{
	int const idxScopeResolution = name.find('@');
	return (idxScopeResolution != string::npos
		? name.substr(0, idxScopeResolution)
		: name);
}

//------------------------------------------------
// �\���̃p�����[�^�̖��O
// 
// �f�o�b�O��񂩂�擾����B�Ȃ���΁u(idx)�v�Ƃ���B
//------------------------------------------------
string getStPrmName(stprm_t stprm, int idx)
{
	int const subid = hpimod::findStPrmIndex(stprm);
	if ( subid >= 0 ) {
		if ( auto const name = g_dbginfo->ax->getPrmName(subid) ) {
			return removeScopeResolution(name);

		// thismod ����
		} else if ( stprm->mptype == MPTYPE_MODULEVAR || stprm->mptype == MPTYPE_IMODULEVAR || stprm->mptype == MPTYPE_TMODULEVAR ) {
			return "thismod";
		}
	}
	return makeArrayIndexString(1, &idx);
}

//------------------------------------------------
// �ϐ��̖��O�������� (failure: nullptr)
//------------------------------------------------
static char const* findVarName(PVal const* pval)
{
	auto const idx = pval - ctx->mem_var;
	return (0 <= idx && idx < ctx->hsphed->max_val)
		? exinfo->HspFunc_varname(idx)
		: nullptr;
}