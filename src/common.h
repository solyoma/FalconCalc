#pragma once
#ifndef _COMMON_H
	#define _COMMON_H

enum AppLanguage { lanNotSet, lanEng, lanHun };

//extern const char* STATE_ID_STRING	;
//extern const char* DAT_ID_STRING		;
//extern const char* FalconCalc_HIST_FILE;
//extern const char* FalconCalc_Dat_FILE;
//extern const char* FalconCalc_State_FILE;
//
//extern const int   VERSION_INT;
//extern const char* VERSION_STRING;
constexpr const char* STATE_ID_STRING = "FalconCalc State File V";
constexpr const char* DAT_ID_STRING = "FalconCalc Data File V";
constexpr const char* FalconCalc_Hist_File = "FalconCalc.hist";
constexpr const char* FalconCalc_Dat_File = "FalconCalc.dat";
constexpr const char* FalconCalc_State_File = "FalconCalc.cfg";

constexpr const int   VERSION_INT = 0x000900;
constexpr const char* VERSION_STRING = "0.9.0";

#endif // _COMMON_H