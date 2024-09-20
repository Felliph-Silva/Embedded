//@Felliph-Silva
//March 2024
//Genius Game
#include <Arduino.h>

#define Difficulty_level 7
#define NUM_LEDS 4

//Redefine function to satatic memory;
uint8_t Led[NUM_LEDS] = {13, 12, 11, 10};
uint8_t button[NUM_LEDS] = {5, 4, 3, 2};
uint8_t round_game = 0; //Variable to count rounds
uint8_t button_count = 0; //Counts how many buttons were pressed
bool showed = false; //Variable to indicate if the LED sequence is showed to user

uint8_t Led_sequence[Difficulty_level]; //Storage the LED and Button sequence
uint8_t button_sequence[Difficulty_level];

uint8_t compare(); //Function to compare if the sequence of led is equal to the sequence of button;
void flash_Led(uint8_t num, int time = 1000); //Function to flash a LED based in his position, ad set the duration he turn on.
void add_new_sequence(); //Add a random number to led sequence, based on NUM_LEDS
void save_tap(uint8_t i); //Save the position of Button in the button sequence
void game_over(bool finished); //Reset round in game over, false is fail and true is success
void playing(); //Function that saves buttons pressed by the user and flashes the corresponding LED

void setup() {

  for(int i = 0; i < NUM_LEDS; i++) //Initit the pins
  {
    pinMode(Led[i], OUTPUT);
    pinMode(button[i], INPUT_PULLUP);
    digitalWrite(Led[i], LOW);
  }
  randomSeed(analogRead(0));
}

void loop() {

if (round_game == 0) //Verify if the game is in startup mode
  {
    for (uint8_t i = 0; i < NUM_LEDS; i++)
    {
      flash_Led(i,300);
      delay(300);
    }
    add_new_sequence();
    round_game++;
    delay(500);
  }

    if (round_game > Difficulty_level) //End game condition
  {
    game_over(true);
  }

  if (round_game > 0 && round_game <= Difficulty_level) //Game running conditions
  {
    if (button_count == 0 && showed != true)
    {
      for (uint8_t i = 0; i < round_game; i++)
      {
        flash_Led(Led_sequence[i]);
        delay(500);
      }
      showed = !showed;
    }

    playing();

    if (button_count == round_game)
    {
      compare();
      showed = !showed;
    }
  }
}

uint8_t compare() 
{
  for (uint8_t i = 0; i < round_game; i++)
  {
    if (Led_sequence[i]!= button_sequence[i])
    {
      for (uint8_t i = 0; i < 3; i++)
      {
        flash_Led(3, 300);
        delay(300);
      }      
      game_over(false);
      return 0;
    }
  }

  for (uint8_t i = 0; i < 3; i++)
  {
    flash_Led(0, 300);
    delay(300);
  }
  round_game++;
  button_count = 0;
  add_new_sequence();
  return 0;
}

void flash_Led(uint8_t num, int time)
{
  digitalWrite(Led[num], HIGH);
  delay(time); //Then implement this whit millis
  digitalWrite(Led[num], LOW);
}

void add_new_sequence()
{
  Led_sequence[round_game] = random(0,NUM_LEDS);
}

void save_tap(uint8_t i)
{
  button_sequence[button_count] = i;
  button_count++;
  delay(100);
}

void game_over(bool finished)
{
  if(finished == true)
  {
    for (uint8_t j = 0; j < 2; j++)
    {
      for (uint8_t i = 0; i < NUM_LEDS; i++){
        flash_Led(i, 300);
        delay(300);
    }
    }
  }

  if (finished == false)
  {
    for (uint8_t i = 0; i < 3; i++)
    {
    flash_Led(3,300);
    delay(300);
    }
  }
round_game = 0;
button_count = 0;
}

void playing()
{
  for (uint8_t i = 0; i < NUM_LEDS; i++)
  {
    if (digitalRead(button[i]) == LOW)
    {
      save_tap(i);
      flash_Led(i, 400);
    }
  }
}
