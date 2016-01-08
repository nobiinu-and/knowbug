
#pragma once

#include <string>
#include <vector>
#include <unordered_set>

#include <Windows.h>
#include "hspsdk/hsp3plugin.h"
#undef stat

namespace hpiutil {

static auto const ArrayDimMax = size_t { 4 };

//static auto const BracketIdxL = "(";
//static auto const BracketIdxR = ")";

static auto const HSPVAR_FLAG_COMOBJ = HSPVAR_FLAG_COMSTRUCT;
static auto const HSPVAR_FLAG_VARIANT = 7;

using vartype_t = unsigned short;
using varmode_t = signed short;
using label_t = unsigned short const*; // a.k.a. HSPVAR_LABEL
using csptr_t = unsigned short const*;
using stdat_t = STRUCTDAT const*;
using stprm_t = STRUCTPRM const*;

// HspVarProc�̉��Z�֐�
using operator_t = void(*)(PDAT*, void const*);

// �f�o�b�O�E�B���h�E�̃R�[���o�b�N�֐�
using debug_callback_t = BOOL(CALLBACK*)(HSP3DEBUG*, int, int, int);

// �f�o�b�O�E�B���h�E�ւ̒ʒmID
enum DebugNotice
{
	DebugNotice_Stop = 0,
	DebugNotice_Logmes = 1,
};

// �萔 /MPTYPE_(\w+)/ �̒l�ɑΉ�����K���Ȗ��O�𓾂�
extern char const* nameFromMPType(int mptype);

// �ÓI�ϐ��̖��O�𓾂� (���s���� nullptr)
extern char const* nameFromStaticVar(PVal const* pval);

// ���W���[���N���X���𓾂� (�N���[���Ȃ疖���� `&` ������)
extern std::string nameFromModuleClass(stdat_t stdat, bool isClone);

/**
�G�C���A�X�̖��O�𓾂�
index �͂��̃G�C���A�X�̌��̈�����ɂ�����ԍ��B
DInfo ����݂���Ȃ���� "(i)" ���Ԃ�B
//*/
extern std::string nameFromStPrm(stprm_t stprm, int index);

/**
���x�����𓾂�
DInfo ����݂���Ȃ���� "label(%p)" ���Ԃ�
//*/
extern std::string nameFromLabel(label_t lb);

// �t�@�C���Q�Ɩ��̈ꗗ
extern auto fileRefNames() -> std::unordered_set<std::string> const&;

//�����񃊃e����
extern std::string literalFormString(char const* s);

//�z��Y���̕�����̐���
extern std::string stringifyArrayIndex(std::vector<int> const& indexes);

//�C���q����菜�������ʎq
extern std::string nameExcludingScopeResolution(std::string const& name);

}
