#include <arduino.h>

#define PIN_HEARTBEAT A5

//-----------------------------------------------------------------------------------------------
void HSV2RGB(int h, int s, int v, int& r, int& g, int& b){
  int deg = h * 360 / 1024;
  if(deg <= 60 || 300 < deg){
    r = 255;
    if(deg <= 60){
      b = 0;
      g =  255 * deg / 60;
    }else{
      b = 255 - (deg -300) * 255 / 60;
      g = 0;
    }
  }
  else if(60 < deg && deg <= 180){
    g = 255;
    if(deg < 120){
      b = 0;
      r = 255 - (deg-60) * 255 / 60;
    }else{
      b = (deg-120) * 255 / 60;
      r = 0;
    }
    
  }
  else{
    b = 255;  
    if(deg < 240){
      r = 0;
      g = 255 - (deg-180) * 255 / 60;
    }else{
      r = 255 * (deg-240) / 60;
      g = 0;
    }  
  }
}


//-----------------------------------------------------------------------------------------------

int HeartBeatSetup(){
  //pinMode(PIN_HEARTBEAT, INPUT);
  analogReadResolution(10);   
}

int HeartBeatLoop(){
  
  int  h = analogRead(PIN_HEARTBEAT);
  
//  int r,g,b;
//  HSV2RGB(h,s,v,r,g,b);
//
//  analogWrite(PIN_LEDR, r);
//  analogWrite(PIN_LEDG, g);
//  analogWrite(PIN_LEDB, b);

  return h;
}
