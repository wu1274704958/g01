#include "Pumper.h"
#include <ctime>
#include <stdio.h>
#include "infoProvider/DefInfoProvider.h"
#include "playlistHandler/LoopPlaylist.h"
#include "PlaylistMgr.hpp"
#include "fileLoader/DefFileLoader.h"
#include "fileLoader/DefFileLoaderWide.h"
#include "musicPlayer/DefMusicPlayer.h"
#include "infoProvider/DefInfoProvider.h"
#include "musicLoader/DefMusicLoader.h"
#include "musicLoader/FlacLoader.h"
#include "soundVisualizationMgr/DefSoundVisualizationMgr.h"

using namespace eqd_mp;

using T = PlaylistMgr<DefMusicPlayer<DefFileLoaderWide,DefMusicLoader,FlacLoader>,DefInfoProvider,PlaylistPair<EPumpMode::Loop,LoopPlaylist>>;     

fv::Pumper::Pumper( MusicPlayer &player) : m_player(player)
{
	m_mode = NONE;
	pump_dir = false;
	m_index = 0;
	srand((unsigned)time(NULL));
	next_music = nullptr;
}

void fv::Pumper::setNextMusic(const eqd_mp::MusicInfo *nm)
{
	//next_music = std::make_shared<eqd_mp::MusicInfo>(*nm);
}

bool fv::Pumper::setNextMusic(const wchar_t* name)
{
	auto current = this->m_root.top();
	for (int i = 0; i < current->size(); ++i)
	{
		auto& n = current->at(i).Name();
		if (n.find(name) != std::wstring::npos)
		{
			///next_music = std::make_shared<eqd_mp::MusicInfo>(current->at(i));
			return true;
		}
	}
	return false;
}

bool fv::Pumper::hasNextMusic() const
{
    return (bool)next_music;
}

fv::Pumper::~Pumper()
{
	cleanup();
}

void fv::Pumper::pump()
{
	switch (m_mode)
	{
	case NONE:
		none();
		break;
	case LOOP:
		loop();
		break;
	case RAND:
		rand();
		break;
	default:
		none();
		break;
	}
}

fv::Pumper::PUMP_MODE fv::Pumper::getMode()
{
	return m_mode;
}

void fv::Pumper::setMode(PUMP_MODE mo)
{
	m_mode = mo;
	if(mo == RAND)
		srand((unsigned)time(NULL));
}

int fv::Pumper::getIndex()
{
	return m_index;
}

void fv::Pumper::setIndex(int ind)
{
	m_index = ind;
}

bool fv::Pumper::getPumpDir()
{
	return pump_dir;
}

void fv::Pumper::setPumpDir(bool v)
{
	this->pump_dir = v;
}

void fv::Pumper::setFillMusicFunc(std::function<void(const std::shared_ptr<std::vector<eqd_mp::MusicInfo>>&)> f)
{
	fill_music_func = f;
}

template<size_t N>
void fv::Pumper::all_templet()
{
	if (next_music)
	{
		m_player.playStream(*next_music);
		if(onAutoPlay)
			onAutoPlay(*next_music);
		next_music.reset();
		return;
	}
	if constexpr(N == NONE)
	{
		return;
	}
	if (!m_root.top()->empty())
	{
		if constexpr(N == RAND)
		{
			m_index = ::rand() % m_root.top()->size();
		}
		else if constexpr(N == LOOP)
		{
			if (m_index >= m_root.top()->size())
			{
				m_index = 0;
			}
		}
		if (m_root.top()->at(m_index).Type() == eqd_mp::MusicInfo::EType::Album && pump_dir)
		{
			std::shared_ptr<std::vector<eqd_mp::MusicInfo>> temp = std::make_shared<std::vector<eqd_mp::MusicInfo>>();
			temp->reserve(5);
			//GetFileName::getFileNameW(*temp, m_root.top()->at(m_index).getAbsolutePath());
			m_root.push(temp);
			if(fill_music_func)
				fill_music_func(temp);
			m_index = 0;
		}
		else {
			m_player.playStream(m_root.top()->at(m_index));
			if(onAutoPlay)
				onAutoPlay(m_root.top()->at(m_index));
			++m_index;
		}
	}
}

void fv::Pumper::rand()
{
	all_templet<RAND>();
}

void fv::Pumper::loop()
{
	all_templet<LOOP>();
}

void fv::Pumper::none()
{
	all_templet<NONE>();
}

void fv::Pumper::cleanup()
{
	
}

void fv::Pumper::init(const char* root_dir)
{
	std::shared_ptr<std::vector<eqd_mp::MusicInfo>> temp = std::make_shared<std::vector<eqd_mp::MusicInfo>>();
	temp->reserve(5);
	//GetFileName::getFileNameA(*temp, root_dir);
	m_root.push(temp);
	if (fill_music_func)
		fill_music_func(temp);
	m_index = 0;
}

void fv::Pumper::init(const wchar_t* root_dir)
{
 	std::shared_ptr<std::vector<eqd_mp::MusicInfo>> temp = std::make_shared<std::vector<eqd_mp::MusicInfo>>();
	temp->reserve(5);
	//GetFileName::getFileNameW(*temp, root_dir);
    eqd_mp::DefInfoProvider provider(root_dir);
    *temp = provider.getRoot();
	m_root.push(temp);
	if (fill_music_func)
		fill_music_func(temp);
	m_index = 0;
}

std::shared_ptr<std::vector<eqd_mp::MusicInfo>> fv::Pumper::pop()
{
	if (m_root.size() <= 1)
		return nullptr;
	std::shared_ptr<std::vector<eqd_mp::MusicInfo>> res = m_root.top();
	m_root.pop();
	return res;
}

void fv::Pumper::onclick(int idx)
{
	auto& mfs = m_root.top();
	if (mfs && mfs->size() > idx)
	{
		auto& pmf = (*mfs)[idx];
		if (pmf.Type() == eqd_mp::MusicInfo::EType::Album)
		{
			auto temp_v = std::make_shared<std::vector<eqd_mp::MusicInfo>>();
			temp_v->reserve(5);
			//GetFileName::getFileNameW(*temp_v, pmf.getAbsolutePath());

			//std::lock_guard<std::mutex> lock(load_file_name_mutex);
			m_root.push(temp_v);
			m_index = 0;
			if (fill_music_func)
				fill_music_func(temp_v);
		}
		else {
			m_player.playStream(pmf);
			if(onAutoPlay)
				onAutoPlay(pmf);
			setIndex(idx + 1);
		}
	}

}
