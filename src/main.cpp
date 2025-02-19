#include <Arduino.h>
#include "main.h"
#include "game_system.h"
#include "led_manager.h"
#include "time_utility.h"
#include "button_manager.h"
#include "potenziometer_manager.h"


/// Define the current state of the game
GameState gameState = SETUP;

/// Variables managing timers for events
unsigned long sleepModeStartTime;
unsigned long switchOnGreenLedsStartTime;
unsigned long switchOffGreenLedStartTime;
unsigned long gameOverStartTime;

/// Player have 10s to start the game or the system go in deep sleep mode
const unsigned long sleepModeTime = 10000;

/// Variable used on not blocking delays
unsigned long prevTime = 0;

/// State of the MC phase, true if still have to light up some leds, false otherwise
bool ledsTurningOn = true;

void sleep() {
    Serial.println("Sleep mode actived");
    switchGreenLeds(false);
    switchLed(RED_LED, false);
    sleepNow();
}

void setup() {
    ledsInit();
    buttonsInit();
    potentiometerInit();
    Serial.begin(9600);
}

void loop() {
    switch (gameState)
    {
    case SETUP:
        Serial.println("Welcome to the Restore the Light Game. Press Key B1 to Start");
        switchGreenLeds(false);
        ledFading(RED_LED);
        sleepModeStartTime = millis();
        gameState = INITIALIZATION;

        break;
    case INITIALIZATION:
        basicTimer(sleepModeTime, &sleepModeStartTime, sleep);
        
        if (digitalRead(BUTTON1) == HIGH) {
            switchOnGreenLedsStartTime = millis();
            gameInit(getDifficulty());
            Serial.println("GO!");
            gameState = LEDS_ON;
        }

        break;
    case LEDS_ON:
        basicTimer(T1, &switchOnGreenLedsStartTime, switchGreenLeds, true);

        if (getGreenLedsNumber() == getGreenLedsOnNumber()) {
            gameState = LEDS_OFF;
        }

        break;
    case LEDS_OFF:
        basicTimer(currentT2, &switchOffGreenLedStartTime, switchRandomLedOff);

        // Check if the MC phase is finished
        if (getGreenLedsOnNumber() == 0) {
            //Initialize the PLAYER state
            activateButtonsGameInterrupt();
            gameOverStartTime = millis();
            //Change to PLAYER state            
            gameState = PLAYER;
        }

        break;
    case PLAYER:
        /* here check if the player pressed wrong button */
        /* here check if the player win the game */
        /* this is one of the last function to launch in this state */
        basicTimer(currentT3, &gameOverStartTime, setGameOver);
        /* this is the last function to launch in this state */
        
        break;
    case NEWLEVEL:
        // !! Update dei parametri di gioco T2, T3, "numCorrectButtons"
        gameState = LEDS_ON;
        break;
    case GAMEOVER:
        // !! Aggiungi il map al red led fade down 10s
        printFinalScore();
        ledFading(RED_LED);
        gameState = SETUP;
        break;
    default:
        break;
    }
}

/// @brief Set the game state to GAMEOVER (used with an iterrupt).
void setGameOver() {
    deactivateButtonsGameInterrupt();
    gameState = GAMEOVER;
}