const int ld=13, button=3;
int menuInput, i, number, result, buttonCount=0, buttonState=0, lastButtonState=0;


void setup() {
 Serial.begin(9600);
 pinMode(ld,OUTPUT);
 //pinMode(button,INPUT);
     
}

 
void loop() {
  
  while(Serial.available() > 0) {

    menuInput = Serial.read(); 
    switch (menuInput){

      case 1:
            
        digitalWrite(ld,HIGH);                     
        break;
            
      case 2:
            
        digitalWrite(ld,LOW); 
        break;
            
      case 3: 
              
        for(int i=0;i<3;i++)
        {
            digitalWrite(13,HIGH);
            delay(996);
            digitalWrite(13,LOW);
            delay(996);
        }
        break;
            
      case 4:
        delay(100);
      
        number =Serial.read();
        result=(number*number);
        Serial.println(result); 
        Serial.flush();
        break;

      case 5:
      buttonCount=0;
      delay(100);
      number=Serial.read();
        for (i=0; i<number; i++) {
          while (1) {
            buttonState = digitalRead(button);
            
   
            if (buttonState != lastButtonState) {
              if (buttonState == HIGH) {
                
                buttonCount++;
                Serial.println(buttonCount);
                lastButtonState = buttonState;
                break;
                
              } 
           
              delay(50);
              
            }
          
            lastButtonState = buttonState;
            
          }
        }
        
     }

  }
  
}
