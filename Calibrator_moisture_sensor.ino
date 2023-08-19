 int dry = 1024;                //1024 (previous reading)
int wet = 465;                 //780 (previous reading)
void setup()
{
  Serial.begin(9600);
  delay(500);
}

void loop() {
  int sensorVal = analogRead(A0);
  Serial.println(sensorVal);
  int percent = map(sensorVal,wet, dry, 100, 0);
  Serial.print(percent);
  Serial.println("%");

}
