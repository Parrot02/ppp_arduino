#include <PPP.h>
#include <NetworkClientSecure.h>
#include <NetworkClient.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <Arduino_JSON.h>
#include "DHT.h"

#define PPP_MODEM_APN "claro.com.br" 
#define PPP_MODEM_PIN "0000"  // or NULL

#define PPP_MODEM_TX      17
#define PPP_MODEM_RX      16
#define PPP_MODEM_RTS     27 // -1 
#define PPP_MODEM_CTS     26 // -1 
#define PPP_MODEM_FC      ESP_MODEM_FLOW_CONTROL_HW
#define PPP_MODEM_MODEL   PPP_MODEM_GENERIC

// Variáveis 

const char* user = "youtube";
const char* password = "Ll123456";
const char* clientid = "esp32-youtube-awdawdawdawdawdaw";
const char* pub_topic = "devices/esp32";
const char* sub_topic = "devices/esp32";
const char* will_topic = "logs/esp32";
const char* URL = "06c5f64164d14759bb3b8c2d6b4bb33c.s1.eu.hivemq.cloud"; // EMQX 
float temp; // Temperatura
float hum; // Umidade 
int intervalo = 10000;
int connectionRetry = 0;
unsigned long ultimaExec = 0; 

bool dataMode = false; 
NetworkClientSecure client;
PubSubClient mqtt(client);
DHT sensor(4, DHT22);
JSONVar dados; 

static const char *root_ca PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)EOF";


void onEvent(arduino_event_id_t event, arduino_event_info_t info) {
  switch (event) {
    case ARDUINO_EVENT_PPP_START:        Serial.println("PPP Started"); break;
    case ARDUINO_EVENT_PPP_CONNECTED:    Serial.println("PPP Connected"); break;
    case ARDUINO_EVENT_PPP_GOT_IP:       Serial.println("PPP Got IP"); break;
    case ARDUINO_EVENT_PPP_LOST_IP:      Serial.println("PPP Lost IP"); break;
    case ARDUINO_EVENT_PPP_DISCONNECTED: Serial.println("PPP Disconnected"); break;
    case ARDUINO_EVENT_PPP_STOP:         Serial.println("PPP Stopped"); break;
    default:                             break;
  }
}

void setClock() {
  configTime(0, 0, "pool.ntp.org");

  Serial.print(F("Waiting for NTP time sync: "));
  time_t nowSecs = time(nullptr);
  while (nowSecs < 8 * 3600 * 2) {
    delay(500);
    Serial.print(F("."));
    yield();
    nowSecs = time(nullptr);
  }

  Serial.println();
  struct tm timeinfo;
  gmtime_r(&nowSecs, &timeinfo);
  Serial.print(F("Current time: "));
  Serial.print(asctime(&timeinfo));
}

void reconnect(){
    Serial.print("[MQTT] Conectando...");

    Serial.println("Tentativas de reconexão: "); 
    Serial.println(connectionRetry);
    if(mqtt.connect(clientid, user, password, will_topic, 0, false, "Cliente desconectado!", true)){
        Serial.print("[MQTT] Conectado!");
        mqtt.publish(will_topic, "Um salve para a galera do canal da FAT no Youtube!");
        mqtt.subscribe(sub_topic);
    } else {
        Serial.print("[MQTT] Houve uma falha! erro =");
        connectionRetry += 1; 
        Serial.print(mqtt.state());
        Serial.println(" ");
        delay(2000);
    }
}

void callback(char* topic, byte* payload, unsigned int length){
    Serial.print("Mensagem recebida: [");
    Serial.print(topic);
    Serial.print("]");

    char message[length];

    for(int i = 0; i < length; i++){
      message[i] = (char) payload[i];
    }

    Serial.println(message);
}

void connectRoutine(){
    PPP.setApn(PPP_MODEM_APN);
    PPP.setPins(PPP_MODEM_TX, PPP_MODEM_RX);
    Serial.println("Starting the modem. It might take a while!");
    PPP.begin(PPP_MODEM_MODEL, 2);
    Serial.print("Manufacturer: ");
    Serial.println(PPP.cmd("AT+CGMI", 10000));
    Serial.print("Model: ");
    Serial.println(PPP.moduleName());
    Serial.print("IMEI: ");

    bool attached = PPP.attached();
    if (!attached) {
      int i = 0;
      unsigned int s = millis();
      Serial.print("Waiting to connect to network");
      while (!attached && ((++i) < 600)) {
        Serial.print(".");
        delay(100);
        attached = PPP.attached();
      }
      Serial.print((millis() - s) / 1000.0, 1);
      Serial.println("s");
      attached = PPP.attached();
    }

    Serial.print("Attached: ");
    Serial.println(attached);
    Serial.print("State: ");
    Serial.println(PPP.radioState());
    if (attached) {
      Serial.print("Operator: ");
      Serial.println(PPP.operatorName());
      Serial.print("IMSI: ");
      Serial.println(PPP.IMSI());
      Serial.print("RSSI: ");
      Serial.println(PPP.RSSI());
      int ber = PPP.BER();
      if (ber > 0) {
        Serial.print("BER: ");
        Serial.println(ber);
        Serial.print("NetMode: ");
        Serial.println(PPP.networkMode());
      }

      Serial.println("Switching to data mode...");
      PPP.mode(ESP_MODEM_MODE_DATA);  // Data and Command mixed mode
      if (!PPP.waitStatusBits(ESP_NETIF_CONNECTED_BIT, 1000)) {
        Serial.println("Failed to connect to internet!");
      } else {
        Serial.println("Connected to internet!");
        connectionRetry = 0; 
      }
    } else {
      Serial.println("Failed to connect to network!");
    }
}

void sendDados(){
       temp = sensor.readTemperature();
       hum = sensor.readHumidity();

       dados["temperatura"] = temp; 
       dados["umidade"] = hum; 
       String tempHum = JSON.stringify(dados);

      mqtt.publish("esp32/youtube", tempHum.c_str());
      delay(10000);
}

void setup() {
    Serial.begin(115200);
    // sensor.begin(); // Inicializa o sensor 

    // Listen for modem events
    Network.onEvent(onEvent);
    
    // Set SSL 
    client.setCACert(root_ca);
    delay(2000);
    // MQTT setup 
    mqtt.setServer(URL, 8883);
    mqtt.setCallback(callback);
    // Configure the modem
    // Serial.println(PPP.IMEI());
    connectRoutine();

    setClock();
}

void loop() {

  if(connectionRetry > 2){
      PPP.end();
      connectRoutine();
  }

  if(!mqtt.connected()){
      reconnect();
  } 
    mqtt.loop();
    // sendDados();
}
