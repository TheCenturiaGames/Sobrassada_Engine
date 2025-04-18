#pragma once

#include <stdint.h>

using WwiseID                          = uint32_t;

// Soundbanks
constexpr const wchar_t* BANKNAME_INIT = L"Init.bnk";
constexpr const wchar_t* BANKNAME_MAIN = L"main.bnk";

/*
* Wwise IDs are generated when generating the soundbanks. It's more efficient to use
* them than strings to avoid string hashing during execution time. When generating
* a soundbank, IDs remain the same for previously existing elements, and only new / modified
* ones have a change in their IDs
*/ 

// Events
constexpr const WwiseID ICE            = 344481046;
constexpr const WwiseID TELEPORT       = 530129416;

// Game parameters
constexpr const WwiseID PITCH          = 1908158473;
constexpr const WwiseID SPATIALIZATION = 2395249601;
constexpr const WwiseID VOLUME         = 2415836739;

// Switches, states, etc. (To add if we use them in the game)
