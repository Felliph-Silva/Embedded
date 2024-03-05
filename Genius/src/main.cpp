//@Felliph-Silva
//March 2024

#include <Arduino.h>
#include <time.h>

#define Difficulty_level 7

uint8_t Led[] = {0, 1, 2, 3};
uint8_t button[] = {4, 5, 6, 7};


bool compare(uint8_t *sequence_led, uint8_t  *sequence_button, uint8_t times) //Function to compare if the sequence of led is equal to the sequence of button;
{
  for (uint8_t i = 0; i < times; i++)
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
      (Led[i], HIGH);
    }
    else((Led[i], LOW));
  }
}

void add_to_sequence(uint8_t *sequence, uint8_t size, uint8_t num) //Add a new value to array of sequence
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

void setup() {
  for(int i = 0; i < 4; i++)
  {
    pinMode(Led[i], OUTPUT);
    pinMode(button[i], INPUT_PULLUP);
    digitalWrite(Led[i], LOW);
  }
  uint8_t *Led_sequence = NULL; //Storage the Led sequence and button sequence by dynamic allocation
  uint8_t *button_sequence = NULL;
}

void loop() {
  srand(time(NULL));

  uint8_t rdm = rand() % 4;
  
  
  delay(100);
}
