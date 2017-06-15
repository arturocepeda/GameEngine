
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Core
//
//  --- GEParser.cpp ---
//
//////////////////////////////////////////////////////////////////

#include "GEParser.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

using namespace GE;
using namespace GE::Core;

int Parser::parseInt(const char* StrValue)
{
   return atoi(StrValue);
}

uint Parser::parseUInt(const char* StrValue)
{
   return (uint)atoi(StrValue);
}

float Parser::parseFloat(const char* StrValue)
{
   return (float)strtod(StrValue, 0);
}

bool Parser::parseBool(const char* StrValue)
{
   return strcmp(StrValue, "true") == 0;
}

byte Parser::parseByte(const char* StrValue)
{
   return (byte)atoi(StrValue);
}

Vector2 Parser::parseVector2(const char* StrValue)
{
   Vector2 vVector;
   sscanf(StrValue, "x:%f y:%f", &vVector.X, &vVector.Y);
   return vVector;
}

Vector3 Parser::parseVector3(const char* StrValue)
{
   Vector3 vVector;
   sscanf(StrValue, "x:%f y:%f z:%f", &vVector.X, &vVector.Y, &vVector.Z);
   return vVector;
}

Color Parser::parseColor(const char* StrValue)
{
   Color sColor;
   sscanf(StrValue, "r:%f g:%f b:%f a:%f", &sColor.Red, &sColor.Green, &sColor.Blue, &sColor.Alpha);
   return sColor;
}

void Parser::writeInt(int Value, char* OutStr)
{
   sprintf(OutStr, "%d", Value);
}

void Parser::writeUInt(uint Value, char* OutStr)
{
   sprintf(OutStr, "%d", Value);
}

void Parser::writeFloat(float Value, char* OutStr)
{
   sprintf(OutStr, "%.3f", Value);
}

void Parser::writeBool(bool Value, char* OutStr)   
{
   strcpy(OutStr, Value ? "true" : "false");
}

void Parser::writeByte(byte Value, char* OutStr)
{
   sprintf(OutStr, "%d", (int)Value);
}

void Parser::writeVector2(const Vector2& Value, char* OutStr)
{
   sprintf(OutStr, "x:%.3f y:%.3f", Value.X, Value.Y);
}

void Parser::writeVector3(const Vector3& Value, char* OutStr)
{
   sprintf(OutStr, "x:%.3f y:%.3f z:%.3f", Value.X, Value.Y, Value.Z);
}

void Parser::writeColor(const Color& Value, char* OutStr)
{
   sprintf(OutStr, "r:%.3f g:%.3f b:%.3f a:%.3f", Value.Red, Value.Green, Value.Blue, Value.Alpha);
}
