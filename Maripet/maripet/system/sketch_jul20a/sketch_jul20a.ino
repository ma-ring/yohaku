
void setup() {
  // put your setup code here, to run once:
  pinMode(9,OUTPUT);
  analogWrite(9,0);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly
  int num0 = '0';
  int val = 0;
  while(Serial.available() > 0){
      int sum  = 0;
      do{
        if(val >= num0){
          sum  = sum * 10 + val - num0;
        }
        val = Serial.read();      
      }
      while(val != '\n' &&  val != '\r');
      sum %= 255;
      Serial.println(sum);    
      analogWrite(9,sum);
  }

}
