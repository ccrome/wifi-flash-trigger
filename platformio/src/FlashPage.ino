#include <ELClientWebServer.h>
// Use Timer 1 (16-bit) for input capture & output compare for high precision timing.
#define CLK_FREQUENCY (8000000.0)
// valid values are 1, 8, 64, 256, 1024
#define TIMER1_PRESCALE (1024)
#define TIMER1_FREQUENCY (CLK_FREQUENCY/TIMER1_PRESCALE)

#define INPUT_CAPTURE_PIN 8
#define FLASH_OUTPUT_PIN 7
#define LED_PIN 13
//                                                                  +---------------------------------------------------------+
// Timer state machine:                                             V                                                         | yes
// IDLE ---->  WAITING_FOR_TRIGGER ----> INITIAL_DELAY -----> HOLD_TIME -> REPEAT_TIME_MINUS_HOLD ----> MORE FLASHES TO GO? ---
//                    ^                                                                                                       | no
//                    |------------- EXTRA_DELAY <----------------------------------------------------------------------------+
//
// start trigger whenever entering HOLD_TIME
// stop trigger whenever exiting HOLD_TIME

typedef enum
{
    IDLE = 0,
    WAITING_FOR_TRIGGER = 1,
    INITIAL_DELAY = 2, 
    HOLD_TIME = 3,
    REPEAT_TIME_MINUS_HOLD = 4,
    EXTRA_DELAY = 5,
} flash_sm_t;

typedef enum {
    UNKNOWN = 0,
    SOUND,
    LIGHT,
} flash_mode_t;

typedef struct {
    flash_mode_t mode;
    int extra_delay_ms; // minimum waiting time between triggers
    float initial_delay_ms;
    int repeat_count;
    float repeat_delay_ms;
    float hold_time_ms;
    flash_sm_t flash_sm;
    int repeats_to_go;
    int flash_output_pin;
    unsigned long triggered_at;
    int trigger_display_flag;
} flash_config_t;

flash_config_t flash_config;

void timer1_config(int e);
void flashRefreshCb(char * url);

void init_flash_config()
{
    memset(&flash_config, 0, sizeof(flash_config));
    flash_config.initial_delay_ms = 5.0;
    flash_config.repeat_delay_ms = 3.0;
    flash_config.repeat_count = 4;
    flash_config.hold_time_ms = .25; // on full power, speedlites can be up to 4ms...
    flash_config.extra_delay_ms = 200.0;
    flash_config.flash_output_pin = FLASH_OUTPUT_PIN ;
    flash_config.mode = SOUND;
}

void flashLoop()
{ 
    if (((millis() - flash_config.triggered_at) > 5000) &&
        flash_config.trigger_display_flag)
    {
        flash_config.trigger_display_flag = 0;
        flashRefreshCb(NULL);
    }
}

// called at button pressing
void flashButtonPressCb(char * button)
{ 
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
    String fld = field;
    if( fld == F("initial_delay_ms") )
        flash_config.initial_delay_ms = webServer.getArgFloat();
    else if( fld == F("repeat_count") )
        flash_config.repeat_count = webServer.getArgInt();
    else if( fld == F("repeat_delay_ms") )
        flash_config.repeat_delay_ms = webServer.getArgFloat();
    else if (fld == F("flash_mode") ) {
        String mode = webServer.getArgString();
        flash_config.mode = mode == F("light") ? LIGHT : SOUND;
    }
        
        
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
    if (flash_config.trigger_display_flag)
        s += "<H1>Triggered!</H1>";
    s+= "Initial Delay: " + String(flash_config.initial_delay_ms) + "ms<br>";
    s+= "Repeat Count: " + String(flash_config.repeat_count) + "<br>";
    s+= "Repeat Delay: " + String(flash_config.repeat_delay_ms) + "ms<br>";
    webServer.setArgString(F("text"), s.c_str());
}

// called at page loading
void flashLoadCb(char * url)
{
    webServer.setArgFloat(F("initial_delay_ms"), flash_config.initial_delay_ms);
    webServer.setArgInt(F("repeat_delay_ms"),  flash_config.repeat_delay_ms);
    webServer.setArgFloat(F("repeat_count"),     flash_config.repeat_count);
    webServer.setArgString(F("flash_mode"), flash_config.mode == SOUND ? F("sound") : F("light"));
    flashRefreshCb( url );
}

void enable_output_compare()
{
    TIFR1 = 0xff; // clear pending interrupts
    // TIMSK1 |= (1<<1);
    TIMSK1 = (1<<1);
}
void disable_output_compare()
{
    // TIMSK1 &= ~(1<<1); // disable output compare
    TIMSK1 = 0;
    TIFR1 = 0xff;         // and it's flag
}

void enable_input_capture()
{
    TIFR1 = 0xff; // clear pending interrupts
    // TIMSK1 |= (1<<5); // enable input capture
    TIMSK1 = (1<<5); // enable input capture
}

void disable_input_capture()
{
    // TIMSK1 &= ~(1<<5); // disable input capture
    TIMSK1 = 0;
    TIFR1 = 0xff; // clear input capture flag
    
}

void flash_sm_init()
{
    digitalWrite(12, 0);
    digitalWrite(A0, 0);
    digitalWrite(A1, 0);

    flash_trigger_off();
    flash_config.repeats_to_go = flash_config.repeat_count;
    flash_config.flash_sm = WAITING_FOR_TRIGGER;
    timer1_config(1);
    disable_output_compare();
    enable_input_capture();
}

// FLASH setup code
void flashInit()
{
    pinMode(12, OUTPUT);
    pinMode(A0, OUTPUT);
    pinMode(A1, OUTPUT);
    digitalWrite(12, 0);
    digitalWrite(A0, 0);
    digitalWrite(A1, 0);
    init_flash_config();
    pinMode(FLASH_OUTPUT_PIN, OUTPUT);
    pinMode(INPUT_CAPTURE_PIN, INPUT);
    pinMode(LED_PIN,          OUTPUT);
    URLHandler *flashHandler = webServer.createURLHandler(F("/FlashConfig.html.json"));
    flashHandler->buttonCb.attach(flashButtonPressCb);
    flashHandler->setFieldCb.attach(flashSetFieldCb);
    flashHandler->loadCb.attach(flashLoadCb);
    flashHandler->refreshCb.attach(flashRefreshCb);
    flash_sm_init();
}

void report_error(int error_code)
{
    while(1) {
        for (int i = 0; i < error_code; i++) {
            digitalWrite(LED_PIN, !digitalRead(LED_PIN));
            delay(250);
            digitalWrite(LED_PIN, !digitalRead(LED_PIN));
            delay(250);
        }
        delay(3000);
    }
}

void flash_trigger_off()
{
    digitalWrite(flash_config.flash_output_pin, 0);
}

void flash_trigger_on()
{
    digitalWrite(flash_config.flash_output_pin, 1);
}

unsigned int ms_2_counts(float milliseconds)
{
    return round(milliseconds / 1000.0 * TIMER1_FREQUENCY);   // ms * s/ms * counts/second  = counts
}

// interrupt service routines
ISR(TIMER1_COMPA_vect)
{
    digitalWrite(A0, 1);
    digitalWrite(A1, 1);
    int set_trigger = 0;
    unsigned int trigger_time =  TCNT1;
    switch (flash_config.flash_sm) {
    case IDLE:
        // This is an error
        report_error(1);
        break;
    case WAITING_FOR_TRIGGER:
        report_error(2);
        break;
    case INITIAL_DELAY:
        // initial delay is complete. Now go to hold time
        flash_config.flash_sm = HOLD_TIME;
        trigger_time += ms_2_counts(flash_config.hold_time_ms);
        set_trigger = 1;
        break;
    case HOLD_TIME:
        // Hold time is complete.  delay for repeat time minus hold time
        flash_config.repeats_to_go --;
        flash_config.flash_sm = REPEAT_TIME_MINUS_HOLD;
        trigger_time += ms_2_counts(flash_config.repeat_delay_ms - flash_config.hold_time_ms);
        set_trigger = 1;
        break;
    case REPEAT_TIME_MINUS_HOLD:
        // repeat time is complete.. either quit or go back to hold time
        if ((flash_config.repeats_to_go) > 0)
        {
            flash_config.flash_sm = HOLD_TIME;
            trigger_time += ms_2_counts(flash_config.hold_time_ms);
        } else {
            flash_config.flash_sm = EXTRA_DELAY;
            trigger_time += ms_2_counts(flash_config.extra_delay_ms);
        }
        set_trigger = 1;
        break;
    case EXTRA_DELAY:
        // extra delay is complete
        flash_sm_init();
        break;
    }
    if (flash_config.flash_sm != WAITING_FOR_TRIGGER) {
        if (set_trigger) {
            OCR1A = trigger_time;
        }
        if (flash_config.flash_sm == HOLD_TIME)
            flash_trigger_on();
        else
            flash_trigger_off();
    }
    digitalWrite(A1, 0);
}
ISR(TIMER1_COMPB_vect)
{
    report_error(6);
}
ISR(TIMER1_CAPT_vect)
{
    disable_input_capture();
    flash_config.flash_sm = INITIAL_DELAY;
    flash_config.repeats_to_go = flash_config.repeat_count;
    OCR1A = ICR1 + ms_2_counts(flash_config.initial_delay_ms);
    digitalWrite(LED_PIN,1); // blink LED
    enable_output_compare();
    flash_config.triggered_at = millis();
    flash_config.trigger_display_flag = 1;
}
ISR(TIMER1_OVF_vect)
{
    report_error(7);
}

void timer1_config(int edge)
{
    if (edge)
        edge = 1;
    else
        edge = 0;


    // clock source 2:0;
    int cs1 = 0;
    switch (TIMER1_PRESCALE) {
    case 1   : cs1 = 1; break;
    case 8   : cs1 = 2; break;
    case 64  : cs1 = 3; break;
    case 256 : cs1 = 4; break;
    case 1024: cs1 = 5; break;
    default:  report_error(10);break;
    }

    int WGM = 0;
    // set up the timer 1 clocking & interrupt handlers
    TCCR1A = 0;
    TCCR1B = 0;
    TCCR1C = 0;
    TIMSK1 = 0;
    TIFR1  = 0xff;
    TCNT1  = 0;
    TCCR1A =
        ((WGM & 0x3) << 0)  // WGM[1:0]
        ;
    TCCR1B =
        (1    << 7)  |              // TURN on noise canceller
        (edge << 6)  |              // set trigger edge/
        (((WGM >> 2) & 0x3) << 3) | // WGM[3:2]
        (cs1 << 0)                  // CS1
        ;
}

