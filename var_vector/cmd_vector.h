// vector - Command header

#ifndef IG_VECTOR_COMMAND_H
#define IG_VECTOR_COMMAND_H

#include "hsp3plugin_custom.h"
using namespace hpimod;

#include "vt_vector.h"

extern vector_t code_get_vector();

// �R�}���h�p�֐��̃v���g�^�C�v�錾
extern void VectorDelete();				// �j��

extern int VectorMake( void** ppResult );			// as literal
extern int VectorSlice( void** ppResult );
extern int VectorSliceOut( void** ppResult );
extern int VectorDup( void** ppResult );

extern int VectorIsNull( void** ppResult );
extern int VectorVarinfo( void** ppResult );
extern int VectorSize( void** ppResult );

extern void VectorDimtype();
extern void VectorClone();

extern void VectorChain(bool bClear);	// �A�� (or ����)
#if 0
extern void VectorMoving( int cmd );	// �v�f��������n
extern int  VectorMovingFunc( void** ppResult, int cmd );

extern void VectorInsert();				// �v�f�ǉ�
extern void VectorInsert1();
extern void VectorPushFront();
extern void VectorPushBack();
extern void VectorRemove();				// �v�f�폜
extern void VectorRemove1();
extern void VectorPopFront();
extern void VectorPopBack();
extern void VectorReplace();
extern int VectorInsert( void** ppResult ) ;
extern int VectorInsert1( void** ppResult ) ;
extern int VectorPushFront( void** ppResult );
extern int VectorPushBack( void** ppResult );
extern int VectorRemove( void** ppResult );
extern int VectorRemove1( void** ppResult );
extern int VectorPopFront( void** ppResult );
extern int VectorPopBack( void** ppResult );
extern int VectorReplace( void** ppResult );
#endif

extern int VectorResult( void** ppResult );
extern int VectorExpr( void** ppResult );
extern int VectorJoin( void** ppResult );
extern int VectorAt( void** ppResult );

// �I����
extern void VectorCmdTerminate();

// �V�X�e���ϐ�

// �萔
namespace VectorCmdId {
	int const
		Move    = 0x20,
		Swap    = 0x21,
		Rotate  = 0x22,
		Reverse = 0x23;
};

#endif
