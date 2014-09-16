// PValRef

#ifndef IG_PVALREF_H
#define IG_PVALREF_H

#include "hsp3plugin_custom.h"

#ifdef _DEBUG
# define PVALREF_DBGCODE(...) //__VA_ARGS__
#else
# define PVALREF_DBGCODE(...) //
#endif

namespace hpimod
{

//------------------------------------------------
// (PVal*) with �Q�ƃJ�E���^
//------------------------------------------------
struct PValRef
{
	PVal pval;
private:
	int cntRefed;			// �Q�ƃJ�E���^ (�A�N�ւ���ĂȂ��̂Œ��ӂ��邱��)
	int opt1, opt2;			// �\��

	PVALREF_DBGCODE(
		static int stt_counter;
		int id;
	)
public:
	static PValRef* const MagicNull;

	//-----------------------------------------------
	// �֐�
	//-----------------------------------------------
	static PValRef* New( int vflag = HSPVAR_FLAG_INT );
	static void Delete( PValRef* pval );

	static void AddRef( PValRef* pval );
	static void Release( PValRef* pval );
	static PVal* AddRef( PVal* pval ) { PValRef::AddRef( (PValRef*)(pval) ); return pval; }
};

static PVal*          AsPVal   (       PValRef* pval ) { return reinterpret_cast<PVal*         >( pval ); }
static PVal const*    AsPVal   ( PValRef const* pval ) { return reinterpret_cast<PVal const*   >( pval ); }
static PValRef*       AsPValRef(       PVal*    pval ) { return reinterpret_cast<PValRef*      >( pval ); }
static PValRef const* AsPValRef( PVal const*    pval ) { return reinterpret_cast<PValRef const*>( pval ); }

static bool IsNull( PValRef const* pval ) { return ( !pval || pval == PValRef::MagicNull ); }
static size_t Size( PValRef const* pval ) { return AsPVal(pval)->len[1]; }
static bool  Empty( PValRef const* pval ) { return AsPVal(pval)->len[1] == 0; }

static int Compare( PValRef const* lhs, PValRef const* rhs )
{
	// null?
	bool bNullLhs = IsNull(lhs);
	bool bNullRhs = IsNull(rhs);

	if ( bNullLhs || bNullRhs ) {
		return ( bNullLhs )
			? ( bNullRhs ? 0 : -1 )
			: 1;		// assert(IsNull(bNullRhs) == true)
	}

	// �v�f��

	return 0;
}

}	// namespace hpimod

#endif
