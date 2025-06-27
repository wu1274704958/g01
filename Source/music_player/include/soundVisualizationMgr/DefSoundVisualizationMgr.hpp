#pragma once

#define DSM_TEMPLATE_HEADER template<typename MP> \
    requires VaildMusicPlayer<MP>

namespace eqd_mp
{
    DSM_TEMPLATE_HEADER
    bool DefSoundVisualizationMgr<MP>::registeListener(EFFTDataType type,SoundVisualizationListener& listener)
    {
        std::lock_guard guard(_isLocked);
        if (_listenerTypes.contains(&listener))
            return false;
        if (!_listeners.contains(type))
            _listeners.emplace(std::make_pair<EFFTDataType,std::vector<SoundVisualizationListener*>>(MoveTemp(type),{}));
        _listeners[type].push_back(&listener);
        _listenerTypes.emplace(std::make_pair<SoundVisualizationListener*,EFFTDataType>(&listener,MoveTemp(type)));
        listener.onRegisted(type,getDataSize(type));
        return true;
    }

    DSM_TEMPLATE_HEADER
    bool DefSoundVisualizationMgr<MP>::unregisteListener(SoundVisualizationListener& listener)
    {
        std::lock_guard guard(_isLocked);
        if (!_listenerTypes.contains(&listener))
            return false;
        auto type = _listenerTypes[&listener];
        _listenerTypes.erase(&listener);
        if (!_listeners.contains(type))
            return false;
        auto& vec = _listeners[type];
        auto result = std::ranges::find(vec, &listener);
        if (result != vec.end())
        {
            vec.erase(result);
            listener.onUnRegisted();
            return true;
        }
        return false;
    }
    
    DSM_TEMPLATE_HEADER
    DefSoundVisualizationMgr<MP>::~DefSoundVisualizationMgr()
    {
        unregisteAllListener();
    }
    
    DSM_TEMPLATE_HEADER
    void DefSoundVisualizationMgr<MP>::unregisteAllListener()
    {
        std::lock_guard guard(_isLocked);
        for (auto it = _listeners.begin(); it != _listeners.end(); ++it)
        {
            for (auto& sv : it->second)
            {
                sv->onUnRegisted();
            }
        }
        _listeners.clear();
        _listenerTypes.clear();
    }
    

    DSM_TEMPLATE_HEADER
    size_t DefSoundVisualizationMgr<MP>::getDataSize(EFFTDataType type) const
    {
        size_t base = 0;
        if ((type & EFFTDataType::FFT256) == EFFTDataType::FFT256)
            base = 256;
        if ((type & EFFTDataType::FFT512) == EFFTDataType::FFT512)
            base = 512;
        if ((type & EFFTDataType::FFT1024) == EFFTDataType::FFT1024)
            base = 1024;
        if ((type & EFFTDataType::FFT2048) == EFFTDataType::FFT2048)
            base = 2048;
        if ((type & EFFTDataType::FFT4096) == EFFTDataType::FFT4096)
            base = 4096;
        if ((type & EFFTDataType::FFT8192) == EFFTDataType::FFT8192)
            base = 8192;
        if ((type & EFFTDataType::FFT16384) == EFFTDataType::FFT16384)
            base = 16384;
        if ((type & EFFTDataType::FFT32768) == EFFTDataType::FFT32768)
            base = 32768;
        double multi = 0.5;
        if ((type & EFFTDataType::FFT_COMPLEX) == EFFTDataType::FFT_COMPLEX)
            multi *= 4;
        if ((type & EFFTDataType::FFT_INDIVIDUAL) == EFFTDataType::FFT_INDIVIDUAL && _musicPlayer.isPlaying())
            multi *= _musicPlayer.getChannelCount();
        return static_cast<size_t>(base * multi);
    }

    DSM_TEMPLATE_HEADER
    void DefSoundVisualizationMgr<MP>::tick()
    {
        if (!_musicPlayer.isPlaying())
            return;
        for (auto it = _listeners.begin(); it != _listeners.end(); ++it)
        {
            auto size = getDataSize(it->first);
            if (size == 0)
                continue;
            if (_buf.size() < size)
                _buf.resize(size);
            if (-1 != BASS_ChannelGetData(_musicPlayer.getCurrentMusic(), _buf.data(), static_cast<DWORD>(it->first)))
            {
                for (auto& sv : it->second)
                {
                    sv->onVisualizationData(std::span<float>(_buf.data(),size));
                }
            }
        }
    }
}

#undef DSM_TEMPLATE_HEADER
