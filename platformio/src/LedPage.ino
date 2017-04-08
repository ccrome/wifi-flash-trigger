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
void flashSetFieldCb(char * field)
{
  digitalWrite(LED_PIN, !digitalRead(LED_PIN)); // blink LED
  String fld = field;
  if( fld == F("initial_delay_ms") )
    // flash_config.initial_delay_ms = webServer.getArgString();
    ; 
  else if( fld == F("repeat_count") )
    flash_config.repeat_count = webServer.getArgInt();
  else if( fld == F("repeat_count") )
    flash_config.repeat_delay_ms = webServer.getArgFloat();
}

// called at page refreshing
void flashRefreshCb(char * url)
{
}

// called at page loading
void flashLoadCb(char * url)
{
  digitalWrite(LED_PIN, !digitalRead(LED_PIN)); // blink LED
  // webServer.setArgString(F("initial_delay_ms"), flash_config.initial_delay_ms);
  // webServer.setArgFloat(F("repeat_delay_ms"),  flash_config.repeat_delay_ms);
  // webServer.setArgInt(F("repeat_count"),     flash_config.repeat_count);
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


// the PIN to flash
#define LED_PIN  13

int8_t   blinking = 0;            // whether LED is blinking
uint8_t  blinking_duty = 2;       // blinking duty
int8_t   blinking_frequency = 10; // blinking frequency

uint32_t blinking_next_ts = 0;    // the next timestamp to blink
uint16_t blinking_phase   = 100;  // blinking phase
uint16_t blinking_period  = 200;  // blinking period (2000 / frequency)

#define MAX_LOGS 5
uint32_t log_ts[MAX_LOGS];        // log timestamp
uint8_t  log_msg[MAX_LOGS];       // log message
uint8_t  log_ptr = 0;             // log pointer

typedef enum
{
  LOG_DUTY_25_75 = 0xE1,
  LOG_DUTY_50_50,
  LOG_DUTY_75_25,
  LOG_SET_LED_ON = 0xF0,
  LOG_SET_LED_BLINKING,
  LOG_SET_LED_OFF,
} LogMessage;

// LED loop code
void ledLoop()
{ 
  if( blinking ) // if blinking is enabled
  {
    if( blinking_next_ts <= millis() )
    {
      digitalWrite(LED_PIN, !digitalRead(LED_PIN)); // blink LED
      blinking_next_ts += blinking_phase;
      blinking_phase = blinking_period - blinking_phase;
    }
  }
}

// adds a new log message
void ledAddLog(uint8_t msg)
{
  // check if max log is reached
  if( log_ptr >= MAX_LOGS )
    log_ptr = MAX_LOGS - 1;

  // move logs back with one and delete the oldest log
  for(int8_t i=log_ptr-1; i >= 0; i--)
  {
    log_ts[i+1] = log_ts[i];
    log_msg[i+1] = log_msg[i];
  }
  
  log_msg[0] = msg;     // log message
  log_ts[0] = millis(); // log timestamp
  log_ptr++;            // a new log was added
}

// create log messages to string
String ledHistoryToLog()
{
  String str = "[";

  for(uint8_t i=0; i < log_ptr; i++)
  {
    if( i != 0 )
      str += ',';

    str += '\"';
    str += (log_ts[i] / 1000);
    str += "s: ";

    switch(log_msg[i]) // log message
    {
      case LOG_DUTY_25_75:
        str += F("set duty to 25%-75%");
        break;
      case LOG_DUTY_50_50:
        str += F("set duty to 50%-50%");
        break;
      case LOG_DUTY_75_25:
        str += F("set duty to 75%-25%");
        break;
      case LOG_SET_LED_ON:
        str += F("set led on");
        break;
      case LOG_SET_LED_BLINKING:
        str += F("set led blinking");
        break;
      case LOG_SET_LED_OFF:
        str += F("set led off");
        break;
      default:
        str += F("set frequency to ");
        str += (int)log_msg[i];
        str += F(" Hz");
        break;
    }
    str += '\"';
  }
  str += ']';
  
  return str;
}

// called at button pressing
void ledButtonPressCb(char * button)
{
  String btn = button;
  if( btn == F("btn_light") )
    flash_config.mode = LIGHT;
  else if( btn == F("btn_sound") )
    flash_config.mode = SOUND;
  else
    flash_config.mode = UNKNOWN;
}

// setting the value of a field
//
// handle data as fast as possible
// - huge HTML forms can arrive in multiple packets
// - if this method is slow, UART receive buffer may overrun

int setCount = 0;
int refreshCount = 0;
void ledSetFieldCb(char * field)
{
  setCount ++;
  String fld = field;
  if( fld == F("frequency") )
  {
    int8_t oldf = blinking_frequency;
    blinking_frequency = webServer.getArgInt();

    blinking_period = 2000 / blinking_frequency;
    blinking_phase = blinking_duty * blinking_period / 4;
  
    if( oldf != blinking_frequency )
    {
      ledAddLog(blinking_frequency);
      if( blinking )
        digitalWrite(LED_PIN, false);
    }
  }
  else if( fld == F("frequency_x") )
  {
    int8_t oldf = blinking_frequency;
    blinking_frequency = webServer.getArgInt();

    blinking_period = 2000 / blinking_frequency;
    blinking_phase = blinking_duty * blinking_period / 4;
  
    if( oldf != blinking_frequency )
    {
      ledAddLog(blinking_frequency);
      if( blinking )
        digitalWrite(LED_PIN, false);
    }
  }
  else if( fld == F("duty") )
  {
    int8_t oldp = blinking_duty;
    String arg = webServer.getArgString();

    if( arg == F("25_75") )
      blinking_duty = 1;
    else if( arg == F("50_50") )
      blinking_duty = 2;
    else if( arg == F("75_25") )
      blinking_duty = 3;

    if( blinking )
      digitalWrite(LED_PIN, false);

    blinking_phase = blinking_duty * blinking_period / 4;

    if( oldp != blinking_duty )
      ledAddLog(LOG_DUTY_25_75 - 1 + blinking_duty);
  }
//  else if( fld == F("initial_delay_ms") )
//    flash_config.initial_delay_ms = webServer.getArgFloat();
  else if( fld == F("repeat_count") )
    flash_config.repeat_count = webServer.getArgInt();
  else if( fld == F("repeat_count") )
    flash_config.repeat_delay_ms = webServer.getArgFloat();
}

// called at page refreshing
void ledRefreshCb(char * url)
{
  refreshCount++;
  String status = "Status: (";
  status += setCount;
  status += ", ";
  status += refreshCount;
  status += ")";
  status += "<br>";
  switch (flash_config.mode)
    {
    case SOUND: status += "Sound Trigger Mode";    break;
    case LIGHT: status += "Light Trigger Mode";    break;
    default   : status += "Unknown mode., error!"; break;
    }
  status += "</br>";
  status += "<br>initial_delay = " + String(flash_config.initial_delay_ms); 
  status += "<br>repeat_count = " + String(flash_config.repeat_count); 
  status += "<br>repeat_delay_ms = " + String(flash_config.repeat_delay_ms);
  webServer.setArgString(F("text"), status.c_str());
}

// called at page loading
void ledLoadCb(char * url)
{
  webServer.setArgInt(F("frequency"), blinking_frequency);
  webServer.setArgInt(F("frequency_x"), (int)flash_config.initial_delay_ms+15);
  // webServer.setArgString(F("initial_delay_ms"), flash_config.initial_delay_ms);
  ledRefreshCb( url );
}

// LED setup code
void ledInit()
{
  // set mode to output and turn LED off
  init_flash_config();
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, false);
  
  URLHandler *ledHandler = webServer.createURLHandler(F("/LED.html.json"));
  ledHandler->buttonCb.attach(ledButtonPressCb);
  ledHandler->setFieldCb.attach(ledSetFieldCb);
  ledHandler->loadCb.attach(ledLoadCb);
  ledHandler->refreshCb.attach(ledRefreshCb);
}

