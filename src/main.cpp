#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <BluetoothSerial.h>

// Pin configuration
#define BUZZER_PIN 2
#define BUTTON_PIN 4

// Default WiFi credentials
String apSSID = "tracker";
String apPassword = "123456";

// Web server on port 80
WebServer server(80);
Preferences preferences;
BluetoothSerial SerialBT;

// State variables
bool apActive = false;
unsigned long lastActivity = 0;
const unsigned long TIMEOUT = 60000; // 60 secondi

// Device monitoring
bool monitorWiFi = false;
bool monitorBT = false;
bool buzzerActive = false;

int lastWiFiClients = 0;
bool lastBTConnected = false;

// Button debounce
unsigned long lastButtonPress = 0;
const unsigned long debounceDelay = 300;

void buzz(int duration) {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(duration);
  digitalWrite(BUZZER_PIN, LOW);
}

void startAccessPoint() {
  Serial.println("Avvio Access Point...");
  WiFi.mode(WIFI_AP);
  WiFi.softAP(apSSID.c_str(), apPassword.c_str());

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  apActive = true;
  lastActivity = millis();

  // Beep di conferma
  buzz(200);
  delay(100);
  buzz(200);
}

void stopAccessPoint() {
  Serial.println("Spegnimento Access Point...");
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_OFF);
  apActive = false;

  // Beep di spegnimento
  buzz(500);
}

// Pagina HTML principale
String getHTML() {
  String html = R"(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Tracker WiFi Config</title>
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }
    body {
      font-family: Arial, sans-serif;
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      min-height: 100vh;
      display: flex;
      justify-content: center;
      align-items: center;
      padding: 20px;
    }
    .container {
      background: white;
      border-radius: 20px;
      padding: 30px;
      max-width: 500px;
      width: 100%;
      box-shadow: 0 20px 60px rgba(0,0,0,0.3);
    }
    h1 {
      color: #333;
      margin-bottom: 10px;
      font-size: 24px;
    }
    .info {
      background: #f0f4ff;
      padding: 15px;
      border-radius: 10px;
      margin-bottom: 20px;
      font-size: 14px;
      color: #666;
    }
    .section {
      margin-bottom: 25px;
    }
    h2 {
      color: #667eea;
      margin-bottom: 15px;
      font-size: 18px;
      border-bottom: 2px solid #667eea;
      padding-bottom: 5px;
    }
    label {
      display: block;
      margin-bottom: 5px;
      color: #555;
      font-weight: bold;
    }
    input[type="text"], input[type="password"] {
      width: 100%;
      padding: 10px;
      margin-bottom: 15px;
      border: 2px solid #ddd;
      border-radius: 8px;
      font-size: 14px;
      transition: border 0.3s;
    }
    input[type="text"]:focus, input[type="password"]:focus {
      outline: none;
      border-color: #667eea;
    }
    .checkbox-group {
      background: #f9f9f9;
      padding: 15px;
      border-radius: 8px;
      margin-bottom: 10px;
    }
    .checkbox-item {
      display: flex;
      align-items: center;
      margin-bottom: 10px;
    }
    .checkbox-item:last-child {
      margin-bottom: 0;
    }
    input[type="checkbox"] {
      width: 20px;
      height: 20px;
      margin-right: 10px;
      cursor: pointer;
    }
    button {
      width: 100%;
      padding: 12px;
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      color: white;
      border: none;
      border-radius: 8px;
      font-size: 16px;
      font-weight: bold;
      cursor: pointer;
      transition: transform 0.2s;
    }
    button:hover {
      transform: translateY(-2px);
    }
    button:active {
      transform: translateY(0);
    }
    .status {
      text-align: center;
      margin-top: 15px;
      padding: 10px;
      border-radius: 8px;
      font-size: 14px;
    }
    .success {
      background: #d4edda;
      color: #155724;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>🔧 Tracker WiFi Config</h1>
    <div class="info">
      IP: )";
  html += WiFi.softAPIP().toString();
  html += R"( | Timeout: 60s inattività
    </div>

    <div class="section">
      <h2>📡 Credenziali Hotspot</h2>
      <form action="/save" method="POST">
        <label>Nome Rete (SSID):</label>
        <input type="text" name="ssid" value=")";
  html += apSSID;
  html += R"(" required>

        <label>Password:</label>
        <input type="password" name="password" value=")";
  html += apPassword;
  html += R"(" required minlength="8">

        <button type="submit">💾 Salva Credenziali</button>
      </form>
    </div>

    <div class="section">
      <h2>🔔 Monitoraggio Connessioni</h2>
      <div class="checkbox-group">
        <div class="checkbox-item">
          <input type="checkbox" id="wifiMon" )";
  if (monitorWiFi) html += "checked";
  html += R"( onchange="toggleMonitor('wifi', this.checked)">
          <label for="wifiMon">Monitora WiFi - Buzzer se client si disconnette</label>
        </div>
        <div class="checkbox-item">
          <input type="checkbox" id="btMon" )";
  if (monitorBT) html += "checked";
  html += R"( onchange="toggleMonitor('bt', this.checked)">
          <label for="btMon">Monitora Bluetooth - Buzzer se dispositivo si disconnette</label>
        </div>
      </div>
      <div class="status success">
        WiFi Clients: <span id="wifiCount">0</span> | BT: <span id="btStatus">)";
  html += lastBTConnected ? "Connesso" : "Disconnesso";
  html += R"(</span>
      </div>
    </div>

    <div class="section">
      <button onclick="testBuzzer()">🔊 Test Buzzer</button>
    </div>
  </div>

  <script>
    function toggleMonitor(type, enabled) {
      fetch('/monitor?type=' + type + '&enabled=' + (enabled ? '1' : '0'))
        .then(r => r.text())
        .then(data => console.log(data));
    }

    function testBuzzer() {
      fetch('/buzz')
        .then(r => r.text())
        .then(data => alert('Buzzer testato!'));
    }

    // Auto-refresh status ogni 2 secondi
    setInterval(() => {
      fetch('/status')
        .then(r => r.json())
        .then(data => {
          document.getElementById('wifiCount').textContent = data.wifiClients;
          document.getElementById('btStatus').textContent = data.btConnected ? 'Connesso' : 'Disconnesso';
        });
    }, 2000);
  </script>
</body>
</html>
)";
  return html;
}

// Handler per la pagina principale
void handleRoot() {
  lastActivity = millis();
  server.send(200, "text/html", getHTML());
}

// Handler per salvare le credenziali
void handleSave() {
  lastActivity = millis();

  if (server.hasArg("ssid") && server.hasArg("password")) {
    apSSID = server.arg("ssid");
    apPassword = server.arg("password");

    // Salva in memoria permanente
    preferences.begin("wifi", false);
    preferences.putString("ssid", apSSID);
    preferences.putString("password", apPassword);
    preferences.end();

    Serial.println("Nuove credenziali salvate:");
    Serial.println("SSID: " + apSSID);
    Serial.println("Password: " + apPassword);

    // Riavvia AP con nuove credenziali
    stopAccessPoint();
    delay(1000);
    startAccessPoint();

    server.send(200, "text/html", "<html><body><h1>Salvato!</h1><p>Riconnettiti con le nuove credenziali.</p><script>setTimeout(()=>{window.location='/'},3000)</script></body></html>");
  } else {
    server.send(400, "text/plain", "Parametri mancanti");
  }
}

// Handler per il monitoraggio
void handleMonitor() {
  lastActivity = millis();

  if (server.hasArg("type") && server.hasArg("enabled")) {
    String type = server.arg("type");
    bool enabled = server.arg("enabled") == "1";

    if (type == "wifi") {
      monitorWiFi = enabled;
      preferences.begin("monitor", false);
      preferences.putBool("wifi", monitorWiFi);
      preferences.end();
      Serial.println("Monitor WiFi: " + String(enabled ? "ON" : "OFF"));
    } else if (type == "bt") {
      monitorBT = enabled;
      preferences.begin("monitor", false);
      preferences.putBool("bt", monitorBT);
      preferences.end();
      Serial.println("Monitor BT: " + String(enabled ? "ON" : "OFF"));
    }

    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Parametri mancanti");
  }
}

// Handler per lo status (JSON)
void handleStatus() {
  lastActivity = millis();

  int wifiClients = WiFi.softAPgetStationNum();
  bool btConnected = SerialBT.hasClient();

  String json = "{";
  json += "\"wifiClients\":" + String(wifiClients) + ",";
  json += "\"btConnected\":" + String(btConnected ? "true" : "false");
  json += "}";

  server.send(200, "application/json", json);
}

// Handler per test buzzer
void handleBuzz() {
  lastActivity = millis();
  buzz(300);
  server.send(200, "text/plain", "Buzzed!");
}

void setup() {
  Serial.begin(115200);

  // Configurazione pin
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  digitalWrite(BUZZER_PIN, LOW);

  // Carica credenziali salvate
  preferences.begin("wifi", true);
  apSSID = preferences.getString("ssid", "tracker");
  apPassword = preferences.getString("password", "123456");
  preferences.end();

  // Carica preferenze di monitoraggio
  preferences.begin("monitor", true);
  monitorWiFi = preferences.getBool("wifi", false);
  monitorBT = preferences.getBool("bt", false);
  preferences.end();

  Serial.println("\n--- Tracker WiFi Hotspot ---");
  Serial.println("SSID: " + apSSID);
  Serial.println("Password: " + apPassword);
  Serial.println("\nPremi il pulsante per avviare l'hotspot");

  // Inizializza Bluetooth
  SerialBT.begin("TrackerBT");
  Serial.println("Bluetooth avviato: TrackerBT");

  // Setup web server routes
  server.on("/", handleRoot);
  server.on("/save", HTTP_POST, handleSave);
  server.on("/monitor", handleMonitor);
  server.on("/status", handleStatus);
  server.on("/buzz", handleBuzz);

  // Beep di avvio
  buzz(100);
  delay(100);
  buzz(100);
}

void loop() {
  // Gestione pulsante (con debounce)
  if (digitalRead(BUTTON_PIN) == LOW) {
    unsigned long now = millis();
    if (now - lastButtonPress > debounceDelay) {
      lastButtonPress = now;

      if (!apActive) {
        startAccessPoint();
        server.begin();
        Serial.println("Web server avviato");
      } else {
        Serial.println("AP già attivo - premi per mantenere attivo");
        lastActivity = millis(); // Reset timeout
        buzz(100);
      }
    }
  }

  // Gestione web server se AP attivo
  if (apActive) {
    server.handleClient();

    // Timeout inattività
    if (millis() - lastActivity > TIMEOUT) {
      Serial.println("Timeout inattività raggiunto");
      stopAccessPoint();
      server.stop();
    }
  }

  // Monitoraggio connessioni WiFi
  if (monitorWiFi && apActive) {
    int currentClients = WiFi.softAPgetStationNum();
    if (lastWiFiClients > 0 && currentClients == 0) {
      Serial.println("⚠️ Client WiFi disconnesso!");
      buzzerActive = true;
    } else if (currentClients > 0) {
      buzzerActive = false;
    }
    lastWiFiClients = currentClients;
  }

  // Monitoraggio connessioni Bluetooth
  if (monitorBT) {
    bool btConnected = SerialBT.hasClient();
    if (lastBTConnected && !btConnected) {
      Serial.println("⚠️ Dispositivo BT disconnesso!");
      buzzerActive = true;
    } else if (btConnected) {
      buzzerActive = false;
    }
    lastBTConnected = btConnected;
  }

  // Attivazione buzzer continuo se necessario
  if (buzzerActive) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(200);
    digitalWrite(BUZZER_PIN, LOW);
    delay(200);
  }

  delay(50);
}
