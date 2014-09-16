// ���������N���X

#ifndef IG_CLASS_PARAMETER_INFORMATION_H
#define IG_CLASS_PARAMETER_INFORMATION_H

/**
@summary:
	�����������Ǘ�����B
	�����Ăяo�����͒m��Ȃ��BCCall �ɏ����E�g�p�����B
	cmd_sub.cpp, CBound �Ȃǂ��A�Ăяo����Ƒ΂ɂ��Đ����E�Ǘ�����B
**/

#include <vector>
#include "cmd_call.h"

//------------------------------------------------
// ���������X�g�̓���^�C�v
// 
// @ HSPVAR_FLAG_* �ƕ��p�B
// @ _NONE �������A( PRM_TYPE_* < 0 )�B
//------------------------------------------------
int const PRM_TYPE_NONE   = ( 0);
int const PRM_TYPE_FLEX   = (-1);	// �ϒ�����
int const PRM_TYPE_VAR    = (-2);	// var   �w�� ( �Q�Ɠn���v�� )
int const PRM_TYPE_ARRAY  = (-3);	// array �w�� ( �Q�Ɠn���v�� )
int const PRM_TYPE_MODVAR = (-4);	// modvar�w�� ( �Q�Ɠn���v�� )
int const PRM_TYPE_ANY    = (-5);	// any   �w��
int const PRM_TYPE_LOCAL  = (-6);	// local �w�� ( �����s�v )

extern bool PrmType_IsRef(int prmtype);
extern int PrmType_Size(int prmtype);
extern int PrmType_FromMPType(int mptype);

//##############################################################################
//                �錾�� : CPrmInfo
//##############################################################################
class CPrmInfo
{
public:
	using prmlist_t = std::vector<int>;

	static CPrmInfo const undeclaredFunc;	// ���錾�֐��̉�����
	static CPrmInfo const noprmFunc;		// �����Ȃ��֐��̉�����

	//############################################
	//    �����o�ϐ�
	//############################################
private:
	size_t mcntPrms;		// �����̐�( �ŏ� )
	size_t mcntLocals;		// ���[�J���ϐ��̌�
	bool mbFlex;			// �ϒ�������
	prmlist_t mprmlist;		// prmtype �̔z��

	//############################################
	//    �����o�֐�
	//############################################
public:
	//--------------------------------------------
	// �\�z
	//--------------------------------------------
	explicit CPrmInfo( prmlist_t const* pPrmlist = nullptr, bool bFlex = false );
	CPrmInfo( CPrmInfo const& src ) { this->copy( src ); }

	~CPrmInfo() = default;

	//--------------------------------------------
	// �ݒ�n
	//--------------------------------------------
	void    setFlex(bool bFlex);
	void setPrmlist(prmlist_t const& prmlist);

	//--------------------------------------------
	// �擾�n
	//--------------------------------------------
	size_t cntPrms() const;
	size_t cntLocals() const;
	bool   isFlex() const;
	int  getPrmType( size_t idx ) const;
	int  getStackSize() const;							// prmstack �ŕK�v�ƂȂ�T�C�Y (�ϒ�����������)
	int  getStackSizeWithFlex( size_t cntFlex ) const;	// �V (�ϒ���������)

	//--------------------------------------------
	// ���̑�
	//--------------------------------------------
	PVal* getDefaultArg( size_t iArg ) const;
	void checkCorrectArg( PVal const* pvArg, size_t iArg, bool bByRef = false ) const;

	//--------------------------------------------
	// ���Z�q�I�[�o�[���[�h
	//--------------------------------------------
	CPrmInfo& operator = ( CPrmInfo const& src ) { return this->copy( src ); }

	int compare( CPrmInfo const& rhs ) const;
	bool operator == ( CPrmInfo const& rhs ) const { return compare( rhs ) == 0; }
	bool operator != ( CPrmInfo const& rhs ) const { return !( *this == rhs ); }

	//############################################
	//    ���������o�֐�
	//############################################
private:
	CPrmInfo& copy( CPrmInfo const& obj );

	// ���̑�
public:
	static CPrmInfo Create(hpimod::stdat_t);
};

#endif
