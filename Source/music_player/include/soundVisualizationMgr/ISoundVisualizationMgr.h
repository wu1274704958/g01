#pragma once
#include "api_def.h"
#include <span>

namespace eqd_mp
{

    enum class EFFTDataType : uint32_t
    {
        FFT256              = 0x80000000,
        FFT512              = 0x80000001, 
        FFT1024             = 0x80000002,
        FFT2048             = 0x80000003,
        FFT4096             = 0x80000004,
        FFT8192             = 0x80000005,
        FFT16384            = 0x80000006,
        FFT32768            = 0x80000007,
        FFT_INDIVIDUAL      = 0x10,
        FFT_NOWINDOW        = 0x20,
        FFT_REMOVEDC        = 0x40,
        FFT_COMPLEX         = 0x80,
        FFT_NYQUIST         = 0x100,
    };

    EFFTDataType EQD_MP_API operator&(EFFTDataType Lhs, EFFTDataType RHS);

    struct EQD_MP_API SoundVisualizationListener
    {
        virtual ~SoundVisualizationListener() = default;
        virtual void onRegisted(EFFTDataType type,size_t dataSize);
        virtual void onUnRegisted();
        virtual void onVisualizationData(const std::span<float>& data);
    };
    
    struct EQD_MP_API ISoundVisualizationMgr
    {
        bool registeListener(EFFTDataType type,SoundVisualizationListener& listener);
        bool unregisteListener(SoundVisualizationListener& listener);
        size_t getDataSize(EFFTDataType type) const;
        void tick();
    };

    
    
}

