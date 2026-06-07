#include "src/utils/Arduboy2Ext.h"
#include "src/utils/Constants.h"
#include "src/utils/Structs.h"
#include "src/utils/AI.h"  
#include "src/entities/Entities.h"
#include "src/utils/Horizon.h"
#include "src/images/Images.h"

#define DEBUG_BREAK    //asm volatile("break\n");


static Player player;

Arduboy2Ext arduboy;
HorizonState hs;

GameState gameState = GameState::SplashScreen;
EnemyPlane enemies[Constants::Entity_Count];   // enemies[Heart_Idx] is the health pickup

FP8 speedFactor = Constants::SpeedFactor_Init;

uint8_t flashFrames = 0;
uint8_t behindYouFrames = 0;
uint8_t hitFlashFrames = 0;
uint8_t stallFrames = 0;


void setup() {
  //Serial.begin(115200);
	arduboy.begin();
	//arduboy.boot();
	//arduboy.display();
	//arduboy.flashlight();
	//arduboy.systemButtons();
	//arduboy.clear();
  arduboy.setFrameRate(40);

}


void loop() {

	if (!arduboy.nextFrame()) return;
	arduboy.pollButtons();

	switch (gameState) {

		case GameState::SplashScreen:
			splashScreen_Display();
			break;

		case GameState::Title_Init:
			title_Init();
			break;

		case GameState::Title:
			title_Display();
			break;

		case GameState::Playing:
			gameUpdate();
			break;

		case GameState::GameOver_Init:
			gameOver_Init();
			break;

		case GameState::GameOver_Drop:
			gameOver_Drop();
			break;

		case GameState::GameOver:
			gameOver_Display();
			break;

	}

	arduboy.display(true);

}
