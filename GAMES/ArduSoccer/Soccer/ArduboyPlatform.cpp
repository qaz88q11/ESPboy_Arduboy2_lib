#include "ArduboyPlatform.h"
#include "Engine.h"
#include <ESP8266WiFi.h>
#include <espnow.h>

ArduboyPlatform Platform;

const uint8_t UPDATE_PACKET = 0xfa;
const uint8_t NACK_PACKET = 0xfb;
const uint8_t SYNC_PACKET = 0xfc;
const uint8_t HEARTBEAT_PACKET = 0xfd;
const uint8_t RECONNECT_SYNC_PACKET = 0xfe;

constexpr unsigned long RECONNECT_DELAY = 2000;
constexpr unsigned long HEARTBEAT_INTERVAL = 500;
constexpr unsigned long CONNECTION_TIMEOUT = 3000;
constexpr unsigned long DISCOVERY_TIMEOUT = 15000;
constexpr unsigned long LED_BLINK_INTERVAL = 200;
constexpr uint8_t MAX_RECONNECT_ATTEMPTS = 5;
constexpr unsigned long RECONNECT_COOLDOWN = 5000;

uint8_t broadcastMac[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t remoteMac[6] = {0};
uint8_t selfMac[6] = {0};
bool remoteMacKnown = false;

volatile bool packetReceived = false;
volatile bool packetSent = true;
unsigned long lastPacketReceivedTime = 0;
unsigned long lastHeartbeatTime = 0;
unsigned long lastLedToggleTime = 0;
unsigned long connectionLostTime = 0;
unsigned long discoveryStartTime = 0;
unsigned long reconnectCooldownTime = 0;

bool ledState = false;
bool isReconnecting = false;
bool smoothRecoveryActive = false;
uint8_t reconnectAttempts = 0;
uint8_t smoothRecoveryStep = 0;

GameState savedGameState = {0};
NetworkPacket receivedPacket = {0};
LedPattern currentLedPattern = LedPattern::OFF;

unsigned long lastResendTime = 0;
uint8_t resendCount = 0;
constexpr uint8_t MAX_RESEND = 3;

// ДИАГНОСТИКА
unsigned long lastDiagTime = 0;
uint32_t packetsReceived = 0;
uint32_t packetsSent = 0;
uint32_t validFrames = 0;
uint32_t missedFrames = 0;

void onDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
    packetSent = true;
    packetsSent++;
    if (sendStatus != 0) {
        currentLedPattern = LedPattern::ERROR;
    }
}

void onDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len) {
    if (len != sizeof(NetworkPacket)) return;
    
    if (memcmp(mac, selfMac, 6) == 0) return;
    
    NetworkPacket *pkt = (NetworkPacket *)incomingData;
    
    if (pkt->packetType == HEARTBEAT_PACKET && pkt->data == (Platform.hostId & 0xFF)) {
        return;
    }
    
    memcpy(&receivedPacket, incomingData, len);
    packetsReceived++;
    
    if (!remoteMacKnown || memcmp(remoteMac, mac, 6) != 0) {
        if (memcmp(mac, selfMac, 6) != 0) {
            memcpy(remoteMac, mac, 6);
            remoteMacKnown = true;
        }
    }
    
    lastPacketReceivedTime = millis();
    packetReceived = true;
    
    if (isReconnecting && receivedPacket.packetType != HEARTBEAT_PACKET) {
        reconnectAttempts = 0;
    }
}

void ArduboyPlatform::updateInput() {
    lastInputState[LOCAL_PLAYER] = inputState[LOCAL_PLAYER];
    inputState[LOCAL_PLAYER] = 0;
    
    if(arduboy.pressed(A_BUTTON)) inputState[LOCAL_PLAYER] |= Input_Btn_A;
    if(arduboy.pressed(B_BUTTON)) inputState[LOCAL_PLAYER] |= Input_Btn_B;
    if(arduboy.pressed(UP_BUTTON)) inputState[LOCAL_PLAYER] |= Input_Dpad_Up;
    if(arduboy.pressed(DOWN_BUTTON)) inputState[LOCAL_PLAYER] |= Input_Dpad_Down;
    if(arduboy.pressed(LEFT_BUTTON)) inputState[LOCAL_PLAYER] |= Input_Dpad_Left;
    if(arduboy.pressed(RIGHT_BUTTON)) inputState[LOCAL_PLAYER] |= Input_Dpad_Right;
}

void ArduboyPlatform::updateLedPattern() {
    unsigned long currentTime = millis();
    const uint8_t MAX_BRIGHTNESS = 3;
    const uint8_t MIN_BRIGHTNESS = 0;
    
    switch(currentLedPattern) {
        case LedPattern::SEARCHING:
            if(currentTime - lastLedToggleTime > LED_BLINK_INTERVAL / 2) {
                ledState = !ledState;
                lastLedToggleTime = currentTime;
                arduboy.setRGBled(ledState ? MAX_BRIGHTNESS : MIN_BRIGHTNESS, 
                                ledState ? MAX_BRIGHTNESS : MIN_BRIGHTNESS, 0);
            }
            break;
            
        case LedPattern::CONNECTING:
            if(currentTime - lastLedToggleTime > LED_BLINK_INTERVAL * 2) {
                ledState = !ledState;
                lastLedToggleTime = currentTime;
                arduboy.setRGBled(0, 0, ledState ? MAX_BRIGHTNESS : MIN_BRIGHTNESS);
            }
            break;
            
        case LedPattern::CONNECTED:
            arduboy.setRGBled(0, MAX_BRIGHTNESS, 0);
            break;
            
        case LedPattern::ERROR:
            if(currentTime - lastLedToggleTime > LED_BLINK_INTERVAL / 4) {
                ledState = !ledState;
                lastLedToggleTime = currentTime;
                arduboy.setRGBled(ledState ? MAX_BRIGHTNESS : MIN_BRIGHTNESS, 0, 0);
            }
            break;
            
        case LedPattern::RECONNECTING:
            if(currentTime - lastLedToggleTime > LED_BLINK_INTERVAL) {
                static uint8_t flashCount = 0;
                flashCount = (flashCount + 1) % 4;
                arduboy.setRGBled(flashCount < 2 ? MAX_BRIGHTNESS : MIN_BRIGHTNESS, 
                                flashCount < 2 ? MAX_BRIGHTNESS/2 : MIN_BRIGHTNESS, 0);
                lastLedToggleTime = currentTime;
            }
            break;
            
        case LedPattern::DISCOVERY_TIMEOUT_PATTERN:
            if(currentTime - lastLedToggleTime > LED_BLINK_INTERVAL * 3) {
                ledState = !ledState;
                lastLedToggleTime = currentTime;
                arduboy.setRGBled(ledState ? MAX_BRIGHTNESS : MIN_BRIGHTNESS, 0, 
                                ledState ? MIN_BRIGHTNESS : 1);
            }
            break;
            
        case LedPattern::OFF:
        default:
            arduboy.setRGBled(MIN_BRIGHTNESS, MIN_BRIGHTNESS, MIN_BRIGHTNESS);
            break;
    }
}

void ArduboyPlatform::update() {
    // Обработка аудио
    if(arduboy.audio.enabled() != !m_isMuted) {
        if(m_isMuted) {
            arduboy.audio.off();
        } else {
            arduboy.audio.on();
        }
    }

    updateLedPattern();
    
    unsigned long currentTime = millis();
    
    // ДИАГНОСТИКА каждые 2 секунды
/*
    if(currentTime - lastDiagTime > 2000) {
        Serial.print("Pkts recv: "); Serial.print(packetsReceived);
        Serial.print(" sent: "); Serial.print(packetsSent);
        Serial.print(" valid: "); Serial.print(validFrames);
        Serial.print(" missed: "); Serial.print(missedFrames);
        Serial.print(" buf: "); Serial.println(inputBuffer.count);
        lastDiagTime = currentTime;
    }
*/

    // Отключенный режим
    if(connectionStatus == ConnectionStatus::Disconnected && !isReconnecting) {
        updateInput();
        hostId++;
        currentLedPattern = LedPattern::OFF;
        return;
    }
    
    // Режим переподключения
    if(isReconnecting) {
        if(smoothRecoveryActive) {
            handleSmoothRecovery();
        } else {
            handleReconnection();
        }
        updateInput();
        return;
    }
    
    // *** АСИНХРОННЫЙ РЕЖИМ ИГРЫ ***
    
    // 1. Всегда проверяем соединение
    checkConnection();
    
    // 2. Всегда обрабатываем входящие пакеты
    parseNetwork();
    
    // 3. Обновляем локальный ввод каждый кадр
    updateInput();
    
    // 4. Отправляем ввод асинхронно
    if(currentTime - lastSendTime >= SEND_DELAY) {
        if(packetSent) {  // Проверяем что можно отправлять
            sendNetworkPacket(UPDATE_PACKET, inputState[LOCAL_PLAYER]);
            lastSendTime = currentTime;
        }
    }
    
    // 5. Отправляем heartbeat
    sendHeartbeat();
    
    // 6. Обработка таймаута подтверждения
    if(isWaitingForRemote && (currentTime - lastPacketSentTime > 50)) {
        if(resendCount < MAX_RESEND) {
            if(packetSent) {
                sendNetworkPacket(UPDATE_PACKET, inputState[LOCAL_PLAYER]);
                resendCount++;
            }
        } else {
            isWaitingForRemote = false;
            resendCount = 0;
            missedFrames++;
        }
    }
    
    // 7. ВСЕГДА применяем буферизированный ввод противника
    uint8_t remoteInput;
    if(inputBuffer.pop(remoteInput)) {
        lastInputState[REMOTE_PLAYER] = inputState[REMOTE_PLAYER];
        inputState[REMOTE_PLAYER] = remoteInput;
        validFrames++;
    }
    // Если буфер пуст - НЕ трогаем inputState[REMOTE_PLAYER]
    // Игра продолжает использовать последнее известное состояние
    
    yield();
}

void ArduboyPlatform::sendHeartbeat() {
    unsigned long currentTime = millis();
    
    if(currentTime - lastHeartbeatTime > HEARTBEAT_INTERVAL) {
        NetworkPacket packet;
        packet.packetType = HEARTBEAT_PACKET;
        packet.networkFrame = 0;
        packet.data = hostId & 0xFF;
        packet.checkSum = HEARTBEAT_PACKET + (hostId & 0xFF);
        
        if(packetSent && remoteMacKnown) {
            packetSent = false;
            esp_now_send(remoteMac, (uint8_t *)&packet, sizeof(packet));
            lastHeartbeatTime = currentTime;
        }
    }
}

void ArduboyPlatform::checkConnection() {
    unsigned long currentTime = millis();
    
    if(currentTime - lastPacketReceivedTime > CONNECTION_TIMEOUT && !isReconnecting) {
        startReconnection();
    }
}

void ArduboyPlatform::startReconnection() {
    if(reconnectAttempts >= MAX_RECONNECT_ATTEMPTS) {
        currentLedPattern = LedPattern::ERROR;
        delay(3000);
        disconnectMultiplayer();
        return;
    }
    
    if(millis() - reconnectCooldownTime < RECONNECT_COOLDOWN && reconnectAttempts > 0) {
        return;
    }
    
    isReconnecting = true;
    currentLedPattern = LedPattern::RECONNECTING;
    connectionLostTime = millis();
    reconnectCooldownTime = millis();
    
    saveGameState();
    inputBuffer.clear();
    
    if(remoteMacKnown) {
        esp_now_del_peer(remoteMac);
        esp_now_add_peer(broadcastMac, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
    }
}

void ArduboyPlatform::handleReconnection() {
    unsigned long currentTime = millis();
    
    if(currentTime - lastPacketSentTime > RECONNECT_DELAY) {
        NetworkPacket packet;
        packet.packetType = RECONNECT_SYNC_PACKET;
        packet.networkFrame = savedGameState.frameNumber;
        packet.data = hostId;
        packet.checkSum = RECONNECT_SYNC_PACKET + savedGameState.frameNumber + hostId;
        
        if(packetSent) {
            packetSent = false;
            uint8_t *targetMac = remoteMacKnown ? remoteMac : broadcastMac;
            esp_now_send(targetMac, (uint8_t *)&packet, sizeof(packet));
            lastPacketSentTime = currentTime;
        }
    }
    
    parseNetwork();
    
    if(packetReceived && (receivedPacket.packetType == SYNC_PACKET || 
                          receivedPacket.packetType == RECONNECT_SYNC_PACKET)) {
        if(remoteMacKnown) {
            esp_now_del_peer(broadcastMac);
            esp_now_add_peer(remoteMac, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
        }
        
        startSmoothRecovery();
    }
    
    if(currentTime - connectionLostTime > DISCOVERY_TIMEOUT) {
        reconnectAttempts++;
        
        if(reconnectAttempts < MAX_RECONNECT_ATTEMPTS) {
            connectionLostTime = currentTime;
            currentLedPattern = LedPattern::ERROR;
            delay(1000);
            currentLedPattern = LedPattern::RECONNECTING;
        } else {
            currentLedPattern = LedPattern::ERROR;
            delay(3000);
            disconnectMultiplayer();
        }
    }
}

void ArduboyPlatform::startSmoothRecovery() {
    smoothRecoveryActive = true;
    smoothRecoveryStep = 0;
    currentLedPattern = LedPattern::CONNECTING;
    inputBuffer.clear();
}

void ArduboyPlatform::handleSmoothRecovery() {
    unsigned long currentTime = millis();
    const uint8_t MAX_BRIGHTNESS = 3;
    
    switch(smoothRecoveryStep) {
        case 0:
            sendNetworkPacket(RECONNECT_SYNC_PACKET, savedGameState.frameNumber);
            smoothRecoveryStep++;
            break;
            
        case 1:
            if(packetReceived && receivedPacket.packetType == RECONNECT_SYNC_PACKET) {
                smoothRecoveryStep++;
            }
            if(currentTime - lastPacketSentTime > 1000) {
                smoothRecoveryStep = 0;
            }
            break;
            
        case 2:
            restoreGameState();
            smoothRecoveryStep++;
            break;
            
        case 3:
            sendNetworkPacket(UPDATE_PACKET, inputState[LOCAL_PLAYER]);
            smoothRecoveryStep++;
            break;
            
        case 4:
            if(packetReceived && receivedPacket.packetType == UPDATE_PACKET) {
                inputState[REMOTE_PLAYER] = receivedPacket.data;
                
                isReconnecting = false;
                smoothRecoveryActive = false;
                reconnectAttempts = 0;
                currentLedPattern = LedPattern::CONNECTED;
                isWaitingForRemote = false;
                resendCount = 0;
                lastPacketReceivedTime = currentTime;
                lastSendTime = currentTime;
                
                arduboy.setRGBled(0, MAX_BRIGHTNESS, 0);
                delay(500);
                currentLedPattern = LedPattern::CONNECTED;
            }
            if(currentTime - lastPacketSentTime > 1000) {
                smoothRecoveryStep = 3;
            }
            break;
    }
}

void ArduboyPlatform::saveGameState() {
    savedGameState.player1Input = inputState[LOCAL_PLAYER];
    savedGameState.player2Input = inputState[REMOTE_PLAYER];
    savedGameState.frameNumber = networkFrame;
    savedGameState.timestamp = millis();
}

void ArduboyPlatform::restoreGameState() {
    inputState[LOCAL_PLAYER] = savedGameState.player1Input;
    inputState[REMOTE_PLAYER] = savedGameState.player2Input;
    networkFrame = savedGameState.frameNumber;
}

void ArduboyPlatform::sendNetworkPacket(uint8_t packetType, uint8_t data) {
    if (!packetSent) {
        return;
    }
    
    NetworkPacket packet;
    packet.packetType = packetType;
    packet.networkFrame = networkFrame;
    packet.data = data;
    packet.checkSum = packetType + networkFrame + data;
    
    packetSent = false;
    
    uint8_t *targetMac = remoteMacKnown ? remoteMac : broadcastMac;
    esp_now_send(targetMac, (uint8_t *)&packet, sizeof(packet));
    
    lastPacketSentTime = millis();
}

void ArduboyPlatform::parseNetwork() {
    if (!packetReceived) {
        return;
    }
    
    packetReceived = false;
    
    NetworkPacket *pkt = &receivedPacket;
    
    // Игнорируем свои heartbeat
    if (pkt->packetType == HEARTBEAT_PACKET && pkt->data == (hostId & 0xFF)) {
        return;
    }
    
    uint8_t testChecksum = pkt->packetType + pkt->networkFrame + pkt->data;
    
    if(pkt->checkSum != testChecksum) {
        currentLedPattern = LedPattern::ERROR;
        sendNetworkPacket(NACK_PACKET);
        return;
    }
    
    lastPacketReceivedTime = millis();
    
    if(pkt->packetType == HEARTBEAT_PACKET) {
        return;
    }
    
    if(pkt->packetType == RECONNECT_SYNC_PACKET) {
        if(isReconnecting) {
            sendNetworkPacket(RECONNECT_SYNC_PACKET, hostId);
        }
        return;
    }
    
    if(pkt->packetType == SYNC_PACKET && connectionStatus == ConnectionStatus::Disconnected) {
        //Serial.print("SYNC received. My hostId: "); Serial.print(hostId);
        //Serial.print(" Remote hostId: "); Serial.println(pkt->data);
        
        if(hostId > pkt->data) {
            connectionStatus = ConnectionStatus::SerialHost;
            currentLedPattern = LedPattern::CONNECTED;
            //Serial.println("I am HOST");
        }
        else if(hostId < pkt->data) {
            connectionStatus = ConnectionStatus::SerialClient;
            currentLedPattern = LedPattern::CONNECTED;
            //Serial.println("I am CLIENT");
        }
        return;
    }
    
    if(pkt->packetType == UPDATE_PACKET && connectionStatus != ConnectionStatus::Disconnected) {
        // КРИТИЧЕСКОЕ ИСПРАВЛЕНИЕ: ВСЕГДА добавляем в буфер
        // Независимо от networkFrame!
        inputBuffer.push(pkt->data);
        
        // Обновляем networkFrame ТОЛЬКО если он совпадает
        if(pkt->networkFrame == networkFrame) {
            networkFrame++;
            isWaitingForRemote = false;
            resendCount = 0;
            currentLedPattern = LedPattern::CONNECTED;
        }
        // Не наказываем за рассинхрон - просто продолжаем
        
        //Serial.print("UPDATE recv. Remote frame: "); Serial.print(pkt->networkFrame);
        //Serial.print(" My frame: "); Serial.print(networkFrame);
        //Serial.print(" Data: 0x"); Serial.println(pkt->data, HEX);
        return;
    }
    
    if(pkt->packetType == NACK_PACKET && connectionStatus != ConnectionStatus::Disconnected) {
        if(pkt->networkFrame == networkFrame) {
            sendNetworkPacket(UPDATE_PACKET, inputState[LOCAL_PLAYER]);
        }
        else if(pkt->networkFrame == (uint8_t)(networkFrame - 1)) {
            networkFrame--;
            sendNetworkPacket(UPDATE_PACKET, lastInputState[LOCAL_PLAYER]);
            networkFrame++;
        }
        return;
    }
}

void ArduboyPlatform::disconnectMultiplayer() {
    //Serial.println("Disconnecting...");
    esp_now_deinit();
    WiFi.disconnect();
    connectionStatus = ConnectionStatus::Disconnected;
    remoteMacKnown = false;
    isReconnecting = false;
    smoothRecoveryActive = false;
    reconnectAttempts = 0;
    currentLedPattern = LedPattern::OFF;
    inputBuffer.clear();
    memset(remoteMac, 0, 6);
    arduboy.setRGBled(0, 0, 0);
}

bool ArduboyPlatform::connectMultiplayer() {
    //Serial.begin(115200);
    //Serial.println("Starting multiplayer connection...");
    
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    
    WiFi.macAddress(selfMac);
    //Serial.print("My MAC: ");
    //for(int i=0; i<6; i++) {
    //    Serial.print(selfMac[i], HEX);
    //    if(i<5) Serial.print(":");
    //}
    //Serial.println();
    
    if (esp_now_init() != 0) {
        //Serial.println("ESP-NOW init failed!");
        currentLedPattern = LedPattern::ERROR;
        return false;
    }
    
    esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
    esp_now_register_send_cb(onDataSent);
    esp_now_register_recv_cb(onDataRecv);
    
    esp_now_add_peer(broadcastMac, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
    
    arduboy.display(true);
    
    networkFrame = 0;
    isWaitingForRemote = false;
    connectionStatus = ConnectionStatus::Disconnected;
    packetReceived = false;
    packetSent = true;
    remoteMacKnown = false;
    isReconnecting = false;
    smoothRecoveryActive = false;
    reconnectAttempts = 0;
    resendCount = 0;
    packetsReceived = 0;
    packetsSent = 0;
    validFrames = 0;
    missedFrames = 0;
    lastPacketSentTime = millis();
    lastPacketReceivedTime = millis();
    lastSendTime = millis();
    discoveryStartTime = millis();
    
    inputBuffer.clear();
    
    currentLedPattern = LedPattern::SEARCHING;
    
    //Serial.print("Sending SYNC with hostId: "); Serial.println(hostId);
    sendNetworkPacket(SYNC_PACKET, hostId);
    
    while(connectionStatus == ConnectionStatus::Disconnected) {
        updateLedPattern();
        
        if((millis() - lastPacketSentTime) > 1000) {
            //Serial.println("Resending SYNC...");
            sendNetworkPacket(SYNC_PACKET, hostId);
        }
        
        if((millis() - discoveryStartTime) > DISCOVERY_TIMEOUT) {
            //Serial.println("Discovery timeout!");
            currentLedPattern = LedPattern::DISCOVERY_TIMEOUT_PATTERN;
            
            unsigned long timeoutStart = millis();
            while(millis() - timeoutStart < 3000) {
                updateLedPattern();
                yield();
                delay(10);
            }
            
            disconnectMultiplayer();
            return false;
        }
        
        parseNetwork();
        yield();
        delay(1);
    }
    
    currentLedPattern = LedPattern::CONNECTING;
    
    if(remoteMacKnown) {
        //Serial.print("Remote MAC found: ");
        //for(int i=0; i<6; i++) {
        //    Serial.print(remoteMac[i], HEX);
        //    if(i<5) Serial.print(":");
        //}
        //Serial.println();
        
        esp_now_del_peer(broadcastMac);
        esp_now_add_peer(remoteMac, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
    }
    
    lastPacketReceivedTime = millis();
    lastSendTime = millis();
    
    sendNetworkPacket(SYNC_PACKET, hostId);
    
    currentLedPattern = LedPattern::CONNECTED;
    //Serial.println("Connected!");
    
    return connectionStatus == ConnectionStatus::SerialHost;
}