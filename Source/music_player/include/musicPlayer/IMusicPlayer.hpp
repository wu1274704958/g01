#pragma once

namespace eqd_mp
{
    template <typename T>
    bool IMusicPlayer<T>::play(const MusicInfo& info, bool loop)
    {
        return false;
    }

    template <typename T>
    bool IMusicPlayer<T>::stop()
    {
        return false;
    }

    template <typename T>
    bool IMusicPlayer<T>::pause()
    {
        return false;
    }

    template <typename T>
    bool IMusicPlayer<T>::resume()
    {
        return false;
    }

    template <typename T>
    bool IMusicPlayer<T>::rewind()
    {
        return false;
    }

    template <typename T>
    bool IMusicPlayer<T>::isPlaying() const
    {
        return false;
    }

    template <typename T>
    const MusicInfo* IMusicPlayer<T>::getCurrentPlaying() const
    {
        return nullptr;
    }

    template <typename T>
    bool IMusicPlayer<T>::isSupport(const MusicInfo&) const
    {
        return false;
    }

    template <typename T>
    void IMusicPlayer<T>::clear()
    {
        
    }

    template <typename T>
    int IMusicPlayer<T>::getError() const
    {
        return 0;
    }

    template <typename T>
    bool IMusicPlayer<T>::initialized() const
    {
        return false;
    }

    template <typename T>
    const T& IMusicPlayer<T>::getCurrentMusic() const
    {
        throw std::runtime_error("IMusicPlayer<T>::getCurrentMusic() is not implemented");
    }

    template <typename T>
    int IMusicPlayer<T>::getChannelCount() const
    {
        return 0;
    }
}
