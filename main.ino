#include <Arduino.h>

#include <RPC.h>

#ifndef ARDUINO_PORTENTA_H7_M4
#define ARDUINO_PORTENTA_H7_M4
#endif

#ifndef ARDUINO_ARCH_MBED
#define ARDUINO_ARCH_MBED
#endif

#include <Portenta_H7_TimerInterrupt.h>
#include <Portenta_H7_ISR_Timer.h>

#define FORWARD_LEFT  PC_8
#define FORWARD_RIGHT PA_11

#define REVERSE_LEFT  PA_10
#define REVERSE_RIGHT PA_9

#define PWM_LEFT PA_13
#define PWM_RIGHT PA_14

#define ENCODER_LEFT  PE_11
#define ENCODER_RIGHT PE_12

static enum {
  DEFAULT = 0,
  DRIVING = 1
}state;

volatile int pulses_left = 0;
volatile int pulses_right = 0;

volatile float speed_left = 0.0f;
volatile float speed_right = 0.0f;

volatile int prev_pulses_left = 0;
volatile int prev_pulses_right = 0;

static int direction_left = 1;
static int direction_right = 1;

static int set_speed_left = 0;
static int set_speed_right = 0;

Portenta_H7_Timer ITimer(TIM15);
Portenta_H7_ISR_Timer ISR_Timer;

#define HW_TIMER_INTERVAL_US 100L
#define SPEED_UPDATE_TIME 500L // ms

void TimerHandler(void){
  ISR_Timer.run();
}

void encoder_counter_callback(){
  speed_left = (pulses_left - prev_pulses_left) >> 1;
  speed_right = (pulses_right - prev_pulses_right) >> 1; // PULSES PER SECOND
}

void encoder_pulse_right() {
  pulses_right += 1;
}

void encoder_pulse_left() {
  pulses_left += 1;
}
  
void setup() {
  /*
  pinMode(PA_11, OUTPUT); // FORWARD RIGHT
  pinMode(PC_8, OUTPUT); // FORWARD RIGHT

  pinMode(PA_10, OUTPUT); // REVERSE LEFT
  pinMode(PA_9, OUTPUT);  // REVERSE RIGHT

  pinMode(PE_11, OUTPUT);  // PULSE LEFT
  pinMode(PE_12, OUTPUT);  // PULSE RIGHT

  pinMode(PF_0)
  */

  Serial.begin(115200);
  while(!Serial){

  }

  RPC.begin();

  delay(500);

  RPC.bind("speed_left", []{ return speed_left; });
  RPC.bind("speed_right", []{ return speed_right; });
  

  pinMode(LED_BUILTIN, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(ENCODER_LEFT), encoder_pulse_left, RISING);
  attachInterrupt(digitalPinToInterrupt(ENCODER_RIGHT), encoder_pulse_right, RISING);

  // Interval in microsecs
  if (!ITimer.attachInterruptInterval(HW_TIMER_INTERVAL_US, TimerHandler)){
    Serial.println("Can't set ITimer correctly. Select another freq. or interval");
  }
  // Just to demonstrate, don't use too many ISR Timers if not absolutely necessary
  // You can use up to 16 timer for each ISR_Timer
  ISR_Timer.setInterval(SPEED_UPDATE_TIME, encoder_counter_callback);
}

void loop(){
  switch (state) {
    case DEFAULT:
      digitalWrite(FORWARD_LEFT, 0);
      digitalWrite(REVERSE_LEFT, 0);

      digitalWrite(FORWARD_RIGHT, 0);
      digitalWrite(FORWARD_RIGHT, 0);

      digitalWrite(PWM_LEFT, 1);
      digitalWrite(PWM_RIGHT, 1);

      // Maybe do standby low to set it to standby

      break;
      
    case DRIVING:
      // Left wheel
      digitalWrite(FORWARD_LEFT, direction_left);
      digitalWrite(REVERSE_LEFT, -1*direction_left);

      analogWrite(PWM_LEFT, set_speed_left);

      // Right wheel
      digitalWrite(FORWARD_RIGHT, direction_right);
      digitalWrite(REVERSE_RIGHT, -1*direction_right);

      analogWrite(PWM_RIGHT, set_speed_right);

      break;

    default:
      break;
  }
  delay(100);
}

