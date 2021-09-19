//#include <vector.h>
#include <stdio.h>
#include <string.h>

typedef enum { //
  BAND_PASS_ECG,
  BAND_PASS_PRESSURE,
  MOVING_AVERAGE
} E_sensor_mode;

class MyFilter {
  private:
    int len_buf = 0;
    //    std::vector = buf;
    double *buf_data;
    E_sensor_mode sensorMode;
    //0:heart rate
    //1:pressure
    int val_filter = 0;

    std::vector<double> mBuf;
    int mSampleN = 20;

  public:
    void mySetup(int input_len_buf, E_sensor_mode smode) {
      len_buf = input_len_buf;
      sensorMode = smode;
      buf_data =  (double *)malloc(sizeof(double) * input_len_buf);
      int size_buf = sizeof(buf_data);
      memset(buf_data, 0, size_buf);
      Serial.print("buf size=");
      Serial.println(size_buf);
      return;
    }

    void setRmsN(int val){
      mSampleN = val;
    }
    
    void updateFifo(int input_data) {
      //stack data
      //      int buf[sizeof(len_buf)];
      int size_buf = len_buf;
      for (int i = 0; i < size_buf - 1; i++) {
        int tmp_data = buf_data[size_buf - i - 2];
        //update buffer
        buf_data[size_buf - i - 1] = tmp_data;
        //        Serial.print("buf_size[");
        //        Serial.print(size_buf - i);
        //        Serial.print("] = ");
        //        Serial.println(sizeof(buf_data));
        //        Serial.print("buf_data[");
        //        Serial.print(size_buf - i);
        //        Serial.print("] = ");
        //        Serial.println(buf_data[size_buf - i - 1]);
      }
      buf_data[0] = input_data;
      return;
    }
    bool checkReady() {
      bool is_ready = false;
      if (buf_data[len_buf - 1] != 0) {
        is_ready = true;
      }
      return is_ready;
    }
    double filter(const double *filter, unsigned int num_taps) {
      double output = 0;
      for (int i = 0; i < num_taps; i++) {
        output += buf_data[i] * filter[i];
      }
      return output;
    }

    double movingAverage() {
      double output = 0;
      for (int i = 0; i < len_buf; i++) {
        output += (buf_data[i] / len_buf);
      }
      return output ;
    }

    double RMS(double val){
      mBuf.push_back(val);

      while(mBuf.size() > mSampleN){
        mBuf.erase(mBuf.begin());
      }
      
      if(mBuf.size() == mSampleN){
        double sum = 0;
        for_each(mBuf.begin(), mBuf.end(), [&sum](double x){sum += x*x;});

        return sqrt( sum / mSampleN );
      }
      else{
        return val;
      }
    }

    double MaxValue(double val){
      mBuf.push_back(val);

      while(mBuf.size() > mSampleN){
        mBuf.erase(mBuf.begin());
      }
      
      if(mBuf.size() == mSampleN){
        return *std::max_element(mBuf.begin(), mBuf.end());
      }
      else{
        return val;
      }
    }
};
