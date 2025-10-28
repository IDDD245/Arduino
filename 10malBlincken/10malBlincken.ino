int InputPin = 9;
int LEDPin = 11;
int wieOftWurdeTasteGedrueckt = 0;

void setup() {
// put your setup code here, to run once:
Serial.begin(9600);
pinMode(InputPin,INPUT);
pinMode(LEDPin,OUTPUT);
}

void loop() {  
  if (digitalRead(InputPin)) {
    if (wieOftWurdeTasteGedrueckt < 5 ) {
      for(int i=0; i<10; i++) {
        Serial.println("Ich blinke zum: " + String(i+1)+". Mal für Runde: "+String(wieOftWurdeTasteGedrueckt +1));
        digitalWrite(LEDPin,HIGH);
        delay(100);
        digitalWrite(LEDPin,LOW);   
        delay(100);  
      }
    } else {
      Serial.println("Du darfst höchstens 5 Mal drücken");
    }  
    wieOftWurdeTasteGedrueckt++;
  }
}