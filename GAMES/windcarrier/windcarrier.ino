#include <Arduboy2.h>
#include <ArduboyTones.h>
//#include <avr/eeprom.h>
#include "image.h"

Arduboy2 arduboy;
ArduboyTones sound(arduboy.audio.enabled);
uint8_t eepAdr = EEPROM_STORAGE_SPACE_START;

#define CLOUDGROUND 40
#define MAPSIZE 16

const uint8_t stage0[MAPSIZE] PROGMEM = {2,2,1,0,2,0,1,2,2,2,2,2,2,2,2,2,};
const uint8_t stage1[MAPSIZE] PROGMEM = {2,2,1,2,4,2,1,2,2,4,4,2,1,0,1,2,};
const uint8_t stage2[MAPSIZE] PROGMEM = {0,1,1,1,4,1,2,2,2,2,1,1,4,1,1,0,};
const uint8_t* bPlants;

uint8_t scores[3];

uint8_t gameMode = 0;
uint16_t titleCount = 0; 
uint8_t titleMode = 0;
uint8_t cStage = 0;

uint8_t dayCount = 1;
uint16_t hCount = 0;

float vaKazaguruma;
float aKazaguruma;
uint8_t aniCount;
uint16_t camX = 0;

uint16_t plX = 0;
uint16_t plY = 0;
uint8_t particleAnime = 0;
uint8_t cloudID;  //触っている雲

uint8_t plMuki = 0;

const float WINDSPEED = 0.06;
typedef struct
{
    float x, y, vx, vy;
    uint16_t water;
    uint8_t cloudMode;
    uint8_t aniCount;
}cloud;
typedef struct
{
  uint16_t x;
  uint8_t y,muki;
}dot;
typedef struct
{
  uint8_t type;
  uint8_t growType;
  uint16_t level;
}ground;

const uint8_t MAXCLOUD = 4;
const uint8_t MAXPARTICLE = 64;
cloud clouds[MAXCLOUD];
uint8_t rainClouds[MAXCLOUD];
dot particle[MAXPARTICLE];
ground tiles[MAPSIZE];
void showParticle(uint16_t x,uint8_t y,uint8_t muki)
{
  for (uint8_t i = 0;i < MAXPARTICLE;i ++)
  {
    if (particle[i].muki == 5)
    {
      particle[i].muki = muki;
      particle[i].x = x;
      particle[i].y = y;
      return;
    }
  }
}
uint8_t readByteEEPROM(uint8_t add)
{
    //eeprom_busy_wait();
    return EEPROM.read(eepAdr+add);//eeprom_read_byte((uint8_t*)(eepAdr+add));
}

void mainGame()
{
  bool windKazaguruma = false;
  hCount ++;
  if (hCount >= 60 * 5)
  {
    hCount = 0;
    if (dayCount < 254){dayCount ++;}
  }
  //キャラ移動
  if (arduboy.pressed(LEFT_BUTTON) && plX > 0) {plX -= 1;plMuki = 0;}
  else if (arduboy.pressed(UP_BUTTON) && plY > 0) {plY -= 1;plMuki = 1;}
  else if (arduboy.pressed(RIGHT_BUTTON) && plX < (MAPSIZE * 32) - 16) {plX += 1;plMuki = 2;}
  else if (arduboy.pressed(DOWN_BUTTON) && plY < 47) {plY += 1;plMuki = 3;}
  //移動
  cloudID = MAXCLOUD + 1;
  if (arduboy.pressed(B_BUTTON))
  {
      if (arduboy.pressed(LEFT_BUTTON) && camX > 0) {camX -= 1;}
      else if (arduboy.pressed(RIGHT_BUTTON) && camX < (MAPSIZE * 32) - 128) {camX += 1;}
      else
      {
        //雲タッチ
        for (uint8_t i = 0;i < MAXCLOUD;i ++)
        {
          if (clouds[i].cloudMode != 0)
          {
            if (clouds[i].y < plY + 16 && clouds[i].y + 16 > plY && clouds[i].x < plX + 16 && clouds[i].x + 32 > plX)
            {
              cloudID = i;
            }
          }
        }
      }
  }
  //雨判定

  if (cloudID != MAXCLOUD + 1 && arduboy.justPressed(A_BUTTON))
  {
    if (clouds[cloudID].cloudMode == 1) {clouds[cloudID].cloudMode = 2;}
    else if (clouds[cloudID].cloudMode == 2) {clouds[cloudID].cloudMode = 1;}
  }
  else if (arduboy.pressed(A_BUTTON)) 
  {
    if (particleAnime == 0)
    {
      if (plMuki == 0) { showParticle(camX + 127,plY + random(16),plMuki);}
      else if (plMuki == 1) { showParticle(plX + random(16),63,plMuki);}
      else if (plMuki == 2) { showParticle(camX,plY + random(16),plMuki);}
      else if (plMuki == 3) { showParticle(plX + random(16),0,plMuki);}
      particleAnime = 5 + random(5);
    }
    if (particleAnime > 0) {particleAnime --;}
    //雲を流す
    if (plMuki % 2 == 0)    //横向き
    {
      for (uint8_t i = 0;i < MAXCLOUD;i ++)
      {
        if (clouds[i].cloudMode != 0)
        {
          if (clouds[i].y < plY + 16 && clouds[i].y + 16 > plY)
          {
            if (plMuki == 0){clouds[i].vx -= WINDSPEED;}
            if (plMuki == 2){clouds[i].vx += WINDSPEED;}
          }
        }
      }
    }
    else  //横向き
    {
      for (uint8_t i = 0;i < MAXCLOUD;i ++)
      {
        if (clouds[i].cloudMode != 0)
        {
          if (clouds[i].x < plX + 16 && clouds[i].x + 32 > plX)
          {
            if (plMuki == 1)
            {
              //上昇気流の場合水の吸い上げも判定
              clouds[i].vy -= WINDSPEED;
              if (tiles[plX / 32].type == 2 && clouds[i].water < 60 * 30)
              {
                clouds[i].water += 4;
                clouds[i].aniCount ++;
                clouds[i].aniCount %= 30;
              }
            }
            if (plMuki == 3){clouds[i].vy += WINDSPEED;}
          }
        }
      }
    }
    //風車
    if (plY >= 32 && plMuki % 2 == 0)
    {
      if (plMuki == 0 && vaKazaguruma < 1.0f) { vaKazaguruma += 0.01f;}
      if (plMuki == 2 && vaKazaguruma > -1.0f) { vaKazaguruma -= 0.01f;}
      windKazaguruma = true;
    }
  }
  if(!windKazaguruma)
  {
    if (vaKazaguruma > 0.02f) {vaKazaguruma -= 0.01f;}
    else if (vaKazaguruma < -0.02f) {vaKazaguruma += 0.01f;}
    else {vaKazaguruma = 0;}
  }
  aKazaguruma += vaKazaguruma;
  if (aKazaguruma < 0) {aKazaguruma += 4;}
  if (aKazaguruma >= 4) {aKazaguruma -= 4;}
  //雲を流す
  for (uint8_t i = 0;i < MAXCLOUD;i ++)
  {
    if (clouds[i].cloudMode != 0)
    {
      float oldX = clouds[i].x;
      float oldY = clouds[i].y;
      if (fabsf(clouds[i].vx) > WINDSPEED / 2){clouds[i].x += clouds[i].vx;}
      //当たり判定
      for (uint8_t j = 0;j < MAXCLOUD;j ++)
      {
        if (clouds[j].cloudMode != 0 && j != i)
        {
          if (clouds[i].x < clouds[j].x + 32 && clouds[i].x + 32 > clouds[j].x)
          {
            if (clouds[i].y < clouds[j].y + 16 && clouds[i].y + 16 > clouds[j].y)
            {
              clouds[i].x = oldX;
              clouds[i].vx = -clouds[i].vx / 2;
            }
          }
        }
      }
      if (fabsf(clouds[i].vy) > WINDSPEED / 2){clouds[i].y += clouds[i].vy;}
      //当たり判定
      for (uint8_t j = 0;j < MAXCLOUD;j ++)
      {
        if (clouds[j].cloudMode != 0 && j != i)
        {
          if (clouds[i].x < clouds[j].x + 32 && clouds[i].x + 32 > clouds[j].x)
          {
            if (clouds[i].y < clouds[j].y + 16 && clouds[i].y + 16 > clouds[j].y)
            {
              clouds[i].y = oldY;
              clouds[i].vy = -clouds[i].vy / 2;
            }
          }
        }
      }
      //跳ね返す
      if (clouds[i].x < 0 || clouds[i].x >= MAPSIZE * 32 - 32)
      {
        clouds[i].x = oldX;
        clouds[i].vx = -clouds[i].vx / 2;
      }
      if (clouds[i].y < 0 || clouds[i].y >= CLOUDGROUND)
      {
        clouds[i].y = oldY;
        clouds[i].vy = -clouds[i].vy / 2;
      }
      //空気抵抗
      if (clouds[i].vx > 0) { clouds[i].vx -= WINDSPEED / 2;}
      else if (clouds[i].vx < 0) { clouds[i].vx += WINDSPEED / 2;}
      if (clouds[i].vy > 0) { clouds[i].vy -= WINDSPEED / 2;}
      else if (clouds[i].vy < 0) { clouds[i].vy += WINDSPEED / 2;}
    }
    if (clouds[i].cloudMode == 2)
    {

      if (clouds[i].water > 0 )
      {
        clouds[i].water --;
        clouds[i].aniCount ++;
        clouds[i].aniCount %= 30;
        if (aniCount % 5 == 0)
        {
          showParticle(clouds[i].x + random(32),clouds[i].y + 16,3);
        }
      }
      else
      {
        clouds[i].cloudMode = 1;
      }
    }
    rainClouds[i] = (clouds[i].x + 16) / 32;
  }

  //パーティクルを流す
  for (uint8_t i = 0;i < MAXPARTICLE;i ++)
  {
    if (particle[i].muki != 5)
    {
      if (particle[i].muki == 0){particle[i].x -= 2;}
      else if (particle[i].muki == 1){particle[i].y -= 2;}
      else if (particle[i].muki == 2){particle[i].x += 2;}
      else if (particle[i].muki == 3){particle[i].y += 2;}
      if (particle[i].x < camX || particle[i].x >= camX + 128 || particle[i].y >= 64)
      {
        particle[i].muki = 5;
      }
    }
  }

  //ワールドメイン
  for (uint8_t i = 0;i < MAPSIZE;i ++)
  {
    if (tiles[i].type == 1)
    {
      int8_t hitCloud = -1;
      for (uint8_t j = 0;j < MAXCLOUD;j ++)
      {
        if (i == rainClouds[j]) {hitCloud = j;break;}
      }
      //晴れ成長
      if (hitCloud == -1 && tiles[i].growType % 2 == 0)
      {
        tiles[i].level ++;
      }
      //雨成長
      else if (clouds[hitCloud].cloudMode == 2 && tiles[i].growType % 2 == 1)
      {
        tiles[i].level ++;
      }
      if (tiles[i].level >= 10 * 60)
      {
        tiles[i].level = 0;
        if (tiles[i].growType < 7){tiles[i].growType ++;sound.tones(SE_GROW);}
        bool allGrow = true;
        for (int j = 0;j < MAPSIZE;j ++)
        {
          if (tiles[j].type == 1 && tiles[j].growType < 7){allGrow = false;}
        }
        //クリア
        if (allGrow)
        {
          gameMode = 1;
          titleCount = 0;
          sound.tones(SE_CLEAR);
          if (scores[cStage] > dayCount)
          {
            scores[cStage] = dayCount;
            //eeprom_busy_wait();
            //eeprom_write_byte((uint8_t*)(eepAdr+cStage + 1), dayCount);
            EEPROM.write((eepAdr+cStage + 1), dayCount);
          }
        }
      }
    }
  }
}
void drawNumber(uint8_t num)
{
  char numStr[4];
  if (num != 255)
  {
    numStr[0] = num / 100 + 48;
    numStr[1] = (num / 10) % 10 + 48;
    numStr[2] = num % 10 + 48;
    numStr[3] = 0;
  }
  else
  {
    numStr[0] = '-';
    numStr[1] = '-';
    numStr[2] = '-';
    numStr[3] = 0;
  }
  arduboy.print(numStr);
}
void mainDraw()
{
  int cKazaguruma = (int)(aKazaguruma);
  aniCount += 1;
  if (aniCount >= 60)
  {
    aniCount = 0;
  }
  arduboy.clear();
  //タイル
  for (uint8_t i = 0;i < 5; i ++)
  {
    int tileX = -(camX % 32) + i * 32;
    ground tile = tiles[(camX / 32) + i];
    if (tile.type == 2 && aniCount > 30) {tile.type ++;}
    if (tile.type == 1)
    {
      uint8_t DPTable[3];
      DPTable[0] = 0;
      DPTable[1] = 0;
      DPTable[2] = 0;
      if (tile.growType >= 7)
      {
        DPTable[0] = 3;
        DPTable[1] = 3;
        DPTable[2] = 4;
      }
      else
      {

        if (tile.growType < 4)
        {
          DPTable[0] = tile.growType;
        }
        else
        {
          DPTable[0] = 3;
          DPTable[1] = tile.growType - 3;
        }
      }
      for (uint8_t j = 0;j < 3;j ++)
      {
        Arduboy2Base::drawBitmap(tileX,48 - j * 8,&bPlants[DPTable[j] * 32],32,8,WHITE);        
      }
    }
    else if (tile.type == 4)
    {
      Arduboy2Base::drawBitmap(tileX + 8,40,&bKazaguruma[cKazaguruma * 32],16,16,WHITE); 
    }
    Arduboy2Base::drawBitmap(tileX,56,&bGroundTile[tile.type * 32],32,8,WHITE);
  }
  //くも
  for (uint8_t i = 0;i < MAXCLOUD;i ++)
  {
    if (clouds[i].cloudMode >= 1)
    {
      if (clouds[i].aniCount >= 15)
      {
        Arduboy2Base::drawBitmap((int)(clouds[i].x - camX), (int)(clouds[i].y), bCloud2, 32, 16, WHITE);    
      }
      else
      {
        Arduboy2Base::drawBitmap((int)(clouds[i].x - camX), (int)(clouds[i].y), bCloud, 32, 16, WHITE);    
      }
    }
  }
  for (uint8_t i = 0;i < MAXPARTICLE;i ++)
  {

    if (particle[i].muki != 5)
    {
      Arduboy2Base::drawPixel(particle[i].x - camX,particle[i].y);
    }
  }
  //やじるし
  if (cloudID != MAXCLOUD + 1)
  {
    Arduboy2Base::drawBitmap(plX - camX, plY, bRain, 16, 16, WHITE);
  }
  else
  {
    Arduboy2Base::drawBitmap(plX - camX, plY, &bArrow[plMuki * 32], 16, 16, WHITE);
    Arduboy2Base::drawBitmap(plX - camX, plY, bWind, 16, 16, WHITE);
  }
  arduboy.setCursor(0, 0);
  arduboy.print("Day");
  drawNumber(dayCount);
  arduboy.display();
}
void syokika(const uint8_t* stageAdd)
{
  for (uint8_t i = 0;i < MAXPARTICLE;i ++)
  {
    particle[i].muki = 5;
  }
  for (uint8_t i = 0;i < MAXCLOUD;i ++)
  {
    clouds[i].cloudMode = 0;
    clouds[i].water = 0;
    clouds[i].vx = 0;
    clouds[i].vy = 0;
  }
  for (uint8_t i = 0;i < MAPSIZE;i ++)
  {
    tiles[i].type = pgm_read_byte(&stageAdd[i]);
    if (tiles[i].type == 1){tiles[i].growType = 1;tiles[i].level = 0;}
  }
  dayCount = 0;
  hCount = 0;
  camX = 0;
  plX = 0;
  plY = 0;
  gameMode = 0;
}


void setup() {
  arduboy.begin();
  arduboy.audio.on();
  arduboy.setFrameRate(60);
  scores[0] = readByteEEPROM(1);
  scores[1] = readByteEEPROM(2);
  scores[2] = readByteEEPROM(3);
  for (uint8_t i = 0;i < 3;i ++)
  {
    if (scores[i] == 0){scores[i] = 255;}
  }
}

void loop() {
  if (!arduboy.nextFrame()) return;
  arduboy.pollButtons();
  if (titleMode == 0)
  {
    if (arduboy.justPressed(A_BUTTON))
    {
      titleMode = 1;
      sound.tones(SE_ON);
    }
    arduboy.clear();
    Arduboy2Base::drawBitmap(0,0, windTitle, 128, 64, WHITE);
    arduboy.display();
  }
  else if (titleMode == 1)
  {
    if (arduboy.justPressed(UP_BUTTON) && cStage > 0)
    {
      cStage --;
    }
    else if (arduboy.justPressed(DOWN_BUTTON) && cStage < 2)
    {
      cStage ++;
    }
    if (arduboy.justPressed(A_BUTTON))
    {   
      if (cStage == 0)
      {
        syokika(stage0);
        clouds[0].x = 32;
        clouds[0].y = 32;
        clouds[0].cloudMode = 1;
        bPlants = bPlantsN;
      }
      else if (cStage == 1)
      {
        syokika(stage1);
        clouds[0].x = 32;
        clouds[0].y = 32;
        clouds[0].cloudMode = 1;
        clouds[1].x = 96;
        clouds[1].y = 32;
        clouds[1].cloudMode = 1;
        bPlants = bPlantsW;
      }
      else
      {
        syokika(stage2);
        clouds[0].x = 32;
        clouds[0].y = 32;
        clouds[0].cloudMode = 1;
        clouds[1].x = 96;
        clouds[1].y = 32;
        clouds[1].cloudMode = 1;
        clouds[2].x = 144;
        clouds[2].y = 32;
        clouds[2].cloudMode = 1;
        bPlants = bPlantsV;
      }
      titleMode = 2;
      sound.tones(SE_ON);
    }
    arduboy.clear();
    arduboy.setCursor(43, 8);
    arduboy.print("Stage 1 ");drawNumber(scores[0]);
    arduboy.setCursor(43, 16);
    arduboy.print("Stage 2 ");drawNumber(scores[1]);
    arduboy.setCursor(43, 24);
    arduboy.print("Stage 3 ");drawNumber(scores[2]);
    Arduboy2Base::drawBitmap(27,4 + cStage * 8, &bArrow[2 * 32], 16, 16, WHITE);
    if (scores[0] != 255) {Arduboy2Base::drawBitmap(12,8,&bPlantsN[4 * 32],8,8,WHITE);}
    if (scores[1] != 255) {Arduboy2Base::drawBitmap(12,16,&bPlantsW[4 * 32],8,8,WHITE);}
    if (scores[2] != 255) {Arduboy2Base::drawBitmap(12,24,&bPlantsV[4 * 32],8,8,WHITE);}
    if (scores[0] != 255 && scores[1] != 255 && scores[2] != 255)
    {
      for (int i = 0;i < 4;i ++)
      {
        Arduboy2Base::drawBitmap(i * 32,40,&bPlantsN[4 * 32],32,8,WHITE);
      }
    }
    for (int i = 0;i < 4;i ++)
    {
      Arduboy2Base::drawBitmap(i * 32,48,&bPlantsN[3 * 32],32,8,WHITE);
      Arduboy2Base::drawBitmap(i * 32,56,&bGroundTile[0],32,8,WHITE);
    }
    arduboy.display();
  }
  else
  {
    if (gameMode == 0)
    {
      mainGame();
    }
    else
    {
      titleCount ++;
      if (titleCount >= 3 * 60){titleMode = 0;}
    }
    mainDraw();
  }
}
