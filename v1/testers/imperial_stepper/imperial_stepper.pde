// Yep, this is actually -*- c++ -*-

//x axis pins
#define X_STEP_PIN      15
#define X_DIR_PIN       18
#define X_ENABLE_PIN    19
#define X_MIN_PIN       20
#define X_MAX_PIN       21

//y axis pins
#define Y_STEP_PIN      23
#define Y_DIR_PIN       22
#define Y_ENABLE_PIN    24
#define Y_MIN_PIN       25
#define Y_MAX_PIN       26

//z axis pins
#define Z_STEP_PIN      27
#define Z_DIR_PIN       28
#define Z_ENABLE_PIN    29
#define Z_MIN_PIN       30
#define Z_MAX_PIN       31

//pin for controlling the PSU.
#define PS_ON_PIN       14

void init_psu()
{
#ifdef PS_ON_PIN
  pinMode(PS_ON_PIN, OUTPUT);
  turn_psu_on();
#endif
}

void turn_psu_on()
{
#ifdef PS_ON_PIN
  digitalWrite(PS_ON_PIN, LOW);
  delay(2000); //wait for PSU to actually turn on.
#endif
}

void turn_psu_off()
{
#ifdef PS_ON_PIN
  digitalWrite(PS_ON_PIN, HIGH);
#endif
}

void setup()
{
  Serial.begin(38400);
  Serial.println("You have failed me for the last time.");

  //initialize all our pins.
  pinMode(X_STEP_PIN, OUTPUT);
  pinMode(X_DIR_PIN, OUTPUT);
  pinMode(X_ENABLE_PIN, OUTPUT);
  pinMode(X_MIN_PIN, INPUT);
  pinMode(X_MAX_PIN, INPUT);

  pinMode(Y_STEP_PIN, OUTPUT);
  pinMode(Y_DIR_PIN, OUTPUT);
  pinMode(Y_ENABLE_PIN, OUTPUT);
  pinMode(Y_MIN_PIN, INPUT);
  pinMode(Y_MAX_PIN, INPUT);

  pinMode(Z_STEP_PIN, OUTPUT);
  pinMode(Z_DIR_PIN, OUTPUT);
  pinMode(Z_ENABLE_PIN, OUTPUT);
  pinMode(Z_MIN_PIN, INPUT);
  pinMode(Z_MAX_PIN, INPUT);
  
  digitalWrite(X_MIN_PIN, HIGH);
  digitalWrite(X_MAX_PIN, HIGH);
  digitalWrite(Y_MIN_PIN, HIGH);
  digitalWrite(Y_MAX_PIN, HIGH);
  digitalWrite(Z_MIN_PIN, HIGH);
  digitalWrite(Z_MAX_PIN, HIGH);

  init_psu();
  calculate_tones();
}

void loop()
{
  Serial.println("Forward!");
  digitalWrite(X_DIR_PIN, HIGH);
  digitalWrite(Y_DIR_PIN, HIGH);
  digitalWrite(Z_DIR_PIN, HIGH);
  play_song(X_MAX_PIN, Y_MAX_PIN, Z_MAX_PIN);

  delay(500);

  Serial.println("Reverse!");
  digitalWrite(X_DIR_PIN, LOW);
  digitalWrite(Y_DIR_PIN, LOW);
  digitalWrite(Z_DIR_PIN, LOW);
  play_song(X_MIN_PIN, Y_MIN_PIN, Z_MIN_PIN);

  delay(500);
}

boolean at_switch(byte pin)
{
  return !digitalRead(pin); 
}

#define TONE_COUNT 27

float frequencies[TONE_COUNT] = {
  196.00, //G2   0
  207.65, //G#2   1
  220.00, //A2    2
  233.08, //Bb2   3
  246.94, //B2    4
  261.63, //C3    5
  277.18, //C#3   6
  293.66, //D3    7
  311.13, //D#3   8
  329.63, //E3    9
  349.23, //F3    10
  369.99, //F#3   11
  392.00, //G3    12
  415.30, //G#3   13
  440.00, //A3    14
  466.16, //Bb3   15
  493.88, //B3    16
  523.25, //C4    17
  554.37, //C#4   18
  587.33, //D4    19
  622.25, //D#4   20
  659.26, //E4    21
  698.46, //F4    22
  739.99, //F#4   23
  783.99, //G4    24
  830.61, //G#4   25
  880.00  //A4    26
};


int tones[TONE_COUNT];

#define NOTE_COUNT 66
int notes[] = {
  12,12,12, 8,15,12, 8,15,12,   // 9
  19,19,19,20,15,12, 8,15,12,   // 9
  24,12,12,24,23,22,21,20,21,   // 9
  13,18,17,16,15,14,15,         // 7
   8,11, 8,11,15,12,15,19,      // 8
  24,12,12,24,23,22,21,20,21,   // 9
  13,18,17,16,15,14,15,         // 7
   8,11, 8,15,12, 8,15,12       // 8
};
int lengths[] = {
    4, 4, 4, 3, 1, 4, 3, 1, 8,
    4, 4, 4, 3, 1, 4, 3, 1, 8,
    4, 3, 1, 4, 3, 1, 1, 1, 4,
    2, 4, 3, 1, 1, 1, 4,
    2, 4, 3, 1, 4, 3, 1, 8,
    4, 3, 1, 4, 3, 1, 1, 1, 4,
    2, 4, 3, 1, 1, 1, 4,
    2, 4, 3, 1, 4, 3, 1, 8
};

void calculate_tones()
{
  for (byte i=0; i<TONE_COUNT; i++)
    tones[i] = (int)(1000000.0/ (2.0 * frequencies[i]));
}

void play_song(byte xPin, byte yPin, byte zPin)
{
  digitalWrite(X_ENABLE_PIN, LOW); //enable
  digitalWrite(Y_ENABLE_PIN, LOW); //enable
  digitalWrite(Z_ENABLE_PIN, LOW); //enable

  for (byte i=0; i<NOTE_COUNT; i++)
  {
    if (!at_switch(xPin) && !at_switch(yPin) && !at_switch(zPin))
    {
      play_note(tones[notes[i]], 80000*lengths[i]);
      delay(10); 
    }
  }
  digitalWrite(X_ENABLE_PIN, HIGH); //disable
  digitalWrite(Y_ENABLE_PIN, HIGH); //disable
  digitalWrite(Z_ENABLE_PIN, HIGH); //disable
}

void play_note(int note, long time)
{
  int count = round(time / note);

  for (int i=0; i<count; i++)
  {
    digitalWrite(X_STEP_PIN, HIGH);
    digitalWrite(Y_STEP_PIN, HIGH);
    digitalWrite(Z_STEP_PIN, HIGH);
    delayMicroseconds(note);
    digitalWrite(X_STEP_PIN, LOW);
    digitalWrite(Y_STEP_PIN, LOW);
    digitalWrite(Z_STEP_PIN, LOW);
    delayMicroseconds(note);
  }
}
