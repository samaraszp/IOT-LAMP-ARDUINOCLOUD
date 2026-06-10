#include "arduino_secrets.h"
#include "thingProperties.h"

// Mapeamento dos pinos no ESP32-S3
const int PINO_TEMP   = 4;   
const int PINO_LDR    = 5;   
const int PINO_POT    = 6;   
const int PINO_BOTAO  = 7;   
const int PINO_LED_R  = 15;  
const int PINO_LED_G  = 16;  
const int PINO_LED_B  = 17;  
const int PINO_BUZZER = 18;  

// Variaveis de controle
volatile bool sistema_ativo = true; 
unsigned long tempo_anterior = 0;
const long intervalo = 2000;         

// Funcao de interrupcao do botao de emergencia
void IRAM_ATTR tratarBotao() {
  static unsigned long ultimo_debounce = 0;
  unsigned long tempo_atual = millis();
  
  if (tempo_atual - ultimo_debounce > 200) {
    sistema_ativo = !sistema_ativo; 
    ultimo_debounce = tempo_atual;
  }
}

void setup() {
  Serial.begin(9600);
  delay(1500); 

  // Configuracao das saidas
  pinMode(PINO_LED_R, OUTPUT);
  pinMode(PINO_LED_G, OUTPUT);
  pinMode(PINO_LED_B, OUTPUT);
  pinMode(PINO_BUZZER, OUTPUT);
  
  // Configuracao do botao com interrupcao
  pinMode(PINO_BOTAO, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PINO_BOTAO), tratarBotao, FALLING);

  // Inicializacao do Arduino IoT Cloud
  initProperties();
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();
}

void loop() {
  ArduinoCloud.update(); 
  
  unsigned long tempo_atual = millis();

  // Se o botao de emergencia for ativado, desliga tudo
  if (!sistema_ativo) {
    apagarLED();
    noTone(PINO_BUZZER);
    led_status = false; 
    return; 
  }

  // Leitura dos sensores a cada 2 segundos
  if (tempo_atual - tempo_anterior >= intervalo) {
    tempo_anterior = tempo_atual;

    // Leitura da temperatura
    int leitura_temp_bruta = analogRead(PINO_TEMP);
    temperature = (leitura_temp_bruta * 3.3 / 4095.0) * 100.0; 

    // Leitura do LDR
    int leitura_ldr = analogRead(PINO_LDR);
    lux_level = map(leitura_ldr, 0, 4095, 100, 0); 

    // Envio para o Monitor Serial
    Serial.print("Temperatura: ");
    Serial.print(temperature);
    Serial.println(" C");
    Serial.print("Luminosidade: ");
    Serial.print(lux_level);
    Serial.println(" %");

    // Condicao de perigo por temperatura
    if (temperature < 0.0 || temperature > 25.0) {
      Serial.println("Perigo! Desligar!");
      apagarLED();
      tone(PINO_BUZZER, 1000); 
      led_status = false;
    } 
    // Funcionamento normal do sistema
    else {
      noTone(PINO_BUZZER); 
      
      // Automacao por luz (Ambiente Escuro)
      if (lux_level < 30.0) {
        led_status = true; 
        
        // Controle de cor pelo potenciometro
        int valor_pot = analogRead(PINO_POT);
        
        if (valor_pot < 1365) {
          definirCorRGB(255, 0, 0); 
          led_color = "Vermelho";
        } else if (valor_pot >= 1365 && valor_pot < 2730) {
          definirCorRGB(255, 255, 0); 
          led_color = "Amarelo";
        } else {
          definirCorRGB(0, 0, 255); 
          led_color = "Azul";
        }
      } 
      // Ambiente Claro
      else {
        apagarLED();
        led_status = false;
      }
    }
  }
}

// Funcoes para o LED RGB
void definirCorRGB(int r, int g, int b) {
  analogWrite(PINO_LED_R, r);
  analogWrite(PINO_LED_G, g);
  analogWrite(PINO_LED_B, b);
}

void apagarLED() {
  analogWrite(PINO_LED_R, 0);
  analogWrite(PINO_LED_G, 0);
  analogWrite(PINO_LED_B, 0);
}

// Controle do LED via Dashboard
void onLedStatusChange()  {
  if (led_status && sistema_ativo) {
    definirCorRGB(255, 255, 255); 
    Serial.println("LED ligado via Nuvem");
  } else {
    apagarLED();
    Serial.println("LED desligado via Nuvem");
  }
}

// Controle de cor via Dashboard
void onLedColorChange()  {
  if (!led_status || !sistema_ativo) return;

  if (led_color == "Vermelho" || led_color == "vermelho") {
    definirCorRGB(255, 0, 0);
  } else if (led_color == "Amarelo" || led_color == "amarelo") {
    definirCorRGB(255, 255, 0);
  } else if (led_color == "Azul" || led_color == "azul") {
    definirCorRGB(0, 0, 255);
  }
  Serial.print("Cor alterada via Nuvem para: ");
  Serial.println(led_color);
}