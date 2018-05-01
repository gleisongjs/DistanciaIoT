
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <Ultrasonic.h>
#include <PubSubClient.h>

#define agua 0
#define trigPin 1 //mesmo pino que o tx
#define echoPin 2

#define TOPICOSUB "casaconectada/sistemalayla" //topico mqtt de escuta
#define TOPICOPUB "casaconectada/sistemalayla/agua"
#define ID_MQTT "sistemalayla"

//const char* BROKER_MQTT = "192.168.0.14";
const char* BROKER_MQTT = "192.168.0.17";
//const char* BROKER_MQTT = "172.20.10.5";
int BROKER_PORT = 1883;

float distancia=0;
long microsec = 0;

Ultrasonic ultrasonic(trigPin, echoPin);

// WIFI
const char* SSID = "G&V"; 
const char* PASSWORD = "jsilva996**";

WiFiClient espClient; // Cria o objeto espClient
PubSubClient MQTT(espClient); // Instancia o Cliente MQTT passando o objeto espClient
int EstadoSaida = 0;
 

//const char* SSID = "Transformers II";
//const char* PASSWORD = "12345678*";

void setup() {
  Serial.begin(115200);
  pinMode(echoPin, INPUT); //define o pino 1 como entrada (recebe o echo)
  pinMode(trigPin, OUTPUT);//define o pino 2 como saida (envia o echo)
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);
  utilizandoOta();
  initWiFi();
  initMQTT();
}

void loop(){
  
    //distanciaDoSensor();
    ArduinoOTA.handle();
    EnviaEstadoOutputMQTT();
    VerificaConexoesWiFIEMQTT();
  }




void initMQTT() 
{
    MQTT.setServer(BROKER_MQTT, BROKER_PORT);   
    MQTT.setCallback(mqtt_callback);           
}

void mqtt_callback(char* topic, byte* payload, unsigned int length)
{
  String msg;
  
  for (int i = 0; i < length; i++)
  {
    char c = (char)payload[i];
    msg += c;
  }
  
  if (msg.equals("OFF"))
  EstadoSaida = 0;
  
  
  if (msg.equals("ON"))
  EstadoSaida = 1;
}

void EnviaEstadoOutputMQTT(void)
{

  char distanciaMqtt[5];
  distanciaDoSensor();

 
  if (EstadoSaida == 0)
  {
    //passando o valor float para char
    sprintf(distanciaMqtt,"%6.2f",distancia);
    MQTT.publish(TOPICOPUB, distanciaMqtt);
    // digitalWrite(agua, LOW);              
    // cont=0;
    // tempoAtual=0;
    // agualigada=0;
  }
  
  
  if (EstadoSaida == 1)
  {
    MQTT.publish(TOPICOPUB,  "ON");
    // digitalWrite(agua, HIGH);
    // delay(5000);
    // cont = cont+1;
    // agualigada=1;
  }
  
}
  

  void distanciaDoSensor(){
    
  microsec = ultrasonic.timing();
  distancia = ultrasonic.convert(microsec, Ultrasonic::CM);
  delay(1200);    
  }




  

 void utilizandoOta(){

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname("Sensor Ultrassonic");

   ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
 }

 void VerificaConexoesWiFIEMQTT(void)
{
    if (!MQTT.connected()) 
        reconnectMQTT(); //se não há conexão com o Broker, a conexão é refeita
    
     reconectWiFi(); //se não há conexão com o WiFI, a conexão é refeita
}

void initWiFi() 
{
    delay(10);
    Serial.println("------Conexao WI-FI------");
    Serial.print("Conectando-se na rede: ");
    Serial.println(SSID);
    Serial.println("Aguarde");
    
    reconectWiFi();
}

void reconectWiFi() 
{
    //se já está conectado a rede WI-FI, nada é feito. 
    //Caso contrário, são efetuadas tentativas de conexão
    if (WiFi.status() == WL_CONNECTED)
        return;
        
    WiFi.begin(SSID, PASSWORD); // Conecta na rede WI-FI
    
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(100);
        Serial.print(".");
    }
  
    Serial.println();
    Serial.print("Conectado com sucesso na rede ");
    Serial.print(SSID);
    Serial.println("IP obtido: ");
    Serial.println(WiFi.localIP());
}

void reconnectMQTT() 
{
    while (!MQTT.connected()) 
    {
        Serial.print("* Tentando se conectar ao Broker MQTT: ");
        Serial.println(BROKER_MQTT);
        if (MQTT.connect(ID_MQTT)) 
        {
            Serial.println("Conectado com sucesso ao broker MQTT!");
            MQTT.subscribe(TOPICOSUB); 
        } 
        else 
        {
            Serial.println("Falha ao reconectar no broker.");
            Serial.println("Havera nova tentatica de conexao em 2s");
            delay(1000);
        }
}
}



