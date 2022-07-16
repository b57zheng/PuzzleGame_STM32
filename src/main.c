// Code for pattern matcher
// Writen by Kelvin Peng and Bowen Zheng, October 2021

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "PM.h"

int main(void)
{
    // initialize the hardware abstraction layer
    HAL_Init();

    // Peripherals are disabled by defalt 
    // We use the Reset and Clock COntrol registers to enable the GPIO peripherals

    // enable port A for theh on-board LED
    __HAL_RCC_GPIOA_CLK_ENABLE();
    // enable port B for the rotary encoder inputs
    __HAL_RCC_GPIOB_CLK_ENABLE();
    // enbale port C for on-board push buttons
    __HAL_RCC_GPIOC_CLK_ENABLE();
    // set up for serial commnication (serial monitor) to the computer
    SerialSetup(9600);

    // total numbe of flashes for the pattern
    const int NUM_FLASHES = 10;
    // boolean value for the pattern( long flash == 1 && short flash == 0)
    bool pattern[NUM_FLASHES];

    // initialize LED pin
    GPIO_TypeDef *LEDPort = GPIOA;
    uint16_t LEDPin = GPIO_PIN_8;
    InitializePin(LEDPort, LEDPin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, 0);

    while (1)
    {
        // wait for the user to press button
        while (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13)) {}

        // generates the random pattern from system time 
        srand(HAL_GetTick());
        for (int i = 0; i < NUM_FLASHES; ++i)
        {
            pattern[i] = random() % 2;
            SerialPutc(pattern[i] + 48);
        }
        SerialPutc(13);

        // display the pattern
        const int SHORT_PUL_LEN = 250;  // short pulse length
        const int LONG_PUL_LEN = 1000;  // long pulse length
        const int LED_OFF_LEN = 1000;   // length between flashes
        for (int i = 0; i < NUM_FLASHES; ++i)
        {
            HAL_GPIO_WritePin(LEDPort, LEDPin, 1);
            if (pattern[i])
            {
                HAL_Delay(LONG_PUL_LEN);
            } 
            else 
            {
                HAL_Delay(SHORT_PUL_LEN);
            }
            HAL_GPIO_WritePin(LEDPort, LEDPin, 0);
            HAL_Delay(LED_OFF_LEN);
        }

        // detects user inputs and records the pattern
        // LED on when user presses the button
        // push will not be registered if push length < 0.025 second (avoid accidental push)
        const int SHORT_PULSE_THRESHOLD = 750;
        const int NOT_REGISTER_THRESHOLD = 25;
        bool userPattern[NUM_FLASHES];
        for (int i = 0; i < NUM_FLASHES; ++i)
        {
            while (1)
            {
                // wait for the user to press the button and record
                while (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13)) {}
                uint32_t start = HAL_GetTick();
                HAL_GPIO_WritePin(LEDPort, LEDPin, 1);
                // wait for the user to release the button and record
                while (!HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13)) {}
                uint32_t end = HAL_GetTick();
                HAL_GPIO_WritePin(LEDPort, LEDPin, 0);
                if (end - start > NOT_REGISTER_THRESHOLD)
                {
                    userPattern[i] = (end - start > SHORT_PULSE_THRESHOLD);
                    break;
                }
            }
            SerialPutc(userPattern[i] + 48);
        }
        SerialPutc(13);

        // compare user input to displayed pattern
        bool wrongPattern = 0;
        for (int i = 0; i < NUM_FLASHES; ++i)
        {
            if (pattern[i] != userPattern [i])
            {
                wrongPattern = 1;
                break;
            }
        }

        //LED flashes 2 times if user failed to match the pattern and restart the game
        //LED ON if user matched the pattern
        if (wrongPattern)
        {
            const int FAILED_FLASHES = 2;
            const int FAILED_FLASH_LEN = 100;
            const int FAILED_CD_LEN = 1000;
            for (int i = 0; i < FAILED_FLASHES; ++i)
            {
                HAL_GPIO_WritePin(LEDPort, LEDPin, 1);
                HAL_Delay(FAILED_FLASH_LEN);
                HAL_GPIO_WritePin(LEDPort, LEDPin, 0);
                HAL_Delay(FAILED_FLASH_LEN);
            }
            HAL_Delay(FAILED_CD_LEN);
        }
        else
        {
            HAL_GPIO_WritePin(LEDPort, LEDPin, 1);
            break;
        }
    }
    return 0;
}

// This functiois called by HAL onece every milisecond
void SysTick_Handler(void)
{
    //tell HAL a new tick has occured 
    HAL_IncTick();
}