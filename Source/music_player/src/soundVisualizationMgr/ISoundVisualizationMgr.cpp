#include "soundVisualizationMgr/ISoundVisualizationMgr.h"

eqd_mp::EFFTDataType eqd_mp::operator&(EFFTDataType Lhs, EFFTDataType RHS)
{
    return static_cast<EFFTDataType>(static_cast<uint32_t>(Lhs) & static_cast<uint32_t>(RHS));
}

void eqd_mp::SoundVisualizationListener::onRegisted(EFFTDataType type, size_t dataSize)
{
    
}

void eqd_mp::SoundVisualizationListener::onUnRegisted()
{
    
}

void eqd_mp::SoundVisualizationListener::onVisualizationData(const std::span<float>& data)
{
    
}

bool eqd_mp::ISoundVisualizationMgr::registeListener(EFFTDataType type, SoundVisualizationListener& listener)
{
    return false;
}

bool eqd_mp::ISoundVisualizationMgr::unregisteListener(SoundVisualizationListener& listener)
{
    return false;
}

size_t eqd_mp::ISoundVisualizationMgr::getDataSize(EFFTDataType type) const
{
    return 0;
}

void eqd_mp::ISoundVisualizationMgr::tick()
{
    
}
