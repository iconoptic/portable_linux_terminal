/*
*/
int clk = 0;
int red = 0;
int green = 0;
int blue = 0;
int combine;
int frequency = 20;
int microcycle =  (1/frequency)*1000000;
int countr, countg, countb = 0;
int count = 0;
int thresR, thresG, thresB = 0;


// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(9600);
  pinMode(A0,INPUT);
  pinMode(A1,INPUT);
  pinMode(A2,INPUT);
  pinMode(A9,INPUT);
}

// the loop function runs over and over again forever
void loop() {
  clk = analogRead(A9);
  if(clk < 962){
    thresR = analogRead(A0);
    thresG = analogRead(A1);
    thresB = analogRead(A2);
    countr = 0;
    countg = 0;
    countb = 0;
    combine = 0;
    while(clk < 962){
      clk = analogRead(A9);
    }
    delayMicroseconds(microcycle/2);
    delayMicroseconds(microcycle/8);
    red = analogRead(A0);
    green = analogRead(A1);
    blue = analogRead(A2);
    //Serial.print("Red: ");
    //Serial.println(red);
    //Serial.print("Green: ");
    //Serial.println(green);
    Serial.print("Blue: ");
    Serial.println(blue);
    if(red < thresR*0.7){
      countr++;
    }
    if(green < thresG*0.7){
      countg++;
    }
    if(blue < thresB*0.7){
      countb++;
    }
    delayMicroseconds(microcycle/4);
    red = analogRead(A0);
    green = analogRead(A1);
    blue = analogRead(A2);
    //Serial.print("Red: ");
    //Serial.println(red);
    //Serial.print("Green: ");
    //Serial.println(green);
    Serial.print("Blue: ");
    Serial.println(blue);
    if(red < thresR*0.7){
      countr++;
    }
    if(green < thresG*0.7){
      countg++;
    }
    if(blue < thresB*0.7){
      countb++;
    }
    delayMicroseconds(microcycle/4);
    red = analogRead(A0);
    green = analogRead(A1);
    blue = analogRead(A2);
    //Serial.print("Red: ");
    //Serial.println(red);
    //Serial.print("Green: ");
    //Serial.println(green);
    Serial.print("Blue: ");
    Serial.println(blue);
    if(red < thresR*0.7){
      countr++;
    }
    if(green < thresG*0.7){
      countg++;
    }
    if(blue < thresB*0.7){
      countb++;
    }
    
    combine = (countr*16)+(countg*4)+(countb);
    count++;
    Serial.print(count);
    Serial.print("Result: ");
    Serial.println(combine);  
  }

}

