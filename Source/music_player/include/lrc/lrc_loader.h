#pragma once

#include "comm.h"
#include <optional>
#include <map>
#include <memory>

namespace lrc {

	

	struct LrcLoader
	{
		using LrcType = Lrc;
	    std::shared_ptr<LrcType> loadFromText(const std::string& key,const std::string& text);
		std::shared_ptr<LrcType> load(const std::string& path);
		std::map<std::string, std::shared_ptr<LrcType>> map;
	};


}