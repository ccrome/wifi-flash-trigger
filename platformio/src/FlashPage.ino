#include <ELClientWebServer.h>
#define LED_PIN  13
typedef enum {
    UNKNOWN = 0,
    SOUND,
    LIGHT,
} flash_mode_t;

typedef struct {
    flash_mode_t mode;
    float initial_delay_ms;
    int repeat_count;
    float repeat_delay_ms;
} flash_config_t;

flash_config_t flash_config;

void init_flash_config()
{
    memset(&flash_config, 0, sizeof(flash_config));
    flash_config.initial_delay_ms = 10.0;
    flash_config.repeat_delay_ms = 3.0;
    flash_config.repeat_count = 1;
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
void flashSetFieldCb(char * field)
{
    digitalWrite(LED_PIN, !digitalRead(LED_PIN)); // blink LED
    String fld = field;
    if( fld == F("initial_delay_ms") )
        flash_config.initial_delay_ms = webServer.getArgFloat();
    else if( fld == F("repeat_count") )
        flash_config.repeat_count = webServer.getArgInt();
    else if( fld == F("repeat_delay_ms") )
        flash_config.repeat_delay_ms = webServer.getArgFloat();
}

// called at page refreshing
void flashRefreshCb(char * url)
{
    String s;
    switch (flash_config.mode)
    {
    case SOUND: s += "Sound Trigger Mode<br>"     ; break;
    case LIGHT: s += "Light Trigger Mode<br>"     ; break;
    default   : s += "Unknown Trigger Mode<br>"   ; break;
    }
    s+= "Initial Delay: " + String(flash_config.initial_delay_ms) + "ms<br>";
    s+= "Repeat Count: " + String(flash_config.repeat_count) + "<br>";
    s+= "Repeat Delay: " + String(flash_config.repeat_delay_ms) + "ms<br>";
    webServer.setArgString(F("text"), s.c_str());
}

// called at page loading
void flashLoadCb(char * url)
{
    digitalWrite(LED_PIN, !digitalRead(LED_PIN)); // blink LED
    webServer.setArgFloat(F("initial_delay_ms"), flash_config.initial_delay_ms);
    webServer.setArgInt(F("repeat_delay_ms"),  flash_config.repeat_delay_ms);
    webServer.setArgFloat(F("repeat_count"),     flash_config.repeat_count);
    flashRefreshCb( url );
}

// FLASH setup code
void flashInit()
{
    init_flash_config();
    URLHandler *flashHandler = webServer.createURLHandler(F("/FlashConfig.html.json"));
    flashHandler->buttonCb.attach(flashButtonPressCb);
    flashHandler->setFieldCb.attach(flashSetFieldCb);
    flashHandler->loadCb.attach(flashLoadCb);
    flashHandler->refreshCb.attach(flashRefreshCb);
}

