//Calibration
//valeur mesurée dans l'air : 846 au maxi / mais en régle générale 844
//valeur mesurée dans l'eau : 468 au mini 

void setup() {
  Serial.begin(9600); // open serial port, set the baud rate as 9600 bps
  //Capactive
  pinMode(D3,OUTPUT); //GND
  pinMode(D4,OUTPUT); //3,3V
  digitalWrite(D3,LOW);
  digitalWrite(D4,LOW);
  //Resistive
  pinMode(D6,OUTPUT); //GND
  pinMode(D7,OUTPUT); //3,3V
  digitalWrite(D6,LOW);
  digitalWrite(D7,LOW);  
}
void loop() {
  int val;
  //Capacitive
  digitalWrite(D4,HIGH);
  delay(10);
  val = analogRead(0); 
  delay(10);
  val = analogRead(0); 
  Serial.print("C : ");
  Serial.print(val); 
  digitalWrite(D4,LOW);
  delay(100);
  
  //Resistive
  digitalWrite(D7,HIGH);
  delay(10);
  val = analogRead(0); 
  delay(10);
  val = analogRead(0); 
  Serial.print(" R : ");
  Serial.println(val); 
  digitalWrite(D7,LOW);
  delay(1000);
}
