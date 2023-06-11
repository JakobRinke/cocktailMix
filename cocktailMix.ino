

const uint8_t buttonPins[] =  {2, 3, 4};

const uint8_t ledPins[] = {8, 9, 10, 11};

const float recipies[sizeof(buttonPins)][sizeof(ledPins)] = {
  { 4, 0, 2, 4},
  { 2, 2, 1, 1},
  { 0, 0, 7, 2},
};


const uint8_t waitPin = 7;

void setup() {
  for(int i = 0; i < sizeof(buttonPins); i++) {
    pinMode(buttonPins[i], INPUT);
  }
  
  for(int i = 0; i < sizeof(ledPins); i++) {
    pinMode(ledPins[i}, OUTPUT);
  }
  pinMode(waitPin, OUTPUT);

}

void loop() {
  for (int button = 0; button < sizeof(buttonPins)) {
    if (digitalRead(buttonPins[button]) == HIGH) {
      break;
    }
  }
  
  // put your main code here, to run repeatedly:
  delay(5)
}
