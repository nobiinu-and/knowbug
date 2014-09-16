// Vector ���̃f�[�^�N���X

#include "CVector.h"
#include "PValRef.h"

#include "sub_vector.h"
#include "mod_makepval.h"

#include <algorithm>

using namespace hpimod;

#define MALLOC  hspmalloc
#define MEXPAND hspexpand
#define MFREE   hspfree

CVector* const CVector::Null      = (CVector*)HspFalse;
CVector* const CVector::MagicNull = (CVector*)HspTrue;

//------------------------------------------------
// �\�z
//------------------------------------------------
CVector::CVector( PVal* pval )
	: ptr_( 0 )
	, cntRefed_( 0 )
	, cntTmp_( 0 )
{
	size_ = 0;
	capa_ = 0;

#ifdef DBGOUT_VECTOR_ADDREF_OR_RELEASE
	static int stt_counter = 0;
	id_ = stt_counter ++;
	dbgout("[%d] new", id_ );
#endif
}

CVector* CVector::New( PVal* pval )
{
	return new( MALLOC( sizeof(CVector) ) ) CVector( pval );
		//new CVector( pval )
}

CVector* CVector::NewTemp(PVal* pval)
{
	auto const&& self = CVector::New(pval);
	self->BecomeTmpObj();
	return self;
}

//------------------------------------------------
// ���
//------------------------------------------------
CVector::~CVector()
{
	Free();
}

//------------------------------------------------
// ��̏����̎���
//------------------------------------------------
void CVector::Free()
{
	// �����ϐ������ׂĉ������
	Clear();

	// vector ���������
	if ( ptr_ ) { MFREE(ptr_); ptr_ = nullptr; }	
	return;
}

void CVector::Delete( CVector* src )
{
	assert( src );
	src->~CVector();
	MFREE(src);
//	delete src;
}

//------------------------------------------------
// �w��v�f�� PVal* �𓾂�
// 
// @ �Ȃ���Ί���l�Œǉ�����B
//------------------------------------------------
PVal* CVector::At( size_t idx )
{
	size_t const len = Size();

	if ( IsValid(idx) ) {
		return AtUnsafe( idx );

	// �Ȃ� =>> ����l�Œǉ�
	} else {
		Alloc( idx + 1 );
		return AtLast();
	}
}

//------------------------------------------------
// ��r
//------------------------------------------------
int CVector::Compare( CVector const& src ) const
{
	size_t const len    = Size();
	size_t const lenRhs = src.Size();

	if ( len != lenRhs ) return (len < lenRhs ? -1 : 1);

	for ( size_t i = 0; i < len; ++ i ) {
		if ( AtUnsafe(i) != AtUnsafe(i) ) return -1;		// �������Ƃ肠�������̂����������Ƃɂ���
	}

	return 0;
}

int CVector::Compare( CVector const* lhs, CVector const* rhs )
{
	bool const bNullLhs = isNull(lhs);
	bool const bNullRhs = isNull(rhs);

	if ( bNullLhs ) {
		return (bNullRhs ? 0 : -1);
	} else if ( bNullRhs ) {
		return (bNullLhs ? 0 :  1);
	} else {
		return lhs->Compare( *rhs );
	}
}

//##########################################################
//        �������Ǘ�
//##########################################################
//------------------------------------------------
// �w����̗v�f���m�ۂ���
//------------------------------------------------
void CVector::AllocImpl( size_t const newSize, bool bInit )
{
	if ( newSize > size_ ) {
		size_t const oldSize = size_;

		Reserve( newSize );
		SetSizeImpl( newSize );

		// �g��������������
		if ( bInit ) {
			for ( size_t i = oldSize; i < newSize; ++ i ) {
				AtUnsafe(i) = NewElem();
			}
		}
#ifdef _DEBUG
		else {
			for ( size_t i = oldSize; i < newSize; ++i ) {
				AtUnsafe(i) = AsPVal(PValRef::MagicNull);
			}
		}
#endif
	}

	return;
}

//------------------------------------------------
// �o�b�t�@���g������
//------------------------------------------------
void CVector::ReserveImpl( size_t const exSize )
{
	capa_ += exSize;

	if ( !ptr_ ) {
		capa_ = std::max( exSize, 64 / sizeof(PVal*) );		// hspmalloc �͍ŏ��ł� 64 �o�C�g�Ƃ�̂�
		ptr_ = reinterpret_cast<decltype(ptr_)>( MALLOC( capa_ * sizeof(PVal*) ));
	} else {
		ptr_ = reinterpret_cast<decltype(ptr_)>( MEXPAND((char*)ptr_, capa_ * sizeof(PVal*)) );
	}
	return;
}

//------------------------------------------------
// size_ �̒l��ύX����
//------------------------------------------------
void CVector::SetSizeImpl( size_t const newSize )
{
	size_ = newSize;
//	if ( mpvOwn ) mpvOwn->len[1] = newSize;
	return;
}

//------------------------------------------------
// �v�f�𐶐��E�폜����
//------------------------------------------------
PVal* CVector::NewElem( int vflag )
{
	PValRef* const pval = PValRef::New(vflag);
	PValRef::AddRef( pval );
	return AsPVal( pval );
}

void  CVector::DeleteElem( PVal* pval )
{
	if ( !pval ) return;
	PValRef::Release( AsPValRef(pval) );
	return;
}

//##########################################################
//        �R���e�i����
//##########################################################
//------------------------------------------------
// �S����
//------------------------------------------------
void CVector::Clear()
{
	for ( size_t i = 0; i < Size(); ++ i ) {
		DeleteElem( AtUnsafe(i) );
	}

	SetSizeImpl( 0 );
	return;
}

//------------------------------------------------
// �A��
// 
// @ src �����v�f [iBgn, iEnd) �� this �ɂ��ǉ�����B
// @prm bCopyElem: PVal �𕡐����邩�ۂ�
//------------------------------------------------
void CVector::ChainImpl( CVector const& src, size_t iBgn, size_t iEnd, bool bCopyElem )
{
	assert( src.IsValid( iBgn, iEnd ) );

	size_t const offset    = Size();
	size_t const lenAppend = iEnd - iBgn;

	// �o�b�t�@�g�� (�g�����������������Ȃ�)
	ExpandImpl( lenAppend, false );

	// �E�ӂ̎����ׂĂ̗v�f��ǉ�����
	for ( size_t i = 0; i < lenAppend; ++ i ) {
		auto& pvDst =     AtUnsafe(offset + i);
		auto& pvSrc = src.AtUnsafe(iBgn + i);

		// ��������ꍇ
		if ( bCopyElem ) {
			pvDst = NewElem();
		//	memset( pvDst, 0, sizeof(PVal) );
			PVal_copy( pvDst, pvSrc );

		// �Q�Ƌ��L�݂̂̏ꍇ
		} else {
			pvDst = PValRef::AddRef( pvSrc );
		}
	}
	return;
}

//------------------------------------------------
// �}��
//------------------------------------------------
PVal* CVector::Insert( size_t idx )
{
	InsertImpl( idx, idx + 1, true );
	return AtUnsafe(idx);
}

void CVector::InsertImpl( size_t iBgn, size_t iEnd, bool bInit )
{
	// �����g��
	if ( !IsValid(iBgn) ) {
		AllocImpl( iEnd, bInit );

	// �X�y�[�X���󂯂�
	} else {
		ExpandImpl( iEnd - iBgn, false );			// �g���L���� (���������Ȃ�)

		// �}�������ꏊ���󂯂�
		MemMove( iEnd, iBgn, (Size() - iBgn) );		// ����ɂ��炷

		// ����������
		if ( bInit ) {
			for ( size_t i = iBgn; i < iEnd; ++ i ) {
				AtUnsafe(i) = NewElem();
			}
		}
	}
	return;
}

//------------------------------------------------
// ����
//------------------------------------------------
void CVector::Remove( size_t idx )
{
	RemoveImpl( idx, idx + 1 );

	// [idx] ���l�߂� (�����I����)
	MemMove( idx, idx + 1, (Size() - (idx + 1)) );
	SetSizeImpl( size_ - 1 );	// �v�f������

	return;
}

void CVector::Remove( size_t iBgn, size_t iEnd )
{
	assert( IsValid( iBgn, iEnd ) );

	RemoveImpl( iBgn, iEnd );

	// [iBgn, iEnd) ���l�߂� (�����I����)
	MemMove( iBgn, iEnd, (Size() - iEnd) );
	SetSizeImpl( size_ - (iEnd - iBgn) );	// �v�f������
	return;
}

void CVector::RemoveImpl( size_t iBgn, size_t iEnd )
{
	// [iBgn, iEnd) ����� (�s��l�ɂȂ�̂Œ���)
	for ( size_t i = iBgn; i < iEnd; ++ i ) {
		DeleteElem( AtUnsafe(i) );
	}
	return;
}

//------------------------------------------------
// �u��
//------------------------------------------------
void CVector::Replace( size_t iBgn, size_t iEnd, CVector const* src )
{
	assert( IsValid(iBgn, iEnd) );

	size_t const cntElems = (src ? src->Size() : 0);

	ReplaceImpl( iBgn, iEnd, cntElems );
	
	// �Q�ƕ���
	for ( size_t i = 0; i < cntElems; ++ i ) {
		assert(!!src);

		// lhs �� ReplaceImpl �ɂĉ���ς݁A�s��l
		AtUnsafe(iBgn + i) = PValRef::AddRef(src->AtUnsafe(i));
	}
	return;
}

void CVector::ReplaceImpl( size_t iBgn, size_t iEnd, size_t cntElems )
{
	size_t const lenRange = iEnd - iBgn;

	if ( lenRange < cntElems ) {
		// ��Ԃ̌��X�̗v�f���������� (�l�߂Ȃ�)
		RemoveImpl( iBgn, iEnd );

		// ��ԂɎ��܂�Ȃ����̋�Ԃ��m�ۂ��� (���������Ȃ�)
		InsertImpl( iEnd, iBgn + cntElems, false );

	} else if ( lenRange > cntElems ) {
		size_t const lenShrink = lenRange - cntElems;
		RemoveImpl(iBgn, iEnd - lenShrink);

		// ��Ԃ̕����傫���Ȃ�A�k�߂Ă���
		Remove(iEnd - lenShrink, iEnd);
	}
	return;
}

//##########################################################
//        ���ԑ���
//##########################################################
// �ړ�
void CVector::Move( size_t iDst, size_t iSrc )
{
	if ( Size() < 2 || iDst == iSrc ) return;
	if ( !(IsValid(iDst) && IsValid(iSrc)) ) throw std::bad_exception("Invalid vector-index(es)");

	// �ړ����̒l��ۑ�����
	PVal* const tmp = AtUnsafe(iSrc);

	// �ړ�����
	if ( iSrc < iDst ) {
		MemMove( iSrc, iSrc + 1, (iDst - iSrc) );	// �O �� ��
	} else {
		MemMove( iDst + 1, iDst, (iSrc - iDst) );	// �O �� ��
	}

	AtUnsafe(iDst) = tmp;
	return;
}

// ����
void CVector::Swap( size_t idx1, size_t idx2 )
{
	if ( Size() < 2 || idx1 == idx2 ) return;
	if ( !(IsValid(idx1) && IsValid(idx2)) ) throw std::bad_exception("Invalid vector-index(es)");

	std::swap( AtUnsafe(idx1), AtUnsafe(idx2) );
	return;
}

// ����
void CVector::Rotate( int step_ )
{
	size_t const len = Size();
	size_t const step = ((step_ % len) + len) % len;	// [0, len) �ɔ[�܂��]
	if ( len < 2 || step == 0 ) return;

	std::rotate( begin(), (begin() + step), end() );
	return;
}

// ���]
void CVector::Reverse()
{
	if ( Size() < 2 ) return;
	std::reverse( begin(), end() );		// �S���
	return;
}

void CVector::Reverse( size_t iBgn, size_t iEnd )
{
	if ( Size() < 2 || iBgn == iEnd ) return;

	if ( iBgn > iEnd ) {
		if ( iEnd < 2 ) throw std::bad_exception("Invalid vector-reverse interval");
		return Reverse( iEnd + 1, iBgn + 1 );			// assert( iBgn < iEnd ), assert( 0 < iEnd )
	}
	if ( IsValid(iEnd - 1) ) throw std::bad_exception("Invalid vector-reverse interval");

	std::reverse( (begin() + iBgn), (begin() + iEnd) );	// ��� [iBgn, iEnd)
	return;
}
