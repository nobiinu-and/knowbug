// hsp code

#ifndef IG_CLASS_HSP_CODE_H
#define IG_CLASS_HSP_CODE_H

#include <vector>
#include "hsp3plugin_custom.h"

class CHspCode
{
	using label_t = hpimod::label_t;
	using code_t = unsigned short;

	using dslist_t = std::vector<char*>;

public:
	CHspCode()
		: mBufSize( 64 )
		, mpDs   ( nullptr )
		, mDsSize( 0 )
		, mDsUsed( 0 )
		, mDslist()
		, mLen  ( 0 )
	//	, exflg( 0 )
	{
		mpCode = (code_t*)hspmalloc( mBufSize * sizeof(code_t) );
		needDsBuf( 32 / 2 );		// �ŏ� 32 �͊m�ۂ��Ă���
		return;
	}

	~CHspCode()
	{
		hspfree( mpCode ); mpCode = nullptr;
		hspfree( mpDs   ); mpDs   = nullptr;	// �g�p���� DS �� mDslist �Ɋ܂܂Ȃ��̂�

		// �g�p�ς� DS ���X�g��j��
		for each ( auto& it in mDslist ) { hspfree( it ); }

		mDslist.clear();
		return;
	}

	// �擾�n
	label_t getlb   () const { return mpCode; }
	label_t getlbNow() const { return &mpCode[mLen]; }

	// �ǉ��n
	void put( int _type, int _code, int _exflg )
	{
		needCsBuf( 8 );		// �ő� 32 [bit] = 8 [byte] �v����

		int const lead = ((_type & CSTYPE) | _exflg);

		// 16 bit
		if ( static_cast<unsigned int>(_code) < 0x10000 ) {
			mpCode[mLen ++] = lead;
			mpCode[mLen ++] = _code;

		// 32 bit
		} else {
			mpCode[mLen ++] = ( lead | 0x8000 );
			*reinterpret_cast<int*>(&mpCode[mLen]) = _code;
			mLen += 2;
		}
		return;
	}

	void putVal( label_t const v, int exflg = 0 ) { put( TYPE_LABEL,  (int)v,      exflg ); }
	void putVal( int const     v, int exflg = 0 ) { put( TYPE_INUM,   v,           exflg ); }
	void putVal( char const*   v, int exflg = 0 ) { put( TYPE_STRING, putDsVal(v), exflg ); }
	void putVal( const double  v, int exflg = 0 ) { put( TYPE_DNUM,   putDsVal(v), exflg ); }
	void putVar( PVal const*   v, int exflg = 0 ) {
		// !! v �� ctx->mem_var �̔{���łȂ�������s����
		put( TYPE_VAR,    v - ctx->mem_var, exflg );
	}

	void putOmt() { put( TYPE_MARK, '?', EXFLG_2 ); }

	void putReturn() { put( TYPE_PROGCMD, 0x002, EXFLG_1 ); }

private:
	// CS �o�b�t�@�̊m�ۗv��
	void needCsBuf( size_t sizeMin )
	{
		if ( (mBufSize - mLen) <= sizeMin ) {
			mBufSize = (mBufSize * 2) + sizeMin;
			mpCode = (code_t*)hspexpand( (char*)mpCode, mBufSize );
		}
		return;
	}

	// DS �o�b�t�@�̊m�ۗv��
	void needDsBuf( size_t sizeMin )
	{
		if ( (mDsSize - mDsUsed) <= sizeMin ) {
			if ( mpDs ) mDslist.push_back( mpDs );	// �g�p�ς� DS ���X�g�ɓo�^
			mDsSize = (mDsSize + sizeMin) * 2;
			mDsUsed = 0;
			mpDs    = (char*)hspmalloc( mDsSize * sizeof(char) );
		}
		return;
	}

	char* getDsNext() const { return &mpDs[mDsUsed]; }

	char* putDs( void const* data, size_t size )
	{
		needDsBuf( size );

		char* pDst = &mpDs[mDsUsed];
		memcpy( pDst, data, size );
		mDsUsed += size;

		return pDst;
	}

	int putDsVal( char const*  s ) { return putDs(  s, strlen(s) + 1  ) - ctx->mem_mds; }	// ���̒l x �� ( mds[x] == pDst ) �𖞂���
	int putDsVal( const double d ) { return putDs( &d, sizeof(double) ) - ctx->mem_mds; }

public:
	int putDsStPrm( short subid, short mptype, int offsetOfPrmstk )
	{
		STRUCTPRM stprm;//  = { mptype, subid, offsetOfPrmstk };
			stprm.mptype = mptype;
			stprm.subid = subid;
			stprm.offset = offsetOfPrmstk;

		// getDsNext() �� ctx->mem_minfo �̍��� sizeof(STRUCTPRM) �̔{���ɂȂ�悤�ɂ���
		{
			int const d = (char*)ctx->mem_minfo - getDsNext();
			int const r = (d) % sizeof(STRUCTPRM);	// ���̗]�� (��Βl�ŏ���])
			putDs( &stprm, abs(r) );
		//	dbgout("size = %d, dif0 = %d, r = %d, dif1 = %d", sizeof STRUCTPRM, d, r, ((char*)ctx->mem_minfo - getDsNext()) );
		}

		stprm_t const pStPrm = reinterpret_cast<stprm_t>( putDs(&stprm, sizeof(STRUCTPRM)) );
		return pStPrm - ctx->mem_minfo;		// &minfo[val] = pStPrm �ƂȂ� val ���v�Z
	}

private:
	code_t* mpCode;
	size_t mLen;
	size_t mBufSize;

	char*  mpDs;
	size_t mDsSize;
	size_t mDsUsed;
	dslist_t mDslist;	// mpDs �̃��X�g

//	int exflg;

};

#endif

// �]�����
#if 0
{
	// (*type == TYPE_STRUCT)
	// �\���̃p�����[�^�����ۂɎw���Ă�����̂��R�[�h�ɒǉ�����
	auto const pStPrm = &ctx->mem_minfo[ *val ];			
	char* out = (char*)ctx->prmstack;
	if ( !out ) puterror( HSPERR_ILLEGAL_FUNCTION );
	if ( pStPrm->subid != STRUCTPRM_SUBID_STACK ) {		// �����o�ϐ�
		auto thismod = (MPModVarData*)out;
		out = (char*)((FlexValue*)PVal_getptr( thismod->pval, thismod->aptr ))->ptr;
	}
	;
	// ������W�J
	([&body]( char* ptr, int mptype ) {
		switch ( mptype ) {
			case MPTYPE_SINGLEVAR:		// �ϐ��v�f => �ϐ�(aptr)�̌`�ŏo��
			{
				auto vardata = reinterpret_cast<MPVarData*>(ptr);
				auto pval    = vardata->pval;
				body.putVar( pval );
				;
				if ( vardata->aptr != 0 ) {
					body.put( TYPE_MARK, '(', 0 );
					if ( pval->len[2] == 0 ) {
						// �ꎟ��
						body.putVal( vardata->aptr );
					} else {
						// ������ => APTR����
						int idx[4] = {0};
						GetIndexFromAptr( pval, vardata->aptr, idx );
						for ( int i = 0; i < 4 && idx[i] != 0; ++ i ) {
							body.putVal( idx[i], (i == 0 ? 0 : EXFLG_2) );
						}
					}
					body.put( TYPE_MARK, ')', 0 );
				}
				break;
			}
			case MPTYPE_ARRAYVAR:
				return body.putVar( reinterpret_cast<MPVarData*>(ptr)->pval );
			case MPTYPE_LOCALVAR:
				return body.putVar( reinterpret_cast<PVal*>(ptr) );
			;
			case MPTYPE_LABEL:       return body.putVal( *(label_t*)ptr );
			case MPTYPE_LOCALSTRING: return body.putVal( *(char**)ptr );
			case MPTYPE_DNUM: return body.putVal( *reinterpret_cast<double*>(ptr) );
			case MPTYPE_INUM: return body.putVal( *reinterpret_cast<int*>(ptr) );
			;
			default:
				dbgout("mptype = %d", mptype );
				break;
		};
	})( out + pStPrm->offset, pStPrm->mptype );
	;
	code_next();
}

#endif

