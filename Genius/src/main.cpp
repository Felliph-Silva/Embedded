//@Felliph-Silva
//March 2024

#include <Arduino.h>

#define Difficulty_level 7

//Redefine function to satatic memory;
uint8_t Led[] = {13, 12, 11, 10};
uint8_t button[] = {5, 4, 3, 2};
uint8_t round_game = 0;
uint8_t button_count = 0;

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

void flash_Led(uint8_t num, int time = 1000) //Function to turn on certain LED based in his position on array.
{
  for (uint8_t i = 0; i < 4; i++)
  {
    if(i == num)
    {
      pinMode(Led[i], HIGH);
    }
    else pinMode(Led[i], LOW);
  }
  delay(time);

    for (uint8_t i = 0; i < 4; i++)
    {
      pinMode(Led[i], LOW);
    }

}

void add_new_sequence() //Add a random number to led sequence
{
  Led_sequence[round_game] = random(0,4);
}

void save_tap(uint8_t i) //Save the position of Button in the button sequence
{
  button_sequence[button_count] = i;
  button_count++;
}

void game_over(bool finished) //Reset round in game over
{
  if(finished == true)
  {
    for (uint8_t i = 0; i < 3; i++)
    flash_Led(0); 
  }

  if (finished == true)
  {
    for (uint8_t i = 0; i < 3; i++)
    flash_Led(3); 
  }
    round_game = 0;
    button_count = 0;
}

void playing() //Function that saves button pressed by the user
{
  for (uint8_t i = 0; i < 4; i++)
  {
    if (digitalRead(button[i]) == LOW)
    {
      save_tap(i);
      while(digitalRead(button[i]) == LOW)
      {
        //Implement function to check the time pressing the button "Pressing", with millis;
        flash_Led(i);
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

if (round_game == 0)
  {
    for (uint8_t i = 0; i < 4; i++)
    {
      flash_Led(i);
    }
    round_game++;
    add_new_sequence();
  }

if (round_game > 0 && round_game <= Difficulty_level)
{
  for (uint8_t i = 0; i < round_game; i++)
  {
    flash_Led(Led_sequence[i]);
  }

  playing();

  if (button_count == round_game)
  {
    if (compare() == true)
    {
      for (uint8_t i = 0; i < 3; i++)
      {
        flash_Led(0);
      }
      round_game++;
      button_count =0;
    }
    if (compare() == false)
    {
      game_over(false);
    }
  } 
}

if (round_game > Difficulty_level)
{
  game_over(true);
}
}
