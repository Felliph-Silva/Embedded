#include <Arduino.h>
#include <time.h>

uint8_t Led[] = {0, 1, 2, 3};
uint8_t buttom[] = {4, 5, 6, 7};

bool compare(uint8_t *sequence_led, uint8_t  *sequence_buttom, uint8_t times) //Function to compare if the sequence of led is equal to the sequence of buttom;
{
  for (uint8_t i = 0; i < times; i++)
  {
    if (sequence_led[i]!= sequence_buttom[i])
    {
      return false;
    }
  }
  return true;
}


void setup() {
  for(int i = 0; i < 4; i++)
  {
    pinMode(Led[i], OUTPUT);
    pinMode(buttom[i], INPUT_PULLUP);
  }
}

void loop() {
  srand(time(NULL));
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}