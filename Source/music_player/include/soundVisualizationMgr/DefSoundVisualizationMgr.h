#pragma once
#include <unordered_map>
#include <vector>
#include "api_def.h"
#include "ISoundVisualizationMgr.h"
#include "musicPlayer/IMusicPlayer.h"

namespace eqd_mp
{
    template<typename MP>
    requires VaildMusicPlayer<MP>
    struct EQD_MP_API DefSoundVisualizationMgr : public ISoundVisualizationMgr
    {
        DefSoundVisualizationMgr(MP& MusicPlayer)
            : _musicPlayer(MusicPlayer)
        {
            
        }
        ~DefSoundVisualizationMgr();

        bool registeListener(EFFTDataType type,SoundVisualizationListener& listener);
        bool unregisteListener(SoundVisualizationListener& listener);
        void unregisteAllListener();
        size_t getDataSize(EFFTDataType type) const;
        void tick();

    protected:
        MP& _musicPlayer;
        std::unordered_map<EFFTDataType, std::vector<SoundVisualizationListener*>> _listeners;
        std::unordered_map<SoundVisualizationListener*,EFFTDataType> _listenerTypes;
        std::vector<float> _buf;
    };
}


#include "DefSoundVisualizationMgr.hpp"