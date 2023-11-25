#include "commands.h"


bool sendCommand(int code, int responseSize);// Some command have response:  Reading ENDSTOPS
bool sendCommandIFF(int code,float v1,float v2, int responseSize); 

bool setgetValue2F(int code,float v1,float v2, float & out1,float & out2);
bool setValue2F(int code,float v1,float v2);


bool getValue2F(int, float &, float &);
bool getValue3F(int, float &, float &, float &);
bool getValue2I(int, int &, int & );
bool getValue3I(int, int &, int & , int &);
