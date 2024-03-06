//@Felliph-Silva
//March 2024

#include <Arduino.h>
#include <time.h>

#define Difficulty_level 7

uint8_t Led[] = {13, 12, 11, 10};
uint8_t button[] = {3, 2, 1, 0};
uint8_t size_sequence_Led = 0;
uint8_t size_sequence_button = 0;
uint8_t round_game = 0;
uint8_t *Led_sequence = NULL; //Storage the Led sequence and button sequence by dynamic allocation
uint8_t *button_sequence = NULL;

enum GAME_STATE //Enum structure for game state
{ 
  START,
  READY,
  WAITING,
  NEXT_ROUND,
  SUCESSFULL_END,
  FAILLURE_END
};

bool compare(uint8_t *sequence_led, uint8_t *sequence_button) //Function to compare if the sequence of led is equal to the sequence of button;
{
  if(size_sequence_button != size_sequence_Led)
  {
    return false;
  }

  for (uint8_t i = 0; i < size_sequence_button; i++)
  {
    if (sequence_led[i]!= sequence_button[i])
    {
      return false;
    }
  }
  return true;
}

void turn_on_Led(uint8_t Led[], uint8_t num) //Function to turn on certain LED based in his position on array.
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

void reset_sequence(uint8_t *sequence) //Clear sequence
{
  sequence = NULL; //Turn the sequence pointer null
}

void game_over() //Blink the leds in game over
{
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
      size_sequence_button++;
      add_to_sequence(button_sequence, size_sequence_button, button[i]);
      while(digitalRead(button[i]) == LOW)
      {

      }
    }
  }
  delay(100);
}

GAME_STATE game_state() //Function that returns the game state
{
  if (round_game == 0)
  {
    return START;
  }
  else if (round_game == size_sequence_Led)
  {
    return READY;
  }

  else if (size_sequence_button < size_sequence_Led)
  {
    return WAITING;
  }

  else if (size_sequence_button == size_sequence_Led)
  {
    if (compare(Led_sequence, button_sequence))
    {
      if (round_game == Difficulty_level)
      {
        return SUCESSFULL_END;
      }
      return NEXT_ROUND;
    }
    else return FAILLURE_END;
  }
  return FAILLURE_END;
}

void setup() {

  Serial.begin(9600);
  for(int i = 0; i < 4; i++) //Initit the pins
  {
    pinMode(Led[i], OUTPUT);
    pinMode(button[i], INPUT_PULLUP);
    digitalWrite(Led[i], LOW);
  }
}

void loop() {
  srand(time(NULL));

  switch (game_state())
  {
  case START:
    for (uint8_t i = 0; i < 4; i++)
    {
      while(digitalRead(button[i])==LOW)
      {

      }
    }
    round_game++;
    Led_sequence++;
    delay(100);
    break;

  case READY:
    add_to_sequence(Led_sequence, size_sequence_Led, rand() % 4);
    for (uint8_t i = 0; i < size_sequence_Led; i++)
    {
      turn_on_Led(Led, Led_sequence[i]);
      delay(500);
    }
    break;
  
  case WAITING:
  pressing_button();

    break;

  case NEXT_ROUND:
    size_sequence_Led++;
    size_sequence_button = 0;
    reset_sequence(button_sequence);
    round_game++;
    for (uint8_t i = 0; i < 3; i++)
      {
      turn_on_Led(Led, 0);
      }
    break;
  
  case SUCESSFULL_END:
    size_sequence_Led = 0;
    round_game = 0;
    reset_sequence(button_sequence);
    reset_sequence(Led_sequence);
    for (uint8_t i = 0; i < 3; i++)
      {
      turn_on_Led(Led, 0);
      delay(500);
      }
    break;
  
  case FAILLURE_END:
    round_game = 0;
    size_sequence_Led = 0;
    reset_sequence(button_sequence);
    reset_sequence(Led_sequence);
    game_over();
    break;

  default:
    break;
  }
}
