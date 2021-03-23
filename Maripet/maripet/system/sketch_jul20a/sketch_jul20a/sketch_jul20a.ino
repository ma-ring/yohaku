#define pin 6
#define pin0 11

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  
  pinMode(pin,OUTPUT);
  pinMode(pin0,OUTPUT);
  analogWrite(pin,0);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(Serial.available() > 0){
    int val = 0;
    int ans = 0;
    int num0 = '0';
    do{
      if(val >= num0){
        ans = ans * 10 +(val - num0); 
      }     
      val = Serial.read();
    }
    while(val != '\r' && val != '\n');
    analogWrite(pin,ans%255);
    analogWrite(pin0,ans%255);
    Serial.println(ans);
    
  }
}
