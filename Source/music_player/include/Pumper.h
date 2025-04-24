#pragma once

#include <memory>
#include <vector>
#include <stack>
#include <functional>
#include "api_def.h"
#include "MusicInfo.h"
#include "MusicPlayer.h"


namespace fv {
	class EQD_MP_API Pumper
	{
	public:
		enum PUMP_MODE
		{
			NONE = 0,
			LOOP,
			RAND
		};

		Pumper( MusicPlayer &player);
		~Pumper();

		void pump();
		PUMP_MODE getMode();
		void setMode(PUMP_MODE mo);
		int getIndex();
		void setIndex(int ind);
		bool getPumpDir();
		void setPumpDir(bool pump_dir);
		void setFillMusicFunc(std::function<void(const std::shared_ptr<std::vector<eqd_mp::MusicInfo>>&)> f);
		void setNextMusic(const eqd_mp::MusicInfo *nm);
		bool setNextMusic(const wchar_t* name);
        bool hasNextMusic() const;
		
		void init(const char* root_dir);
		void init(const wchar_t* root_dir);
		std::shared_ptr<std::vector<eqd_mp::MusicInfo>> pop();
		void onclick(int idx);

		std::function<void(const eqd_mp::MusicInfo&)> onAutoPlay;

	private:
		void rand();
		void loop();
		void none();
		void cleanup();

		template<size_t N>
		void all_templet();

		std::function<void(const std::shared_ptr<std::vector<eqd_mp::MusicInfo>>&)> fill_music_func;

		PUMP_MODE m_mode;
		std::stack<std::shared_ptr<std::vector<eqd_mp::MusicInfo>>> m_root;
		MusicPlayer &m_player;
		int m_index;
		bool pump_dir;
		std::shared_ptr<eqd_mp::MusicInfo> next_music;
	};
}
