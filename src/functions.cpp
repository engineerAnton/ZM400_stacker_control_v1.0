#include <functions.h>
#include <A4988.h>
#include <global.h>
#include <singleLEDLibrary.h>
#include <MillisTimer.h>

// Паттерны для светодиодов и буззера
int long_1_ptrn [] = {500, 1000};
int long_1_arr = 2;
int long_2_ptrn [] = {500, 200, 500, 1000};
int long_2_arr = 4;
int long_3_ptrn [] = {500, 200, 500, 200, 500, 1000};
int long_3_arr = 6;
int long_4_ptrn [] = {500, 200, 500, 200, 500, 200, 500, 1000};
int long_4_arr = 8;

int short_2_ptrn [] = {200, 100, 200, 2000};
int short_2_arr = 4;
int short_3_ptrn [] = {200, 100, 200, 100, 200, 2000};
int short_3_arr = 6;
int short_4_ptrn [] = {200, 100, 200, 100, 200, 100, 200, 2000};
int short_4_arr = 8;

sllib led_r(led_red);
sllib led_g(led_green);
sllib buzz(buzzer);

// Приводы
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
  Serial.println("Power on, initialization start...");
}

MillisTimer carr_tim = MillisTimer(2000);

// Проверка состояния всех датчиков
bool check_all(){
    if (digitalRead(sw_cont) == true && digitalRead(sw_bed_high) == true && digitalRead(sw_bed_low) == false && digitalRead(opt_label) == false && digitalRead(opt_carr) == true){
        return true;
    }else{        
        return false;
    }
};

// Инициализация контейнера
bool cont_init(){
    if (digitalRead(sw_cont) == true){
        led_r.setOffSingle();
        buzz.setOffSingle();
        return true;   
    }else{
        led_r.patternSingle(long_1_ptrn, long_1_arr);
        buzz.patternSingle(long_1_ptrn, long_1_arr);
        return false;
    }
};

// Инициализация блока протяжки
bool feeder_init(){
    if (digitalRead(opt_label) == true){
        led_r.patternSingle(long_2_ptrn, long_2_arr);
        buzz.patternSingle(long_2_ptrn, long_2_arr);
        return false;
    }else{
        led_r.setOffSingle();
        buzz.setOffSingle();
        return true;
    }
};

// Инициализация механизма каретки
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
    }else{
        return true;
    }
};

// Движение каретки на один оборот
bool carr_move(){
    bool flag = 0; 
    uint32_t now = millis();
    while ((millis() - now < 2000)){        
        if (digitalRead(opt_carr) == true && flag == 0){
            led_g_on();
            mot_CR.startMove(1);
            mot_CR.nextAction();
        }
        if (digitalRead(opt_carr) == false){
            flag = 1;
            mot_CR.startMove(1);
            mot_CR.nextAction();
        }
        if (digitalRead(opt_carr) == true && flag == 1){
            mot_CR.stop();
            led_g_off();
            return true;
        }
    }
    return false;
}    

// Инициализация стола
bool bed_init(){
    if (digitalRead(sw_bed_high) == false){
        uint32_t now = millis();
        while (millis() - now < 15000){            
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
    }else{
        return true;
    }
};

// Движение стола на одну позицию
void bed_down(){
    mot_BD.rotate(-15);
}

// Опустить блок протяжки
void feeder_down(){
    digitalWrite(coil, HIGH);
}

// Поднять блок протяжки
void feeder_up(){
    digitalWrite(coil, LOW);
}

// Движение ролика протяжки
bool feeder_move(){    
    uint32_t now = millis();
    while (millis() - now < 1000){
        mot_FR.rotate(500);
        if (digitalRead(opt_label) == false){
            return true;
        }
    }        
    return false;
};

// Включить зелёный светодиод
void led_g_on(){
    digitalWrite(led_green, HIGH);
}

// Выключить зелёный светодиод
void led_g_off(){
    digitalWrite(led_green, LOW);
}

// Извлечение контейнера
bool cont_remove(){
    led_g.patternSingle(short_4_ptrn, short_4_arr);
    buzz.patternSingle(short_4_ptrn, short_4_arr);
    if (digitalRead(sw_cont) == false){
        led_g.setOffSingle();
        buzz.setOffSingle();
        return true;
    }
    return false;
}

// Установка контейнера
bool cont_insert(){
    led_g.patternSingle(short_2_ptrn, short_2_arr);
    buzz.patternSingle(short_2_ptrn, short_2_arr);
    if (digitalRead(sw_cont) == true && digitalRead(butt_res) == false){
        led_g.setOffSingle();
        buzz.setOffSingle();
        return true;
    }else{
        return false;
    }
}