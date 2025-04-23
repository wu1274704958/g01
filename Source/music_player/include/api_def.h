#pragma once

#ifdef EQD_MP_API
#undef EQD_MP_API
#define EQD_MP_API __declspec(dllexport)
#else
#define EQD_MP_API __declspec(dllimport)
#endif