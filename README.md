# 📡 Tracker WiFi Hotspot - ESP32-S3

Progetto PlatformIO per ESP32 Seeeduino XIAO S3 che crea un hotspot WiFi controllato da pulsante con interfaccia web per configurazione e monitoraggio connessioni.

## 🔧 Hardware Richiesto

- **ESP32-S3 Seeeduino XIAO**
- **Buzzer** collegato al **Pin 2**
- **Pulsante** collegato al **Pin 4** (pull-up interno attivo)

## ⚡ Funzionalità

### 1. **Hotspot WiFi on-demand**
- Premi il pulsante → Attiva hotspot WiFi
- Nome default: `tracker`
- Password default: `123456`
- Timeout automatico dopo 60 secondi di inattività

### 2. **Interfaccia Web di Configurazione**
- Collegati all'hotspot e vai su `http://192.168.4.1`
- Cambia nome e password dell'hotspot
- Le credenziali vengono salvate in memoria permanente

### 3. **Monitoraggio Connessioni Intelligente**
- **WiFi**: Attiva il buzzer se un client si disconnette
- **Bluetooth**: Attiva il buzzer se un dispositivo BT si disconnette
- Configurabile tramite checkbox nell'interfaccia web

### 4. **Feedback Sonoro**
- Avvio sistema: 2 beep corti
- Avvio hotspot: 2 beep
- Spegnimento hotspot: 1 beep lungo
- Disconnessione rilevata: beep intermittente continuo

## 🚀 Come Usare

### Upload del Codice
```bash
# Compila e carica il codice
pio run --target upload

# Monitora il serial
pio device monitor
```

### Utilizzo
1. **Avvia l'hotspot**: Premi il pulsante sul Pin 4
2. **Connettiti**: Usa WiFi "tracker" con password "123456"
3. **Configura**: Apri il browser su `http://192.168.4.1`
4. **Personalizza**:
   - Cambia nome e password hotspot
   - Attiva monitoraggio WiFi/Bluetooth
   - Testa il buzzer

### Monitoraggio Bluetooth
- Nome dispositivo BT: `TrackerBT`
- Connetti il tuo telefono via Bluetooth
- Attiva "Monitora Bluetooth" nell'interfaccia web
- Se il telefono si disconnette → buzzer attivo

### Mantenere Attivo l'Hotspot
- Premi nuovamente il pulsante per resettare il timer di 60 secondi
- Oppure interagisci con l'interfaccia web (ogni richiesta resetta il timer)

## 📝 Note Tecniche

- **Memoria permanente**: Le credenziali e preferenze sono salvate in NVS (Non-Volatile Storage)
- **Debounce pulsante**: 300ms per evitare pressioni multiple
- **Auto-refresh**: L'interfaccia web aggiorna lo stato ogni 2 secondi
- **IP fisso AP**: 192.168.4.1

## 🎯 Scenari d'Uso

### Esempio 1: Anti-furto per Bici
1. Attiva hotspot
2. Connetti il telefono via Bluetooth (nome "TrackerBT")
3. Attiva "Monitora Bluetooth"
4. Se qualcuno porta via la bici (telefono fuori range) → buzzer squilla

### Esempio 2: Allarme Connessione WiFi
1. Attiva hotspot
2. Connetti un dispositivo WiFi
3. Attiva "Monitora WiFi"
4. Se il dispositivo si disconnette → buzzer squilla

## 🛠️ Personalizzazione

### Modificare il Timeout
Nel file `src/main.cpp`:
```cpp
const unsigned long TIMEOUT = 60000; // Cambia qui (in millisecondi)
```

### Modificare i Pin
```cpp
#define BUZZER_PIN 2  // Cambia il pin del buzzer
#define BUTTON_PIN 4  // Cambia il pin del pulsante
```

### Cambiare Credenziali Default
```cpp
String apSSID = "tracker";      // Nome default
String apPassword = "123456";   // Password default (min 8 caratteri)
```

## 📱 Interfaccia Web

L'interfaccia è completamente responsive e moderna con:
- Design gradient viola/blu
- Aggiornamento automatico dello stato
- Salvataggio istantaneo delle preferenze
- Test buzzer con un click

## 🔒 Sicurezza

- Password WiFi minimo 8 caratteri (validazione client-side)
- Timeout automatico per risparmiare energia
- Bluetooth visibile solo durante l'uso

## 📄 Licenza

Open Source - Usa e modifica liberamente!

---

**Buon tracking! 🎯**
