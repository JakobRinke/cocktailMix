
// Pumpengeschwindigkeit in ml/s
const double pumpSpeed = 30;

// Größe des Glases in ml
const double cupSize = 400;

const uint8_t buttonPins[] =  {2, 3, 4};

const uint8_t pumpPins[] = {8, 9, 10, 11};

// Recipies 
const double recipies[sizeof(buttonPins)][sizeof(pumpPins)] = {
  { 0.3, 0.0, 0.4, 0.3},
  { 0.0, 0.5, 0.1, 0.4},
  { 0.3, 0.2, 0.0, 0.5},
};


const uint8_t waitPin = 7;


void setup() {
  Serial.begin(9600);
  for(int i = 0; i < sizeof(buttonPins); i++) {
    pinMode(buttonPins[i], INPUT);
  }
  
  for(int i = 0; i < sizeof(pumpPins); i++) {
    pinMode(pumpPins[i], OUTPUT);
  }
  pinMode(waitPin, OUTPUT);

}



int calcFIllTime(int button, int i) {
  return (int) (1000 * recipies[button][i] * cupSize / pumpSpeed);
}

void putInRow(int button) {
  for(int i = 0; i < sizeof(pumpPins); i++) {
    Serial.println(i);
    if (recipies[button][i] > 0) {
         digitalWrite(pumpPins[i], HIGH);
         delay(calcFIllTime(button, i));
         digitalWrite(pumpPins[i], LOW);
    }  
  }
}


void loop() {
  for (int button = 0; button < sizeof(buttonPins); button++) {
    Serial.print("Test: ");
    Serial.println(button);
    if (digitalRead(buttonPins[button]) == LOW) {
      Serial.println("BASED");
      digitalWrite(waitPin, HIGH);
      putInRow(button);
      delay(3000);
      digitalWrite(waitPin, LOW);
      break;
    }
  }
  delay(500);
}
