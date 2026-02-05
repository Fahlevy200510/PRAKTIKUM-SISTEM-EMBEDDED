# Jobsheet Modul 13: Network Communication - IoT Connectivity

## Informasi Umum

| Item | Keterangan |
|------|------------|
| **Mata Kuliah** | Praktikum Sistem Embedded |
| **Modul** | 13 - Network Communication |
| **Waktu** | 4 x 170 menit (4 pertemuan) |
| **Platform** | ESP32 DevKit V1, STM32F103C8T6 |

## Capaian Pembelajaran

Setelah menyelesaikan praktikum ini, mahasiswa mampu:
1. Mengkonfigurasi WiFi pada ESP32 dalam berbagai mode
2. Mengimplementasikan HTTP Client dan Server
3. Menggunakan protokol MQTT untuk IoT communication
4. Membangun aplikasi real-time dengan WebSocket
5. Mengimplementasikan Bluetooth Classic dan BLE
6. Membuat network bridge antara STM32 dan ESP32

---

# Pertemuan 1: WiFi & TCP/IP Fundamentals

## Praktikum 1: WiFi Station Mode (ESP32)

### Tujuan
Memahami koneksi WiFi dalam mode Station.

### Alat dan Bahan
- ESP32 DevKit V1
- LED + Resistor 330Ω
- Router WiFi

### Langkah Kerja

1. **Buat project baru** dengan nama `ESP32_01_WiFi_Station`

2. **Konfigurasi `platformio.ini`:**
```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
lib_deps = 
    bblanchon/ArduinoJson@^6.21.0
```

3. **Program WiFi Station:**
```cpp
#include <Arduino.h>
#include <WiFi.h>

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

const int LED_PIN = 2;

void printWiFiStatus() {
    Serial.println("\n=== WiFi Status ===");
    Serial.printf("SSID: %s\n", WiFi.SSID().c_str());
    Serial.printf("IP: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("Gateway: %s\n", WiFi.gatewayIP().toString().c_str());
    Serial.printf("DNS: %s\n", WiFi.dnsIP().toString().c_str());
    Serial.printf("MAC: %s\n", WiFi.macAddress().c_str());
    Serial.printf("RSSI: %d dBm\n", WiFi.RSSI());
    Serial.println("==================");
}

void WiFiEvent(WiFiEvent_t event) {
    switch (event) {
        case SYSTEM_EVENT_STA_START:
            Serial.println("[WiFi] Station started");
            break;
        case SYSTEM_EVENT_STA_CONNECTED:
            Serial.println("[WiFi] Connected to AP");
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            Serial.println("[WiFi] Got IP address");
            digitalWrite(LED_PIN, HIGH);
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            Serial.println("[WiFi] Disconnected");
            digitalWrite(LED_PIN, LOW);
            WiFi.reconnect();
            break;
        default:
            break;
    }
}

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    
    Serial.println("\n=== ESP32 WiFi Station Demo ===");
    
    WiFi.onEvent(WiFiEvent);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    
    Serial.printf("Connecting to %s", ssid);
    
    int timeout = 30;
    while (WiFi.status() != WL_CONNECTED && timeout > 0) {
        delay(1000);
        Serial.print(".");
        timeout--;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        printWiFiStatus();
    } else {
        Serial.println("\nConnection failed!");
    }
}

void loop() {
    static unsigned long lastPrint = 0;
    
    if (millis() - lastPrint > 10000) {
        if (WiFi.status() == WL_CONNECTED) {
            Serial.printf("[%lu] Connected - RSSI: %d dBm\n", 
                         millis()/1000, WiFi.RSSI());
        } else {
            Serial.printf("[%lu] Disconnected - reconnecting...\n", 
                         millis()/1000);
        }
        lastPrint = millis();
    }
}
```

### Tugas
1. Ubah kredensial WiFi sesuai jaringan Anda
2. Amati RSSI saat mendekat/menjauh dari router
3. Catat perubahan status saat router dimatikan/dinyalakan

---

## Praktikum 2: WiFi Access Point (ESP32)

### Tujuan
Membuat ESP32 sebagai Access Point untuk konfigurasi.

### Langkah Kerja

1. **Buat project** `ESP32_02_WiFi_AccessPoint`

2. **Program WiFi AP:**
```cpp
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

const char* ap_ssid = "ESP32_Config";
const char* ap_password = "12345678";

WebServer server(80);
int connectedClients = 0;

String generateHTML() {
    String html = "<!DOCTYPE html><html><head>";
    html += "<meta name='viewport' content='width=device-width'>";
    html += "<style>";
    html += "body{font-family:Arial;text-align:center;padding:20px;}";
    html += ".info{background:#f0f0f0;padding:15px;border-radius:10px;margin:10px;}";
    html += "</style></head><body>";
    html += "<h1>ESP32 Access Point</h1>";
    html += "<div class='info'>";
    html += "<p>AP IP: " + WiFi.softAPIP().toString() + "</p>";
    html += "<p>Connected Clients: " + String(WiFi.softAPgetStationNum()) + "</p>";
    html += "<p>Free Heap: " + String(ESP.getFreeHeap()) + " bytes</p>";
    html += "</div>";
    html += "<button onclick=\"location.reload()\">Refresh</button>";
    html += "</body></html>";
    return html;
}

void handleRoot() {
    server.send(200, "text/html", generateHTML());
}

void handleAPI() {
    String json = "{\"ip\":\"" + WiFi.softAPIP().toString() + "\",";
    json += "\"clients\":" + String(WiFi.softAPgetStationNum()) + ",";
    json += "\"heap\":" + String(ESP.getFreeHeap()) + "}";
    server.send(200, "application/json", json);
}

void setup() {
    Serial.begin(115200);
    Serial.println("\n=== ESP32 Access Point Demo ===");
    
    WiFi.mode(WIFI_AP);
    
    IPAddress local_IP(192, 168, 4, 1);
    IPAddress gateway(192, 168, 4, 1);
    IPAddress subnet(255, 255, 255, 0);
    
    WiFi.softAPConfig(local_IP, gateway, subnet);
    WiFi.softAP(ap_ssid, ap_password, 1, 0, 4);
    
    Serial.printf("AP SSID: %s\n", ap_ssid);
    Serial.printf("AP IP: %s\n", WiFi.softAPIP().toString().c_str());
    
    server.on("/", handleRoot);
    server.on("/api", handleAPI);
    server.begin();
    
    Serial.println("Web server started");
}

void loop() {
    server.handleClient();
    
    int clients = WiFi.softAPgetStationNum();
    if (clients != connectedClients) {
        connectedClients = clients;
        Serial.printf("Clients connected: %d\n", clients);
    }
}
```

### Tugas
1. Hubungkan smartphone ke AP ESP32
2. Akses halaman web di browser
3. Monitor jumlah client yang terkoneksi

---

## Praktikum 3: TCP Server (ESP32)

### Tujuan
Membangun TCP Server untuk komunikasi socket.

### Langkah Kerja

1. **Buat project** `ESP32_03_TCP_Server`

2. **Program TCP Server:**
```cpp
#include <Arduino.h>
#include <WiFi.h>

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

WiFiServer server(8888);
WiFiClient clients[4];
int clientCount = 0;

void broadcastMessage(const char* msg, int senderIdx = -1) {
    for (int i = 0; i < 4; i++) {
        if (clients[i] && clients[i].connected() && i != senderIdx) {
            clients[i].println(msg);
        }
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("\n=== ESP32 TCP Server ===");
    
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    
    Serial.printf("\nConnected! IP: %s\n", WiFi.localIP().toString().c_str());
    
    server.begin();
    Serial.println("TCP Server started on port 8888");
    Serial.println("Use: nc <IP> 8888 to connect");
}

void loop() {
    // Check for new connections
    WiFiClient newClient = server.available();
    if (newClient) {
        // Find empty slot
        for (int i = 0; i < 4; i++) {
            if (!clients[i] || !clients[i].connected()) {
                clients[i] = newClient;
                Serial.printf("Client %d connected from %s\n", 
                             i, clients[i].remoteIP().toString().c_str());
                clients[i].printf("Welcome! You are client #%d\r\n", i);
                
                char msg[64];
                snprintf(msg, sizeof(msg), "Client #%d joined", i);
                broadcastMessage(msg, i);
                break;
            }
        }
    }
    
    // Handle client data
    for (int i = 0; i < 4; i++) {
        if (clients[i] && clients[i].connected()) {
            if (clients[i].available()) {
                String data = clients[i].readStringUntil('\n');
                data.trim();
                
                Serial.printf("[Client %d]: %s\n", i, data.c_str());
                
                // Broadcast to others
                char msg[128];
                snprintf(msg, sizeof(msg), "[Client %d]: %s", i, data.c_str());
                broadcastMessage(msg, i);
                
                // Echo back
                clients[i].printf("Echo: %s\r\n", data.c_str());
            }
        }
    }
    
    delay(10);
}
```

### Tugas
1. Gunakan netcat atau Telnet untuk koneksi
2. Hubungkan multiple client
3. Test broadcast message

---

## Praktikum 4: UDP Communication (ESP32)

### Tujuan
Implementasi komunikasi UDP untuk data real-time.

### Langkah Kerja

1. **Buat project** `ESP32_04_UDP_Communication`

2. **Program UDP:**
```cpp
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

WiFiUDP udp;
const int UDP_PORT = 1234;

char packetBuffer[256];

void setup() {
    Serial.begin(115200);
    Serial.println("\n=== ESP32 UDP Demo ===");
    
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    
    Serial.printf("\nConnected! IP: %s\n", WiFi.localIP().toString().c_str());
    
    udp.begin(UDP_PORT);
    Serial.printf("UDP listening on port %d\n", UDP_PORT);
    Serial.println("Send UDP packet to test:");
    Serial.printf("echo 'Hello ESP32' | nc -u %s %d\n", 
                 WiFi.localIP().toString().c_str(), UDP_PORT);
}

void loop() {
    int packetSize = udp.parsePacket();
    
    if (packetSize) {
        Serial.printf("\n=== Packet Received ===\n");
        Serial.printf("From: %s:%d\n", 
                     udp.remoteIP().toString().c_str(), 
                     udp.remotePort());
        Serial.printf("Size: %d bytes\n", packetSize);
        
        int len = udp.read(packetBuffer, 255);
        packetBuffer[len] = '\0';
        Serial.printf("Data: %s\n", packetBuffer);
        
        // Send response
        udp.beginPacket(udp.remoteIP(), udp.remotePort());
        udp.printf("ESP32 received: %s", packetBuffer);
        udp.endPacket();
        
        Serial.println("Response sent!");
    }
    
    // Periodic broadcast
    static unsigned long lastBroadcast = 0;
    if (millis() - lastBroadcast > 5000) {
        IPAddress broadcastIP = WiFi.localIP();
        broadcastIP[3] = 255;
        
        udp.beginPacket(broadcastIP, UDP_PORT);
        udp.printf("ESP32 beacon: %lu", millis()/1000);
        udp.endPacket();
        
        Serial.printf("Broadcast sent to %s\n", broadcastIP.toString().c_str());
        lastBroadcast = millis();
    }
}
```

### Tugas
1. Test UDP dengan netcat
2. Monitor broadcast packets
3. Bandingkan latency dengan TCP

---

## Praktikum 5: STM32 Serial Communication (STM32)

### Tujuan
Menyiapkan STM32 untuk serial communication ke ESP32.

### Alat dan Bahan
- STM32F103C8T6 Blue Pill
- ST-Link V2

### Langkah Kerja

1. **Buat project** `STM32_01_Serial_Bridge_Prep`

2. **Konfigurasi `platformio.ini`:**
```ini
[env:bluepill_f103c8]
platform = ststm32
board = bluepill_f103c8
framework = arduino
upload_protocol = stlink
monitor_speed = 115200
build_flags = 
    -D SERIAL_UART_INSTANCE=1
```

3. **Program Serial:**
```cpp
#include <Arduino.h>

// UART2 untuk komunikasi dengan ESP32 (PA2=TX, PA3=RX)
HardwareSerial Serial2(PA3, PA2);

const int LED_PIN = PC13;

struct SensorData {
    float temperature;
    float humidity;
    uint32_t timestamp;
};

SensorData generateSensorData() {
    SensorData data;
    data.temperature = 20.0 + (random(0, 150) / 10.0);
    data.humidity = 40.0 + (random(0, 400) / 10.0);
    data.timestamp = millis();
    return data;
}

void sendJSON(SensorData& data) {
    char json[128];
    snprintf(json, sizeof(json), 
             "{\"temp\":%.1f,\"hum\":%.1f,\"ts\":%lu}",
             data.temperature, data.humidity, data.timestamp);
    
    Serial2.println(json);
    Serial.println("Sent: " + String(json));
}

void setup() {
    Serial.begin(115200);
    Serial2.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    
    Serial.println("\n=== STM32 Serial Bridge Client ===");
    Serial.println("UART2 ready for ESP32 connection");
    Serial.println("Wiring: STM32 PA2(TX) -> ESP32 RX");
    Serial.println("        STM32 PA3(RX) <- ESP32 TX");
}

void loop() {
    static unsigned long lastSend = 0;
    
    // Send sensor data every 5 seconds
    if (millis() - lastSend > 5000) {
        SensorData data = generateSensorData();
        sendJSON(data);
        
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        lastSend = millis();
    }
    
    // Process responses from ESP32
    if (Serial2.available()) {
        String response = Serial2.readStringUntil('\n');
        Serial.println("ESP32 Response: " + response);
        
        // Parse response
        if (response.indexOf("ACK") >= 0) {
            Serial.println("Data acknowledged by ESP32");
        }
    }
    
    // Forward Serial input to ESP32
    if (Serial.available()) {
        String cmd = Serial.readStringUntil('\n');
        Serial2.println(cmd);
        Serial.println("Forwarded: " + cmd);
    }
}
```

### Tugas
1. Hubungkan STM32 ke computer via Serial
2. Monitor output JSON
3. Persiapkan wiring untuk ESP32

---

# Pertemuan 2: HTTP & REST API

## Praktikum 6: HTTP Client GET (ESP32)

### Tujuan
Mengambil data dari web API menggunakan HTTP GET.

### Langkah Kerja

1. **Buat project** `ESP32_05_HTTP_Client`

2. **Konfigurasi `platformio.ini`:**
```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
lib_deps = 
    bblanchon/ArduinoJson@^6.21.0
```

3. **Program HTTP Client:**
```cpp
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Public test APIs
const char* weatherAPI = "http://api.open-meteo.com/v1/forecast?latitude=-6.2&longitude=106.8&current_weather=true";
const char* jsonPlaceholder = "https://jsonplaceholder.typicode.com/todos/1";

void httpGET(const char* url) {
    HTTPClient http;
    
    Serial.printf("\n=== GET Request ===\n");
    Serial.printf("URL: %s\n", url);
    
    http.begin(url);
    http.setTimeout(10000);
    
    int httpCode = http.GET();
    
    Serial.printf("HTTP Code: %d\n", httpCode);
    
    if (httpCode > 0) {
        if (httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            Serial.println("Response:");
            Serial.println(payload);
            
            // Parse JSON
            DynamicJsonDocument doc(2048);
            DeserializationError error = deserializeJson(doc, payload);
            
            if (!error) {
                Serial.println("\n--- Parsed Data ---");
                serializeJsonPretty(doc, Serial);
                Serial.println();
            }
        }
    } else {
        Serial.printf("Error: %s\n", http.errorToString(httpCode).c_str());
    }
    
    http.end();
}

void setup() {
    Serial.begin(115200);
    Serial.println("\n=== ESP32 HTTP Client ===");
    
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.printf("\nConnected! IP: %s\n", WiFi.localIP().toString().c_str());
}

void loop() {
    if (WiFi.status() == WL_CONNECTED) {
        httpGET(jsonPlaceholder);
        delay(10000);
    }
}
```

### Tugas
1. Test dengan berbagai API publik
2. Parse dan tampilkan data spesifik
3. Handle berbagai error codes

---

## Praktikum 7: HTTP Client POST (ESP32)

### Tujuan
Mengirim data ke server menggunakan HTTP POST.

### Langkah Kerja

1. **Buat project** `ESP32_06_HTTP_POST`

2. **Program HTTP POST:**
```cpp
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

const char* postURL = "https://jsonplaceholder.typicode.com/posts";

float readTemperature() {
    return 20.0 + (random(0, 150) / 10.0);
}

float readHumidity() {
    return 40.0 + (random(0, 400) / 10.0);
}

bool httpPOST(const char* url, String& jsonPayload) {
    HTTPClient http;
    
    Serial.printf("\n=== POST Request ===\n");
    Serial.printf("URL: %s\n", url);
    Serial.printf("Payload: %s\n", jsonPayload.c_str());
    
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    http.setTimeout(10000);
    
    int httpCode = http.POST(jsonPayload);
    
    Serial.printf("HTTP Code: %d\n", httpCode);
    
    if (httpCode > 0) {
        String response = http.getString();
        Serial.println("Response: " + response);
        http.end();
        return (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_CREATED);
    }
    
    Serial.printf("Error: %s\n", http.errorToString(httpCode).c_str());
    http.end();
    return false;
}

void sendSensorData() {
    StaticJsonDocument<256> doc;
    
    doc["device_id"] = "ESP32_001";
    doc["temperature"] = readTemperature();
    doc["humidity"] = readHumidity();
    doc["timestamp"] = millis();
    doc["location"] = "Lab Embedded";
    
    String jsonString;
    serializeJson(doc, jsonString);
    
    if (httpPOST(postURL, jsonString)) {
        Serial.println("Data sent successfully!");
    } else {
        Serial.println("Failed to send data!");
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("\n=== ESP32 HTTP POST ===");
    
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.printf("\nConnected! IP: %s\n", WiFi.localIP().toString().c_str());
}

void loop() {
    if (WiFi.status() == WL_CONNECTED) {
        sendSensorData();
        delay(15000);
    }
}
```

### Tugas
1. Kirim data sensor simulasi
2. Coba berbagai endpoint
3. Implementasikan retry mechanism

---

## Praktikum 8: Web Server dengan HTML (ESP32)

### Tujuan
Membangun web server dengan tampilan HTML interaktif.

### Langkah Kerja

1. **Buat project** `ESP32_07_Web_Server`

2. **Program Web Server:**
```cpp
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

WebServer server(80);

const int LED_PIN = 2;
bool ledState = false;
float temperature = 25.0;
float humidity = 60.0;

const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>ESP32 Dashboard</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            background: #1a1a2e;
            color: white;
            text-align: center;
            padding: 20px;
        }
        .container {
            max-width: 400px;
            margin: 0 auto;
        }
        .card {
            background: #16213e;
            border-radius: 10px;
            padding: 20px;
            margin: 15px 0;
            box-shadow: 0 4px 6px rgba(0,0,0,0.3);
        }
        .value {
            font-size: 48px;
            font-weight: bold;
            color: #e94560;
        }
        .unit {
            font-size: 24px;
            color: #888;
        }
        button {
            background: #e94560;
            color: white;
            border: none;
            padding: 15px 40px;
            font-size: 18px;
            border-radius: 25px;
            cursor: pointer;
            margin: 10px;
        }
        button:hover {
            background: #ff6b6b;
        }
        button.off {
            background: #444;
        }
        .status {
            padding: 10px;
            border-radius: 20px;
            display: inline-block;
        }
        .on { background: #4CAF50; }
        .off { background: #666; }
    </style>
</head>
<body>
    <div class="container">
        <h1>ESP32 Dashboard</h1>
        
        <div class="card">
            <h3>Temperature</h3>
            <span class="value" id="temp">--</span>
            <span class="unit">°C</span>
        </div>
        
        <div class="card">
            <h3>Humidity</h3>
            <span class="value" id="hum">--</span>
            <span class="unit">%</span>
        </div>
        
        <div class="card">
            <h3>LED Control</h3>
            <p>Status: <span class="status" id="ledStatus">--</span></p>
            <button onclick="toggleLED()">Toggle LED</button>
        </div>
    </div>
    
    <script>
        function updateData() {
            fetch('/api/data')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('temp').textContent = data.temperature.toFixed(1);
                    document.getElementById('hum').textContent = data.humidity.toFixed(1);
                    const status = document.getElementById('ledStatus');
                    status.textContent = data.led ? 'ON' : 'OFF';
                    status.className = 'status ' + (data.led ? 'on' : 'off');
                });
        }
        
        function toggleLED() {
            fetch('/api/led/toggle', {method: 'POST'})
                .then(() => updateData());
        }
        
        updateData();
        setInterval(updateData, 2000);
    </script>
</body>
</html>
)rawliteral";

void handleRoot() {
    server.send(200, "text/html", htmlPage);
}

void handleGetData() {
    // Simulate sensor readings
    temperature = 20.0 + random(0, 100) / 10.0;
    humidity = 50.0 + random(0, 200) / 10.0;
    
    String json = "{";
    json += "\"temperature\":" + String(temperature, 1) + ",";
    json += "\"humidity\":" + String(humidity, 1) + ",";
    json += "\"led\":" + String(ledState ? "true" : "false") + ",";
    json += "\"uptime\":" + String(millis()/1000);
    json += "}";
    
    server.send(200, "application/json", json);
}

void handleLEDToggle() {
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
    server.send(200, "application/json", "{\"status\":\"ok\"}");
}

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    
    Serial.println("\n=== ESP32 Web Server ===");
    
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.printf("\nConnected! IP: %s\n", WiFi.localIP().toString().c_str());
    
    server.on("/", handleRoot);
    server.on("/api/data", HTTP_GET, handleGetData);
    server.on("/api/led/toggle", HTTP_POST, handleLEDToggle);
    
    server.begin();
    Serial.println("Web server started!");
    Serial.printf("Open http://%s in browser\n", WiFi.localIP().toString().c_str());
}

void loop() {
    server.handleClient();
}
```

### Tugas
1. Akses dashboard dari browser
2. Test toggle LED dari web
3. Modifikasi tampilan HTML

---

## Praktikum 9: RESTful API (ESP32)

### Tujuan
Membangun RESTful API lengkap.

### Langkah Kerja

1. **Buat project** `ESP32_08_REST_API`

2. **Program REST API:**
```cpp
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

WebServer server(80);

// Virtual devices
struct Device {
    String id;
    String name;
    String type;
    bool state;
    int value;
};

Device devices[5] = {
    {"dev1", "Living Room Light", "light", false, 100},
    {"dev2", "Bedroom Light", "light", false, 50},
    {"dev3", "Fan", "fan", false, 0},
    {"dev4", "Temperature Sensor", "sensor", true, 25},
    {"dev5", "Humidity Sensor", "sensor", true, 60}
};

void sendJSON(int code, const String& json) {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(code, "application/json", json);
}

// GET /api/devices - List all devices
void handleGetDevices() {
    StaticJsonDocument<1024> doc;
    JsonArray array = doc.to<JsonArray>();
    
    for (int i = 0; i < 5; i++) {
        JsonObject obj = array.createNestedObject();
        obj["id"] = devices[i].id;
        obj["name"] = devices[i].name;
        obj["type"] = devices[i].type;
        obj["state"] = devices[i].state;
        obj["value"] = devices[i].value;
    }
    
    String json;
    serializeJson(doc, json);
    sendJSON(200, json);
}

// GET /api/devices/:id - Get specific device
void handleGetDevice() {
    String id = server.pathArg(0);
    
    for (int i = 0; i < 5; i++) {
        if (devices[i].id == id) {
            StaticJsonDocument<256> doc;
            doc["id"] = devices[i].id;
            doc["name"] = devices[i].name;
            doc["type"] = devices[i].type;
            doc["state"] = devices[i].state;
            doc["value"] = devices[i].value;
            
            String json;
            serializeJson(doc, json);
            sendJSON(200, json);
            return;
        }
    }
    
    sendJSON(404, "{\"error\":\"Device not found\"}");
}

// PUT /api/devices/:id - Update device
void handleUpdateDevice() {
    String id = server.pathArg(0);
    String body = server.arg("plain");
    
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, body);
    
    if (error) {
        sendJSON(400, "{\"error\":\"Invalid JSON\"}");
        return;
    }
    
    for (int i = 0; i < 5; i++) {
        if (devices[i].id == id) {
            if (doc.containsKey("state")) {
                devices[i].state = doc["state"];
            }
            if (doc.containsKey("value")) {
                devices[i].value = doc["value"];
            }
            
            Serial.printf("Updated %s: state=%d, value=%d\n", 
                         id.c_str(), devices[i].state, devices[i].value);
            
            sendJSON(200, "{\"status\":\"updated\"}");
            return;
        }
    }
    
    sendJSON(404, "{\"error\":\"Device not found\"}");
}

// Handle CORS preflight
void handleCORS() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
    server.send(204);
}

void setup() {
    Serial.begin(115200);
    Serial.println("\n=== ESP32 REST API ===");
    
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.printf("\nConnected! IP: %s\n", WiFi.localIP().toString().c_str());
    
    // Routes
    server.on("/api/devices", HTTP_GET, handleGetDevices);
    server.on("/api/devices/{}", HTTP_GET, handleGetDevice);
    server.on("/api/devices/{}", HTTP_PUT, handleUpdateDevice);
    server.on("/api/devices/{}", HTTP_OPTIONS, handleCORS);
    
    server.begin();
    Serial.println("REST API ready!");
    Serial.println("Endpoints:");
    Serial.println("  GET  /api/devices");
    Serial.println("  GET  /api/devices/:id");
    Serial.println("  PUT  /api/devices/:id");
}

void loop() {
    server.handleClient();
}
```

### Tugas
1. Test API dengan curl atau Postman
2. Implementasikan POST untuk add device
3. Implementasikan DELETE untuk remove device

---

## Praktikum 10: STM32-ESP32 HTTP Bridge (STM32 + ESP32)

### Tujuan
Menghubungkan STM32 ke Internet melalui ESP32.

### Langkah Kerja

**STM32 Side - Buat project** `STM32_02_HTTP_Client`

```cpp
#include <Arduino.h>
#include <ArduinoJson.h>

HardwareSerial Serial2(PA3, PA2);

const int LED_PIN = PC13;

void sendHTTPRequest(const char* method, const char* url, const char* payload = NULL) {
    StaticJsonDocument<256> cmd;
    cmd["cmd"] = "http";
    cmd["method"] = method;
    cmd["url"] = url;
    if (payload) {
        cmd["payload"] = payload;
    }
    
    String json;
    serializeJson(cmd, json);
    Serial2.println(json);
    
    Serial.println("Sent HTTP request via ESP32");
}

void processResponse(String& response) {
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, response);
    
    if (error) {
        Serial.println("Parse error: " + response);
        return;
    }
    
    if (doc.containsKey("status")) {
        int status = doc["status"];
        Serial.printf("HTTP Status: %d\n", status);
        
        if (doc.containsKey("body")) {
            Serial.println("Body: " + String((const char*)doc["body"]));
        }
    }
}

void setup() {
    Serial.begin(115200);
    Serial2.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    
    Serial.println("\n=== STM32 HTTP Client via ESP32 ===");
    
    delay(3000);  // Wait for ESP32 to boot
    Serial.println("Ready to send HTTP requests");
}

void loop() {
    static unsigned long lastRequest = 0;
    
    if (millis() - lastRequest > 30000) {
        // Create sensor data
        StaticJsonDocument<128> data;
        data["device"] = "STM32_001";
        data["temp"] = 20 + random(0, 100) / 10.0;
        data["uptime"] = millis() / 1000;
        
        String payload;
        serializeJson(data, payload);
        
        sendHTTPRequest("POST", "https://jsonplaceholder.typicode.com/posts", payload.c_str());
        
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        lastRequest = millis();
    }
    
    // Process responses
    if (Serial2.available()) {
        String response = Serial2.readStringUntil('\n');
        Serial.println("ESP32: " + response);
        processResponse(response);
    }
}
```

**ESP32 Bridge - Buat project** `ESP32_09_HTTP_Bridge`

```cpp
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

HardwareSerial SerialSTM32(2);

void processCommand(String& json) {
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, json);
    
    if (error) {
        SerialSTM32.println("{\"error\":\"Invalid JSON\"}");
        return;
    }
    
    const char* cmd = doc["cmd"];
    
    if (strcmp(cmd, "http") == 0) {
        const char* method = doc["method"];
        const char* url = doc["url"];
        
        HTTPClient http;
        http.begin(url);
        http.setTimeout(10000);
        
        int httpCode;
        if (strcmp(method, "GET") == 0) {
            httpCode = http.GET();
        } else if (strcmp(method, "POST") == 0) {
            http.addHeader("Content-Type", "application/json");
            const char* payload = doc["payload"];
            httpCode = http.POST(payload);
        } else {
            httpCode = -1;
        }
        
        StaticJsonDocument<512> response;
        response["status"] = httpCode;
        
        if (httpCode > 0) {
            String body = http.getString();
            if (body.length() < 256) {
                response["body"] = body;
            } else {
                response["body"] = "Response too large";
            }
        } else {
            response["error"] = http.errorToString(httpCode);
        }
        
        http.end();
        
        String responseJson;
        serializeJson(response, responseJson);
        SerialSTM32.println(responseJson);
        
        Serial.println("HTTP Response sent to STM32");
    }
}

void setup() {
    Serial.begin(115200);
    SerialSTM32.begin(115200, SERIAL_8N1, 16, 17);
    
    Serial.println("\n=== ESP32 HTTP Bridge ===");
    
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.printf("\nConnected! IP: %s\n", WiFi.localIP().toString().c_str());
    
    Serial.println("Bridge ready for STM32 commands");
}

void loop() {
    if (SerialSTM32.available()) {
        String json = SerialSTM32.readStringUntil('\n');
        Serial.println("STM32 Command: " + json);
        processCommand(json);
    }
}
```

### Tugas
1. Hubungkan STM32 dan ESP32 via UART
2. Test HTTP GET dan POST dari STM32
3. Monitor traffic di kedua serial

---

# Pertemuan 3: MQTT & Real-time Communication

## Praktikum 11: MQTT Basic Publisher (ESP32)

### Tujuan
Mengirim data sensor ke MQTT broker.

### Langkah Kerja

1. **Buat project** `ESP32_10_MQTT_Publisher`

2. **Konfigurasi `platformio.ini`:**
```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
lib_deps = 
    knolleary/PubSubClient@^2.8
    bblanchon/ArduinoJson@^6.21.0
```

3. **Program MQTT Publisher:**
```cpp
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient mqtt(espClient);

const char* clientID = "ESP32_Publisher_001";
const char* topicSensor = "embedded/praktikum/sensors";
const char* topicStatus = "embedded/praktikum/status";

unsigned long lastPublish = 0;
const int publishInterval = 5000;

float readTemperature() {
    return 20.0 + random(0, 150) / 10.0;
}

float readHumidity() {
    return 40.0 + random(0, 400) / 10.0;
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    Serial.printf("Message on [%s]: ", topic);
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();
}

void reconnectMQTT() {
    while (!mqtt.connected()) {
        Serial.print("Connecting to MQTT...");
        
        if (mqtt.connect(clientID)) {
            Serial.println("connected!");
            
            // Publish online status
            StaticJsonDocument<128> status;
            status["device"] = clientID;
            status["status"] = "online";
            status["ip"] = WiFi.localIP().toString();
            
            char buffer[128];
            serializeJson(status, buffer);
            mqtt.publish(topicStatus, buffer, true);  // retained message
            
        } else {
            Serial.printf("failed, rc=%d. Retry in 5s\n", mqtt.state());
            delay(5000);
        }
    }
}

void publishSensorData() {
    StaticJsonDocument<256> doc;
    
    doc["device_id"] = clientID;
    doc["temperature"] = readTemperature();
    doc["humidity"] = readHumidity();
    doc["timestamp"] = millis();
    doc["rssi"] = WiFi.RSSI();
    
    char buffer[256];
    serializeJson(doc, buffer);
    
    if (mqtt.publish(topicSensor, buffer)) {
        Serial.println("Published: " + String(buffer));
    } else {
        Serial.println("Publish failed!");
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("\n=== ESP32 MQTT Publisher ===");
    
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.printf("\nConnected! IP: %s\n", WiFi.localIP().toString().c_str());
    
    mqtt.setServer(mqtt_server, mqtt_port);
    mqtt.setCallback(mqttCallback);
    
    Serial.printf("MQTT Broker: %s:%d\n", mqtt_server, mqtt_port);
    Serial.printf("Publish Topic: %s\n", topicSensor);
}

void loop() {
    if (!mqtt.connected()) {
        reconnectMQTT();
    }
    mqtt.loop();
    
    if (millis() - lastPublish > publishInterval) {
        publishSensorData();
        lastPublish = millis();
    }
}
```

### Tugas
1. Publish data setiap 5 detik
2. Monitor dengan MQTT Explorer atau HiveMQ web client
3. Test berbagai QoS levels

---

## Praktikum 12: MQTT Subscriber (ESP32)

### Tujuan
Menerima commands via MQTT.

### Langkah Kerja

1. **Buat project** `ESP32_11_MQTT_Subscriber`

2. **Program MQTT Subscriber:**
```cpp
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient mqtt(espClient);

const char* clientID = "ESP32_Subscriber_001";
const char* topicControl = "embedded/praktikum/control/#";
const char* topicResponse = "embedded/praktikum/response";

const int LED_PIN = 2;
const int BUZZER_PIN = 4;

void handleCommand(const char* topic, JsonDocument& doc) {
    Serial.printf("Processing command from [%s]\n", topic);
    
    if (doc.containsKey("led")) {
        bool state = doc["led"];
        digitalWrite(LED_PIN, state);
        Serial.printf("LED: %s\n", state ? "ON" : "OFF");
    }
    
    if (doc.containsKey("buzzer")) {
        int duration = doc["buzzer"];
        digitalWrite(BUZZER_PIN, HIGH);
        delay(duration);
        digitalWrite(BUZZER_PIN, LOW);
        Serial.printf("Buzzer: %dms\n", duration);
    }
    
    if (doc.containsKey("blink")) {
        int times = doc["blink"];
        for (int i = 0; i < times; i++) {
            digitalWrite(LED_PIN, HIGH);
            delay(200);
            digitalWrite(LED_PIN, LOW);
            delay(200);
        }
        Serial.printf("Blink: %d times\n", times);
    }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    Serial.printf("\n=== Message Received ===\n");
    Serial.printf("Topic: %s\n", topic);
    Serial.printf("Length: %d bytes\n", length);
    
    // Parse JSON
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, payload, length);
    
    if (error) {
        Serial.printf("JSON Error: %s\n", error.c_str());
        return;
    }
    
    Serial.print("Payload: ");
    serializeJson(doc, Serial);
    Serial.println();
    
    handleCommand(topic, doc);
    
    // Send response
    StaticJsonDocument<128> response;
    response["device"] = clientID;
    response["status"] = "processed";
    response["timestamp"] = millis();
    
    char buffer[128];
    serializeJson(response, buffer);
    mqtt.publish(topicResponse, buffer);
}

void reconnectMQTT() {
    while (!mqtt.connected()) {
        Serial.print("Connecting to MQTT...");
        
        if (mqtt.connect(clientID)) {
            Serial.println("connected!");
            
            // Subscribe to control topics
            mqtt.subscribe(topicControl);
            Serial.printf("Subscribed to: %s\n", topicControl);
            
        } else {
            Serial.printf("failed, rc=%d\n", mqtt.state());
            delay(5000);
        }
    }
}

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    
    Serial.println("\n=== ESP32 MQTT Subscriber ===");
    
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.printf("\nConnected! IP: %s\n", WiFi.localIP().toString().c_str());
    
    mqtt.setServer(mqtt_server, mqtt_port);
    mqtt.setCallback(mqttCallback);
    mqtt.setBufferSize(512);
    
    Serial.println("Send commands to: embedded/praktikum/control/");
    Serial.println("Example: {\"led\":true}");
    Serial.println("         {\"buzzer\":500}");
    Serial.println("         {\"blink\":3}");
}

void loop() {
    if (!mqtt.connected()) {
        reconnectMQTT();
    }
    mqtt.loop();
}
```

### Tugas
1. Subscribe ke topic control
2. Kirim command dari MQTT client lain
3. Verifikasi LED dan buzzer response

---

## Praktikum 13: WebSocket Server (ESP32)

### Tujuan
Membangun real-time dashboard dengan WebSocket.

### Langkah Kerja

1. **Buat project** `ESP32_12_WebSocket`

2. **Konfigurasi `platformio.ini`:**
```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
lib_deps = 
    links2004/WebSockets@^2.4.0
    bblanchon/ArduinoJson@^6.21.0
```

3. **Program WebSocket:**
```cpp
#include <Arduino.h>
#include <WiFi.h>
#include <WebSocketsServer.h>
#include <WebServer.h>
#include <ArduinoJson.h>

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

WebServer http(80);
WebSocketsServer webSocket(81);

const int LED_PIN = 2;
bool ledState = false;

const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>ESP32 WebSocket</title>
    <style>
        body {
            font-family: Arial;
            background: #0f0f23;
            color: #cccccc;
            text-align: center;
            padding: 20px;
        }
        .card {
            background: #1a1a3e;
            border-radius: 15px;
            padding: 25px;
            margin: 15px auto;
            max-width: 350px;
        }
        .value {
            font-size: 60px;
            font-weight: bold;
            color: #00ff88;
        }
        .label {
            font-size: 14px;
            color: #888;
            margin-top: 5px;
        }
        button {
            background: #ff4444;
            color: white;
            border: none;
            padding: 15px 40px;
            font-size: 18px;
            border-radius: 30px;
            cursor: pointer;
            margin-top: 15px;
        }
        button.on {
            background: #44ff44;
        }
        #status {
            font-size: 12px;
            color: #666;
            margin-top: 20px;
        }
        .connected { color: #44ff44; }
        .disconnected { color: #ff4444; }
    </style>
</head>
<body>
    <h1>🔌 Real-time Dashboard</h1>
    
    <div class="card">
        <div class="value" id="temp">--</div>
        <div class="label">Temperature (°C)</div>
    </div>
    
    <div class="card">
        <div class="value" id="hum">--</div>
        <div class="label">Humidity (%)</div>
    </div>
    
    <div class="card">
        <button id="ledBtn" onclick="toggleLED()">LED OFF</button>
        <div class="label" id="ledLabel">Click to toggle</div>
    </div>
    
    <div id="status">Connecting...</div>
    
    <script>
        let ws;
        let reconnectTimer;
        
        function connect() {
            ws = new WebSocket('ws://' + location.hostname + ':81');
            
            ws.onopen = () => {
                document.getElementById('status').textContent = '● Connected';
                document.getElementById('status').className = 'connected';
            };
            
            ws.onclose = () => {
                document.getElementById('status').textContent = '○ Disconnected - Reconnecting...';
                document.getElementById('status').className = 'disconnected';
                reconnectTimer = setTimeout(connect, 2000);
            };
            
            ws.onmessage = (event) => {
                const data = JSON.parse(event.data);
                
                if (data.type === 'sensor') {
                    document.getElementById('temp').textContent = data.temperature.toFixed(1);
                    document.getElementById('hum').textContent = data.humidity.toFixed(1);
                }
                
                if (data.type === 'led') {
                    const btn = document.getElementById('ledBtn');
                    btn.textContent = data.state ? 'LED ON' : 'LED OFF';
                    btn.className = data.state ? 'on' : '';
                }
            };
        }
        
        function toggleLED() {
            if (ws.readyState === WebSocket.OPEN) {
                ws.send(JSON.stringify({cmd: 'toggle_led'}));
            }
        }
        
        connect();
    </script>
</body>
</html>
)rawliteral";

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
    switch (type) {
        case WStype_DISCONNECTED:
            Serial.printf("[WS] Client %u disconnected\n", num);
            break;
            
        case WStype_CONNECTED: {
            IPAddress ip = webSocket.remoteIP(num);
            Serial.printf("[WS] Client %u connected from %s\n", num, ip.toString().c_str());
            
            // Send current state
            StaticJsonDocument<128> doc;
            doc["type"] = "led";
            doc["state"] = ledState;
            
            char buffer[128];
            serializeJson(doc, buffer);
            webSocket.sendTXT(num, buffer);
            break;
        }
        
        case WStype_TEXT: {
            Serial.printf("[WS] Client %u: %s\n", num, payload);
            
            StaticJsonDocument<128> doc;
            deserializeJson(doc, payload);
            
            if (doc["cmd"] == "toggle_led") {
                ledState = !ledState;
                digitalWrite(LED_PIN, ledState);
                
                StaticJsonDocument<128> response;
                response["type"] = "led";
                response["state"] = ledState;
                
                char buffer[128];
                serializeJson(response, buffer);
                webSocket.broadcastTXT(buffer);
            }
            break;
        }
    }
}

void broadcastSensorData() {
    StaticJsonDocument<256> doc;
    doc["type"] = "sensor";
    doc["temperature"] = 20.0 + random(0, 150) / 10.0;
    doc["humidity"] = 40.0 + random(0, 400) / 10.0;
    doc["timestamp"] = millis();
    
    char buffer[256];
    serializeJson(doc, buffer);
    webSocket.broadcastTXT(buffer);
}

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    
    Serial.println("\n=== ESP32 WebSocket Server ===");
    
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.printf("\nConnected! IP: %s\n", WiFi.localIP().toString().c_str());
    
    http.on("/", []() {
        http.send(200, "text/html", htmlPage);
    });
    http.begin();
    
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
    
    Serial.printf("Open http://%s in browser\n", WiFi.localIP().toString().c_str());
}

void loop() {
    http.handleClient();
    webSocket.loop();
    
    static unsigned long lastBroadcast = 0;
    if (millis() - lastBroadcast > 1000) {
        broadcastSensorData();
        lastBroadcast = millis();
    }
}
```

### Tugas
1. Buka dashboard di browser
2. Amati update real-time setiap detik
3. Test LED toggle dari multiple clients

---

## Praktikum 14: Bluetooth Serial (ESP32)

### Tujuan
Komunikasi via Bluetooth Classic.

### Langkah Kerja

1. **Buat project** `ESP32_13_Bluetooth_Serial`

2. **Program Bluetooth:**
```cpp
#include <Arduino.h>
#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

const int LED_PIN = 2;
String deviceName = "ESP32_BT_Device";

void processCommand(String& cmd) {
    cmd.trim();
    cmd.toLowerCase();
    
    if (cmd == "led on") {
        digitalWrite(LED_PIN, HIGH);
        SerialBT.println("LED turned ON");
    }
    else if (cmd == "led off") {
        digitalWrite(LED_PIN, LOW);
        SerialBT.println("LED turned OFF");
    }
    else if (cmd == "status") {
        SerialBT.println("=== Device Status ===");
        SerialBT.printf("LED: %s\n", digitalRead(LED_PIN) ? "ON" : "OFF");
        SerialBT.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
        SerialBT.printf("Uptime: %lu seconds\n", millis()/1000);
    }
    else if (cmd == "help") {
        SerialBT.println("=== Commands ===");
        SerialBT.println("led on  - Turn LED on");
        SerialBT.println("led off - Turn LED off");
        SerialBT.println("status  - Show device status");
        SerialBT.println("help    - Show this help");
    }
    else {
        SerialBT.println("Unknown command. Type 'help' for commands.");
    }
}

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    
    Serial.println("\n=== ESP32 Bluetooth Serial ===");
    
    if (!SerialBT.begin(deviceName)) {
        Serial.println("Bluetooth init failed!");
        while (1);
    }
    
    Serial.printf("Bluetooth device '%s' ready\n", deviceName.c_str());
    Serial.println("Pair with your phone and use a serial terminal app");
}

void loop() {
    // Bluetooth -> Serial
    if (SerialBT.available()) {
        String cmd = SerialBT.readStringUntil('\n');
        Serial.println("BT received: " + cmd);
        processCommand(cmd);
    }
    
    // Serial -> Bluetooth
    if (Serial.available()) {
        String msg = Serial.readStringUntil('\n');
        SerialBT.println(msg);
        Serial.println("Sent to BT: " + msg);
    }
}
```

### Tugas
1. Pair ESP32 dengan smartphone
2. Gunakan app "Serial Bluetooth Terminal"
3. Test semua commands

---

## Praktikum 15: BLE Sensor (ESP32)

### Tujuan
Membuat BLE peripheral untuk sensor data.

### Langkah Kerja

1. **Buat project** `ESP32_14_BLE_Sensor`

2. **Program BLE:**
```cpp
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define SERVICE_UUID        "12345678-1234-5678-1234-56789abcdef0"
#define CHAR_TEMP_UUID      "12345678-1234-5678-1234-56789abcdef1"
#define CHAR_HUM_UUID       "12345678-1234-5678-1234-56789abcdef2"
#define CHAR_LED_UUID       "12345678-1234-5678-1234-56789abcdef3"

BLEServer* pServer = NULL;
BLECharacteristic* pTempChar = NULL;
BLECharacteristic* pHumChar = NULL;
BLECharacteristic* pLedChar = NULL;

bool deviceConnected = false;
bool oldDeviceConnected = false;
const int LED_PIN = 2;

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        deviceConnected = true;
        Serial.println("Client connected");
    }
    
    void onDisconnect(BLEServer* pServer) {
        deviceConnected = false;
        Serial.println("Client disconnected");
    }
};

class LedCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pCharacteristic) {
        std::string value = pCharacteristic->getValue();
        if (value.length() > 0) {
            bool ledState = (value[0] == '1' || value[0] == 1);
            digitalWrite(LED_PIN, ledState);
            Serial.printf("LED set to: %s\n", ledState ? "ON" : "OFF");
        }
    }
};

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    
    Serial.println("\n=== ESP32 BLE Sensor ===");
    
    BLEDevice::init("ESP32_BLE_Sensor");
    
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());
    
    BLEService *pService = pServer->createService(SERVICE_UUID);
    
    // Temperature characteristic (Read + Notify)
    pTempChar = pService->createCharacteristic(
        CHAR_TEMP_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
    );
    pTempChar->addDescriptor(new BLE2902());
    
    // Humidity characteristic (Read + Notify)
    pHumChar = pService->createCharacteristic(
        CHAR_HUM_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
    );
    pHumChar->addDescriptor(new BLE2902());
    
    // LED characteristic (Read + Write)
    pLedChar = pService->createCharacteristic(
        CHAR_LED_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE
    );
    pLedChar->setCallbacks(new LedCallbacks());
    pLedChar->setValue("0");
    
    pService->start();
    
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->start();
    
    Serial.println("BLE advertising started");
    Serial.printf("Service UUID: %s\n", SERVICE_UUID);
}

void loop() {
    if (deviceConnected) {
        // Update temperature
        float temp = 20.0 + random(0, 150) / 10.0;
        char tempStr[8];
        snprintf(tempStr, sizeof(tempStr), "%.1f", temp);
        pTempChar->setValue(tempStr);
        pTempChar->notify();
        
        // Update humidity
        float hum = 40.0 + random(0, 400) / 10.0;
        char humStr[8];
        snprintf(humStr, sizeof(humStr), "%.1f", hum);
        pHumChar->setValue(humStr);
        pHumChar->notify();
        
        Serial.printf("Notified - Temp: %s, Hum: %s\n", tempStr, humStr);
    }
    
    // Handle reconnection
    if (!deviceConnected && oldDeviceConnected) {
        delay(500);
        pServer->startAdvertising();
        Serial.println("Restarting advertising");
    }
    
    oldDeviceConnected = deviceConnected;
    delay(1000);
}
```

### Tugas
1. Gunakan app "nRF Connect" untuk test BLE
2. Monitor notifications dari sensor
3. Test write ke LED characteristic

---

# Pertemuan 4: Integration & STM32 Bridge

## Praktikum 16: STM32-ESP32 MQTT Bridge (STM32)

### Tujuan
Mengirim data STM32 ke MQTT via ESP32.

### Langkah Kerja

**STM32 Side - Buat project** `STM32_03_MQTT_Bridge`

```cpp
#include <Arduino.h>
#include <ArduinoJson.h>

HardwareSerial Serial2(PA3, PA2);

const int LED_PIN = PC13;
const int TEMP_PIN = PA0;

unsigned long lastPublish = 0;
const int publishInterval = 5000;

void sendMQTTPublish(const char* topic, const char* payload) {
    StaticJsonDocument<256> cmd;
    cmd["cmd"] = "mqtt_publish";
    cmd["topic"] = topic;
    cmd["payload"] = payload;
    
    String json;
    serializeJson(cmd, json);
    Serial2.println(json);
}

void sendMQTTSubscribe(const char* topic) {
    StaticJsonDocument<128> cmd;
    cmd["cmd"] = "mqtt_subscribe";
    cmd["topic"] = topic;
    
    String json;
    serializeJson(cmd, json);
    Serial2.println(json);
}

void processMessage(String& json) {
    StaticJsonDocument<256> doc;
    if (deserializeJson(doc, json)) return;
    
    const char* type = doc["type"];
    
    if (strcmp(type, "mqtt_message") == 0) {
        const char* topic = doc["topic"];
        const char* payload = doc["payload"];
        
        Serial.printf("MQTT [%s]: %s\n", topic, payload);
        
        // Process commands
        StaticJsonDocument<128> cmd;
        if (deserializeJson(cmd, payload) == 0) {
            if (cmd.containsKey("led")) {
                digitalWrite(LED_PIN, !cmd["led"].as<bool>());  // Inverted for Blue Pill
            }
        }
    }
    else if (strcmp(type, "status") == 0) {
        Serial.printf("Bridge status: %s\n", doc["status"].as<const char*>());
    }
}

float readTemperature() {
    int raw = analogRead(TEMP_PIN);
    float voltage = raw * 3.3 / 4096.0;
    return voltage * 100.0;  // Simple conversion
}

void setup() {
    Serial.begin(115200);
    Serial2.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    pinMode(TEMP_PIN, INPUT);
    
    Serial.println("\n=== STM32 MQTT Bridge Client ===");
    
    delay(5000);  // Wait for ESP32 bridge
    
    // Subscribe to control topic
    sendMQTTSubscribe("embedded/stm32/control");
    
    Serial.println("Ready");
}

void loop() {
    // Publish sensor data periodically
    if (millis() - lastPublish > publishInterval) {
        StaticJsonDocument<128> data;
        data["device"] = "STM32_001";
        data["temperature"] = readTemperature();
        data["uptime"] = millis() / 1000;
        
        String payload;
        serializeJson(data, payload);
        
        sendMQTTPublish("embedded/stm32/sensors", payload.c_str());
        
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        lastPublish = millis();
    }
    
    // Process messages from ESP32
    if (Serial2.available()) {
        String line = Serial2.readStringUntil('\n');
        processMessage(line);
    }
}
```

**ESP32 Bridge - Buat project** `ESP32_15_MQTT_Bridge`

```cpp
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* mqtt_server = "broker.hivemq.com";

HardwareSerial SerialSTM32(2);

WiFiClient espClient;
PubSubClient mqtt(espClient);

String subscribedTopics[10];
int topicCount = 0;

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    // Forward to STM32
    StaticJsonDocument<256> doc;
    doc["type"] = "mqtt_message";
    doc["topic"] = topic;
    
    char payloadStr[256];
    memcpy(payloadStr, payload, min(length, (unsigned int)255));
    payloadStr[min(length, (unsigned int)255)] = '\0';
    doc["payload"] = payloadStr;
    
    String json;
    serializeJson(doc, json);
    SerialSTM32.println(json);
    
    Serial.printf("MQTT -> STM32: %s\n", json.c_str());
}

void processSTM32Command(String& json) {
    StaticJsonDocument<256> doc;
    if (deserializeJson(doc, json)) return;
    
    const char* cmd = doc["cmd"];
    
    if (strcmp(cmd, "mqtt_publish") == 0) {
        const char* topic = doc["topic"];
        const char* payload = doc["payload"];
        
        if (mqtt.publish(topic, payload)) {
            Serial.printf("Published to [%s]: %s\n", topic, payload);
        }
    }
    else if (strcmp(cmd, "mqtt_subscribe") == 0) {
        const char* topic = doc["topic"];
        
        if (mqtt.subscribe(topic)) {
            subscribedTopics[topicCount++] = String(topic);
            Serial.printf("Subscribed to: %s\n", topic);
        }
    }
}

void reconnectMQTT() {
    while (!mqtt.connected()) {
        Serial.print("Connecting MQTT...");
        
        if (mqtt.connect("ESP32_MQTT_Bridge")) {
            Serial.println("connected!");
            
            // Re-subscribe all topics
            for (int i = 0; i < topicCount; i++) {
                mqtt.subscribe(subscribedTopics[i].c_str());
            }
            
            // Notify STM32
            SerialSTM32.println("{\"type\":\"status\",\"status\":\"connected\"}");
        } else {
            delay(5000);
        }
    }
}

void setup() {
    Serial.begin(115200);
    SerialSTM32.begin(115200, SERIAL_8N1, 16, 17);
    
    Serial.println("\n=== ESP32 MQTT Bridge ===");
    
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.printf("\nWiFi connected: %s\n", WiFi.localIP().toString().c_str());
    
    mqtt.setServer(mqtt_server, 1883);
    mqtt.setCallback(mqttCallback);
    mqtt.setBufferSize(512);
    
    Serial.println("Bridge ready");
}

void loop() {
    if (!mqtt.connected()) {
        reconnectMQTT();
    }
    mqtt.loop();
    
    if (SerialSTM32.available()) {
        String json = SerialSTM32.readStringUntil('\n');
        Serial.println("STM32: " + json);
        processSTM32Command(json);
    }
}
```

### Tugas
1. Hubungkan STM32 dan ESP32
2. Monitor data di MQTT broker
3. Test control LED dari MQTT

---

## Praktikum 17: Full IoT System (ESP32)

### Tujuan
Mengintegrasikan semua protokol dalam satu sistem.

### Langkah Kerja

1. **Buat project** `ESP32_16_Full_IoT`

2. **Program Full IoT:**
```cpp
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* mqtt_server = "broker.hivemq.com";

WebServer http(80);
WebSocketsServer webSocket(81);
WiFiClient espClient;
PubSubClient mqtt(espClient);

const int LED_PIN = 2;
bool ledState = false;
float temperature = 25.0;
float humidity = 60.0;

// HTML Dashboard
const char* dashboard = R"(
<!DOCTYPE html>
<html>
<head>
    <meta name='viewport' content='width=device-width,initial-scale=1'>
    <title>Full IoT Dashboard</title>
    <style>
        body{font-family:Arial;background:#1e1e2e;color:#fff;text-align:center;padding:20px}
        .card{background:#2a2a3e;border-radius:15px;padding:20px;margin:10px auto;max-width:300px}
        .value{font-size:48px;color:#00d9ff}
        button{background:#ff6b6b;color:#fff;border:none;padding:15px 30px;border-radius:25px;margin:5px;cursor:pointer}
        button.on{background:#6bff6b}
        #log{text-align:left;font-family:monospace;font-size:12px;background:#1a1a2a;padding:10px;border-radius:10px;height:150px;overflow-y:auto}
    </style>
</head>
<body>
    <h1>🌐 Full IoT Dashboard</h1>
    <div class='card'>
        <div class='value' id='temp'>--</div>
        <div>Temperature °C</div>
    </div>
    <div class='card'>
        <div class='value' id='hum'>--</div>
        <div>Humidity %</div>
    </div>
    <div class='card'>
        <button id='led' onclick='toggleLED()'>LED OFF</button>
        <button onclick='publishMQTT()'>MQTT Publish</button>
    </div>
    <div class='card'>
        <h3>Activity Log</h3>
        <div id='log'></div>
    </div>
    <script>
        let ws=new WebSocket('ws://'+location.hostname+':81');
        ws.onmessage=(e)=>{
            let d=JSON.parse(e.data);
            if(d.temp)document.getElementById('temp').textContent=d.temp.toFixed(1);
            if(d.hum)document.getElementById('hum').textContent=d.hum.toFixed(1);
            if(d.led!==undefined){
                let b=document.getElementById('led');
                b.textContent=d.led?'LED ON':'LED OFF';
                b.className=d.led?'on':'';
            }
            if(d.log)addLog(d.log);
        };
        function toggleLED(){ws.send(JSON.stringify({cmd:'led'}));}
        function publishMQTT(){ws.send(JSON.stringify({cmd:'mqtt'}));}
        function addLog(msg){
            let log=document.getElementById('log');
            log.innerHTML='['+new Date().toLocaleTimeString()+'] '+msg+'<br>'+log.innerHTML;
        }
    </script>
</body>
</html>
)";

void broadcastState() {
    StaticJsonDocument<256> doc;
    doc["temp"] = temperature;
    doc["hum"] = humidity;
    doc["led"] = ledState;
    
    char buffer[256];
    serializeJson(doc, buffer);
    webSocket.broadcastTXT(buffer);
}

void broadcastLog(const char* msg) {
    StaticJsonDocument<128> doc;
    doc["log"] = msg;
    
    char buffer[128];
    serializeJson(doc, buffer);
    webSocket.broadcastTXT(buffer);
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
    if (type == WStype_TEXT) {
        StaticJsonDocument<128> doc;
        deserializeJson(doc, payload);
        
        if (doc["cmd"] == "led") {
            ledState = !ledState;
            digitalWrite(LED_PIN, ledState);
            broadcastState();
            broadcastLog(ledState ? "LED turned ON" : "LED turned OFF");
        }
        else if (doc["cmd"] == "mqtt") {
            StaticJsonDocument<128> msg;
            msg["temp"] = temperature;
            msg["hum"] = humidity;
            msg["device"] = "ESP32_IoT";
            
            char buffer[128];
            serializeJson(msg, buffer);
            mqtt.publish("embedded/fulliot/data", buffer);
            broadcastLog("Published to MQTT");
        }
    }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    char msg[64];
    snprintf(msg, sizeof(msg), "MQTT: %.*s", min((int)length, 50), payload);
    broadcastLog(msg);
}

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) delay(500);
    
    Serial.printf("IP: %s\n", WiFi.localIP().toString().c_str());
    
    http.on("/", []() { http.send(200, "text/html", dashboard); });
    http.begin();
    
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
    
    mqtt.setServer(mqtt_server, 1883);
    mqtt.setCallback(mqttCallback);
}

void loop() {
    http.handleClient();
    webSocket.loop();
    
    if (!mqtt.connected()) {
        mqtt.connect("ESP32_FullIoT");
        mqtt.subscribe("embedded/fulliot/control");
    }
    mqtt.loop();
    
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate > 2000) {
        temperature = 20.0 + random(0, 150) / 10.0;
        humidity = 40.0 + random(0, 400) / 10.0;
        broadcastState();
        lastUpdate = millis();
    }
}
```

### Tugas
1. Akses dashboard
2. Test semua fungsi
3. Monitor MQTT traffic

---

## Praktikum 18-20: Challenge Projects

### Praktikum 18: Multi-room Monitoring System
- ESP32 di beberapa lokasi
- Central dashboard mengumpulkan data
- Alert via MQTT

### Praktikum 19: Smart Home Gateway
- ESP32 sebagai gateway
- STM32 nodes via serial
- Control dari web dan MQTT

### Praktikum 20: Industrial Sensor Network
- Multiple sensors
- Data logging
- OTA updates

---

## Checklist Praktikum

| No | Praktikum | Platform | Status |
|----|-----------|----------|--------|
| 1 | WiFi Station | ESP32 | ☐ |
| 2 | WiFi AP | ESP32 | ☐ |
| 3 | TCP Server | ESP32 | ☐ |
| 4 | UDP Communication | ESP32 | ☐ |
| 5 | Serial Bridge Prep | STM32 | ☐ |
| 6 | HTTP GET | ESP32 | ☐ |
| 7 | HTTP POST | ESP32 | ☐ |
| 8 | Web Server | ESP32 | ☐ |
| 9 | REST API | ESP32 | ☐ |
| 10 | HTTP Bridge | STM32+ESP32 | ☐ |
| 11 | MQTT Publisher | ESP32 | ☐ |
| 12 | MQTT Subscriber | ESP32 | ☐ |
| 13 | WebSocket | ESP32 | ☐ |
| 14 | Bluetooth Serial | ESP32 | ☐ |
| 15 | BLE Sensor | ESP32 | ☐ |
| 16 | MQTT Bridge | STM32+ESP32 | ☐ |
| 17 | Full IoT System | ESP32 | ☐ |
| 18-20 | Challenge | Both | ☐ |

---

## Referensi

1. ESP32 Arduino Core Documentation
2. PubSubClient Library
3. ArduinoJson Library
4. WebSockets Library
5. ESP32 BLE Library
