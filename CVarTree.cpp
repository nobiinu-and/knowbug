// VarTree

#include "CVarTree.h"

string const CStaticVarTree::ModuleName_Global = "@";

//------------------------------------------------
// �q�m�[�h�Ƃ��āA�ϐ��m�[�h��ǉ�����
//------------------------------------------------
void CStaticVarTree::ModuleNode::pushVar(char const* name)
{
	// �S�X�R�[�v���������߂�
	char const* const pModname = std::strchr(name, '@');

	// �X�R�[�v���������� => �q�m�[�h�̃��W���[���ɑ���
	if ( pModname ) {
		auto& child = insertChildModule(pModname);
		child.insertChildImpl<VarNode>(name);

	} else {
		insertChildImpl<VarNode>(name);
	}
	return;
}

//------------------------------------------------
// �q�m�[�h�́A�w�肵�����O�̃��W���[���E�m�[�h���擾����
// �Ȃ���Α}������
//------------------------------------------------
CStaticVarTree::ModuleNode& CStaticVarTree::ModuleNode::insertChildModule(char const* pModname)
{
	assert(pModname[0] == '@');

	// '@' ��T�� (���D��A�擪�ɂ̓}�b�`���Ȃ�)
	char const* const pModnameLast = std::strrchr(&pModname[1], '@');

	// �X�R�[�v����������ꍇ�́A���̃��W���[���E�m�[�h�̎q�m�[�h���猟������
	if ( pModnameLast ) {
		// �Ō�̃X�R�[�v����1����菜��������
		auto const modname2 = string(pModname, pModnameLast);

		auto& child = insertChildImpl<ModuleNode>(pModnameLast);
		return child.insertChildModule(modname2.c_str());

	} else {
		return insertChildImpl<ModuleNode>(pModname);
	}
}
