// macro for cmdlist

#ifndef IG_HPIMOD_CMDLIST_MACRO_H
#define IG_HPIMOD_CMDLIST_MACRO_H

// cmdlist �p�̃}�N��
// �X�N���v�g���ɒ񋟂���v���O�C���R�}���h�̈ꗗ���A�ȉ��̃}�N�����g���ċL�q���Ă����AC++ ���� HSP ���ŋ��L����B
// �ǂݍ��ލۂ̃}�N���̒�`�ɂ��A���̈ꗗ����A�e�R�}���h�ɑ΂���R�[�h�𐶐������邱�Ƃ��ł���B

// S, F, V �͂��ꂼ��A���̃R�}���h�����ߌ`���A�֐��`���A�V�X�e���ϐ��`���Ŏg�p�ł��邱�Ƃ��Ӗ�����B
// ���ߌ`���Ɗ֐��`���̗����Ŏg�p�����R�}���h�́A������ ppResult ���󂯎�邪�A
// ���ߌ`���ŌĂ΂ꂽ�Ȃ� ppResult = nullptr �ƂȂ�A�Ԓl�����������B
// �֐��`���ƃV�X�e���ϐ��`���̗����Ŏg�p�����R�}���h�́A�ǉ��̈��� bSysvar ���󂯎��A���ꂪ�^�̂Ƃ��̓V�X�e���ϐ��`���ł���B

// S, F, V �̂�����ł��g���Ȃ��R�}���h�Ƃ́Abyref �̂悤�ȒP�Ȃ�L�[���[�h�ł���B
// �g�p��� cmd_call.(h|cpp) ���Q�Ƃ̂��ƁB

#ifdef _CmdlistModeProcess
# define HpiCmdlistBegin switch ( cmd ) {
# define HpiCmdlistEnd default: puterror(HSPERR_UNSUPPORTED_FUNCTION); }
# define HpiCmdlistSectionBegin(_Name) { using namespace _Name; //switch ( cmd ) {
# define HpiCmdlistSectionEnd } //}  switch
# define HpiCmd___(_Id, _Keyword) case Id::_Keyword: puterror(HSPERR_UNSUPPORTED_FUNCTION);
# if _CmdlistModeProcess == 'S'
#  define HpiCmdS__(_Id, _Keyword) case Id::_Keyword: _Keyword(); break;
#  define HpiCmdSF_ HpiCmdS__
#  define HpiCmdS_V HpiCmdS__
#  define HpiCmdSFV HpiCmdS__
#  define HpiCmd_F_ HpiCmd___
#  define HpiCmd__V HpiCmd___
#  define HpiCmd_FV HpiCmd___
# endif
# if _CmdlistModeProcess == 'F'
#  define HpiCmd_F_(_Id, _Keyword) case Id::_Keyword: return _Keyword(ppResult);
#  define HpiCmdSF_ HpiCmd_F_
#  define HpiCmd_FV HpiCmd_F_
#  define HpiCmdSFV HpiCmd_F_
#  define HpiCmdS__ HpiCmd___
#  define HpiCmd__V HpiCmd___
#  define HpiCmdS_V HpiCmd___
# endif
# if _CmdlistModeProcess == 'V'
#  define HpiCmd__V(_Id, _Keyword) case Id::_Keyword: return _Keyword(ppResult);
#  define HpiCmdS_V HpiCmd__V
#  define HpiCmd_FV HpiCmd__V
#  define HpiCmdSFV HpiCmd__V
#  define HpiCmdS__ HpiCmd___
#  define HpiCmd_F_ HpiCmd___
#  define HpiCmdSF_ HpiCmd___
# endif
# if !(_CmdlistModeProcess == 'S' || _CmdlistModeProcess == 'F' || _CmdlistModeProcess == 'V')
#  error "_CmdlistModeProcess must be 'S', 'F', or 'V'."
# endif
#else
# define HpiCmdlistBegin //
# define HpiCmdlistEnd //
# define HpiCmdlistSectionBegin(_Name) namespace _Name {
# define HpiCmdlistSectionEnd } // namespace
# define HpiCmdlistDefineId_(_Id, _Keyword) namespace Id { static int const _Keyword = _Id; }
# define HpiCmd___(_Id, _Keyword) HpiCmdlistDefineId_(_Id, _Keyword);
# define HpiCmdS__(_Id, _Keyword) HpiCmdlistDefineId_(_Id, _Keyword); extern void _Keyword(); 
# define HpiCmd_F_(_Id, _Keyword) HpiCmdlistDefineId_(_Id, _Keyword); extern int _Keyword(PDAT** ppResult);
# define HpiCmdSF_(_Id, _Keyword) HpiCmdlistDefineId_(_Id, _Keyword); extern int _Keyword(PDAT** ppResult = nullptr);
# define HpiCmd__V(_Id, _Keyword) HpiCmdlistDefineId_(_Id, _Keyword); extern int _Keyword(PDAT** ppResult = nullptr);
# define HpiCmdS_V(_Id, _Keyword) HpiCmdlistDefineId_(_Id, _Keyword); extern int _Keyword(PDAT** ppResult = nullptr);
# define HpiCmd_FV(_Id, _Keyword) HpiCmdlistDefineId_(_Id, _Keyword); extern int _Keyword(PDAT** ppResult = nullptr, bool bSysvar = false);
# define HpiCmdSFV(_Id, _Keyword) HpiCmdlistDefineId_(_Id, _Keyword); extern int _Keyword(PDAT** ppResult = nullptr, bool bSysvar = false);
#endif

#else

// remove all macroes when second load

#undef HpiCmdlistBegin
#undef HpiCmdlistEnd
#undef HpiCmdlistSectionBegin
#undef HpiCmdlistSectionEnd
#undef HpiCmdlistDefineId_
#undef HpiCmd___
#undef HpiCmdS__
#undef HpiCmd_F_
#undef HpiCmdSF_
#undef HpiCmd__V
#undef HpiCmdS_V
#undef HpiCmd_FV
#undef HpiCmdSFV

#undef IG_HPIMOD_CMDLIST_MACRO_H
#endif
