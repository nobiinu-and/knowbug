// visitor - �f�[�^������̐�����

// �A�m�e�[�V���� (#length = ... �Ȃ�) �� CNodeAnnotation �Ƃ���ׂ��ȋC�����邪�A�P�Ƀ������̖��ʌ����Ȃ����ȋC������

#include "main.h"
#include "DebugInfo.h"

#include "Node.h"
#include "CDataStringCreator.h"

#include "module/CStrBuf.h"
#include "module/ptr_cast.h"
#include "module/strf.h"
//#include "module/debug.h"

#ifdef with_Assoc
# include "D:/Docs/prg/cpp/MakeHPI/hpimod/var_assoc/for_knowbug.var_assoc.h"	// ���܂�ɂ������̂Ńt���p�X
#endif
#ifdef with_Vector
# include "D:/Docs/prg/cpp/MakeHPI/hpimod/var_vector/for_knowbug.var_vector.h"	// ���܂�ɂ������̂Ńt���p�X
#endif
#ifdef with_Array
# include "D:/Docs/prg/cpp/MakeHPI/var_array/src/for_knowbug.var_array.h"	// ���܂�ɂ������̂Ńt���p�X
#endif

#include "with_Script.h"
#include "with_ModPtr.h"

namespace DataTree
{
;

//------------------------------------------------
// �\�z
//------------------------------------------------
CDataStringCreator::CDataStringCreator()
	: buf_(new CStrBuf)
{ }

void CDataStringCreator::setLenLimit(int len)
{
	buf_->setLenLimit(len);
	return;
}

string const& CDataStringCreator::getString() const
{
	return buf_->get();
}

//------------------------------------------------
// �K��
//------------------------------------------------
void CDataStringCreator::visit0(ITree* t)
{
	dbgout("visit to %X", t);
	t->acceptVisitor(*this);
	return;
}

//------------------------------------------------
// ���W���[��
//------------------------------------------------
void CDataStringCreator::visit(CNodeModule* p)
{
	catNodeItemPre(p->getName().c_str());

	for ( auto child : p->getChildren() ) {
		catNodeItemEach();
		visit0(child);
	}

	catNodeItemPost();
	return;
}

//------------------------------------------------
// �ϐ�
//------------------------------------------------
void CDataStringCreator::visit(CNodeVarArray* p)
{
	PVal* const pval = p->getPVal();

	switch (pval->flag) {
		case HSPVAR_FLAG_LABEL:
		case HSPVAR_FLAG_STR:
		case HSPVAR_FLAG_DOUBLE:
		case HSPVAR_FLAG_INT:
		case HSPVAR_FLAG_STRUCT:
		{
			catNodeItemPre(p->getName().c_str());

			auto const vartypeName = hpimod::getHvp(pval->flag)->vartype_name;

			// �ꎟ���z��
			if ( pval->len[2] <= 0 ) {
				string const format = makeArrayIndexString(1, p->getLengths());
				catNodeItemAnnotation("vartype",
					strf("%s %s", vartypeName, format.c_str()));

			// �������z��
			} else {
				string const format = makeArrayIndexString(p->getMaxDim(), p->getLengths());
				catNodeItemAnnotation("vartype",
					strf("%s %s (%d in total)", vartypeName, format.c_str(), p->cntElems()));
			}
			
			for ( auto child : p->getChildren() ) {
				catNodeItemEach();
				visit0(child);
			}

			catNodeItemPost();
			break;
		}
		// TODO: �g���^�̏ꍇ�͊O������^����ꂽ�֐��Ɋۓ���������
		default:
			catLeafItem(p->getName().c_str());
			cat("unknown");
			break;
	}
}

//------------------------------------------------
// �ϐ��v�f
//------------------------------------------------
void CDataStringCreator::visit(CNodeVarElem* p)
{
	catLeafItem(p->getName().c_str());
	visit0(p->getChild());
}

//------------------------------------------------
// label �l
//------------------------------------------------
void CDataStringCreator::visit(CNodeLabel* p)
{
	auto const lb = p->getValue();
	int const idx = hpimod::findOTIndex(lb);
	auto const name =
		(idx >= 0 ? g_dbginfo->ax->getLabelName(idx) : nullptr);
	cat( (name ? strf("*%s", name) : strf("*label (0x%08X)", address_cast(lb))) );
	return;
}

//------------------------------------------------
// str �l
//------------------------------------------------
void CDataStringCreator::visit(CNodeString* p)
{
	cat(p->getValue());
}

//------------------------------------------------
// double �l
//------------------------------------------------
void CDataStringCreator::visit(CNodeDouble* p)
{
	cat(strf("%.16f", p->getValue()));
}

//------------------------------------------------
// int �l
//------------------------------------------------
void CDataStringCreator::visit(CNodeInt* p)
{
	int const val = p->getValue();

#ifdef with_ModPtr
	// TODO: node �𐶐�����i�K�ŁAint �̑���� ModInst �𐶐�����
	// "mp#%d" ���o�͂��邽�߂ɂ́AModInst ������ CNodeModPtr (<: IMonoNode) �𐶐�����

	if (ModPtr::isValid(val)) {
		CNodeModInst node(ModPtr::getValue(val));
		cat(strf("mp#%d", ModPtr::getIdx(val)));
		visit0(&node);
		return;
	}
#endif

	cat(strf("%-10d (0x%08X)", val, val));
	return;
}

//------------------------------------------------
// struct �l
// 
// @ ... = (struct: �^��&):
// @	<prmstk>
//------------------------------------------------
void CDataStringCreator::visit(CNodeModInst* p)
{
	auto const fv = p->getValue();

	string const name = strf("(struct: %s%s)",
		hpimod::STRUCTDAT_getName(hpimod::FlexValue_getStDat(fv)),
		((fv->type == FLEXVAL_TYPE_CLONE) ? "&" : "")
	);

	catNodeItemPre(name.c_str());

	visit0(p->getChild());	// prmstk

	catNodeItemPost();
	return;
}

void CDataStringCreator::visit(CNodeModInstNull* p)
{
	cat("(struct: null)");
}

//------------------------------------------------
// prmstk
// 
// struct �^�ϐ��ɂ̂݊i�[�����B
// ���ꂪ�Ă΂�鎞�_�Łu*: ...�v�̎������͍s���Ă���B
//------------------------------------------------
void CDataStringCreator::visit(CNodePrmStk* p)
{
	for ( auto const child : p->getChildren() ) {
		catNodeItemEach();
		visit0(child);
	}
	return;
}

//**********************************************************
//    ������̒ǉ�
//**********************************************************
//------------------------------------------------
// �P���A��
//------------------------------------------------
void CDataStringCreator::cat(char const* s)
{
	buf_->cat(s);
}

//------------------------------------------------
// ���s�̘A��
//------------------------------------------------
void CDataStringCreator::catCrlf()
{
	buf_->catCrlf();
}

//------------------------------------------------
// �o�b�t�@��16�i���Ń������_���v���ĘA��
//------------------------------------------------
void CDataStringCreator::catDump(void const* pBuf, size_t bufsize)
{
	static const size_t stc_maxsize = 0x10000;
	size_t size = bufsize;

	if ( size > stc_maxsize ) {
		cat(strf("�S%d�o�C�g�̓��A%d�o�C�g�݂̂��_���v���܂��B", bufsize, stc_maxsize));
		size = stc_maxsize;
	}

	catln("dump  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F");
	catln("----------------------------------------------------");

	buf_->catDump(pBuf, size, 0x10);

	catCrlf();
	catln(strf("�o�b�t�@�T�C�Y�F%d (bytes)", bufsize));
	return;
}

//**********************************************************
// �����s�����񐶐���
//**********************************************************
// �R���e�i�m�[�h�̕�����̒ǉ�����
void CDataStringFullCreator::catNodeItemPre(char const* name)
{
	dbgout("poly-container %s", name);
	cat(name);
	catln(":");
	incStrNest();
	return;
}

void CDataStringFullCreator::catNodeItemAnnotation(string const& name, string const& value)
{
	catNodeItemEach();

	cat(name);
	cat(" = ");
	cat(value);
	return;
}

// ���[�t�m�[�h�̕�����̒ǉ�����
void CDataStringFullCreator::catLeafItem(char const* name)
{
	dbgout("mono-container %s", name);
	cat(name);
	cat(" = ");
	return;
}

//**********************************************************
// ��s�����񐶐���
//**********************************************************
// �R���e�i�m�[�h�̕�����̒ǉ�����
void CDataStringLineCreator::catNodeItemPre(char const* name)
{
	dbgout("poly-container %s", name);
	cat("[");
	return;
}

void CDataStringLineCreator::catNodeItemEach()
{
	cat(", ");
	return;
}

void CDataStringLineCreator::catNodeItemPost()
{
	cat("]");
	return;
}

// ���[�t�m�[�h�̕�����̒ǉ�����
void CDataStringLineCreator::catLeafItem(char const* name)
{
	dbgout("mono-container %s", name);
	//
	return;
}

}
