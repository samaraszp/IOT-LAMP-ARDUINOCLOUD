#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>

// ======================================================
// Configurações de Autenticação do Dispositivo
// ======================================================

const char DEVICE_LOGIN_NAME[] = "e0023e43-a9fa-453e-8b28-2a90aa8d2f9e";
const char SSID[] = "CIMATEC-VISITANTE";
const char PASS[] = "";
const char DEVICE_KEY[] = "TOM6HBg202qtvx!GfBQ0xIm9v";

// ======================================================
// Variáveis do Arduino IoT Cloud
// ======================================================

String comando;
float temperatura;
int luminosidade;

// ======================================================
// Handler de conexão
// ======================================================

WiFiConnectionHandler ArduinoIoTPreferredConnection(SSID, PASS);

// ======================================================
// Protótipos de funções
// ======================================================

void onComandoChange();
void desligarLED();
void mudarCor(int r, int g, int b);

// ======================================================
// Inicialização das propriedades da nuvem
// ======================================================

void initProperties()
{
    ArduinoCloud.setBoardId(DEVICE_LOGIN_NAME);
    ArduinoCloud.setSecretDeviceKey(DEVICE_KEY);

    ArduinoCloud.addProperty(
        comando,
        READWRITE,
        ON_CHANGE,
        onComandoChange
    );

    ArduinoCloud.addProperty(
        temperatura,
        READ,
        5 * SECONDS,
        NULL
    );

    ArduinoCloud.addProperty(
        luminosidade,
        READ,
        5 * SECONDS,
        NULL
    );
}

// ======================================================
// Definição dos pinos
// ======================================================

const int PIN_RED = 12;
const int PIN_GREEN = 13;
const int PIN_BLUE = 14;
const int PIN_BUZZER = 5;

const int PIN_TEMP = A0;
const int PIN_LDR = A1;

// ======================================================
// Variáveis de controle
// ======================================================

bool tempAtiva = true;
bool detectorAtivo = true;
bool buzzerAtivo = true;

unsigned long tempoLed_MS = 0;
bool corTemporariaAtiva = false;

// ======================================================
// Setup
// ======================================================

void setup()
{
    Serial.begin(115200);
    delay(1500);

    // Correção para problemas de conexão WiFi
    WiFi.disconnect(true);
    WiFi.mode(WIFI_STA);
    delay(1000);

    // Inicializa propriedades da nuvem
    initProperties();

    ArduinoCloud.begin(ArduinoIoTPreferredConnection);

    setDebugMessageLevel(2);
    ArduinoCloud.printDebugInfo();

    // Configura pinos
    pinMode(PIN_RED, OUTPUT);
    pinMode(PIN_GREEN, OUTPUT);
    pinMode(PIN_BLUE, OUTPUT);
    pinMode(PIN_BUZZER, OUTPUT);
}

// ======================================================
// Loop principal
// ======================================================

void loop()
{
    ArduinoCloud.update();

    // -------------------------
    // Sensor de temperatura
    // -------------------------
    if (tempAtiva)
    {
        int leituraTemp = analogRead(PIN_TEMP);

        temperatura =
            (leituraTemp * 5.0 / 1023.0) * 100.0;
    }
    else
    {
        temperatura = 0;
    }

    // -------------------------
    // Sensor de luminosidade
    // -------------------------
    if (detectorAtivo)
    {
        luminosidade = analogRead(PIN_LDR);
    }
    else
    {
        luminosidade = 0;
    }

    // -------------------------
    // Controle de tempo do LED
    // -------------------------
    if (
        corTemporariaAtiva &&
        (millis() - tempoLed_MS >= 1000)
    )
    {
        desligarLED();
        corTemporariaAtiva = false;
    }
}

// ======================================================
// Funções auxiliares
// ======================================================

void desligarLED()
{
    digitalWrite(PIN_RED, LOW);
    digitalWrite(PIN_GREEN, LOW);
    digitalWrite(PIN_BLUE, LOW);
}

void mudarCor(int r, int g, int b)
{
    digitalWrite(PIN_RED, r);
    digitalWrite(PIN_GREEN, g);
    digitalWrite(PIN_BLUE, b);
}

// ======================================================
// Callback da variável "comando"
// ======================================================

void onComandoChange()
{
    Serial.print("Comando recebido: ");
    Serial.println(comando);

    // -------------------------
    // Ligar / Desligar geral
    // -------------------------

    if (comando == "Ligar")
    {
        mudarCor(HIGH, HIGH, HIGH);
        corTemporariaAtiva = false;
    }
    else if (comando == "Desligar")
    {
        desligarLED();
        corTemporariaAtiva = false;
    }

    // -------------------------
    // Cores temporárias
    // -------------------------

    else if (comando == "Vermelho")
    {
        mudarCor(HIGH, LOW, LOW);

        tempoLed_MS = millis();
        corTemporariaAtiva = true;
    }
    else if (comando == "Amarelo")
    {
        mudarCor(HIGH, HIGH, LOW);

        tempoLed_MS = millis();
        corTemporariaAtiva = true;
    }
    else if (comando == "Azul")
    {
        mudarCor(LOW, LOW, HIGH);

        tempoLed_MS = millis();
        corTemporariaAtiva = true;
    }

    // -------------------------
    // Temperatura
    // -------------------------

    else if (comando == "Desativar Temperatura")
    {
        tempAtiva = false;
        Serial.println("Sensor de Temperatura Desativado.");
    }
    else if (comando == "Ativar Temperatura")
    {
        tempAtiva = true;
        Serial.println("Sensor de Temperatura Ativado.");
    }

    // -------------------------
    // Detector (LDR)
    // -------------------------

    else if (comando == "Desativar Detector")
    {
        detectorAtivo = false;
        Serial.println("Fotorresistor Desativado.");
    }
    else if (comando == "Ativar Detector")
    {
        detectorAtivo = true;
        Serial.println("Fotorresistor Ativado.");
    }

    // -------------------------
    // Buzzer
    // -------------------------

    else if (comando == "Desativar Buzzer")
    {
        buzzerAtivo = false;

        digitalWrite(PIN_BUZZER, LOW);

        Serial.println("Buzzer Desativado.");
    }
    else if (comando == "Ativar Buzzer")
    {
        buzzerAtivo = true;

        Serial.println("Buzzer Ativado.");
    }
}
