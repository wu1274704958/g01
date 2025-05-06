// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include <musicPlayer/DefMusicPlayer.h>

#include "PlaylistMgr.hpp"
#include "fileLoader/DefFileLoaderWide.h"
#include "infoProvider/DefInfoProvider.h"
#include "musicLoader/DefMusicLoader.h"
#include "musicLoader/FlacLoader.h"
#include "playlistHandler/LoopPlaylist.h"
#include "playlistHandler/NextSongPlaylist.h"
#include "playlistHandler/RandomPlaylist.h"

using MusicPlayer = eqd_mp::DefMusicPlayer<eqd_mp::DefFileLoaderWide,
                                           eqd_mp::DefMusicLoader,
                                           eqd_mp::FlacLoader>;

using PlaylistMgr = eqd_mp::PlaylistMgr<MusicPlayer, eqd_mp::DefInfoProvider,
                                        eqd_mp::PlaylistPair<eqd_mp::EPumpMode::Loop, eqd_mp::LoopPlaylist>,
                                        eqd_mp::PlaylistPair<eqd_mp::EPumpMode::Rand, eqd_mp::RandomPlaylist>,
                                        eqd_mp::PlaylistPair<eqd_mp::EPumpMode::NextSong, eqd_mp::NextSongPlaylist>>;


