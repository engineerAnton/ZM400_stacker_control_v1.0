#include <functions.h>
#include <A4988.h>
#include <global.h>
#include <singleLEDLibrary.h>
#include <MillisTimer.h>

// Паттерны мигания красным светодиодом и сигналов бузера
int long_1_ptrn [] = {500, 1000};
int long_1_arr = 2;
int long_2_ptrn [] = {500, 200, 500, 1000};
int long_2_arr = 4;
int long_3_ptrn [] = {500, 200, 500, 200, 500, 1000};
int long_3_arr = 6;
int long_4_ptrn [] = {500, 200, 500, 200, 500, 200, 500, 1000};
int long_4_arr = 8;

sllib led_r(led_red);
sllib buzz(buzzer);

// Создание объектов "Приводы"
A4988 mot_CR(mot_CR_steps, mot_CR_dir, mot_CR_step);
A4988 mot_FR(mot_FR_steps, mot_FR_dir, mot_FR_step);
A4988 mot_BD(mot_BD_steps, mot_BD_dir, mot_BD_step);

void setup() {
  Serial.begin(9600);
  Serial2.begin(9600);
  pinMode(cutter_output, INPUT);
  pinMode(mot_CR_en, OUTPUT);
  pinMode(mot_FR_en, OUTPUT);
  pinMode(mot_BD_en, OUTPUT);
  pinMode(coil, OUTPUT);
  pinMode(led_green, OUTPUT);
  pinMode(led_red, OUTPUT);
  pinMode(butt_res, INPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(sw_cont, INPUT);
  pinMode(sw_bed_high, INPUT);
  pinMode(sw_bed_low, INPUT);
  pinMode(opt_label, INPUT);
  pinMode(opt_carr, INPUT);

  digitalWrite(mot_CR_en, LOW);
  digitalWrite(mot_FR_en, LOW);
  digitalWrite(mot_BD_en, LOW);

  mot_CR.begin(200, 1);
  mot_FR.begin(200, 1);
  mot_BD.begin(200, 1);
}

MillisTimer carr_tim = MillisTimer(2000);

bool check_all(){
    if (digitalRead(sw_cont) == true && digitalRead(sw_bed_high) == true && digitalRead(sw_bed_low) == false && digitalRead(opt_label) == false && digitalRead(opt_carr) == true){
        buzz.patternSingle(long_1_ptrn, long_1_arr);
        return true;
    }else{
        buzz.setOffSingle();
        return false;
    }
};

bool cont_init(){
    if (digitalRead(sw_cont) == true){
        led_r.setOffSingle();
        buzz.setOffSingle();
        return true;   
    }
    if (digitalRead(sw_cont) == false){
        led_r.patternSingle(long_1_ptrn, long_1_arr);
        buzz.patternSingle(long_1_ptrn, long_1_arr);
        return false;
    }
};

bool feeder_init(){
    if (digitalRead(opt_label) == true){
        led_r.patternSingle(long_2_ptrn, long_2_arr);
        buzz.patternSingle(long_2_ptrn, long_2_arr);
        return false;
    }
    if (digitalRead(opt_label) == false){
        led_r.setOffSingle();
        buzz.setOffSingle();
        return true;
    }
};

bool carr_init(){
    if (digitalRead(opt_carr) == false){
        uint32_t now = millis();
        while (millis() - now < 1000){            
            mot_CR.startMove(1);
            mot_CR.nextAction();
            led_r.patternSingle(long_3_ptrn, long_3_arr);
            buzz.patternSingle(long_3_ptrn, long_3_arr);
            if (digitalRead(opt_carr) == true){
                mot_CR.stop();
                led_r.setOffSingle();
                buzz.setOffSingle();
                return true;
            }
        }        
        return false;
    }
    if (digitalRead(opt_carr) == true){
        return true;
    }
};

bool bed_init(){
    if (digitalRead(sw_bed_high) == false){
        uint32_t now = millis();
        while (millis() - now < 1000){            
            mot_BD.startMove(1);
            mot_BD.nextAction();
            led_r.patternSingle(long_4_ptrn, long_4_arr);
            buzz.patternSingle(long_4_ptrn, long_4_arr);
            if (digitalRead(sw_bed_high) == true){
                mot_BD.stop();
                led_r.setOffSingle();
                buzz.setOffSingle();
                return true;
            }
        }        
        return false;
    }
    if (digitalRead(sw_bed_high) == true){
        return true;
    }
};
