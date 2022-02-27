#define LED_PIN 11

float TUNR_OFF_INTERVAL = 1000;
float TUNR_ON_INTERVAL = 1000;

//-------------------------------------------
void turnOnLED(){
  float startTime = millis();
  float dTime = 0;
  float val  = 0;
  do{
    dTime = millis() - startTime;
    val = abs(255 * sin(PI *1.5 *  dTime/ TUNR_ON_INTERVAL ));
    analogWrite(LED_PIN,val);
    //delay(100);  
  }while(dTime < TUNR_ON_INTERVAL); 

  //on
  if(val < 0){analogWrite(LED_PIN, 255);}
}
void turnOffLED(){
  float startTime = millis();
  float dTime = 0;
  float val  = 255;
  do{
    dTime = millis() - startTime;
    val = 255 * cos(PI * 0.5 * dTime/ TUNR_OFF_INTERVAL );
    analogWrite(LED_PIN,val);
    //delay(100);  
  }while(dTime < TUNR_OFF_INTERVAL); 
  //off
  if(val < 0){analogWrite(LED_PIN, 0);}
}


//--------------------------------------------
void setup() {
  // put your setup code here, to run once:
  pinMode(LED_PIN, OUTPUT);

  //Serial.begin(9600);
  //Serial.println("start");
}

void loop() {
  // put your main code here, to run repeatedly:
  turnOnLED();
  //Serial.println("turn on");
  delay(2000);
  turnOffLED();
  Serial.println("turn off");
  delay(2000);
}
