

#if 0
#define ESP_ERR_ESPNOW_BASE         (ESP_ERR_WIFI_BASE + 100) /*!< ESPNOW error number base. */
#define ESP_ERR_ESPNOW_NOT_INIT     (ESP_ERR_ESPNOW_BASE + 1) /*!< ESPNOW is not initialized. */
#define ESP_ERR_ESPNOW_ARG          (ESP_ERR_ESPNOW_BASE + 2) /*!< Invalid argument */
#define ESP_ERR_ESPNOW_NO_MEM       (ESP_ERR_ESPNOW_BASE + 3) /*!< Out of memory */
#define ESP_ERR_ESPNOW_FULL         (ESP_ERR_ESPNOW_BASE + 4) /*!< ESPNOW peer list is full */
#define ESP_ERR_ESPNOW_NOT_FOUND    (ESP_ERR_ESPNOW_BASE + 5) /*!< ESPNOW peer is not found */
#define ESP_ERR_ESPNOW_INTERNAL     (ESP_ERR_ESPNOW_BASE + 6) /*!< Internal error */
#define ESP_ERR_ESPNOW_EXIST        (ESP_ERR_ESPNOW_BASE + 7) /*!< ESPNOW peer has existed */
#define ESP_ERR_ESPNOW_IF           (ESP_ERR_ESPNOW_BASE + 8) /*!< Interface error */
#endif

#define USE_US_TIMER                                                // microsecond timer
#include <osapi.h>
#include <ESP8266WiFi.h>
#include <espnow.h>
//#include "ESPAsyncWebSrv.h"
#include <Ticker.h>
#include "ESP8266TimerInterrupt.h"


//////////////////////////////////////////////////////////////////////////////////////////
#define  MAX_NODES          12
#define  GATEWAY_GPIO_SET   12      // D6
#define  SHOW_SYNC          14      // D5
#define  SYNC_INTERVAL     20000
/////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////
struct  Node {
  uint8_t     mac[6];
  bool        node;
  bool        sync;
  bool        beacon;
  bool        started;
  uint32_t    token;
  uint32_t    randgen;
  uint32_t    tick;
  uint64_t    round_trip;
  uint8_t     pairedto[6];
  bool is_paired()const {
    return pairedto[0] != 0;
  }
};

/////////////////////////////////////////////////////////////////////////////////////////
static uint8_t        MacAll[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
static Node           ThisNode;
static Node           Nodes[MAX_NODES];
static uint8_t        NodesCount = 0;
static uint64_t       Micros = 0;
static uint64_t       SycTime = 0;
static bool           Count = false;
static bool           IsGw = false;
Ticker                Timer;
static bool           Dirty = false;
static size_t         Loops;
static bool           Toggle;
static uint32_t       Password = 0x67452345;
static ESP8266Timer   ITimer;


//////////////////////////////////////////////////////////////////////////////////////////
static void print_mac(const uint8_t* mac = nullptr);
static void gw_loop();
static void node_loop();
static void gw_rec(unsigned char*, const Node*);
static void node_rec(unsigned char*, const Node*);
static void gw_send(unsigned char*, uint8_t status);
static void node_send(unsigned char*, uint8_t status);
static void i_am_here();
static void on_rec(unsigned char* mac, unsigned  char *recbuf, unsigned char len);
static void on_send(unsigned char* mac, unsigned char status);
static void pulse();
static void add_node(unsigned char* mac, const Node* pn);
static uint64_t gt(){ return (uint64_t)millis();} //micros64
//////////////////////////////////////////////////////////////////////////////////////////
void setup()
{
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(SHOW_SYNC, OUTPUT);
  pinMode(GATEWAY_GPIO_SET, INPUT_PULLUP);
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(SHOW_SYNC, LOW);
  delay(100);
  print_mac();
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != 0)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(&on_send);
  esp_now_register_recv_cb(&on_rec);
  memset(&ThisNode, 0, sizeof(Node));
  ThisNode.started=true;
  WiFi.macAddress(ThisNode.mac);
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);

  if (digitalRead(GATEWAY_GPIO_SET) == LOW)               // pin in short is the GW
  {
    Serial.println("GATEWAY START ");
    ThisNode.node = false;
    IsGw = true;
    Timer.attach_ms(5000, &i_am_here);                  //  gw beacon
  }
  else
  {
    Serial.println("NODE START ");
    ThisNode.node = true;
    ThisNode.pairedto[0] = 0;
    IsGw = false;
  }
  digitalWrite(SHOW_SYNC, LOW);
  SycTime = millis();
  Serial.println("SETUP OKAY");
  delay(1000);
}

/////////////////////////////////////////////////////////////////////////////////////////
void loop()
{
  if (IsGw)
  {
    gw_loop();
  }
  else
  {
    node_loop();
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
void gw_loop()
{
  if (millis() - SycTime > SYNC_INTERVAL || Dirty)
  {
    if (NodesCount)
    {
      Serial.println("Time sync to all");
      ThisNode.sync = true;
      ThisNode.randgen = millis();
      //ThisNode.token=ThisNode.randgen & Password;
      ThisNode.beacon = false;
      esp_now_send(0, (uint8_t *) &ThisNode, sizeof(ThisNode));
    }
    SycTime = millis();
    Dirty=false;
  }

  if (millis() - Loops > 2000) {
    digitalWrite(LED_BUILTIN, Toggle = !Toggle);
    Loops = millis();
  }
}

/////////////////////////////////////////////////////////////////////////////////////////
void node_loop()
{
  if (Micros)                             // syncronised
  {
    if (gt() - Micros > 10)
    {
      Micros=gt();
      pulse();
    }
  }
  if (ThisNode.is_paired())
  {
    if (millis() - Loops > 1000) {
      digitalWrite(LED_BUILTIN, Toggle = !Toggle);
      Loops = millis();
    }
  }
  else
  {
    if (millis() - Loops > 300) {
      digitalWrite(LED_BUILTIN, Toggle = !Toggle);
      Loops = millis();
    }

  }
}

//////////////////////////////////////////////////////////////////////////////////////////
void i_am_here()
{
  Serial.printf("i am here %d\r\n",ThisNode.started);
  ThisNode.tick = gt();
  ThisNode.randgen = millis();
  //ThisNode.token = ThisNode.randgen & Password;
  ThisNode.beacon = true;
  int ret = esp_now_send(MacAll, (uint8_t *) &ThisNode, sizeof(ThisNode));
  if(ret==0)
  {
    ThisNode.started=false;
  }
}

//////////////////////////////////////////////////////////////////////////////////////////
void print_mac(const uint8_t* mac)
{
  if (mac == nullptr)
  {
    Serial.println(WiFi.macAddress());
  }
  else
  {
    Serial.printf("%02x:%02x:%02x:%02x:%02x:%02x\r\n",
                  int(mac[0]),
                  int(mac[1]),
                  int(mac[2]),
                  int(mac[3]),
                  int(mac[4]),
                  int(mac[5]));
  }
  Serial.flush();
}

/////////////////////////////////////////////////////////////////////////////////////////
bool has_node(const unsigned char* mac)
{
  for (int i = 0; i < NodesCount; i++)
  {
    if (!memcmp(mac, Nodes[i].mac, sizeof(Nodes[i].mac))) {
      return true;
    }
  }
  return false;
}

//////////////////////////////////////////////////////////////////////////////////////////
void on_rec(unsigned char* mac, unsigned  char *recbuf, unsigned char len)
{
  if (IsGw)
  {
    Serial.println("on rec gw");
    if (len == sizeof(Node) && NodesCount < MAX_NODES)
    {
        gw_rec(mac, (Node*)recbuf);
    }
  }
  else
  {
    if (len == sizeof(Node))
    {
      const Node* pn = (Node*)recbuf;
      if (1)
      {
        node_rec(mac, pn);
      }
      else
      {
        Serial.println("INVALID TOKEN");
      }

    }
  }
}

//////////////////////////////////////////////////////////////////////////////////////////
void on_send(uint8_t *mac, uint8_t status)
{
  Serial.print(status == 0 ? "SendOkay: " : "SendFail: ");
  print_mac(mac);
  if (IsGw)
    gw_send(mac, status);
  else
    node_send(mac, status);
}

//////////////////////////////////////////////////////////////////////////////////////////
void pulse()
{
  digitalWrite(SHOW_SYNC, HIGH);
  delay(1);
  digitalWrite(SHOW_SYNC, LOW);
}

//////////////////////////////////////////////////////////////////////////////////////////
void gw_rec( unsigned char* mac, const Node* pn)
{
  if (pn->node && pn->is_paired() == true)
  {
    //uint32_t token = pn->randgen & Password;
    if (1)
    {
      Serial.print("GOT MAC FROM PEER 2");
      print_mac(mac);
      esp_now_del_peer(mac);
      int err = esp_now_add_peer((unsigned char*)mac, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
      if (err != 0)
      {
        Serial.print("Failed to add peer: ");
        print_mac(mac);
        return;
      }
      // send our mac
      ThisNode.tick = gt();
      ThisNode.randgen = millis();
      //ThisNode.token  = ThisNode.randgen & Password;
      int ret = esp_now_send(mac, (uint8_t *) &ThisNode, sizeof(ThisNode));
      if (0 == ret) {
        add_node(mac, pn);
        Serial.print("REGISTERRED "); print_mac(mac);
        Dirty = true;
      }
    }
    else
    {
      Serial.print("TOKEN ERROR "); print_mac(mac);
    }
  }
  else
  {
    if (!has_node(mac))
      Serial.print("GOT MESSAGE FROM UNREGISTERRED NODE");
    else
      Serial.print("GOT MESSAGE FROM NODE");
    print_mac(mac);
  }

}

//////////////////////////////////////////////////////////////////////////////////////////
void node_rec(unsigned char* mac, const Node* pn)
{
  if (pn && pn->node == false)                            // from gw
  {
    if(pn->started)ThisNode.pairedto[0]=0;
    if (ThisNode.pairedto[0] == 0 && pn->beacon)          // got GW beacon
    {
      Serial.print("GOT GATEWAY MAC ");
      print_mac(mac);

      // add peer
      esp_now_del_peer(mac);
      delay(64);
      int err = esp_now_add_peer((unsigned char*)mac, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
      if (err != 0)
      {
        Serial.print("Failed to add GW: ");
        print_mac(mac);
        return;
      }
      memcpy(ThisNode.pairedto, mac, sizeof(ThisNode.pairedto));
      int ret = esp_now_send(mac, (uint8_t *) &ThisNode, sizeof(ThisNode));
      if (0 == ret) {
        Serial.print("JOINING "); print_mac(mac);
      }
      else
      {
        Serial.print("NOT JOIN "); print_mac(mac);
        memset(ThisNode.pairedto, 0, sizeof(ThisNode.pairedto));
      }

    }
    else if (pn->sync == true && ThisNode.is_paired())
    {
      uint64_t micross = gt();
      uint64_t abst = micross-Micros;
      Micros = micross;
      Serial.printf("GOT TIME_SYNC %llu: ", abst);
      print_mac(mac);
    }
  }
}

//////////////////////////////////////////////////////////////////////////////////////////
void gw_send(unsigned char* mac, uint8_t status)
{

}

//////////////////////////////////////////////////////////////////////////////////////////
void node_send(unsigned char* mac, uint8_t status)
{

}

//////////////////////////////////////////////////////////////////////////////////////////
void add_node(unsigned char* mac, const Node* pn)
{
  if (NodesCount == 0) {
    ::memcpy(&Nodes[0], pn, sizeof(Node));
    ++NodesCount;
    Serial.print("NODE ADDED "); print_mac(mac);
    return;
  }
  if (NodesCount < MAX_NODES)
  {
    for (int n = 0; n < NodesCount; n++)
    {
      if (!::memcmp(Nodes[n].mac, mac, sizeof(Node)))
      {
        Serial.print("MAC already registered"); print_mac(mac);
        return;
      }
    }
    ::memcpy(&Nodes[NodesCount], pn, sizeof(Node));
    ++NodesCount;
    Serial.print("NODE ADDED "); print_mac(mac);
  }
}
