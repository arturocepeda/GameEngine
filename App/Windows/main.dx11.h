
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Windows (DirectX 11)
//
//  --- main.dx11.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include <windows.h>
#include "Types/GETypes.h"

#define GE_MOUSE_CHECK_MARGIN          8
#define GE_MOUSE_SET_MARGIN            24

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
GE::Vector2 GetMouseScreenPosition();
