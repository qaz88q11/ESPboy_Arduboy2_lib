#include <ArduboyFX.h>  

void saveHighScore() {

    if (level > cookie.getBestLevel()) {
        cookie.setBestLevel(level);
        FX::saveGameState(cookie);
    }

}

void addLookLeftThenRight(bool talkBubble) {

    skullStack.push(0x000B);
    skullStack.push(0x000B);
    skullStack.push(0x000A);
    skullStack.push(0x000A);
    skullStack.push(0x0009);
    skullStack.push(0x0009);
    skullStack.push(0x0008);
    skullStack.push(0x0008);
    skullStack.push(0x0007);
    skullStack.push(0x0007);
    skullStack.push(0x0006);
    skullStack.push(0x0006);

    if (talkBubble) {
        skullStack.push(0x0205);
    }
    else {
        skullStack.push(0x0005);
    }

    skullStack.push(0x0005);
    skullStack.push(0x0004);
    skullStack.push(0x0004);
    skullStack.push(0x0003);
    skullStack.push(0x0003);
    skullStack.push(0x0002);
    skullStack.push(0x0002);
    skullStack.push(0x0001);
    skullStack.push(0x0001);
    skullStack.push(0x0000);

    if (talkBubble) {
        skullStack.push(0x0100);
    }
    else {
        skullStack.push(0x0000);
    }

}

void addLookLeft(bool talkBubble) {

    skullStack.push(0x000B);
    skullStack.push(0x000B);
    skullStack.push(0x000A);
    skullStack.push(0x000A);

    if (talkBubble) {
        skullStack.push(0x0205);
    }
    else {
        skullStack.push(0x0005);
    }

    skullStack.push(0x0005);
    skullStack.push(0x0004);
    skullStack.push(0x0004);
    skullStack.push(0x0003);
    skullStack.push(0x0003);
    skullStack.push(0x0002);
    skullStack.push(0x0002);
    skullStack.push(0x0001);
    skullStack.push(0x0001);
    skullStack.push(0x0000);

    if (talkBubble) {
        skullStack.push(0x0100);
    }
    else {
        skullStack.push(0x0000);
    }

}

void addLookRight(bool talkBubble) {

    skullStack.push(0x000B);
    skullStack.push(0x000B);
    skullStack.push(0x000A);
    skullStack.push(0x000A);
    skullStack.push(0x0009);
    skullStack.push(0x0009);
    skullStack.push(0x0008);
    skullStack.push(0x0008);
    skullStack.push(0x0007);
    skullStack.push(0x0007);
    skullStack.push(0x0006);
    skullStack.push(0x0006);

    if (talkBubble) {
        skullStack.push(0x0205);
    }
    else {
        skullStack.push(0x0005);
    }

    skullStack.push(0x0005);
    skullStack.push(0x0000);

    if (talkBubble) {
        skullStack.push(0x0100);
    }
    else {
        skullStack.push(0x0000);
    }

}

void addLookDownThenUp(bool talkBubble) {

    skullStack.push(0x0017);
    skullStack.push(0x0017);
    skullStack.push(0x0016);
    skullStack.push(0x0016);
    skullStack.push(0x0015);
    skullStack.push(0x0015);
    skullStack.push(0x0014);
    skullStack.push(0x0014);
    skullStack.push(0x0013);
    skullStack.push(0x0013);
    skullStack.push(0x0012);
    skullStack.push(0x0012);

    if (talkBubble) {
        skullStack.push(0x0211);
    }
    else {
        skullStack.push(0x0011);
    }    

    skullStack.push(0x0011);
    skullStack.push(0x0010);
    skullStack.push(0x0010);
    skullStack.push(0x000F);
    skullStack.push(0x000F);
    skullStack.push(0x000E);
    skullStack.push(0x000E);
    skullStack.push(0x000D);
    skullStack.push(0x000D);
    skullStack.push(0x000C);

    if (talkBubble) {
        skullStack.push(0x010C);
    }
    else {
        skullStack.push(0x000C);
    }    

}

void addLookDown(bool talkBubble) {

    skullStack.push(0x0017);
    skullStack.push(0x0017);
    skullStack.push(0x0016);
    skullStack.push(0x0016);

    if (talkBubble) {
        skullStack.push(0x0211);
    }
    else {
        skullStack.push(0x0011);
    }   

    skullStack.push(0x0011);
    skullStack.push(0x0010);
    skullStack.push(0x0010);
    skullStack.push(0x000F);
    skullStack.push(0x000F);
    skullStack.push(0x000E);
    skullStack.push(0x000E);
    skullStack.push(0x000D);
    skullStack.push(0x000D);
    skullStack.push(0x000C);
 
    if (talkBubble) {
        skullStack.push(0x010C);
    }
    else {
        skullStack.push(0x000C);
    } 

}

void addLookUp(bool talkBubble) {

    skullStack.push(0x0017);
    skullStack.push(0x0016);
    skullStack.push(0x0015);
    skullStack.push(0x0015);
    skullStack.push(0x0014);
    skullStack.push(0x0014);

    if (talkBubble) {
        skullStack.push(0x0213);
    }
    else {
        skullStack.push(0x0013);
    } 

    skullStack.push(0x0013);
    skullStack.push(0x0012);
    skullStack.push(0x0012);
    skullStack.push(0x0011);
    skullStack.push(0x0011);

    if (talkBubble) {
        skullStack.push(0x010C);
    }
    else {
        skullStack.push(0x000C);
    } 

}
void addLongTalk() {

    skullStack.push(0x000B);
    skullStack.push(0x000B);
    skullStack.push(0x0000);
    skullStack.push(0x0000);
    skullStack.push(0x0201);
    skullStack.push(0x0001);
    skullStack.push(0x0001);
    skullStack.push(0x0001);
    skullStack.push(0x0001);
    skullStack.push(0x0001);
    skullStack.push(0x0001);
    skullStack.push(0x0001);
    skullStack.push(0x0001);
    skullStack.push(0x0001);
    skullStack.push(0x0001);
    skullStack.push(0x0000);
    skullStack.push(0x0100);

}

void addDeathStare(bool talkBubble) {

    skullStack.push(0x000B);
    skullStack.push(0x000B);
    skullStack.push(0x0000);
    skullStack.push(0x0000);

    if (talkBubble) {
        skullStack.push(0x0201);
    }
    else {
        skullStack.push(0x0001);
    } 

    skullStack.push(0x0001);
    skullStack.push(0x0001);
    skullStack.push(0x0001);
    skullStack.push(0x0001);
    skullStack.push(0x0001);
    skullStack.push(0x0001);
    skullStack.push(0x0001);
    skullStack.push(0x0001);
    skullStack.push(0x0001);
    skullStack.push(0x0001);
    skullStack.push(0x0000);

    if (talkBubble) {
        skullStack.push(0x0100);
    }
    else {
        skullStack.push(0x0000);
    } 

}

void addPinPrick() {

    skullStack.push(0x000B);
    skullStack.push(0x000B);
    skullStack.push(0x0000);
    skullStack.push(0x0000);
    skullStack.push(0x0001);
    skullStack.push(0x0001);
    skullStack.push(0x0001);
    skullStack.push(0x0001);
    skullStack.push(0x0000);
    skullStack.push(0x0000);

}

void addWideEyes() {

    skullStack.push(0x0021);
    skullStack.push(0x0021);
    skullStack.push(0x0020);
    skullStack.push(0x0020);
    skullStack.push(0x001F);
    skullStack.push(0x001F);
    skullStack.push(0x001E);
    skullStack.push(0x001E);
    skullStack.push(0x001D);
    skullStack.push(0x001D);
    skullStack.push(0x001C);
    skullStack.push(0x001C);
    skullStack.push(0x001B);
    skullStack.push(0x001B);
    skullStack.push(0x001A);
    skullStack.push(0x001A);
    skullStack.push(0x0019);
    skullStack.push(0x0019);
    skullStack.push(0x0018);
    skullStack.push(0x0018);

}

void addRollEyes() {

    skullStack.push(0x0021);
    skullStack.push(0x002F);
    skullStack.push(0x002E);
    skullStack.push(0x002D);
    skullStack.push(0x002C);
    skullStack.push(0x002B);
    skullStack.push(0x002A);
    skullStack.push(0x0029);
    skullStack.push(0x0028);
    skullStack.push(0x0027);
    skullStack.push(0x0026);
    skullStack.push(0x0025);
    skullStack.push(0x0024);

}