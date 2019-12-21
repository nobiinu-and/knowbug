// hsx が内部的に使用する型や関数を定義する。

#pragma once

#include "hsx.h"

// 危険な操作に印をつける。
#define UNSAFE(E) (E)

namespace hsx {
	static auto hsx_data_segment(HSPCTX const* ctx)->Slice<char> {
		auto size = (std::size_t)std::max(0, ctx->hsphed->max_ds) / sizeof(char);
		return Slice<char>{ ctx->mem_mds, size };
	}

	// データ領域の文字列への参照を取得する。
	// ds_index が領域外を参照していたら nullopt。
	static auto hsx_data_segment_to_str(std::size_t ds_index, HSPCTX const* ctx)->std::optional<char const*> {
		if (ds_index >= hsx_data_segment(ctx).size()) {
			return std::nullopt;
		}

		return hsx_data_segment(ctx).begin() + ds_index;
	}

	static auto hsx_exinfo(HSPCTX* ctx)->HSPEXINFO* {
		assert(ctx != nullptr);
		return ctx->exinfo2;
	}

	static auto hsx_exinfo(HSPCTX const* ctx)->HSPEXINFO const* {
		return hsx_exinfo(const_cast<HSPCTX*>(ctx));
	}

	static auto hsx_vartype_to_int(HspType vartype)->int {
		return (int)vartype;
	}

	static auto hsx_vartype_from_int(int vartype)->HspType {
		return (HspType)vartype;
	}

	static auto hsx_vartype_to_varproc(HspType vartype, HSPCTX const* ctx)->HspVarProc* {
		// FIXME: vartype の範囲検査 (ランタイム側は範囲検査しない)
		return hsx_exinfo(ctx)->HspFunc_getproc(hsx_vartype_to_int(vartype));
	}

	// support_flag は定数 HSPVAR_SUPPORT_* のいずれか。
	static auto hsx_varproc_does_support(HspVarProc const* varproc, unsigned short support_flag)->bool {
		assert(varproc != nullptr);
		return (varproc->support & support_flag) != 0;
	}

	// 変数の型の HspVarProc を取得する。
	static auto hsx_pval_to_varproc(PVal const* pval, HSPCTX const* ctx)->HspVarProc const* {
		return hsx_vartype_to_varproc(hsx_pval_to_type(pval), ctx);
	}

	// pdat は pval に含まれる要素の実体ポインタとする。
	static auto hsx_element_data_to_memory_block(PVal const* pval, PDAT const* pdat, HSPCTX const* ctx)->MemoryView {
		assert(pval != nullptr && pdat != nullptr);

		auto varproc = hsx_pval_to_varproc(pval, ctx);

		int buffer_size;
		auto data = varproc->GetBlockSize(const_cast<PVal*>(pval), const_cast<PDAT*>(pdat), &buffer_size);

		if (buffer_size <= 0 || data == nullptr) {
			return MemoryView{};
		}

		return MemoryView{ data, (std::size_t)buffer_size };
	}

	static auto hsx_data_from_int_opt(std::optional<HspInt const*> ptr_opt)->std::optional<HspData> {
		if (!ptr_opt) {
			return std::nullopt;
		}

		return std::make_optional(hsx_data_from_int(*ptr_opt));
	}
}
