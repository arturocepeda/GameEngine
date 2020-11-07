
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda PŽrez
//  Game Engine
//
//  macOS
//
//  --- main.h ---
//
//////////////////////////////////////////////////////////////////

struct GLFWwindow;

void keyboard(GLFWwindow* pWindow, int pKey, int pScancode, int pAction, int pMods);
void keyboardText(GLFWwindow* pWindow, unsigned int pCodePoint);

void mouseButton(GLFWwindow* pWindow, int pButton, int pAction, int pMods);
void mouseMove(GLFWwindow* pWindow, double pX, double pY);
void mouseWheel(GLFWwindow* pWindow, double pXOffset, double pYOffset);
