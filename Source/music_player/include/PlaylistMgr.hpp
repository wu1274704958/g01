#pragma once
#include "CommConcept.hpp"
#include "EPumpMode.h"
#include "playlistHandler/IPlaylistHandler.h"
#include "infoProvider/IInfoProvider.h"
#include "musicPlayer/IMusicPlayer.h"
#include <stack>

namespace eqd_mp
{
    template<EPumpMode M,typename T>
    struct PlaylistPair
    {
        static constexpr EPumpMode Mode = M;
        using Type = T;
    };

    template<typename T>
    concept VaildPlaylistPair = requires
    {
        requires std::is_same_v<std::remove_cvref_t<decltype(T::Mode)>,EPumpMode>;
        requires VaildPlaylistHandler<typename T::Type>;
    };
    
    template<typename MP,typename IP,typename ... PLS>
    requires requires
    {
        requires VaildMusicPlayer<MP>;
        requires VaildInfoProvider<IP>;
        requires (VaildPlaylistPair<PLS> && ...);
    }
    struct PlaylistMgr
    {
        PlaylistMgr(MP& player,EPumpMode mode = EPumpMode::Loop) :
            _player(player),
            _mode(mode)
        {
            
        }
        template<typename ... Args>
        void initInfoProvider(bool autoPlay,Args ... args)
        requires VaildConstructArgs<IP,Args...>
        {
            _musicProvider = std::make_shared<IP>(std::forward<Args>(args)...);
            _musicProvider->addFilter(std::bind(&MP::isSupport,&_player,std::placeholders::_2));
            _sourceList.push(_musicProvider->getRoot());
            _playlist = generatePlaylistByMode(_mode);
            if (!_playlist.empty() && autoPlay)
                play();
        }

        template<typename ... Args>
        std::vector<MusicInfo*> generatePlaylistByMode(EPumpMode mode,Args ... args)
        {
            std::vector<MusicInfo*> result;
            if (_sourceList.empty() || _sourceList.top().empty())
                return result;
            ((mode == PLS::Mode ? ((result = generatePlaylistByModeInside<typename PLS::Type,Args...>(std::forward<Args>(args)...)),false):false),...);
            return result;
        }

        [[nodiscard]] EPumpMode mode() const
        {
            return _mode;
        }

        void set_Mode(EPumpMode Mode)
        {
            if (_mode != Mode)
            {
                _mode = Mode;
                if (_mode == EPumpMode::SingleLoop)
                {
                    _playlist.clear();
                    if (_player.isPlaying() && _player.getCurrentPlaying())
                        _playlist.push_back(const_cast<MusicInfo*>(_player.getCurrentPlaying()));
                    else if(_sourceList.size() > 0 && !_sourceList.top().empty())
                        _playlist.push_back(&_sourceList.top().front());
                }else
                    _playlist = generatePlaylistByMode(_mode);
            }
        }

        void setNextSong(const MusicInfo& next_song)
        {
            generatePlaylistByMode(EPumpMode::NextSong,_playlist,next_song);
        }

        void play()
        {
            if(_currentSong < _playlist.size())
                _player.play(*_playlist[_currentSong],mode() == EPumpMode::SingleLoop);
        }

        void tick()
        {
            if(!_player.isPlaying())
                moveAndPlay(1);
        }
        
        void moveAndPlay(int offset)
        {
            if (_playlist.empty())
                return;
            auto current = static_cast<int32_t>(_currentSong);
            const int32_t size = static_cast<int32_t>(_playlist.size());
            current += offset;
            if (current >= size)
                current = current % size;
            if (current < 0)
                current = (size + current) % size;
            play();
        }

    protected:
        template<typename T,typename ... Args>
        requires VaildPlaylistHandler<T> && VaildConstructArgs<T,Args...>
        std::vector<MusicInfo*> generatePlaylistByModeInside(Args ... args)
        {
            std::vector<MusicInfo*> result;
            T playlistHandler(std::forward<Args>(args)...);
            return playlistHandler.getPlaylist(_sourceList.top());
        }
        
    protected:
        MP& _player;
        EPumpMode _mode;
        std::shared_ptr<IP> _musicProvider;
        std::stack<std::vector<MusicInfo>> _sourceList;
        std::vector<MusicInfo*> _playlist;
        size_t _currentSong = 0;
    };
    
}

