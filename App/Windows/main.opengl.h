
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Windows (OpenGL)
//
//  --- main.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

void render();

void keyboardDown(unsigned char key, int x, int y);
void keyboardUp(unsigned char key, int x, int y);

void mouseButton(int button, int state, int x, int y);
void mouseMove(int x, int y);
