// �O���錾�̂�

#ifndef IG_CLASS_DATA_TREE_NODE_DEC_H
#define IG_CLASS_DATA_TREE_NODE_DEC_H

namespace DataTree
{

class ITree;
class IMonoNode;
class IPolyNode;
class ILeaf;

typedef ITree* tree_t;
//typedef ITree const* ctree_t;

class CLoopNode;

// ��̓I�ȃm�[�h
class CNodeModule;
class CNodeGlobal;
//class CNodeSysvarList;
//class CNodeCallList;

class CNodeVarHolder;
class CNodeVarElem;
class CNodeVarArray;
//class CNodeFuncValue;
//class CNodeCall;
//class CNodeResult;

//class CNodeValue;
class CNodeLabel;
class CNodeString;
class CNodeDouble;
class CNodeInt;
class CNodeModInst;
class CNodeModInstNull;
//class CNodeComobj;
//class CNodeVariant;
class CNodeUnknown;

class CNodePrmStk;
class CNodeSimpleStr;

}

#endif
