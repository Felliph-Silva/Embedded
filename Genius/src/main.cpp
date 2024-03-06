//@Felliph-Silva
//March 2024

#include <Arduino.h>

#define Difficulty_level 7

//Redefine function to satatic memory;
uint8_t Led[] = {13, 12, 11, 10};
uint8_t button[] = {5, 4, 3, 2};
uint8_t round_game = 0;

uint8_t Led_sequence[Difficulty_level]; //Storage the Led sequence and button sequence by dynamic allocation
uint8_t button_sequence[Difficulty_level];


bool compare() //Function to compare if the sequence of led is equal to the sequence of button;
{

  for (uint8_t i = 0; i < round_game; i++)
  {
    if (Led_sequence[i]!= button_sequence[i])
    {
      return false;
    }
  }
  return true;
}

void turn_on_Led(uint8_t num) //Function to turn on certain LED based in his position on array.
{
  for (uint8_t i = 0; i < 4; i++)
  {
    if(i == num)
    {
      pinMode(Led[i], HIGH);
    }
    else pinMode(Led[i], LOW);
  }
  delay(1000);
}

void add_new_sequence()
{
  Led_sequence[round_game] = random(0,4);
}



void add_to_sequence(uint8_t *sequence, uint8_t size, int num) //Add a new value to array of sequence
{
  uint8_t *ptr = sequence;
  sequence = new uint8_t[size+1];

  for (uint8_t i = 0; i < size; i++)
  {
    sequence[i] = ptr[i];
  }
  sequence[size+1] = num;
  delete [] ptr;
}

/*
void reset_sequence(uint8_t sequence[]) //Clear sequence
{


}
*/

void game_over() //Blink the leds in game over
{
  round_game= 0;
  bool state = true;
  for (uint8_t j = 0; j < 4; ++j)
  {
    for (uint8_t i = 0; i < 4; i++)
    {
      digitalWrite(Led[i], state);
    }
    delay(500);
    state =!state;
  }
}

void pressing_button() //Function that saves button pressed by the user
{
  for (uint8_t i = 0; i < 4; i++)
  {
    if (digitalRead(button[i]) == LOW)
    {
      add_to_sequence(button_sequence, button[i]);
      while(digitalRead(button[i]) == LOW)
      {
        turn_on_Led(i);
      }
    }
  }
  delay(100);
}

void setup() {

  Serial.begin(9600);
  for(int i = 0; i < 4; i++) //Initit the pins
  {
    pinMode(Led[i], OUTPUT);
    pinMode(button[i], INPUT_PULLUP);
    digitalWrite(Led[i], LOW);
  }
  randomSeed(analogRead(0));
}

void loop() {

if (round == 0)
  {
    for (uint8_t i = 0; i < 4; i++)
    {
      turn_on_Led(0);
    }
    delay(1000);
  }
}
