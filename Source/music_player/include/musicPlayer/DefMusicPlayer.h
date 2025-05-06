#pragma once
#include "IMusicPlayer.h"
#include "tool.h"
#include "musicLoader/IMusicLoader.h"

namespace eqd_mp
{
    template <typename FL,typename ... ML>
    requires requires
    {
        requires VaildFileLoader<FL,MUSIC_INFO_STR_TYPE>;
        requires (VaildMusicLoader<ML,HSTREAM,MUSIC_INFO_STR_VIEW_TYPE> && ...);
    }
    struct DefMusicPlayer : public IMusicPlayer
    {
        void clear()
        {
            if (_stream)
            {
                BASS_SampleFree(_stream);
                _stream = NULL;
            }
        }
        bool play(const MusicInfo& info,bool loop)
        {
            stop();
            clear();
            std::span<uint8_t> data = _fileLoader.loadFormMemory(info.Path());
            if (data.empty())
                return false;
            auto [success,stream] = _loadMusic<0>(info,data);
            if (!success)
                return false;
            _stream = stream;
            if (BASS_ChannelPlay(_stream, loop))
            {
                _currentPlaying = &info;
                return true;
            }
            return false;
        }

        bool stop()
        {
            if (_stream && BASS_ChannelIsActive(_stream) != BASS_ACTIVE_STOPPED)
            {
                BASS_ChannelStop(_stream);
                return true;
            }
            return false;
        }
        bool pause()
        {
            auto state = BASS_ChannelIsActive(_stream);
            if (_stream && state != BASS_ACTIVE_PAUSED && state != BASS_ACTIVE_STOPPED)
            {
                BASS_ChannelPause(_stream);
                return true;
            }
            return false;
        }
        bool resume()
        {
            auto state = BASS_ChannelIsActive(_stream);
            if (_stream && state != BASS_ACTIVE_PLAYING && state != BASS_ACTIVE_STOPPED)
            {
                BASS_ChannelPlay(_stream, false);
                return true;
            }
            return false;
        }
        bool rewind()
        {
            auto state = BASS_ChannelIsActive(_stream);
            if (_stream && state != BASS_ACTIVE_PLAYING && state != BASS_ACTIVE_STOPPED)
            {
                BASS_ChannelPlay(_stream, true);
                return true;
            }
            return false;
        }
        bool isPlaying() const
        {
            auto state = BASS_ChannelIsActive(_stream);
            return state == BASS_ACTIVE_PLAYING;
        }
        const MusicInfo* getCurrentPlaying() const
        {
            return _currentPlaying;
        }
        bool isSupport(const MusicInfo& music) const
        {
            return (std::get<ML>(_musicLoader).isSupport(music.Suffix()) || ... );
        }
        ~DefMusicPlayer()
        {
            stop();
        }
    protected:
        template<size_t I>
        std::pair<bool,HSTREAM> _loadMusic(const MusicInfo& info,const std::span<uint8_t>& data)
        {
            if constexpr ( I >= std::tuple_size_v<std::tuple<ML...>>)
            {
                return std::make_pair(false,NULL);
            }else
            {
                auto& loader = std::get<I>(_musicLoader);
                if (loader.isSupport(info.Suffix()))
                    return loader.loadMusic(data);
                else
                    return _loadMusic<I+1>(info,data);
            }
        }
    protected:
        HSTREAM _stream = NULL;
        const MusicInfo* _currentPlaying = nullptr;
        FL _fileLoader;
        std::tuple<ML...> _musicLoader;
    };
    
}
