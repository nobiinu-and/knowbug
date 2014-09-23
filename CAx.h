// ax structure analyzed

// dinfo ����͂��ă��x�����A�p�����[�^�������W����

#ifndef IG_CLASS_AX_H
#define IG_CLASS_AX_H

#include <memory>
#include <map>
#include <algorithm>

//#include "main.h"

#include "hsp3plugin.h"
#include "hpimod/basis.h"

static unsigned short wpeek(unsigned char const* p) { return *reinterpret_cast<unsigned short const*>(p); }
static unsigned int tripeek(unsigned char const* p) { return (p[0] | p[1] << 8 | p[2] << 16); }

namespace hpimod {
class CAx
{
private:
	using identTable_t = std::map<int, char const*>;
	using csMap_t = std::map<std::pair<char const*, int>, csptr_t>;

private:
	identTable_t labelNames;
	identTable_t prmNames;
	
	// �t�@�C����, �s�ԍ����� cs �ʒu����肷�邽�߂̂��� (���݂͖�����)
	// �������t�@�C������ di �̊Y���ʒu�ւ̃|�C���^�Ɍ���
	csMap_t csMap;

public:
	CAx() {
		analyzeDInfo();
	}

private:
	// �e�[�u�����环�ʎq������ (failure: nullptr)
	static char const* getIdentName(identTable_t const& table, int iparam) {
		auto const iter = table.find(iparam);
		return (iter != table.end()) ? iter->second : nullptr;
	}

public:
	char const* getLabelName(int otindex) const { return getIdentName(labelNames, otindex); }
	char const* getPrmName(int stprmidx) const { return getIdentName(prmNames, stprmidx); }

private:
	enum DInfoCtx {	// ++ �������̂� enum class �ɂ��Ȃ�
		DInfoCtx_Default = 0,
		DInfoCtx_LabelNames,
		DInfoCtx_PrmNames,
		DInfoCtx_Max
	};

	identTable_t* getIdentTableFromCtx(int dictx) {
		switch ( dictx ) {
			case DInfoCtx_Default:    return nullptr;	// �ϐ����͋L�^���Ȃ�
			case DInfoCtx_LabelNames: return &labelNames;
			case DInfoCtx_PrmNames:   return &prmNames;
			default: throw;
		}
	}

	// dinfo ���
	void analyzeDInfo() {
		/*
		csptr_t cur_cs = ctx->mem_mcs;
		char const* cur_fname;
		int cur_line;

		auto const push_point = [this, &cur_fname, &cur_line, &cur_cs]() {
			csMap.insert({ { cur_fname, cur_line }, cur_cs });
		};
		//*/

		int dictx = DInfoCtx_Default;

		for ( int i = 0; i < ctx->hsphed->max_dinfo; ) {
			switch ( ctx->mem_di[i] ) {
				case 0xFF:
					++dictx;	// enum �� ++ �����
					if ( dictx == DInfoCtx_Max ) {
						assert(i + 2 == ctx->hsphed->max_dinfo);
						return;
					}
					++i;
					break;

				// �\�[�X�t�@�C���w��
				case 0xFE:
				{/*
					int const idxDs = tripeek(&ctx->mem_di[i + 1]);
					int const line = wpeek(&ctx->mem_di[i + 4]);

					if (idxDs != 0) { cur_fname = &ctx->mem_mds[idxDs]; }
					cur_line = line;
					//*/
					i += 6;
					break;
				}
				// ���ʎq�w��
				case 0xFD:
				case 0xFB:
					if ( auto const tbl = getIdentTableFromCtx(dictx) ) {
						auto const ident = &ctx->mem_mds[tripeek(&ctx->mem_di[i + 1])];
						int const iparam = wpeek(&ctx->mem_di[i + 4]);
						tbl->insert({ iparam, ident });
					}
					i += 6;
					break;
				// ���̖��߂܂ł�CS�I�t�Z�b�g�l
				case 0xFC:
					//cur_cs += *reinterpret_cast<csptr_t>(&ctx->mem_di[i + 1]);
					//push_point();
					i += 3;
					break;
				default:
					//cur_cs += ctx->mem_di[i];
					//push_point();
					++i;
					break;
			}
		}
		return;
	}
};
	
} // namespace hpimod

#endif
