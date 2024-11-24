﻿#include <Dolphin/types.h>
#include <Dolphin/CARD.h>
#include <Dolphin/math.h>
#include <Dolphin/OS.h>
#include <Dolphin/string.h>
#include <SMS/MarioUtil/gd-reinit-gx.hxx>

#include <JSystem/J2D/J2DOrthoGraph.hxx>
#include <JSystem/J2D/J2DTextBox.hxx>
#include <J2D/J2DPrint.hxx>

#include <SMS/System/Application.hxx>
#include <SMS/Manager/FlagManager.hxx>
#include <SMS/System/CardManager.hxx>
#include <SMS/Camera/CubeManagerBase.hxx>
#include <SMS/Manager/FlagManager.hxx>

#include <BetterSMS/application.hxx>
#include <BetterSMS/game.hxx>
#include <BetterSMS/module.hxx>
#include <BetterSMS/stage.hxx>
#include <BetterSMS/loading.hxx>
#include <BetterSMS/settings.hxx>
#include <BetterSMS/player.hxx>
#include <BetterSMS/debug.hxx>

#include <System/RenderModeObj.hxx>

#include "characters.hxx"
#include "talking.hxx"
#include "shine.hxx"
#include "ai.hxx"
#include "splitscreen.hxx"
#include "subArea.hxx"
#include "surfGesso.hxx"
#include "yoshi.hxx"
#include "settings.hxx"
#include "pvp.hxx"
#include "players.hxx"

const u8 gSaveBnr[] = {
	0x09, 0x00, 0x00, 0x60, 0x00, 0x20, 0x00, 0x00, 0x01, 0x02, 0x00, 0x8E,
	0x00, 0x00, 0x0C, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x0D, 0x0E, 0x0E, 0x0E, 0x0F, 0x00, 0x00, 0x12, 0x0E, 0x11,
	0x11, 0x11, 0x11, 0x1B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x0F, 0x0F, 0x0E, 0x11, 0x12, 0x00, 0x00, 0x00,
	0x11, 0x11, 0x11, 0x11, 0x11, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1C, 0x0D,
	0x00, 0x12, 0x1D, 0x1B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x1E, 0x1F, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x0E, 0x1B, 0x00, 0x00, 0x0D, 0x1D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x12, 0x1D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x0F, 0x0E, 0x11, 0x10, 0x00, 0x20, 0x1D, 0x12,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x10, 0x0E, 0x0E, 0x0E, 0x00, 0x00, 0x00, 0x01,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x06, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x13, 0x03, 0x14, 0x01, 0x00, 0x00, 0x0D, 0x00, 0x21, 0x17,
	0x14, 0x22, 0x23, 0x0A, 0x00, 0x00, 0x02, 0x03, 0x04, 0x00, 0x00, 0x00,
	0x07, 0x00, 0x08, 0x09, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x15, 0x09,
	0x16, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x03, 0x09, 0x02, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0B, 0x0C,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x17, 0x18, 0x19, 0x1A, 0x08, 0x00,
	0x00, 0x02, 0x14, 0x09, 0x24, 0x25, 0x08, 0x00, 0x00, 0x12, 0x11, 0x11,
	0x26, 0x11, 0x11, 0x1B, 0x00, 0x11, 0x11, 0x11, 0x0D, 0x12, 0x1B, 0x12,
	0x00, 0x1B, 0x11, 0x11, 0x1B, 0x1D, 0x07, 0x00, 0x00, 0x00, 0x1D, 0x1B,
	0x26, 0x1B, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11,
	0x00, 0x1B, 0x1B, 0x1B, 0x00, 0x0F, 0x0F, 0x1D, 0x00, 0x1B, 0x11, 0x11,
	0x00, 0x1F, 0x11, 0x12, 0x00, 0x1B, 0x11, 0x0E, 0x00, 0x0E, 0x11, 0x12,
	0x11, 0x11, 0x11, 0x11, 0x11, 0x07, 0x00, 0x00, 0x11, 0x26, 0x2B, 0x11,
	0x0E, 0x2B, 0x20, 0x0F, 0x11, 0x11, 0x20, 0x11, 0x1F, 0x2D, 0x0E, 0x11,
	0x11, 0x11, 0x11, 0x11, 0x12, 0x00, 0x2C, 0x11, 0x00, 0x00, 0x10, 0x1D,
	0x00, 0x12, 0x1D, 0x1B, 0x1B, 0x2C, 0x0E, 0x11, 0x00, 0x0E, 0x11, 0x11,
	0x11, 0x11, 0x11, 0x11, 0x00, 0x0E, 0x11, 0x1D, 0x11, 0x12, 0x2B, 0x00,
	0x00, 0x0E, 0x11, 0x0D, 0x26, 0x0E, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x11, 0x11, 0x2C, 0x2B, 0x00, 0x00, 0x00, 0x00, 0x0D, 0x11, 0x0E, 0x0D,
	0x00, 0x00, 0x00, 0x00, 0x2B, 0x1B, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x1F, 0x20, 0x00, 0x00, 0x0D, 0x1D, 0x00, 0x00, 0x11, 0x11, 0x1D, 0x0D,
	0x0E, 0x1B, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0F, 0x00, 0x00,
	0x11, 0x11, 0x11, 0x11, 0x11, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x12, 0x27, 0x00, 0x1C, 0x11, 0x1D, 0x00, 0x00, 0x11, 0x0E,
	0x12, 0x0E, 0x11, 0x0E, 0x0D, 0x00, 0x11, 0x0E, 0x0F, 0x0E, 0x11, 0x11,
	0x11, 0x07, 0x11, 0x11, 0x1B, 0x11, 0x11, 0x10, 0x00, 0x1B, 0x1D, 0x12,
	0x11, 0x11, 0x11, 0x11, 0x2B, 0x1F, 0x11, 0x11, 0x20, 0x0D, 0x11, 0x0E,
	0x0D, 0x0E, 0x11, 0x11, 0x20, 0x2B, 0x11, 0x0F, 0x00, 0x1F, 0x11, 0x11,
	0x00, 0x00, 0x00, 0x00, 0x12, 0x11, 0x11, 0x11, 0x0F, 0x00, 0x00, 0x2D,
	0x11, 0x11, 0x11, 0x11, 0x20, 0x00, 0x00, 0x0F, 0x11, 0x1B, 0x2D, 0x1D,
	0x12, 0x00, 0x00, 0x0E, 0x11, 0x12, 0x00, 0x2B, 0x0D, 0x00, 0x00, 0x15,
	0x02, 0x28, 0x03, 0x22, 0x11, 0x0D, 0x00, 0x00, 0x00, 0x06, 0x09, 0x29,
	0x11, 0x11, 0x00, 0x00, 0x00, 0x22, 0x22, 0x29, 0x11, 0x11, 0x00, 0x00,
	0x00, 0x15, 0x28, 0x29, 0x13, 0x0C, 0x29, 0x16, 0x29, 0x16, 0x21, 0x06,
	0x29, 0x09, 0x29, 0x17, 0x04, 0x17, 0x16, 0x03, 0x29, 0x22, 0x04, 0x16,
	0x03, 0x16, 0x04, 0x17, 0x02, 0x17, 0x03, 0x09, 0x14, 0x14, 0x03, 0x04,
	0x03, 0x14, 0x28, 0x00, 0x2A, 0x04, 0x00, 0x00, 0x14, 0x14, 0x0A, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x29, 0x16, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x17, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1D, 0x1B,
	0x11, 0x11, 0x20, 0x00, 0x00, 0x1B, 0x0E, 0x0F, 0x0D, 0x11, 0x11, 0x1D,
	0x00, 0x0F, 0x0E, 0x11, 0x11, 0x11, 0x11, 0x1C, 0x00, 0x0F, 0x11, 0x11,
	0x11, 0x11, 0x11, 0x1C, 0x00, 0x1B, 0x11, 0x0E, 0x00, 0x11, 0x11, 0x12,
	0x00, 0x1B, 0x11, 0x0E, 0x30, 0x26, 0x11, 0x12, 0x00, 0x1D, 0x11, 0x26,
	0x1B, 0x11, 0x11, 0x2B, 0x00, 0x1D, 0x11, 0x11, 0x11, 0x11, 0x0E, 0x07,
	0x11, 0x11, 0x11, 0x11, 0x1C, 0x00, 0x2C, 0x11, 0x11, 0x11, 0x0D, 0x00,
	0x00, 0x00, 0x11, 0x11, 0x11, 0x11, 0x0D, 0x00, 0x00, 0x00, 0x0F, 0x11,
	0x11, 0x0E, 0x0D, 0x00, 0x00, 0x00, 0x1B, 0x11, 0x11, 0x0D, 0x2B, 0x00,
	0x00, 0x0E, 0x11, 0x1D, 0x11, 0x1B, 0x0D, 0x00, 0x00, 0x0E, 0x11, 0x11,
	0x11, 0x12, 0x2B, 0x0D, 0x00, 0x0F, 0x11, 0x11, 0x11, 0x0F, 0x1B, 0x1B,
	0x00, 0x0F, 0x11, 0x11, 0x2B, 0x1B, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x11, 0x26, 0x2D, 0x00, 0x00, 0x00, 0x00, 0x2B, 0x11, 0x11, 0x20, 0x2D,
	0x00, 0x00, 0x00, 0x0D, 0x11, 0x11, 0x1B, 0x07, 0x00, 0x00, 0x00, 0x0D,
	0x0E, 0x2C, 0x11, 0x11, 0x11, 0x11, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11,
	0x26, 0x0E, 0x00, 0x1C, 0x11, 0x1B, 0x1B, 0x0D, 0x20, 0x0E, 0x12, 0x20,
	0x0E, 0x1B, 0x1B, 0x0D, 0x27, 0x11, 0x12, 0x20, 0x0F, 0x11, 0x11, 0x11,
	0x11, 0x07, 0x11, 0x11, 0x0E, 0x11, 0x00, 0x11, 0x11, 0x2B, 0x0F, 0x11,
	0x11, 0x11, 0x11, 0x11, 0x11, 0x2B, 0x0F, 0x11, 0x11, 0x11, 0x11, 0x11,
	0x11, 0x2B, 0x0F, 0x11, 0x1B, 0x2B, 0x11, 0x0F, 0x00, 0x1F, 0x11, 0x11,
	0x11, 0x11, 0x26, 0x07, 0x00, 0x1F, 0x11, 0x0E, 0x11, 0x2C, 0x11, 0x20,
	0x2D, 0x1F, 0x11, 0x0F, 0x11, 0x11, 0x11, 0x20, 0x2D, 0x0E, 0x11, 0x0F,
	0x12, 0x00, 0x00, 0x0E, 0x11, 0x12, 0x00, 0x2B, 0x00, 0x00, 0x00, 0x0E,
	0x11, 0x1B, 0x2B, 0x1D, 0x00, 0x00, 0x00, 0x1D, 0x11, 0x11, 0x11, 0x11,
	0x00, 0x00, 0x00, 0x1D, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E, 0x00, 0x00,
	0x00, 0x02, 0x14, 0x22, 0x11, 0x27, 0x00, 0x0B, 0x22, 0x28, 0x09, 0x22,
	0x0E, 0x20, 0x21, 0x09, 0x09, 0x09, 0x06, 0x22, 0x11, 0x0D, 0x00, 0x0A,
	0x02, 0x29, 0x06, 0x02, 0x04, 0x06, 0x06, 0x14, 0x2E, 0x2E, 0x09, 0x03,
	0x17, 0x31, 0x32, 0x03, 0x08, 0x2B, 0x14, 0x03, 0x17, 0x07, 0x31, 0x16,
	0x15, 0x35, 0x14, 0x03, 0x16, 0x15, 0x15, 0x03, 0x15, 0x31, 0x09, 0x16,
	0x22, 0x2F, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x04, 0x33, 0x34,
	0x08, 0x00, 0x00, 0x00, 0x04, 0x04, 0x09, 0x36, 0x06, 0x37, 0x00, 0x00,
	0x04, 0x04, 0x2F, 0x38, 0x39, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1D, 0x11,
	0x11, 0x11, 0x0D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1B, 0x0E, 0x0E, 0x0E, 0x12, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x11, 0x1F, 0x3A, 0x00, 0x00, 0x00, 0x3B, 0x0E, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0E, 0x0E, 0x0E, 0x11,
	0x00, 0x1B, 0x0E, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x07, 0x1F, 0x1F, 0x3A, 0x00, 0x00, 0x00, 0x1B,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x1F, 0x27, 0x00, 0x00, 0x1B, 0x1F, 0x20, 0x2C, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0E, 0x0E, 0x1D, 0x0E,
	0x0E, 0x2B, 0x1B, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x0F, 0x1B, 0x11, 0x11, 0x30, 0x0E, 0x11, 0x20,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x0F, 0x0E, 0x0E, 0x0E, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00,
	0x00, 0x06, 0x09, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x21,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x0B, 0x22, 0x14, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x21, 0x14, 0x17, 0x04, 0x16, 0x17, 0x17, 0x17, 0x16, 0x03, 0x2F,
	0x3C, 0x04, 0x17, 0x17, 0x16, 0x16, 0x04, 0x02, 0x32, 0x00, 0x38, 0x02,
	0x02, 0x38, 0x02, 0x04, 0x29, 0x28, 0x06, 0x21, 0x22, 0x17, 0x2F, 0x04,
	0x04, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x02, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x16, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x16, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x0E, 0x1F, 0x1F, 0x0F, 0x00,
	0x00, 0x30, 0x0E, 0x1F, 0x0E, 0x0E, 0x1F, 0x27, 0x00, 0x30, 0x1F, 0x4E,
	0x4E, 0x4E, 0x4E, 0x4F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x27, 0x0F, 0x00, 0x00, 0x1F, 0x0E, 0x00, 0x30,
	0x4F, 0x1E, 0x00, 0x00, 0x4E, 0x4E, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x30, 0x0E, 0x1F, 0x1F, 0x0F, 0x00, 0x00, 0x00,
	0x0E, 0x1F, 0x0E, 0x0E, 0x1F, 0x27, 0x41, 0x0F, 0x1F, 0x4E, 0x4E, 0x4E,
	0x4E, 0x4F, 0x41, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0E, 0x27, 0x00, 0x1F,
	0x1F, 0x2B, 0x00, 0x27, 0x1F, 0x4F, 0x00, 0x4E, 0x4E, 0x1C, 0x00, 0x4F,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x41, 0x30, 0x00, 0x00, 0x27, 0x0F, 0x00, 0x00,
	0x51, 0x30, 0x00, 0x00, 0x4F, 0x1E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x1F, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x35, 0x4E, 0x4E, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x41, 0x00, 0x00, 0x00,
	0x00, 0x42, 0x43, 0x43, 0x51, 0x00, 0x00, 0x00, 0x00, 0x52, 0x43, 0x43,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x44, 0x45, 0x00, 0x00, 0x3C, 0x46, 0x46, 0x44,
	0x44, 0x53, 0x00, 0x00, 0x47, 0x43, 0x43, 0x44, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x3D, 0x16, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x47, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x47, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x09, 0x28, 0x17, 0x28, 0x0A, 0x0B,
	0x00, 0x00, 0x38, 0x17, 0x29, 0x00, 0x00, 0x3E, 0x00, 0x48, 0x00, 0x29,
	0x22, 0x00, 0x00, 0x49, 0x00, 0x54, 0x00, 0x15, 0x00, 0x3F, 0x00, 0x55,
	0x38, 0x03, 0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3E, 0x3F,
	0x40, 0x00, 0x00, 0x00, 0x4A, 0x4B, 0x4C, 0x4D, 0x4C, 0x00, 0x00, 0x00,
	0x56, 0x55, 0x4B, 0x4B, 0x4B, 0x00, 0x00, 0x00, 0x00, 0x4E, 0x57, 0x57,
	0x51, 0x58, 0x59, 0x32, 0x00, 0x5E, 0x66, 0x66, 0x66, 0x5E, 0x67, 0x00,
	0x00, 0x00, 0x2E, 0x6D, 0x6E, 0x6F, 0x70, 0x00, 0x00, 0x00, 0x2E, 0x72,
	0x71, 0x7A, 0x70, 0x00, 0x00, 0x57, 0x57, 0x57, 0x00, 0x59, 0x59, 0x5A,
	0x00, 0x66, 0x66, 0x66, 0x00, 0x66, 0x66, 0x68, 0x00, 0x71, 0x71, 0x71,
	0x00, 0x71, 0x71, 0x68, 0x00, 0x7A, 0x7A, 0x7A, 0x00, 0x7A, 0x7A, 0x68,
	0x57, 0x57, 0x59, 0x00, 0x57, 0x57, 0x00, 0x4E, 0x66, 0x66, 0x66, 0x5E,
	0x66, 0x66, 0x00, 0x5E, 0x71, 0x71, 0x70, 0x71, 0x71, 0x6E, 0x00, 0x00,
	0x71, 0x7A, 0x7B, 0x7A, 0x7A, 0x71, 0x00, 0x00, 0x57, 0x57, 0x51, 0x58,
	0x59, 0x32, 0x51, 0x57, 0x66, 0x66, 0x66, 0x5E, 0x67, 0x00, 0x31, 0x66,
	0x2E, 0x6D, 0x6E, 0x71, 0x72, 0x00, 0x00, 0x70, 0x2E, 0x72, 0x71, 0x7A,
	0x70, 0x00, 0x00, 0x7B, 0x57, 0x5B, 0x00, 0x57, 0x57, 0x35, 0x00, 0x57,
	0x66, 0x5E, 0x5C, 0x66, 0x66, 0x32, 0x00, 0x66, 0x71, 0x71, 0x71, 0x6F,
	0x6F, 0x32, 0x00, 0x71, 0x7A, 0x7A, 0x7A, 0x7A, 0x7A, 0x32, 0x00, 0x7A,
	0x57, 0x57, 0x59, 0x5C, 0x57, 0x57, 0x59, 0x00, 0x66, 0x66, 0x68, 0x5C,
	0x66, 0x66, 0x66, 0x5E, 0x71, 0x71, 0x73, 0x74, 0x71, 0x71, 0x70, 0x71,
	0x7A, 0x7A, 0x5C, 0x7C, 0x7A, 0x7A, 0x7B, 0x7A, 0x57, 0x57, 0x00, 0x5A,
	0x5D, 0x5E, 0x5F, 0x57, 0x66, 0x66, 0x00, 0x66, 0x66, 0x66, 0x57, 0x66,
	0x71, 0x6E, 0x00, 0x6E, 0x71, 0x71, 0x5A, 0x74, 0x7A, 0x71, 0x00, 0x6F,
	0x7A, 0x7A, 0x7D, 0x74, 0x57, 0x00, 0x00, 0x00, 0x47, 0x43, 0x43, 0x60,
	0x66, 0x00, 0x00, 0x00, 0x43, 0x43, 0x53, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x43, 0x43, 0x75, 0x00, 0x00, 0x00, 0x00, 0x00, 0x43, 0x44, 0x3C, 0x61,
	0x60, 0x53, 0x3C, 0x61, 0x43, 0x43, 0x44, 0x44, 0x61, 0x61, 0x00, 0x46,
	0x43, 0x60, 0x61, 0x42, 0x61, 0x00, 0x00, 0x43, 0x44, 0x75, 0x00, 0x3C,
	0x00, 0x3C, 0x00, 0x44, 0x44, 0x75, 0x00, 0x3C, 0x44, 0x47, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x44, 0x45, 0x3C, 0x43, 0x69, 0x6A, 0x64, 0x00,
	0x44, 0x45, 0x3C, 0x43, 0x76, 0x77, 0x4C, 0x00, 0x44, 0x45, 0x3C, 0x43,
	0x7E, 0x7F, 0x4D, 0x00, 0x3E, 0x55, 0x55, 0x4B, 0x4B, 0x4B, 0x62, 0x63,
	0x4B, 0x49, 0x4B, 0x48, 0x4D, 0x4B, 0x4B, 0x00, 0x55, 0x55, 0x78, 0x00,
	0x3E, 0x4B, 0x4B, 0x00, 0x4B, 0x55, 0x3F, 0x00, 0x3E, 0x4B, 0x4B, 0x00,
	0x55, 0x64, 0x54, 0x4B, 0x65, 0x00, 0x00, 0x00, 0x4B, 0x6B, 0x62, 0x65,
	0x6C, 0x00, 0x00, 0x00, 0x4B, 0x4B, 0x4B, 0x79, 0x3E, 0x00, 0x00, 0x00,
	0x4B, 0x4B, 0x4B, 0x4D, 0x3E, 0x00, 0x00, 0x00, 0x00, 0x7D, 0x71, 0x70,
	0x80, 0x71, 0x71, 0x2E, 0x00, 0x7D, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x32,
	0x00, 0x86, 0x85, 0x85, 0x85, 0x85, 0x6D, 0x87, 0x00, 0x00, 0x89, 0x8A,
	0x8A, 0x8A, 0x73, 0x00, 0x00, 0x71, 0x71, 0x71, 0x81, 0x71, 0x71, 0x81,
	0x00, 0x80, 0x7B, 0x7B, 0x7B, 0x7B, 0x70, 0x83, 0x00, 0x68, 0x85, 0x85,
	0x85, 0x85, 0x6D, 0x31, 0x00, 0x00, 0x68, 0x8B, 0x8B, 0x88, 0x74, 0x00,
	0x71, 0x71, 0x81, 0x71, 0x71, 0x6F, 0x00, 0x7D, 0x7B, 0x7B, 0x00, 0x82,
	0x7B, 0x85, 0x00, 0x7D, 0x85, 0x85, 0x00, 0x7D, 0x85, 0x82, 0x00, 0x86,
	0x88, 0x8A, 0x00, 0x32, 0x88, 0x87, 0x00, 0x00, 0x71, 0x70, 0x80, 0x71,
	0x71, 0x2E, 0x00, 0x82, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x32, 0x00, 0x68,
	0x85, 0x85, 0x85, 0x85, 0x6D, 0x87, 0x00, 0x81, 0x89, 0x8A, 0x8B, 0x8A,
	0x73, 0x00, 0x00, 0x31, 0x71, 0x71, 0x71, 0x71, 0x71, 0x32, 0x00, 0x71,
	0x7B, 0x7B, 0x68, 0x7B, 0x7B, 0x83, 0x00, 0x7B, 0x85, 0x85, 0x81, 0x85,
	0x85, 0x87, 0x00, 0x85, 0x8A, 0x8B, 0x74, 0x8A, 0x8A, 0x31, 0x00, 0x8B,
	0x71, 0x6F, 0x00, 0x83, 0x71, 0x71, 0x81, 0x71, 0x7B, 0x82, 0x00, 0x00,
	0x7B, 0x7B, 0x00, 0x82, 0x85, 0x7D, 0x00, 0x00, 0x85, 0x85, 0x00, 0x7D,
	0x8B, 0x8C, 0x00, 0x00, 0x88, 0x8B, 0x00, 0x8D, 0x71, 0x71, 0x00, 0x7B,
	0x71, 0x71, 0x7B, 0x84, 0x7B, 0x85, 0x00, 0x82, 0x7B, 0x7B, 0x7C, 0x83,
	0x85, 0x82, 0x00, 0x88, 0x85, 0x6D, 0x73, 0x68, 0x88, 0x87, 0x00, 0x68,
	0x8B, 0x8B, 0x8A, 0x8B, 0x00, 0x00, 0x00, 0x00, 0x44, 0x44, 0x60, 0x60,
	0x80, 0x00, 0x00, 0x00, 0x60, 0x44, 0x44, 0x60, 0x88, 0x00, 0x00, 0x00,
	0x53, 0x44, 0x44, 0x60, 0x8B, 0x00, 0x00, 0x00, 0x61, 0x53, 0x45, 0x45,
	0x45, 0x60, 0x3C, 0x44, 0x44, 0x60, 0x3C, 0x60, 0x60, 0x60, 0x61, 0x47,
	0x44, 0x44, 0x44, 0x43, 0x60, 0x45, 0x3C, 0x75, 0x60, 0x44, 0x44, 0x44,
	0x45, 0x75, 0x00, 0x00, 0x47, 0x44, 0x44, 0x44, 0x43, 0x44, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x43, 0x53, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x45, 0x47, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x47, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x4B, 0x4B, 0x64, 0x3E, 0x4D, 0x4B, 0x64, 0x00,
	0x6C, 0x4B, 0x4B, 0x4B, 0x4B, 0x64, 0x6C, 0x00, 0x6C, 0x4B, 0x4B, 0x4B,
	0x64, 0x63, 0x6C, 0x00, 0x00, 0x62, 0x4B, 0x6B, 0x62, 0x6C, 0x40, 0x00,
	0x4B, 0x4B, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4B, 0x6B, 0x3F, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x64, 0x4D, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x62, 0x4D, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x74, 0x8D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0xB5, 0x00,
	0xC9, 0x82, 0xFB, 0xD0, 0xF2, 0x84, 0xC9, 0x40, 0xD2, 0x4A, 0x80, 0x45,
	0x94, 0x81, 0xFB, 0xD8, 0xA0, 0x80, 0x90, 0x40, 0xD1, 0xC4, 0x94, 0xD1,
	0xB1, 0xDD, 0xA9, 0x99, 0x88, 0x8C, 0xA9, 0x9D, 0x90, 0x8D, 0xA0, 0x40,
	0xFB, 0xDD, 0xA0, 0xC2, 0xFB, 0x8C, 0xF2, 0xCA, 0xF3, 0x9C, 0xDA, 0x4C,
	0xC9, 0xC4, 0xA1, 0x58, 0x88, 0x89, 0x99, 0x14, 0xA9, 0xD8, 0xB2, 0x1D,
	0xA1, 0x15, 0xB1, 0x46, 0xDA, 0x04, 0xB1, 0x86, 0xEA, 0x8C, 0xDA, 0x46,
	0xA5, 0x5C, 0xA1, 0x56, 0xF3, 0x0E, 0xFB, 0x92, 0xA9, 0x48, 0x88, 0x48,
	0xA9, 0xDC, 0x80, 0x05, 0xB2, 0x12, 0xFB, 0x06, 0x90, 0xCC, 0x88, 0x85,
	0x90, 0xC8, 0xFB, 0x96, 0xA9, 0x46, 0x90, 0x89, 0xEB, 0x5A, 0xC1, 0x84,
	0xB8, 0xC0, 0xA9, 0x04, 0x98, 0xD4, 0xA1, 0x18, 0xA0, 0x00, 0xE2, 0x88,
	0x80, 0xC0, 0x81, 0x80, 0x80, 0x40, 0x99, 0x11, 0xD0, 0x02, 0xF4, 0x89,
	0xF0, 0x45, 0xD4, 0x44, 0xF0, 0x48, 0xC0, 0x02, 0x80, 0x80, 0x9B, 0xC8,
	0x9B, 0x48, 0x93, 0x44, 0x8A, 0xC4, 0x8A, 0x42, 0xBA, 0x5D, 0xA9, 0x96,
	0xB1, 0xDA, 0xA1, 0x50, 0xE0, 0x02, 0xD4, 0x04, 0x81, 0x00, 0x93, 0x85,
	0x9B, 0x88, 0xC2, 0x9D, 0x99, 0x0E, 0xB2, 0x18, 0xA5, 0x91, 0xB1, 0xD6,
	0xA1, 0x4D, 0xB2, 0x16, 0xBA, 0x58, 0xBA, 0x9C, 0xE4, 0x44, 0x94, 0x00,
	0x8A, 0x82, 0x8A, 0xC2, 0x8B, 0x04, 0x93, 0x04, 0xC6, 0xDD, 0x90, 0x86,
	0xA5, 0xD0, 0xF9, 0xCC, 0xDB, 0x06, 0x8B, 0x44, 0x86, 0x02, 0xB2, 0x98,
	0xCB, 0x1D, 0xCB, 0x5D, 0xC2, 0xD9, 0xD3, 0x5C, 0xC2, 0x98, 0x95, 0x4C,
	0x95, 0x09, 0xB0, 0x01, 0xF9, 0x8A, 0xD2, 0x84, 0x81, 0xC0, 0x92, 0xC4,
	0xD3, 0x9E, 0xC3, 0x1A, 0x99, 0x4A, 0xB2, 0x54, 0xF1, 0x06, 0xC2, 0x44,
	0xAA, 0x11, 0xA1, 0x8D, 0xB6, 0x96, 0x90, 0xC6, 0xB2, 0x52, 0xBA, 0xD8,
	0xAA, 0x14, 0x88, 0xC6, 0xAA, 0x55, 0x99, 0x90, 0xAA, 0x96, 0xAA, 0x98,
	0x99, 0x8E, 0x88, 0xC8
};

const u8 gSaveIcon[] = { 0x0 };

Settings::SettingsGroup gSettingsGroup(1, 0, Settings::Priority::GAME);

SMSCoop::PlayerTypeSetting gPlayer1TypeSetting("Player 1 character", 0);
SMSCoop::PlayerTypeSetting gPlayer2TypeSetting("Player 2 character", 1);
SMSCoop::CameraTypeSetting gCameraTypeSetting("Camera type", &gPlayer1TypeSetting, &gPlayer2TypeSetting);
SMSCoop::ShineGrabDistanceSetting gShineGrabDistanceSetting("Max shine grab distance");
SMSCoop::SpeedrunSetting gSpeedrunSetting("Speedrun mode");

SMSCoop::ExplainSetting gExplain1("", "P1 character select only works for", 35);
SMSCoop::ExplainSetting gExplain2("", "singleplayer camera or when using", 34);
SMSCoop::ExplainSetting gExplain3("", "expanded memory through emulation.", 35);


static BetterSMS::ModuleInfo sModuleInfo{"Mario Sunshine Coop", 2, 0, &gSettingsGroup};

//
//bool isMemoryExpanded() {
//    return *((u32*)0x800000f0) == 0x04000000;
//}
//
//void TApplication_custom_proc(TApplication* app) {
//    J2DTextBox *gpFPSStringW = nullptr;
//    J2DTextBox *gpFPSStringB = nullptr;
//    SMSSetupGCLogoRenderingInfo(app->mDisplay);
//    app->gameLoop();
//    app->initialize_bootAfter();
//    
//    const char* memoryWarningText = "Warning!!! Memory is not increased.\n\nYou must increase Dolphins memory\nsize in order to play this hack.\n\nTo do this, make sure you are on\na recent build of Dolphin.\nIn Config/Advanced/Memory Override\nIncrease MEM1 all the way to 64MB.";
//    gpFPSStringW                  = new J2DTextBox(gpSystemFont->mFont, memoryWarningText);
//    gpFPSStringB                  = new J2DTextBox(gpSystemFont->mFont, memoryWarningText);
//    gpFPSStringW->mNewlineSize    = 17*1.5;
//    gpFPSStringW->mCharSizeX      = 14*1.5;
//    gpFPSStringW->mCharSizeY      = 17*1.5;
//    gpFPSStringB->mNewlineSize    = 17*1.5;
//    gpFPSStringB->mCharSizeX      = 14*1.5;
//    gpFPSStringB->mCharSizeY      = 17*1.5;
//    gpFPSStringB->mGradientTop    = {0, 0, 0, 255};
//    gpFPSStringB->mGradientBottom = {0, 0, 0, 255};
//
//    while(true) {
//        SMSSetupGCLogoRenderingInfo(app->mDisplay);
//        app->mDisplay->startRendering();
//
//        J2DOrthoGraph ortho(0, 0, BetterSMS::getScreenOrthoWidth(), 448);
//        ortho.setup2D();
//
//        GXSetViewport(0, 0, 640, 480, 0, 1);
//        {
//            Mtx44 mtx;
//            C_MTXOrtho(mtx, 16, 496, -BetterSMS::getScreenRatioAdjustX(),
//                       BetterSMS::getScreenRenderWidth(), -1, 1);
//            GXSetProjection(mtx, GX_ORTHOGRAPHIC);
//        } 
//        
//        // Draw warning
//        auto monitorX = 60 + getScreenRatioAdjustX();
//        auto monitorY = 150;
//        gpFPSStringB->draw(monitorX + 1, monitorY + 2);
//        gpFPSStringW->draw(monitorX, monitorY);
//
//
//        THPPlayerDrawDone();
//        app->mDisplay->endRendering();
//    }
//	
//}
//static pp::togglable_ppc_bl my_patch((u32)SMS_PORT_REGION(0x80005624, 0, 0, 0), (void*)TApplication_custom_proc, false);

OSTime timeStart;
OSTime timeStop;
bool timerFromStart = false;
bool timerCreated = false;
bool timerStarted = false;
bool timerStopped = false;
static J2DTextBox *gpTimerTextBoxW = nullptr;
static J2DTextBox *gpTimerTextBoxB = nullptr;
static char sStringBuffer[17];

void initTimer(TApplication *app) {
    auto *currentHeap = JKRHeap::sRootHeap->becomeCurrentHeap();
    gpTimerTextBoxW                  = new J2DTextBox(gpSystemFont->mFont, "");
    gpTimerTextBoxB                  = new J2DTextBox(gpSystemFont->mFont, "");
    gpTimerTextBoxW->mStrPtr         = sStringBuffer;
    gpTimerTextBoxB->mStrPtr         = sStringBuffer;
    gpTimerTextBoxW->mNewlineSize    = 17;
    gpTimerTextBoxW->mCharSizeX      = 14;
    gpTimerTextBoxW->mCharSizeY      = 17;
    gpTimerTextBoxB->mNewlineSize    = 17;
    gpTimerTextBoxB->mCharSizeX      = 14;
    gpTimerTextBoxB->mCharSizeY      = 17;
    gpTimerTextBoxB->mGradientTop    = {0, 0, 0, 255};
    gpTimerTextBoxB->mGradientBottom = {0, 0, 0, 255};
    currentHeap->becomeCurrentHeap();
    snprintf(sStringBuffer, 17, "%.2d:%.2d:%.2d.%.3d", 0, 0,0, 0);
    timerCreated = true;
}

OSTime hackedFileOffset = 340041;

#define SMS_CHECK_RESET_FLAG(gamepad) (((1 << (gamepad)->mPort) & TMarioGamePad::mResetFlag) == 1)
void updateTimer(TApplication *app) {
    if(!timerFromStart || !timerStarted || !timerCreated || gSpeedrunSetting.getInt() == 0) return;
    if(!timerStopped) {
        timeStop = OSGetTime() + (OSTime)OSMillisecondsToTicks(hackedFileOffset);
    }

    if(timerStarted && SMS_CHECK_RESET_FLAG(app->mGamePads[0])) {
		timerFromStart = false;
    }
    u32 time = (u32)OSTicksToMilliseconds(timeStop - timeStart);
    f32 fTime = time / 1000.0;

    int sec = ((int)fTime);
    int min = sec / 60;
    int hrs = min / 60;

    snprintf(sStringBuffer, 17, "%.2d:%.2d:%.2d.%.3d", hrs, min - hrs * 60, sec - min * 60, (int)((fTime - sec) * 1000));

}

//static size_t getHeapSize(JKRHeap *heap) {
//    return reinterpret_cast<size_t>(heap->mEnd) - reinterpret_cast<size_t>(heap->mStart);
//}
void endRendering_override() {

    //auto size = getHeapSize(JKRHeap::sCurrentHeap);
    //OSReport("Max mem %d %d\n", (size - JKRHeap::sCurrentHeap->getTotalFreeSize()), size);
    if(!timerFromStart || !gpMarDirector || gSpeedrunSetting.getInt() == 0 || !(timerStarted || SMS_isOptionMap__Fv()) || !timerCreated) {
        GXDrawDone();
        return;
    }
    J2DOrthoGraph ortho(0, 0, 640, 448);
    ortho.setup2D();

    GXSetViewport(0, 0, 640, 480, 0, 1);
    {
        Mtx44 mtx;
        C_MTXOrtho(mtx, 16, 496, -BetterSMS::getScreenRatioAdjustX(),
                   BetterSMS::getScreenRenderWidth(), -1, 1);
        GXSetProjection(mtx, GX_ORTHOGRAPHIC);
    }

    {
        gpTimerTextBoxB->draw(0 + 1, 460 + 2);
        gpTimerTextBoxW->draw(0, 460);
    }

    GXDrawDone();
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8001e930, 0, 0, 0), endRendering_override);

void TFlagManager_firstStart_override(TFlagManager* instance) {
    if(gSpeedrunSetting.getInt() == 1) {
        timerStopped = false;
		OSTime startTime = 
        timeStart = OSGetTime();
        timeStop = OSGetTime() + OSMillisecondsToTicks(hackedFileOffset);
		timerFromStart = true;
        timerStarted = true;
    }
    instance->firstStart();
    if(gSpeedrunSetting.getInt() == 1) {
		TFlagManager::smInstance->setBool(true, 0x10392);
		TFlagManager::smInstance->setBool(true, 0x10393);
		TFlagManager::smInstance->setBool(true, 0x10394);
		TFlagManager::smInstance->setBool(true, 0x103A3);
		TFlagManager::smInstance->setBool(true, 0x103A4);
		TFlagManager::smInstance->setBool(true, 0x1039e);
		/*TFlagManager::smInstance->Type3Flag.mPlaneCrashWatched = true;
		TFlagManager::smInstance->Type3Flag.mCourtWatched = true;
		TFlagManager::smInstance->Type3Flag._02 = true;*/
	}
}
SMS_PATCH_BL(SMS_PORT_REGION(0x801642e0, 0, 0, 0), TFlagManager_firstStart_override);


static u32 gTimerDataFlags = 0x60004;
static u32 gMagicBits = 0xc00c00;
void TFlagManager_save_write_override(JSUMemoryOutputStream* stream, s32 length, signed char unk) {
	stream->skip(length, unk);

	u32 flags = (timerFromStart | (timerStarted << 0x1) | (timerStopped << 0x2));
	stream->writeData(&gMagicBits, 4);
	stream->writeData(&flags, 4);
	// Save started time
	OSTime anchor = 1;
	u32 flagIndex = gTimerDataFlags;
	stream->writeData(&timeStart, 8);
	stream->writeData(&timeStop, 8);

	//param1->write(buffer, size);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802939cc, 0, 0, 0), TFlagManager_save_write_override);

void TFlagManager_loadOption_override(TFlagManager* flagManager, JSUMemoryInputStream& stream) {
	//flagManager->correctFlag();

	flagManager->load(stream);

	s32 pos = stream.getPosition();

	u32 magic;
	stream.readData(&magic, 4);

	if(magic == gMagicBits) {
		u32 flags;
		stream.readData(&flags, 4);
		
		timerFromStart = ((flags >> 0x0) & 0x1) != 0;
		timerStarted = ((flags >> 0x1) & 0x1) != 0;
		timerStopped = ((flags >> 0x2) & 0x1) != 0;

		stream.readData(&timeStart, 8);
		stream.readData(&timeStop, 8);

	} else {
		stream.seekPos(pos, BEGIN);
	}
	

	return;
}

SMS_PATCH_BL(SMS_PORT_REGION(0x80164de4, 0, 0, 0), TFlagManager_loadOption_override);
SMS_PATCH_BL(SMS_PORT_REGION(0x8016ce1c, 0, 0, 0), TFlagManager_loadOption_override);

//
//void TCardManager_readBlock_start_override(TCardManager* cardManager, u32 block) {
//    if(gSpeedrunSetting.getInt() == 1) {
//        timerStopped = false;
//        timeStop = OSGetTime();
//        timeStart = OSGetTime();
//        timerStarted = true;
//    }
//    cardManager->readBlock(block);
//    if(gSpeedrunSetting.getInt() == 1) {
//		TFlagManager::smInstance->setBool(true, 0x10392);
//		TFlagManager::smInstance->setBool(true, 0x10393);
//		TFlagManager::smInstance->setBool(true, 0x10394);
//		TFlagManager::smInstance->setBool(true, 0x103A3);
//		TFlagManager::smInstance->setBool(true, 0x103A4);
//		TFlagManager::smInstance->setBool(true, 0x1039e);
//	}
//}
//SMS_PATCH_BL(SMS_PORT_REGION(0x801642f4, 0, 0, 0), TCardManager_readBlock_start_override);

void TBathtub_startDemo_override(void* bathtub) {
    if(gSpeedrunSetting.getInt() == 1) {
        timeStop = OSGetTime() + OSMillisecondsToTicks(hackedFileOffset);
        timerStopped = true;
    }
    startDemo__8TBathtubFv(bathtub);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x801fc090, 0, 0, 0), TBathtub_startDemo_override);

void stopTimer() {
    timerStopped = true;
}


pp::auto_patch static gDisableRedCoinSwitch(0x801c0910, 0x60000000, false);

static s32 checkForSettingsMenu(TMarDirector *director) {
    s32 ret = director->changeState();
    if (director->mAreaID == 15 && director->mEpisodeID == 0) {
        if (gpCubeCamera->getInCubeNo(*(Vec *)gpMarioPos) > 0) {
            TSMSFader *fader = gpApplication.mFader;
            if (fader->mFadeStatus == TSMSFader::FADE_OFF) {
                fader->startFadeoutT(0.4f);
            } else if (fader->mFadeStatus == TSMSFader::FADE_ON) {
                ret = 10;
            }
        }
    }

    if(gSpeedrunSetting.getInt() == SMSCoop::SpeedrunSetting::PRACTICE && !SMS_isOptionMap__Fv()) {


        if(director->mGamePads[0]->mButtons.mInput & JUTGamePad::EButtons::DPAD_DOWN && director->mGamePads[1]->mButtons.mInput & JUTGamePad::EButtons::DPAD_DOWN) {
            timerStarted = false;
            timerStopped = false;
            gDisableRedCoinSwitch.set_enabled(!(director->mGamePads[0]->mButtons.mInput & JUTGamePad::X));
            return TApplication::CONTEXT_DIRECT_LEVEL_SELECT;
        }
        s32 result = director->changeState();

        // Reload level if in practice mode
        if(result != TApplication::CONTEXT_DIRECT_MAIN_LOOP && SMSCoop::isShineGot()) {
			gpApplication.mCurrentScene.set(gpMarDirector->mAreaID, gpMarDirector->mEpisodeID, gpApplication.mCurrentScene.mFlag);
			gpApplication.mNextScene.set(gpMarDirector->mAreaID, gpMarDirector->mEpisodeID, gpApplication.mNextScene.mFlag);
            gDisableRedCoinSwitch.set_enabled(!(director->mGamePads[0]->mButtons.mInput & JUTGamePad::X));
        }

        if(director->mGamePads[0]->mButtons.mInput & JUTGamePad::EButtons::DPAD_UP && director->mGamePads[1]->mButtons.mInput & JUTGamePad::EButtons::DPAD_UP) {
            timerStarted = false;
            timerStopped = false;
            gDisableRedCoinSwitch.set_enabled(!(director->mGamePads[0]->mButtons.mInput & JUTGamePad::X));
            gpApplication.mCurrentScene.set(director->mAreaID, director->mEpisodeID, gpApplication.mCurrentScene.mFlag);
            gpApplication.mNextScene.set(director->mAreaID, director->mEpisodeID, gpApplication.mNextScene.mFlag);
             return TApplication::CONTEXT_DIRECT_STAGE;
        }
        if(SMSCoop::isMarioSet()) {
            if(!timerStarted) {
                timeStart = OSGetTime();
                timerStarted = true;
                timerStopped = false;
                //startAppearTimer__11TGCConsole2Fil(console, 0, 0);
            }
        } else {
            timerStarted = false;
            timerStopped = false;
        }
    }

    return ret;
}

SMS_PATCH_BL(SMS_PORT_REGION(0x80299D0C, 0, 0, 0), checkForSettingsMenu);


SMS_WRITE_32(SMS_PORT_REGION(0x8029D0BC, 0x80294f98, 0, 0), 0x60000000);

// Extend Exception Handler
SMS_WRITE_32(SMS_PORT_REGION(0x802C7638, 0x802bf6cc, 0, 0), 0x60000000);  // gpr info

// Disable hiding game ui from better sms
SMS_WRITE_32(SMS_PORT_REGION(0x80140844, 0, 0, 0), 0x548007ff);


void setDebug(TApplication *application) {
    BetterSMS::setDebugMode(true);
}


static void initModule() {

    OSReport("Initializing Coop Module...\n");
    gSettingsGroup.addSetting(&gCameraTypeSetting);
    gSettingsGroup.addSetting(&gPlayer1TypeSetting);
    gSettingsGroup.addSetting(&gPlayer2TypeSetting);
    gSettingsGroup.addSetting(&gShineGrabDistanceSetting);
    gSettingsGroup.addSetting(&gSpeedrunSetting);
    gSettingsGroup.addSetting(&gExplain1);
    gSettingsGroup.addSetting(&gExplain2);
    gSettingsGroup.addSetting(&gExplain3);

    {
        auto &saveInfo        = gSettingsGroup.getSaveInfo();
        saveInfo.mSaveName    = Settings::getGroupName(gSettingsGroup);
        saveInfo.mBlocks      = 1;
        saveInfo.mGameCode    = 'GMSB';
        saveInfo.mCompany     = 0x3031;  // '01'
        saveInfo.mBannerFmt   = CARD_BANNER_CI;
        saveInfo.mBannerImage = reinterpret_cast<const ResTIMG *>(gSaveBnr);
        saveInfo.mIconFmt     = CARD_ICON_NONE;
        saveInfo.mIconSpeed   = CARD_SPEED_SLOW;
        saveInfo.mIconCount   = 0;
        saveInfo.mIconTable   = reinterpret_cast<const ResTIMG *>(gSaveIcon);
        saveInfo.mSaveGlobal  = false;
    }
    
    BetterSMS::registerModule(&sModuleInfo);

    // TODO: Remove me when making new better sms slim version
    BetterSMS::deregisterModule(BetterSMS::getModuleInfo("Better Sunshine Engine"));

    //// Register callbacks
    
    BetterSMS::Game::registerBootCallback("Coop_debug_mode", setDebug);
    BetterSMS::Stage::registerInitCallback("SMSCoop_initCharacterArchivesCoop", SMSCoop::initCharacterArchives);
    BetterSMS::Stage::registerInitCallback("SMSCoop_talking_reset", SMSCoop::resetTalking);
    BetterSMS::Stage::registerInitCallback("SMSCoop_splitscreen_reset", SMSCoop::resetSplitScreen);
    BetterSMS::Stage::registerInitCallback("SMSCoop_ai_reset", SMSCoop::resetAi);
    BetterSMS::Stage::registerInitCallback("SMSCoop_initPvp", SMSCoop::initPvp);
    BetterSMS::Stage::registerUpdateCallback("SMSCoop_updatePvp", SMSCoop::updatePvp);
    BetterSMS::Stage::registerInitCallback("SMSCoop_subArea_reset", SMSCoop::resetUpSubArea);
    BetterSMS::Stage::registerUpdateCallback("SMSCoop_updateTalking", SMSCoop::updateTalking);
    BetterSMS::Stage::registerUpdateCallback("SMSCoop_updateYoshi", SMSCoop::updateYoshi);
    BetterSMS::Game::registerBootCallback("SMSCoop_initTimer", initTimer);
    BetterSMS::Game::registerLoopCallback("SMSCoop_updateTimer", updateTimer);
    //BetterSMS::Game::registerPostDrawCallback("SMSCoop_drawTimer", drawTimer);
    BetterSMS::Stage::registerUpdateCallback("SMSCoop_updateShine", SMSCoop::updateShineTimer);
    BetterSMS::Stage::registerUpdateCallback("SMSCoop_updateSurfGesso", SMSCoop::updateSurfGesso);
    BetterSMS::Stage::registerInitCallback("SMSCoop_resetShineLogic", SMSCoop::resetShineLogic);
    BetterSMS::Stage::registerInitCallback("SMSCoop_setPlayerSkin", SMSCoop::setPlayerSkin);
    SMSCoop::setSkinForPlayer(0, "/data/mario.arc", false, 0, 0);
    SMSCoop::setSkinForPlayer(1, "/data/luigi.arc", false, 1, 1);

    // Display warning in game if memory not expanded
    //if(!isMemoryExpanded()) {
    //    my_patch.enable();
    //}
}

static void deinitModule() {
    OSReport("Deinitializing Coop Module...\n");
    BetterSMS::Game::deregisterBootCallback("Coop_debug_mode");
    BetterSMS::Stage::deregisterInitCallback("SMSCoop_initCharacterArchivesCoop");
    BetterSMS::Stage::deregisterInitCallback("SMSCoop_talking_reset");
    BetterSMS::Stage::deregisterInitCallback("SMSCoop_splitscreen_reset");
    BetterSMS::Stage::deregisterInitCallback("SMSCoop_ai_reset");
    BetterSMS::Stage::deregisterInitCallback("SMSCoop_subArea_reset");
    BetterSMS::Stage::deregisterUpdateCallback("SMSCoop_updateTalking");
    BetterSMS::Stage::deregisterUpdateCallback("SMSCoop_updateYoshi");
    BetterSMS::Game::deregisterBootCallback("SMSCoop_initTimer");
    BetterSMS::Game::deregisterLoopCallback("SMSCoop_updateTimer");
    //BetterSMS::Game::deregisterPostDrawCallback("SMSCoop_drawTimer");
    BetterSMS::Stage::deregisterUpdateCallback("SMSCoop_updateShine");
    BetterSMS::Stage::deregisterUpdateCallback("SMSCoop_updateSurfGesso");
    BetterSMS::Stage::deregisterInitCallback("SMSCoop_resetShineLogic");
    BetterSMS::Stage::deregisterInitCallback("SMSCoop_initPvp");
    BetterSMS::Stage::deregisterUpdateCallback("SMSCoop_updatePvp");
    BetterSMS::Stage::deregisterUpdateCallback("SMSCoop_setPlayerSkin");

}

// Definition block
KURIBO_MODULE_BEGIN("SMS Coop", "theAzack9", "v1.0") {
    // Set the load and unload callbacks to our registration functions
    KURIBO_EXECUTE_ON_LOAD { 
        initModule(); 
    }
    KURIBO_EXECUTE_ON_UNLOAD { deinitModule(); }
}
KURIBO_MODULE_END()
