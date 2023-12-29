/*
   Bibliotecas para uso do Wifi e do protocolo MQTT
*/
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <EEPROM.h>

/*
   Definição dos pinos
*/
#define MOT_D_A  D0
#define MOT_D_B  D5
#define MOT_E_A  D1
#define MOT_E_B  D4

#define FRENTE 'f'
#define TRAS 't'
#define DIREITA 'd'
#define ESQUERDA 'e'

/*
   Definição do tópico
*/
#define TOPICO "aula_mqtt/mostrabot"

/*
   Configuração da rede de internet
   SSID - Nome da rede
   senha - senha
   broker_mqtt - endereço do servidor mqtt
*/
const char* ssid = "NetComm 5392";
const char* senha = "dufigedara";
const char* broker_mqtt = "test.mosquitto.org";

/*
   Inicialização das bibliotecas de Wifi e de MQTT
*/
WiFiClient espClient;
PubSubClient client(espClient);

/*
   Variáveis utilizadas no programa
*/
int DIF1 = 0;
int DIF2 = 0;

/*
   Configuração da WiFi
*/
void setup_wifi() {

  delay(10);                        //Aguarda 10 milissegundos para inicializar

  Serial.println();                 //Manda pela serial somente uma quebra de linha
  Serial.print("Conectando a ");
  Serial.println(ssid);

  WiFi.begin(ssid, senha);          //inicializa a conexão ao Wifi

  while (WiFi.status() != WL_CONNECTED) {   //Aguarda a conexão (enquanto não conectar imprime um . a cada 500 ms)
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());             //Define a semente para a geração de numeros randomicos

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());   //Conectado e recebido um IP :)
}

/*
    Função de Callback
    É a função que é chamada sempre que for recebida uma mensagem
    Ela recebe o topico como um ponteiro (quem tiver interesse pergunte ao chatGPT o que é
    um ponteiro e exemplos de uso em C++
*/
void callback(char* topico, byte* mensagem, unsigned int tamanho) {
  Serial.print("Messagem recebida [");
  Serial.print(topico);
  Serial.print("] ");
  for (int i = 0; i < tamanho; i++) {
    Serial.print((char)mensagem[i]);  //Converte a mensagem para caracter e faz a impressão 1 a 1 para poder mostrar na serial
  }
  Serial.println();

  if (strcmp(topico, TOPICO "/FRENTE") == 0) { //verifica se o tópico é FRENTE

    int velocidade = 0;
    int tempo = 0;
    char v_ou_t = 'v';

    /*
       Anda para frente conforme velocidade e tempo
       Padrão da mensagem:
       velocidade;tempo
       Exemplo:
       255;1000
    */

    //Passa por toda a mensagem para montar os valores
    for (int i = 0; i < tamanho; i++) {
      //caso identifique um ; altera para montar o tempo
      if (mensagem[i] == ';') {
        v_ou_t = 't';
      } else {
        int numero = mensagem[i] - '0'; //Converte para numero
        if (v_ou_t == 'v') {
          velocidade = velocidade * 10 + numero; //Monta o valor da velocidade
        } else {
          tempo = tempo * 10 + numero; //Monta o valor do tempo
        }
      }
    }

    //Verifica se a velocidade e o tempo estão dentro dos limites
    if (velocidade >= 0 && velocidade <= 255 && tempo >= 0 && tempo <= 5000) {
      //chama a função de andar
      anda(velocidade, 'f', tempo);
    } else {
      Serial.println("Mensagem inválida");
    }
  }

  if (strcmp(topico, TOPICO "/AJUSTE") == 0) { //verifica se o tópico é AJUSTE
    /*
       Altera a velocidade dos motores conforme o lado e valor
       lado - 'e' para esquerda ou 'd' para direita
       valor - até 50, será somado e subtraido da velocidade do motor
       Padrão da mensagem:
       lado;valor
       Exemplo:
       e;10
    */

    char sentido = (char)mensagem[0];
    int diferenca = 0;

    if (sentido == 'd' || sentido == 'e') {
      for (int i = 2; i < tamanho; i++) {
        int numero = mensagem[i] - '0'; //Converte para numero
        diferenca = diferenca * 10 + numero; //Monta o valor do tempo
      }
      if (diferenca >= 0 && diferenca <= 50) {
        set_dif(sentido, diferenca);
        client.publish(TOPICO "/retorno", "Ajuste realizado");
      }
    } else {
      client.publish(TOPICO "/retorno", "Mensagem inválida");
    }

  }
}

/*
  Reconecta ao servidor MQTT
*/
void reconnect() {
  //Faz o loop até reconectar
  while (!client.connected()) {
    Serial.print("Tentando reconectar MQTT");
    //Cria um ID randomico
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    //Tenta conectar
    if (client.connect(clientId.c_str())) {
      Serial.println("Conectado");

      // Faz uma publicação que conectou
      client.publish(TOPICO "/inicio", "OK");

      //Faz a subscrição
      client.subscribe(TOPICO "/#");

    } else {
      Serial.print("Erro, rc = ");
      Serial.print(client.state());
      Serial.println(" Tentando novamente em 5 segundos");
      delay(5000);
    }
  }
}

void setup() {
  pinMode(MOT_D_A, OUTPUT);
  pinMode(MOT_D_B, OUTPUT);
  pinMode(MOT_E_A, OUTPUT);
  pinMode(MOT_E_B, OUTPUT);

  Serial.begin(9600);

  start_dif();

  setup_wifi();

  client.setServer(broker_mqtt, 1883);
  client.setCallback(callback);
}

void loop() {

  //Verifica se ainda está conectado ao Broker
  //Bloco principal para conexão e funcionamento do MQTT
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  //Restante do código, cuidar com delays maiores que 50 ms
  delay(50);
}

void anda(int velocidade, int sentido, int tempo) {
  // 0 - frente, 1 - tras, 2 - direita, 3 - esquerda
  digitalWrite(MOT_D_A, LOW);
  digitalWrite(MOT_D_B, LOW);
  digitalWrite(MOT_E_A, LOW);
  digitalWrite(MOT_E_B, LOW);

  int velocidade_dir = velocidade + DIF1;
  int velocidade_esq = velocidade + DIF2;

  if (velocidade_dir > 255) {
    velocidade_dir = 255;
  }

  if (velocidade_esq > 255) {
    velocidade_esq = 255;
  }

  if (sentido == FRENTE) {
    analogWrite(MOT_D_A, velocidade_dir);
    analogWrite(MOT_E_A, velocidade_esq);
  }

  if (sentido == TRAS) {
    analogWrite(MOT_D_B, velocidade_dir);
    analogWrite(MOT_E_B, velocidade_esq);
  }

  if (sentido == DIREITA) {
    analogWrite(MOT_D_B, velocidade_dir);
    analogWrite(MOT_E_A, velocidade_esq);
  }

  if (sentido == ESQUERDA) {
    analogWrite(MOT_D_A, velocidade_dir);
    analogWrite(MOT_E_B, velocidade_esq);
  }

  delay(tempo);

  digitalWrite(MOT_D_A, LOW);
  digitalWrite(MOT_D_B, LOW);
  digitalWrite(MOT_E_A, LOW);
  digitalWrite(MOT_E_B, LOW);
}

void start_dif()
{
  int diferenca;
  EEPROM.begin(100);
  char EEPROM_CODIGO = EEPROM.read(97);

  if (EEPROM_CODIGO == 'M') {
    diferenca = EEPROM.read(99);
    if (EEPROM.read(98) == 1)
    {
      //SINAL NEGATIVO
      diferenca = 0 - diferenca;
    }

    DIF1 = diferenca;
    DIF2 = 0 - diferenca;
  }
  EEPROM.end();

  Serial.println(DIF1);
  Serial.println(DIF2);
}

void set_dif(char _lado, int diferenca)
{
  EEPROM.begin(100);
  if (_lado == 'e')
  {
    EEPROM.write(98, 1);
    DIF2 = diferenca;
    DIF1 = 0 - diferenca;
  }
  else if (_lado == 'd')
  {
    EEPROM.write(98, 0);
    DIF1 = diferenca;
    DIF2 = 0 - diferenca;
  }

  EEPROM.write(99, diferenca);
  EEPROM.write(97, 'M');

  delay(1000);
  EEPROM.end();

  Serial.println(DIF1);
  Serial.println(DIF2);
}
