#include <ELClientWebServer.h>
#if 0
#define LED_PIN  13
typedef enum {
  UNKNOWN = 0,
  SOUND,
  LIGHT,
} flash_mode_t;

typedef struct {
  flash_mode_t mode;
  int initial_delay_ms;
  int repeat_count;
  int repeat_delay_ms;
} flash_config_t;

flash_config_t flash_config;

void init_flash_config()
{
  memset(&flash_config, 0, sizeof(flash_config));
  flash_config.initial_delay_ms = 10;
}

void flashLoop()
{ 
}

// called at button pressing
void flashButtonPressCb(char * button)
{ 
 digitalWrite(LED_PIN, !digitalRead(LED_PIN)); // blink LED
  String btn = button;
  if( btn == F("btn_light") )
    flash_config.mode = LIGHT;
  else if( btn == F("btn_sound") )
    flash_config.mode = SOUND;
}

// setting the value of a field
//
// handle data as fast as possible
// - huge HTML forms can arrive in multiple packets
// - if this method is slow, UART receive buffer may overrun
void flashSetFieldCb(const char * field)
{
  digitalWrite(LED_PIN, !digitalRead(LED_PIN)); // blink LED
  String fld = field;
  if( fld == F("initial_delay_ms") )
    flash_config.initial_delay_ms = webServer.getArgInt();
  else if( fld == F("repeat_count") )
    flash_config.repeat_count = webServer.getArgInt();
  else if( fld == F("repeat_count") )
    flash_config.repeat_delay_ms = webServer.getArgInt();
}

// called at page refreshing
void flashRefreshCb(char * url)
{
  switch (flash_config.mode)
    {
    case SOUND: webServer.setArgString(F("text"), F("Sound Trigger Mode"));       break;
    case LIGHT: webServer.setArgString(F("text"), F("Light Trigger Mode"));       break;
    default   : webServer.setArgString(F("text"), F("Unknown mode., error!"));    break;
    }
  // webServer.setArgJson(F("led_history"), log.begin());
}

// called at page loading
void flashLoadCb(char * url)
{
  digitalWrite(LED_PIN, !digitalRead(LED_PIN)); // blink LED
  webServer.setArgInt(F("initial_delay_ms"), flash_config.initial_delay_ms);
  webServer.setArgInt(F("repeat_delay_ms"),  flash_config.repeat_delay_ms);
  webServer.setArgInt(F("repeat_count"),     flash_config.repeat_count);
  flashRefreshCb( url );
}

// FLASH setup code
void flashInit()
{
  URLHandler *flashHandler = webServer.createURLHandler(F("/FlashConfig.html.json"));
  flashHandler->buttonCb.attach(flashButtonPressCb);
  flashHandler->setFieldCb.attach(flashSetFieldCb);
  flashHandler->loadCb.attach(flashLoadCb);
  flashHandler->refreshCb.attach(flashRefreshCb);
}

#endif
