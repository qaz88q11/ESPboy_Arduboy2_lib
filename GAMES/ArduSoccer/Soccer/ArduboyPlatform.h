#pragma once

#include <avr/pgmspace.h>
#include <Arduboy2.h>      
#include "Platform.h"

extern Arduboy2Base arduboy;

inline void clearDisplay(uint8_t colour)
{
	uint8_t data = colour ? 0xff : 0;
	uint8_t* ptr = arduboy.sBuffer;
	int count = 128 * 64 / 8;
	while(count--)
		*ptr++ = data;
}

enum class ConnectionStatus
{
	Disconnected,
	SerialHost,
	SerialClient
};

enum class LedPattern {
	OFF,
	SEARCHING,
	CONNECTING,     
	CONNECTED,      
	ERROR,          
	RECONNECTING,   
	DISCOVERY_TIMEOUT_PATTERN
};

struct GameState {
	uint8_t player1Input;
	uint8_t player2Input;
	uint8_t frameNumber;
	unsigned long timestamp;
};

struct NetworkPacket {
	uint8_t packetType;
	uint8_t networkFrame;
	uint8_t data;
	uint8_t checkSum;
} __attribute__((packed));

// Буфер для сглаживания
struct InputBuffer {
	static constexpr uint8_t SIZE = 8;  // Буфер на 8 кадров
	uint8_t buffer[SIZE];
	uint8_t writeIndex = 0;
	uint8_t readIndex = 0;
	uint8_t count = 0;
	
	bool push(uint8_t input) {
		if(count < SIZE) {
			buffer[writeIndex] = input;
			writeIndex = (writeIndex + 1) % SIZE;
			count++;
			return true;
		}
		return false;
	}
	
	bool pop(uint8_t &input) {
		if(count > 0) {
			input = buffer[readIndex];
			readIndex = (readIndex + 1) % SIZE;
			count--;
			return true;
		}
		return false;
	}
	
	void clear() {
		writeIndex = 0;
		readIndex = 0;
		count = 0;
	}
};

class ArduboyPlatform : public PlatformBase
{
public:
	void playSound(const uint16_t* sound);
	bool connectMultiplayer();
	void disconnectMultiplayer();

	void update();
	void sendNetworkPacket(uint8_t packetType, uint8_t data = 0);
	void parseNetwork();
	
	void updateInput();
	
	// Новые методы
	void updateLedPattern();
	void sendHeartbeat();
	void checkConnection();
	void startReconnection();
	void handleReconnection();
	void startSmoothRecovery();
	void handleSmoothRecovery();
	void saveGameState();
	void restoreGameState();
	
	ConnectionStatus connectionStatus = ConnectionStatus::Disconnected;
	uint8_t networkFrame;
	uint8_t hostId;
	unsigned long lastPacketSentTime = 0;
	bool isWaitingForRemote;
	bool m_isMuted = false;
	
	// Асинхронные переменные
	InputBuffer inputBuffer;
	unsigned long lastFrameTime = 0;
	unsigned long lastSendTime = 0;
	static constexpr unsigned long FRAME_DELAY = 16;  // ~60 FPS
	static constexpr unsigned long SEND_DELAY = 8;    // Отправка каждые 8ms
};

extern const uint8_t UPDATE_PACKET;
extern const uint8_t NACK_PACKET;
extern const uint8_t SYNC_PACKET;
extern const uint8_t HEARTBEAT_PACKET;
extern const uint8_t RECONNECT_SYNC_PACKET;

extern uint8_t remoteMac[6];
extern uint8_t selfMac[6];
extern bool remoteMacKnown;
extern volatile bool packetReceived;
extern volatile bool packetSent;
extern unsigned long lastPacketReceivedTime;
extern unsigned long lastHeartbeatTime;
extern unsigned long lastLedToggleTime;
extern unsigned long connectionLostTime;
extern unsigned long discoveryStartTime;
extern unsigned long reconnectCooldownTime;
extern bool ledState;
extern bool isReconnecting;
extern bool smoothRecoveryActive;
extern uint8_t reconnectAttempts;
extern uint8_t smoothRecoveryStep;
extern GameState savedGameState;
extern NetworkPacket receivedPacket;
extern LedPattern currentLedPattern;

void ERROR(const char* msg);

extern ArduboyPlatform Platform;

inline void drawBitmap(int16_t x, int16_t y, const uint8_t* bitmap, uint8_t w, uint8_t h, uint8_t color)
{
	arduboy.drawBitmap(x, y, bitmap, w, h, color);
}

inline void fillRect(int16_t x, int16_t y, uint8_t w, uint8_t h, uint8_t colour)
{
	arduboy.fillRect(x, y, w, h, colour);
}