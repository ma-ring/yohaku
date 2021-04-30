int prePeriod = 1000;
int nextPeriod = 1000;
int period = 1000;

int updateInterval = 1000;
int preUpdate = 0;

int amp = 120;

int setPeriod(int p){
  int currentTime = millis();
  if(currentTime - preUpdate > updateInterval){
    prePeriod = period;
    nextPeriod = p;
    preUpdate = currentTime;
  }
  
}

int blinkLed(){
  int currentTime = millis();
  
  if(nextPeriod != period){
    period += (nextPeriod - prePeriod) * sin((currentTime - preUpdate) / updateInterval);
  }
  
  return amp * sin(PI * currentTime / period);
}
