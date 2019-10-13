#pragma once

#include "hsp_objects.h"
#include "hsp_object_tree.h"

class SourceFileRepository;
class WcDebugger;

// FIXME: 名前がよくないので変える

// HSP 関連の操作をまとめるもの。
// HSP 側から取得できる情報を knowbug 用に加工したりキャッシュしたりする機能を持つ (予定)
class HspRuntime {
	HSP3DEBUG* debug_;
	std::unique_ptr<HspObjects> objects_;
	std::unique_ptr<HspObjectTree> object_tree_;

public:
	static auto create(HSP3DEBUG* debug, OsString&& common_path)->std::unique_ptr<HspRuntime>;

	HspRuntime(
		HSP3DEBUG* debug,
		std::unique_ptr<HspObjects> objects,
		std::unique_ptr<HspObjectTree> object_tree
	)
		: debug_(debug)
		, objects_(std::move(objects))
		, object_tree_(std::move(object_tree))
	{
	}

	auto objects() -> HspObjects& {
		return *objects_;
	}

	auto object_tree() -> HspObjectTree& {
		return *object_tree_;
	}

	void initialize() {
		objects().initialize();
	}

	void update_location();
};
