#pragma once
#if BUILD_SHARED_LIB
#include "api_def.h"
#include <wchar.h>


extern "C" {

	void EQD_MP_API MP3_init(const wchar_t* rootDir);
	void EQD_MP_API MP3_release();

	void EQD_MP_API MP3_getFFTData(float* data, unsigned int sizeTag);
	void EQD_MP_API MP3_tick();
	void EQD_MP_API MP3_setMode(int mode);
	void EQD_MP_API MP3_pump();	
	int EQD_MP_API MP3_setNext(const wchar_t* name);

}

#endif