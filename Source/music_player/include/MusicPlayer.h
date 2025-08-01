#pragma once
#include <bass.h>
#include "api_def.h"
#include "MusicInfo.h"

namespace fv {
	class EQD_MP_API MusicPlayer
	{

	public:
		MusicPlayer(bool enable3d = false);
		~MusicPlayer();

		int playStream(const eqd_mp::MusicInfo& file, bool loop = false);
		void play(bool loop = false);
		void pause();
		void stop();
		size_t getData(void *p, size_t size);
		DWORD getLevel();
		QWORD getPosition(int flag);
		void setPosition(QWORD pos, int flag);
		double getSeconds();
		int getActive();
		void cleanup();
		QWORD getLength();
		bool isOff();
		bool is_playing();


		HSAMPLE getChan();
		bool isEnable3D();
		bool isSupport3D();

		int get_init_err_code();

	protected:
		HSTREAM chan = 0;
		QWORD chan_max_len = 0;
		int init_error_code = 0;
		bool IsEnable3D = false;
		bool IsSupport3D = false;
	};

}
