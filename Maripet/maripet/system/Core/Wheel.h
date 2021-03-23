#define MOTER_GO 0
#define MOTER_RIGHT 1
#define MOTER_LEFT 2
#define MOTER_BACK 3
#define MOTER_STOP 4
#define SPEED_MAX 2000
#define STOP_FADE 1000

class Wheel{
  public:
    void Setup(int, int,int,int,int,int);
    void Setup();

    void Speed(int);

    void setPWM(bool);
    void PWM_Write(int pinR, int pinL,int pwmspeed);
    void setMoveMode(int);
    void Loop();
    int mode = -1;
    
  private:
    bool mode_pwm = false;
    int pwm_speed= 1000;
    int moveMode = -1; //0 go ,1 right, 2 left, 3 stop
    int turnCount ;
    int fadecount = 0;
//    int pinR0;
//    int pinL0;
//    int pinR1;
//    int pinL1;
//    int pinR_pwm;
//    int pinL_pwm;
    void Go();
    void Stop();
    void Back();
    void Right();
    void Left();
    
};
