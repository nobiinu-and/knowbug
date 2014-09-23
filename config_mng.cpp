
#include "module/CIni.h"
#include "module/strf.h"

#include "config_mng.h"

std::unique_ptr<KnowbugConfig> KnowbugConfig::instance_;
KnowbugConfig::Mng g_config;

KnowbugConfig::KnowbugConfig()
{
	char ownpath[MAX_PATH];
	{
		GetModuleFileName( GetModuleHandle(nullptr), ownpath, MAX_PATH );
		
		char drive[5];
		char dir[MAX_PATH];
		char _dummy[MAX_PATH];		// �_�~�[
		
		_splitpath_s( ownpath, drive, dir, _dummy, _dummy );
		sprintf_s( ownpath, "%s%s", drive, dir );
	}
	
	string const ownpath_full = strf("%sknowbug.ini", ownpath);
	CIni ini { ownpath_full.c_str() };
	
	// common �t�H���_�ւ̃p�X
	commonPath = strf( "%scommon", ownpath );
	
	// �ő�\���f�[�^��
	//maxlenVarinfo = ini.getInt( "Varinfo", "maxlen", 0x1000 - 1 );
	logMaxlen     = ini.getInt( "Log",     "maxlen", 0x20000 );
	
	// �^�u������
	tabwidth  = ini.getInt( "Interface", "tabwidth", 3 );
	
	// �E�[�Ő܂�Ԃ����ۂ�
	//bWordwrap = ini.getBool( "Interface", "bWordwrap", false );
	
	// �őO�ʃE�B���h�E���ۂ�
	bTopMost = ini.getBool( "Window", "bTopMost", false );
	
	// �����ۑ��p�X
	logPath = ini.getString( "Log", "autoSavePath", "" );
	
	// �Ԓl�m�[�h���g����
	bResultNode = ini.getBool( "Varinfo", "useResultNode", false );

	// �J�X�^���h���[���邩�ǂ���
	bCustomDraw = ini.getBool( "ColorType", "bCustomDraw", false );
	
	if ( bCustomDraw ) {
		// �^���Ƃ̐F
		for ( int i = 0; i < HSPVAR_FLAG_USERDEF; ++i ) {
			clrText[i] = ini.getInt("ColorType", strf("text#%d", i).c_str(), RGB(0, 0, 0));
		}

		// �Ăяo����g���^�̐F
		auto const keys = ini.enumKeys("ColorTypeExtra");
		for ( auto const& key : keys ) {
			COLORREF const cref = ini.getInt("ColorTypeExtra", key.c_str());
			//dbgout("extracolor: %s = (%d, %d, %d)", key.c_str(), GetRValue(cref), GetGValue(cref), GetBValue(cref) );
			clrTextExtra.insert({ key, cref });
		}
	}

	// �g���^�̕ϐ��f�[�^�𕶎��񉻂���֐�
	auto keys = ini.enumKeys("VardataString/UserdefTypes");
	//dbgout(join(keys.begin(), keys.end(), ", ").c_str());

	for ( auto const& vtname : keys ) {
		auto const dllPath = ini.getString("VardataString/UserdefTypes", vtname.c_str());
		if ( auto const hDll = LoadLibrary(dllPath) ) {
			static char const* const stc_sec = "VardataString/UserdefTypes/Func";

			auto const fnameAddVar = ini.getString(stc_sec, strf("%s.addVar", vtname.c_str()).c_str());
			auto const fAddVar = (addVarUserdef_t)GetProcAddress(hDll, fnameAddVar);
			if ( fnameAddVar[0] != '\0' && !fAddVar ) {
				Knowbug::logmesWarning(strf("�g���^�\���p�� addVar �֐����ǂݍ��܂�Ȃ������B\r\n�^���F%s, �֐����F%s",
					vtname.c_str(), fnameAddVar).c_str());
			}

			auto const fnameAddValue = ini.getString(stc_sec, strf("%s.addValue", vtname.c_str()).c_str());
			auto const fAddValue = (addValueUserdef_t)GetProcAddress(hDll, fnameAddValue);
			if ( fnameAddValue[0] != '\0' && !fAddValue ) {
				Knowbug::logmesWarning(strf("�g���^�\���p�� addValue �֐����ǂݍ��܂�Ȃ������B\r\n�^���F%s, �֐����F%s",
					vtname.c_str(), fnameAddValue).c_str());
			}

#ifdef _DEBUG
			Knowbug::logmes(strf("�^ %s �̊g���\����񂪓ǂݍ��܂ꂽ�B\r\nVswInfo { %08X, %08X, %08X }",
				vtname.c_str(), hDll, fAddVar, fAddValue).c_str());
#endif
			vswInfo.insert({ vtname, VswInfo { hDll, fAddVar, fAddValue } });
		} else {
			Knowbug::logmesWarning(strf("�g���^�\���p�� Dll �̓ǂݍ��݂Ɏ��s�����B\r\n�^���F%s, �p�X�F%s",
				vtname.c_str(), dllPath).c_str());
		}
	}

	// �ٍ�v���O�C���g���^�\�����Ȃ���Βǉ����Ă���
	struct VswInfoForInternal { string vtname; addVarUserdef_t addVar; addValueUserdef_t addValue; };
	static VswInfoForInternal const stc_vswInfoForInternal[] = {
#ifdef with_Assoc
		{ "assoc_k", nullptr, knowbugVsw_addValueAssoc },
#endif
#ifdef with_Vector
		{ "vector_k", knowbugVsw_addVarVector, knowbugVsw_addValueVector },
#endif
#ifdef with_Array
		{ "array_k", knowbugVsw_addVarArray, knowbugVsw_addValueArray },
#endif
#ifdef with_Modcmd
		{ "modcmd_k", nullptr, knowbugVsw_addValueModcmd },
#endif
	};
	for ( auto vsw2 : stc_vswInfoForInternal ) {
		// ini �t�@�C���̎w���D�悷��
		if ( vswInfo.find(vsw2.vtname) == vswInfo.end() ) {
			vswInfo.insert({ vsw2.vtname, VswInfo { nullptr, vsw2.addVar, vsw2.addValue } });
		}
	}
	
	return;
}

KnowbugConfig::~KnowbugConfig()
{
	for ( auto const& info : vswInfo ) {
		if ( info.second.hDll ) FreeLibrary(info.second.hDll);
	}
	return;
}
