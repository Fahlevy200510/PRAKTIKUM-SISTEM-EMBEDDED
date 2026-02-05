# Modul 13: Network Communication - IoT Connectivity

## Daftar Isi
1. [Pendahuluan](#1-pendahuluan)
2. [Arsitektur Jaringan IoT](#2-arsitektur-jaringan-iot)
3. [WiFi pada ESP32](#3-wifi-pada-esp32)
4. [Protokol TCP/IP](#4-protokol-tcpip)
5. [HTTP Client & Server](#5-http-client--server)
6. [MQTT Protocol](#6-mqtt-protocol)
7. [WebSocket](#7-websocket)
8. [Bluetooth Classic & BLE](#8-bluetooth-classic--ble)
9. [STM32 Serial Network Bridge](#9-stm32-serial-network-bridge)
10. [Keamanan Jaringan](#10-keamanan-jaringan)
11. [Best Practices](#11-best-practices)

---

## 1. Pendahuluan

### 1.1 Apa itu IoT Connectivity?

Internet of Things (IoT) connectivity mengacu pada kemampuan perangkat embedded untuk berkomunikasi melalui jaringan, baik lokal (LAN) maupun global (Internet).

```
┌─────────────────────────────────────────────────────────────────────┐
│                    IoT CONNECTIVITY ECOSYSTEM                        │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│    ┌──────────┐         ┌──────────┐         ┌──────────┐          │
│    │ Sensors  │────────▶│  MCU     │────────▶│  Cloud   │          │
│    │          │  Data   │(ESP32/   │  WiFi/  │ Platform │          │
│    └──────────┘         │ STM32)   │  BLE    │          │          │
│                         └──────────┘         └──────────┘          │
│                              │                    │                 │
│                              │                    │                 │
│                              ▼                    ▼                 │
│                        ┌──────────┐        ┌──────────┐            │
│                        │  Mobile  │        │  Web     │            │
│                        │   App    │        │Dashboard │            │
│                        └──────────┘        └──────────┘            │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

### 1.2 Mengapa Network Communication Penting?

| Aspek | Manfaat |
|-------|---------|
| **Remote Monitoring** | Pantau sensor dari mana saja |
| **Data Analytics** | Kirim data ke cloud untuk analisis |
| **Remote Control** | Kontrol aktuator dari jarak jauh |
| **Firmware Update** | OTA (Over-The-Air) update |
| **Integration** | Integrasi dengan sistem lain |

### 1.3 Platform dalam Modul Ini

- **ESP32**: WiFi + Bluetooth built-in
- **STM32**: Serial bridge ke ESP32 untuk network access

---

## 2. Arsitektur Jaringan IoT

### 2.1 Model OSI dan TCP/IP

```
┌─────────────────────────────────────────────────────────────┐
│              OSI Model vs TCP/IP Model                       │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│    OSI Model              TCP/IP Model                       │
│    ─────────              ────────────                       │
│  ┌───────────┐                                              │
│  │Application│          ┌─────────────┐                     │
│  ├───────────┤          │ Application │ HTTP, MQTT, WS      │
│  │Presentation          │   Layer     │                     │
│  ├───────────┤          └─────────────┘                     │
│  │  Session  │                                              │
│  ├───────────┤          ┌─────────────┐                     │
│  │ Transport │          │  Transport  │ TCP, UDP            │
│  ├───────────┤          │    Layer    │                     │
│  │  Network  │          └─────────────┘                     │
│  ├───────────┤          ┌─────────────┐                     │
│  │ Data Link │          │  Internet   │ IP, ICMP            │
│  ├───────────┤          │    Layer    │                     │
│  │ Physical  │          └─────────────┘                     │
│  └───────────┘          ┌─────────────┐                     │
│                         │Network Access│ WiFi, Ethernet     │
│                         └─────────────┘                     │
│                                                              │
└─────────────────────────────────────────────────────────────┘
```

### 2.2 Protokol IoT Umum

```
┌─────────────────────────────────────────────────────────────────────┐
│                     IoT PROTOCOL COMPARISON                          │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│  Protocol   │ Transport │ Pattern    │ QoS │ Best For              │
│  ───────────┼───────────┼────────────┼─────┼───────────────────    │
│  HTTP/REST  │ TCP       │ Req/Resp   │ No  │ API, Web services     │
│  MQTT       │ TCP       │ Pub/Sub    │ Yes │ Sensor data, telemetry│
│  WebSocket  │ TCP       │ Full-duplex│ No  │ Real-time dashboard   │
│  CoAP       │ UDP       │ Req/Resp   │ Yes │ Constrained devices   │
│  Bluetooth  │ BLE/Classic│ Various   │ Yes │ Short-range, wearables│
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 3. WiFi pada ESP32

### 3.1 ESP32 WiFi Capabilities

ESP32 mendukung WiFi 802.11 b/g/n dengan berbagai mode:

| Mode | Deskripsi | Use Case |
|------|-----------|----------|
| **Station (STA)** | Connect ke Access Point | Normal IoT device |
| **Access Point (AP)** | Menjadi Access Point | Configuration portal |
| **AP+STA** | Keduanya simultan | Bridge, Repeater |

### 3.2 WiFi Station Mode

```cpp
#include <WiFi.h>

const char* ssid = "YourNetwork";
const char* password = "YourPassword";

void setup() {
    Serial.begin(115200);
    
    // Start WiFi connection
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    
    Serial.print("Connecting");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    
    Serial.println("\nConnected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
}
```

### 3.3 WiFi Events

```cpp
// Event handler untuk WiFi events
void WiFiEvent(WiFiEvent_t event) {
    switch (event) {
        case SYSTEM_EVENT_STA_GOT_IP:
            Serial.println("Got IP address");
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            Serial.println("Disconnected from WiFi");
            // Attempt reconnection
            WiFi.reconnect();
            break;
        default:
            break;
    }
}

// Register event handler
WiFi.onEvent(WiFiEvent);
```

### 3.4 WiFi Access Point Mode

```cpp
#include <WiFi.h>

const char* ap_ssid = "ESP32_Config";
const char* ap_password = "12345678";

void setup() {
    Serial.begin(115200);
    
    // Configure AP
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ap_ssid, ap_password);
    
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());
}
```

### 3.5 Dual Mode (AP + Station)

```cpp
void setup() {
    WiFi.mode(WIFI_AP_STA);
    
    // Start AP
    WiFi.softAP("ESP32_Bridge", "password");
    
    // Connect to existing network
    WiFi.begin("ExistingNetwork", "password");
    
    // Now ESP32 acts as bridge
}
```

---

## 4. Protokol TCP/IP

### 4.1 TCP Client

```cpp
#include <WiFi.h>

WiFiClient client;

void sendTCPData(const char* host, uint16_t port, const char* data) {
    if (client.connect(host, port)) {
        Serial.println("Connected to server");
        
        client.print(data);
        
        // Wait for response
        while (client.connected()) {
            while (client.available()) {
                char c = client.read();
                Serial.print(c);
            }
        }
        
        client.stop();
    } else {
        Serial.println("Connection failed");
    }
}
```

### 4.2 TCP Server

```cpp
#include <WiFi.h>

WiFiServer server(8080);

void setup() {
    // ... WiFi setup ...
    server.begin();
    Serial.println("TCP Server started");
}

void loop() {
    WiFiClient client = server.available();
    
    if (client) {
        Serial.println("Client connected");
        
        while (client.connected()) {
            if (client.available()) {
                String request = client.readStringUntil('\n');
                Serial.println("Received: " + request);
                
                // Send response
                client.println("ACK: " + request);
            }
        }
        
        client.stop();
        Serial.println("Client disconnected");
    }
}
```

### 4.3 UDP Communication

```cpp
#include <WiFi.h>
#include <WiFiUdp.h>

WiFiUDP udp;
const int udpPort = 1234;

void setup() {
    // ... WiFi setup ...
    udp.begin(udpPort);
    Serial.println("UDP listening on port " + String(udpPort));
}

void loop() {
    int packetSize = udp.parsePacket();
    
    if (packetSize) {
        char buffer[255];
        int len = udp.read(buffer, 255);
        buffer[len] = '\0';
        
        Serial.printf("Received from %s:%d - %s\n",
                     udp.remoteIP().toString().c_str(),
                     udp.remotePort(),
                     buffer);
        
        // Send response
        udp.beginPacket(udp.remoteIP(), udp.remotePort());
        udp.printf("Echo: %s", buffer);
        udp.endPacket();
    }
}
```

---

## 5. HTTP Client & Server

### 5.1 HTTP Client - GET Request

```cpp
#include <WiFi.h>
#include <HTTPClient.h>

void httpGET(const char* url) {
    HTTPClient http;
    
    http.begin(url);
    
    int httpCode = http.GET();
    
    if (httpCode > 0) {
        Serial.printf("HTTP GET code: %d\n", httpCode);
        
        if (httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            Serial.println("Response: " + payload);
        }
    } else {
        Serial.printf("HTTP GET failed: %s\n", 
                     http.errorToString(httpCode).c_str());
    }
    
    http.end();
}
```

### 5.2 HTTP Client - POST Request

```cpp
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

void httpPOST(const char* url, float temperature, float humidity) {
    HTTPClient http;
    
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    
    // Create JSON payload
    StaticJsonDocument<200> doc;
    doc["sensor_id"] = "ESP32_001";
    doc["temperature"] = temperature;
    doc["humidity"] = humidity;
    doc["timestamp"] = millis();
    
    String jsonString;
    serializeJson(doc, jsonString);
    
    int httpCode = http.POST(jsonString);
    
    if (httpCode > 0) {
        Serial.printf("HTTP POST code: %d\n", httpCode);
        String response = http.getString();
        Serial.println("Response: " + response);
    } else {
        Serial.printf("HTTP POST failed: %s\n",
                     http.errorToString(httpCode).c_str());
    }
    
    http.end();
}
```

### 5.3 HTTP Server (Web Server)

```cpp
#include <WiFi.h>
#include <WebServer.h>

WebServer server(80);

float temperature = 25.5;
float humidity = 60.0;

void handleRoot() {
    String html = "<!DOCTYPE html><html>";
    html += "<head><title>ESP32 Sensor</title>";
    html += "<meta http-equiv='refresh' content='5'></head>";
    html += "<body>";
    html += "<h1>ESP32 Sensor Dashboard</h1>";
    html += "<p>Temperature: " + String(temperature) + " °C</p>";
    html += "<p>Humidity: " + String(humidity) + " %</p>";
    html += "</body></html>";
    
    server.send(200, "text/html", html);
}

void handleAPI() {
    StaticJsonDocument<200> doc;
    doc["temperature"] = temperature;
    doc["humidity"] = humidity;
    doc["uptime"] = millis() / 1000;
    
    String json;
    serializeJson(doc, json);
    
    server.send(200, "application/json", json);
}

void setup() {
    // ... WiFi setup ...
    
    server.on("/", handleRoot);
    server.on("/api/sensor", handleAPI);
    
    server.begin();
    Serial.println("HTTP server started");
}

void loop() {
    server.handleClient();
}
```

### 5.4 RESTful API Design

```cpp
// RESTful endpoints
server.on("/api/sensors", HTTP_GET, []() {
    // Return all sensors
    String json = getSensorsJSON();
    server.send(200, "application/json", json);
});

server.on("/api/sensors/{id}", HTTP_GET, []() {
    // Return specific sensor
    String id = server.pathArg(0);
    String json = getSensorJSON(id);
    server.send(200, "application/json", json);
});

server.on("/api/actuators/{id}", HTTP_POST, []() {
    // Control actuator
    String id = server.pathArg(0);
    String body = server.arg("plain");
    controlActuator(id, body);
    server.send(200, "application/json", "{\"status\":\"ok\"}");
});
```

---

## 6. MQTT Protocol

### 6.1 Apa itu MQTT?

MQTT (Message Queuing Telemetry Transport) adalah protokol messaging lightweight untuk IoT.

```
┌──────────────────────────────────────────────────────────────────┐
│                    MQTT ARCHITECTURE                              │
├──────────────────────────────────────────────────────────────────┤
│                                                                   │
│    ┌──────────┐          ┌───────────┐         ┌──────────┐     │
│    │Publisher │─publish─▶│  BROKER   │─deliver─▶│Subscriber│     │
│    │ (ESP32)  │          │(Mosquitto)│          │ (App)    │     │
│    └──────────┘          └───────────┘         └──────────┘     │
│                               │                                  │
│                               │                                  │
│    ┌──────────┐               │                ┌──────────┐     │
│    │Publisher │───publish────┘────deliver─────▶│Subscriber│     │
│    │ (Sensor) │                                │(Dashboard)     │
│    └──────────┘                                └──────────┘     │
│                                                                   │
│    Topic: "sensors/temperature/room1"                            │
│    Payload: {"temp": 25.5, "unit": "C"}                         │
│                                                                   │
└──────────────────────────────────────────────────────────────────┘
```

### 6.2 MQTT Concepts

| Concept | Deskripsi |
|---------|-----------|
| **Broker** | Server yang mengelola messages |
| **Client** | Device yang connect ke broker |
| **Topic** | Channel untuk routing messages |
| **Publish** | Mengirim message ke topic |
| **Subscribe** | Mendaftar untuk menerima messages dari topic |
| **QoS** | Quality of Service level |

### 6.3 QoS Levels

```
QoS 0: At most once (Fire and forget)
┌────────┐    message    ┌────────┐
│ Client │──────────────▶│ Broker │
└────────┘               └────────┘

QoS 1: At least once (Acknowledged)
┌────────┐    message    ┌────────┐
│ Client │──────────────▶│ Broker │
│        │◀─────PUBACK───│        │
└────────┘               └────────┘

QoS 2: Exactly once (Assured delivery)
┌────────┐    message    ┌────────┐
│ Client │──────────────▶│ Broker │
│        │◀────PUBREC────│        │
│        │─────PUBREL───▶│        │
│        │◀────PUBCOMP───│        │
└────────┘               └────────┘
```

### 6.4 MQTT Client Implementation

```cpp
#include <WiFi.h>
#include <PubSubClient.h>

const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient mqtt(espClient);

// Callback saat menerima message
void mqttCallback(char* topic, byte* payload, unsigned int length) {
    Serial.printf("Message on [%s]: ", topic);
    
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();
    
    // Parse JSON payload
    StaticJsonDocument<256> doc;
    deserializeJson(doc, payload, length);
    
    if (doc.containsKey("led")) {
        bool ledState = doc["led"];
        digitalWrite(LED_BUILTIN, ledState);
    }
}

void mqttReconnect() {
    while (!mqtt.connected()) {
        Serial.print("Connecting to MQTT...");
        
        String clientId = "ESP32_" + String(random(0xffff), HEX);
        
        if (mqtt.connect(clientId.c_str())) {
            Serial.println("connected!");
            
            // Subscribe to topics
            mqtt.subscribe("home/control/#");
            
            // Publish online status
            mqtt.publish("home/status", "{\"device\":\"ESP32\",\"status\":\"online\"}");
        } else {
            Serial.printf("failed, rc=%d. Retry in 5s\n", mqtt.state());
            delay(5000);
        }
    }
}

void setup() {
    Serial.begin(115200);
    
    // WiFi setup...
    
    mqtt.setServer(mqtt_server, mqtt_port);
    mqtt.setCallback(mqttCallback);
}

void loop() {
    if (!mqtt.connected()) {
        mqttReconnect();
    }
    mqtt.loop();
    
    // Publish sensor data periodically
    static unsigned long lastPublish = 0;
    if (millis() - lastPublish > 10000) {
        StaticJsonDocument<200> doc;
        doc["temperature"] = random(200, 350) / 10.0;
        doc["humidity"] = random(400, 800) / 10.0;
        
        char buffer[256];
        serializeJson(doc, buffer);
        
        mqtt.publish("home/sensors/living_room", buffer);
        lastPublish = millis();
    }
}
```

### 6.5 MQTT Topic Design

```
Topic Hierarchy Best Practices:

home/
├── sensors/
│   ├── living_room/
│   │   ├── temperature
│   │   └── humidity
│   └── bedroom/
│       ├── temperature
│       └── motion
├── actuators/
│   ├── lights/
│   │   ├── living_room
│   │   └── bedroom
│   └── hvac/
│       └── thermostat
└── status/
    ├── esp32_001
    └── esp32_002

Wildcards:
+ (single level): home/sensors/+/temperature
# (multi level): home/sensors/#
```

---

## 7. WebSocket

### 7.1 WebSocket vs HTTP

```
HTTP (Request-Response):
┌────────┐  Request   ┌────────┐
│ Client │───────────▶│ Server │
│        │◀───────────│        │
└────────┘  Response  └────────┘
(Connection closes)

WebSocket (Full-Duplex):
┌────────┐  Handshake  ┌────────┐
│ Client │◀───────────▶│ Server │
│        │◀───────────▶│        │
│        │  Messages   │        │
│        │◀───────────▶│        │
└────────┘ (Persistent)└────────┘
```

### 7.2 WebSocket Server

```cpp
#include <WiFi.h>
#include <WebSocketsServer.h>

WebSocketsServer webSocket(81);

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
    switch (type) {
        case WStype_DISCONNECTED:
            Serial.printf("[%u] Disconnected!\n", num);
            break;
            
        case WStype_CONNECTED: {
            IPAddress ip = webSocket.remoteIP(num);
            Serial.printf("[%u] Connected from %s\n", num, ip.toString().c_str());
            
            // Send welcome message
            webSocket.sendTXT(num, "{\"type\":\"welcome\",\"msg\":\"Connected to ESP32\"}");
            break;
        }
        
        case WStype_TEXT:
            Serial.printf("[%u] Received: %s\n", num, payload);
            
            // Parse and respond
            StaticJsonDocument<256> doc;
            deserializeJson(doc, payload);
            
            if (doc["cmd"] == "getSensor") {
                StaticJsonDocument<256> response;
                response["type"] = "sensorData";
                response["temperature"] = 25.5;
                response["humidity"] = 60.0;
                
                char buffer[256];
                serializeJson(response, buffer);
                webSocket.sendTXT(num, buffer);
            }
            break;
    }
}

void setup() {
    // WiFi setup...
    
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
    
    Serial.println("WebSocket server started on port 81");
}

void loop() {
    webSocket.loop();
    
    // Broadcast sensor data every 5 seconds
    static unsigned long lastBroadcast = 0;
    if (millis() - lastBroadcast > 5000) {
        StaticJsonDocument<256> doc;
        doc["type"] = "sensorUpdate";
        doc["temperature"] = random(200, 350) / 10.0;
        doc["humidity"] = random(400, 800) / 10.0;
        
        char buffer[256];
        serializeJson(doc, buffer);
        
        webSocket.broadcastTXT(buffer);
        lastBroadcast = millis();
    }
}
```

### 7.3 JavaScript Client (Browser)

```html
<!DOCTYPE html>
<html>
<head>
    <title>ESP32 WebSocket</title>
</head>
<body>
    <h1>ESP32 Real-time Dashboard</h1>
    <div id="temperature">Temperature: --</div>
    <div id="humidity">Humidity: --</div>
    
    <script>
        const ws = new WebSocket('ws://192.168.1.100:81');
        
        ws.onopen = () => {
            console.log('Connected to ESP32');
        };
        
        ws.onmessage = (event) => {
            const data = JSON.parse(event.data);
            
            if (data.type === 'sensorUpdate') {
                document.getElementById('temperature').textContent = 
                    `Temperature: ${data.temperature}°C`;
                document.getElementById('humidity').textContent = 
                    `Humidity: ${data.humidity}%`;
            }
        };
        
        ws.onclose = () => {
            console.log('Disconnected');
            // Reconnect logic
        };
    </script>
</body>
</html>
```

---

## 8. Bluetooth Classic & BLE

### 8.1 Bluetooth Overview

| Feature | Bluetooth Classic | BLE (Low Energy) |
|---------|-------------------|------------------|
| Range | ~100m | ~50m |
| Data Rate | 1-3 Mbps | 125kbps - 2Mbps |
| Power | Higher | Very Low |
| Use Case | Audio, File transfer | Sensors, Beacons |

### 8.2 Bluetooth Classic - Serial Port Profile (SPP)

```cpp
#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

void setup() {
    Serial.begin(115200);
    
    SerialBT.begin("ESP32_BT");  // Bluetooth device name
    Serial.println("Bluetooth ready. Pair with 'ESP32_BT'");
}

void loop() {
    // ESP32 to Bluetooth
    if (Serial.available()) {
        SerialBT.write(Serial.read());
    }
    
    // Bluetooth to ESP32
    if (SerialBT.available()) {
        Serial.write(SerialBT.read());
    }
}
```

### 8.3 BLE Server (Peripheral)

```cpp
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        deviceConnected = true;
        Serial.println("Client connected");
    }
    
    void onDisconnect(BLEServer* pServer) {
        deviceConnected = false;
        Serial.println("Client disconnected");
        // Restart advertising
        pServer->startAdvertising();
    }
};

void setup() {
    Serial.begin(115200);
    
    BLEDevice::init("ESP32_BLE_Sensor");
    
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());
    
    BLEService *pService = pServer->createService(SERVICE_UUID);
    
    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_NOTIFY
    );
    
    pCharacteristic->addDescriptor(new BLE2902());
    
    pService->start();
    
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->start();
    
    Serial.println("BLE Server ready");
}

void loop() {
    if (deviceConnected) {
        // Update characteristic value
        float temperature = random(200, 350) / 10.0;
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%.1f", temperature);
        
        pCharacteristic->setValue(buffer);
        pCharacteristic->notify();
        
        delay(1000);
    }
}
```

### 8.4 BLE Client (Central)

```cpp
#include <BLEDevice.h>
#include <BLEClient.h>

BLEClient* pClient = NULL;
bool connected = false;

static BLEUUID serviceUUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
static BLEUUID charUUID("beb5483e-36e1-4688-b7f5-ea07361b26a8");

void notifyCallback(BLERemoteCharacteristic* pChar, 
                    uint8_t* pData, size_t length, bool isNotify) {
    Serial.printf("Received: %.*s\n", length, pData);
}

void connectToServer(BLEAddress address) {
    pClient = BLEDevice::createClient();
    
    if (pClient->connect(address)) {
        Serial.println("Connected to server");
        
        BLERemoteService* pService = pClient->getService(serviceUUID);
        if (pService) {
            BLERemoteCharacteristic* pChar = pService->getCharacteristic(charUUID);
            if (pChar) {
                if (pChar->canNotify()) {
                    pChar->registerForNotify(notifyCallback);
                }
                connected = true;
            }
        }
    }
}
```

---

## 9. STM32 Serial Network Bridge

### 9.1 Konsep Bridge

STM32 tidak memiliki WiFi built-in. Solusinya adalah menggunakan ESP32 sebagai network bridge.

```
┌──────────────────────────────────────────────────────────────────┐
│                    STM32-ESP32 BRIDGE                             │
├──────────────────────────────────────────────────────────────────┤
│                                                                   │
│  ┌─────────┐   UART    ┌─────────┐   WiFi    ┌─────────┐        │
│  │  STM32  │◀────────▶│  ESP32  │◀─────────▶│  Cloud  │        │
│  │ Sensors │   Serial  │ Bridge  │  Network  │ Server  │        │
│  └─────────┘           └─────────┘           └─────────┘        │
│                                                                   │
│  Protocol: Simple ASCII or JSON over UART                        │
│  Baud Rate: 115200 typically                                     │
│                                                                   │
└──────────────────────────────────────────────────────────────────┘
```

### 9.2 STM32 Side - Serial Sender

```cpp
// STM32 Code (Arduino Framework)
#include <Arduino.h>

// UART2 for ESP32 communication
HardwareSerial SerialESP(PA3, PA2);  // RX, TX

void sendToESP32(const char* json) {
    SerialESP.println(json);
}

void setup() {
    Serial.begin(115200);      // Debug
    SerialESP.begin(115200);   // ESP32 communication
    
    Serial.println("STM32 Bridge Client Ready");
}

void loop() {
    // Read sensors
    float temperature = analogRead(PA0) * 3.3 / 4096 * 100;
    
    // Create JSON
    char json[128];
    snprintf(json, sizeof(json), 
             "{\"cmd\":\"publish\",\"topic\":\"sensors/stm32\",\"temp\":%.1f}",
             temperature);
    
    sendToESP32(json);
    Serial.println("Sent: " + String(json));
    
    // Check for responses
    if (SerialESP.available()) {
        String response = SerialESP.readStringUntil('\n');
        Serial.println("ESP32: " + response);
    }
    
    delay(5000);
}
```

### 9.3 ESP32 Side - Bridge Firmware

```cpp
// ESP32 Bridge Code
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

HardwareSerial SerialSTM32(2);  // UART2

WiFiClient espClient;
PubSubClient mqtt(espClient);

void processSTM32Command(String& json) {
    StaticJsonDocument<256> doc;
    deserializeJson(doc, json);
    
    const char* cmd = doc["cmd"];
    
    if (strcmp(cmd, "publish") == 0) {
        const char* topic = doc["topic"];
        float temp = doc["temp"];
        
        StaticJsonDocument<128> payload;
        payload["temperature"] = temp;
        payload["source"] = "STM32";
        
        char buffer[128];
        serializeJson(payload, buffer);
        
        mqtt.publish(topic, buffer);
        
        SerialSTM32.println("{\"status\":\"ok\"}");
    }
}

void setup() {
    Serial.begin(115200);
    SerialSTM32.begin(115200, SERIAL_8N1, 16, 17);
    
    // Connect WiFi
    WiFi.begin("SSID", "PASSWORD");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }
    
    mqtt.setServer("broker.hivemq.com", 1883);
    
    Serial.println("ESP32 Bridge Ready");
}

void loop() {
    if (!mqtt.connected()) {
        mqtt.connect("ESP32_Bridge");
    }
    mqtt.loop();
    
    // Process STM32 commands
    if (SerialSTM32.available()) {
        String json = SerialSTM32.readStringUntil('\n');
        processSTM32Command(json);
    }
}
```

---

## 10. Keamanan Jaringan

### 10.1 HTTPS (TLS/SSL)

```cpp
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

WiFiClientSecure secureClient;

// Root CA Certificate
const char* rootCA = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDdzCCAl+gAwIBAgIEAgAAuTANBgkqhkiG9w0BAQUFADBaMQswCQYDVQQGEwJJ
...
-----END CERTIFICATE-----
)EOF";

void secureRequest() {
    secureClient.setCACert(rootCA);
    
    HTTPClient https;
    
    if (https.begin(secureClient, "https://api.example.com/data")) {
        int httpCode = https.GET();
        
        if (httpCode > 0) {
            String payload = https.getString();
            Serial.println(payload);
        }
        
        https.end();
    }
}
```

### 10.2 MQTT with TLS

```cpp
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

WiFiClientSecure secureClient;
PubSubClient mqtt(secureClient);

const char* mqtt_server = "secure.hivemq.com";
const int mqtt_port = 8883;

void setup() {
    // Set CA certificate
    secureClient.setCACert(root_ca);
    
    // Or skip verification (not recommended for production)
    // secureClient.setInsecure();
    
    mqtt.setServer(mqtt_server, mqtt_port);
}
```

### 10.3 Best Security Practices

1. **Always use TLS/SSL** untuk komunikasi sensitif
2. **Validate certificates** - jangan skip verification
3. **Use strong passwords** untuk WiFi dan MQTT
4. **Implement authentication** di server/broker
5. **Encrypt payload** jika perlu confidentiality
6. **Use unique device IDs** untuk tracking

---

## 11. Best Practices

### 11.1 Connection Management

```cpp
// Robust WiFi connection with reconnection
void maintainWiFiConnection() {
    static unsigned long lastCheck = 0;
    
    if (millis() - lastCheck > 30000) {  // Check every 30s
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("WiFi disconnected. Reconnecting...");
            WiFi.disconnect();
            WiFi.reconnect();
            
            int attempts = 0;
            while (WiFi.status() != WL_CONNECTED && attempts < 20) {
                delay(500);
                attempts++;
            }
        }
        lastCheck = millis();
    }
}
```

### 11.2 Error Handling

```cpp
// HTTP request with proper error handling
bool sendData(const char* url, const String& data) {
    HTTPClient http;
    
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    http.setTimeout(10000);  // 10 second timeout
    
    int httpCode = http.POST(data);
    
    if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_CREATED) {
        Serial.println("Data sent successfully");
        http.end();
        return true;
    } else if (httpCode > 0) {
        Serial.printf("Server error: %d\n", httpCode);
    } else {
        Serial.printf("Connection error: %s\n", 
                     http.errorToString(httpCode).c_str());
    }
    
    http.end();
    return false;
}
```

### 11.3 Memory Management

```cpp
// Use StaticJsonDocument untuk size tetap (no fragmentation)
void parseJSON(const char* input) {
    StaticJsonDocument<256> doc;  // Stack allocation
    
    DeserializationError error = deserializeJson(doc, input);
    
    if (error) {
        Serial.printf("JSON parse error: %s\n", error.c_str());
        return;
    }
    
    // Process...
}

// Atau DynamicJsonDocument untuk size variabel
void parseLargeJSON(const char* input) {
    DynamicJsonDocument doc(4096);  // Heap allocation
    // ...
}
```

### 11.4 Power Considerations

```cpp
// Use WiFi sleep modes
void configurePowerSaving() {
    // Light sleep between transmissions
    esp_wifi_set_ps(WIFI_PS_MIN_MODEM);
    
    // Or disable WiFi completely when not needed
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    
    // Re-enable when needed
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
}
```

---

## Kesimpulan

Network communication adalah fondasi IoT modern. Pemahaman tentang:

1. **WiFi** - Konektivitas utama ESP32
2. **HTTP/REST** - API web standar
3. **MQTT** - Pub/Sub untuk telemetry
4. **WebSocket** - Real-time bidirectional
5. **Bluetooth** - Short-range connectivity
6. **Serial Bridge** - STM32 network access

Dengan menguasai protokol-protokol ini, Anda dapat membangun sistem IoT yang robust dan scalable.

---

## Referensi

1. ESP-IDF Programming Guide - WiFi
2. MQTT Version 5.0 Specification
3. RFC 6455 - WebSocket Protocol
4. Bluetooth Core Specification v5.2
5. ArduinoJson Library Documentation
