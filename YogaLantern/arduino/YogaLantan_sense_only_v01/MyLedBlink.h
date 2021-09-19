int prePeriod = 1000;
int targetPeriod = 1000;
int period = 1000;

int updateInterval = 5000;
int preUpdate = 0;

static const float TARGET_HR = 1000;
int amp = 120;

unsigned long preTime = 0;
double preEcg = 0;

double HR = 0;
unsigned long preHRtime = 0;
bool hrUpdateFlag = false;

double HR_THRE = 6000;
double HR_MAX = 0;
bool enableHR = false;
double preHR_MAX = 0;
 
std::vector<double> HRbuf;
int HRbufSize = 10;

std::vector<double> HR_MAXbuf;
int HR_MAXbufSize = 10;

void setPeriod(double p, unsigned long currentTime){
    
  if(currentTime - preUpdate > updateInterval){
    prePeriod = period;
    targetPeriod = (HR + TARGET_HR) * 0.5;
    preUpdate = currentTime;
  }
  
}


void setEcg(double val){
  unsigned long currentTime = millis();
  
  // double dEcg = 10 * (val - preEcg) / (currentTime - preTime);
  //Serial.print("ECG ");
  //Serial.println(dEcg);

  if(val > HR_THRE){
    if(hrUpdateFlag){
      //
      HR_MAX = val;
      HR = (currentTime - preHRtime);

      enableHR = (HR > 300);
      
      preHRtime = currentTime;
      hrUpdateFlag = false;

      if(enableHR){
        //moving average
        HRbuf.push_back(HR);
        while(HRbuf.size() > HRbufSize){ HRbuf.erase(HRbuf.begin());}
        if(HRbuf.size() == HRbufSize){ HR =  (std::accumulate(HRbuf.begin(), HRbuf.end(),0.0)) / HRbufSize;}
      }
    }else if(enableHR){
      //最大値を探る
      if(val > HR_MAX){ HR_MAX = val;}
    }
  }
  if(val < HR_THRE * 0.5 && !hrUpdateFlag){
    if(enableHR){
      //5回以上同じMAX値(誤差500)だった場合、MAX*0.8を新たなスレッショルドにする
       HR_MAXbuf.push_back(HR_MAX);
       while(HR_MAXbuf.size() > HR_MAXbufSize){ HR_MAXbuf.erase(HR_MAXbuf.begin());}
       if(HR_MAXbuf.size() == HR_MAXbufSize){ 
        double HR_max_meant =  (std::accumulate(HR_MAXbuf.begin(), HR_MAXbuf.end(),0.0)) / HR_MAXbufSize;
        if(abs(HR_max_meant * 0.7 - HR_THRE) > 1000){HR_THRE = HR_max_meant * 0.5 ;}
       }
      
      //if(HR_MAX_COUNT > 5) HR_THRE = HR_MAX * 0.8;
      preHR_MAX = HR_MAX;
    }
    hrUpdateFlag = true;
  }

  if(currentTime - preHRtime > 5000) {enableHR = false; HR_THRE = 6000;}

  setPeriod(HR,currentTime );  
  Serial.print("THRE = ");
  Serial.print(HR_THRE);
  Serial.print("MAX = ");
  Serial.println(HR_MAX);
  preEcg = val;
  preTime = currentTime;
}

int blinkLed(){
  if(!enableHR) return amp;
  int currentTime = millis();
  
  if(targetPeriod != period){
    float d =  (targetPeriod - prePeriod) * sin((currentTime - preUpdate) / updateInterval);
    if(d * (targetPeriod - period) > 0){
      period += d;
    }
  }
  
  return amp*( 0.5 + sin(PI * currentTime / period));
}
