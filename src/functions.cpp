#include <functions.h>
#include <global.h>
#include <singleLEDLibrary.h>
#include <MillisTimer.h>
#include <AccelStepper.h>

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
AccelStepper mot_CR(AccelStepper::DRIVER, mot_CR_step, mot_CR_dir);
AccelStepper mot_FR(AccelStepper::DRIVER, mot_FR_step, mot_FR_dir);
AccelStepper mot_BD(AccelStepper::DRIVER, mot_BD_step, mot_BD_dir);


void setup() {
  Serial.begin(9600);
  //Serial.setTimeout(10);
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

  initSteppers();

  Serial.println("Power on, initialization start...");
}

void initSteppers() // Инициализация ШД
{
  // Максимальная скорость
  mot_CR.setMaxSpeed(1000);
  mot_FR.setMaxSpeed(1000);
  mot_BD.setMaxSpeed(1000);

  // Установка ускорения
  mot_CR.setAcceleration(4000.0);
  mot_FR.setAcceleration(4000.0);
  mot_BD.setAcceleration(5000.0);

  // Установка пинов для включения питания
  mot_CR.setEnablePin(mot_CR_en);
  mot_FR.setEnablePin(mot_FR_en);
  mot_BD.setEnablePin(mot_BD_en);

  // Установка режима инвертирования для пинов: dir, step, enable
  mot_CR.setPinsInverted(true, false, true);
  mot_FR.setPinsInverted(true, false, true);
  mot_BD.setPinsInverted(true, false, true);

  // Включение подачи питания на моторы
  mot_CR.enableOutputs();
  mot_FR.enableOutputs();
  mot_BD.enableOutputs();
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

// Инициализация механизма каретки
bool carr_init(){
    if (digitalRead(opt_carr) == false){
        uint32_t now = millis();
        mot_CR.setSpeed(600);

        if (millis() - now > 2000){
            mot_CR.stop();
            return false;
        }
        
        mot_CR.runSpeed();
        
        if (digitalRead(opt_carr) == true){
            mot_CR.stop();
            
            return true;
        }
    }else{
        return true;
    }
};

// Движение каретки на один оборот
bool carr_move(){
    uint32_t now = millis();

    mot_CR.setCurrentPosition(0);
    mot_CR.setSpeed(600);

    while (true){
        mot_CR.runSpeed();      
        
        if (millis() - now > 2000){
          mot_CR.stop();
          return false;
        }
        // Условие для избежания пограничного значения датчика
        if (mot_CR.currentPosition() > 10){
          if (digitalRead(opt_carr) == true){
            mot_CR.stop(); 
            return true;
          }  
        }
    }
}

// Индикация состояния аварии
bool carr_fault(bool fault){
    if (fault == 1){
        led_r.patternSingle(long_3_ptrn, long_3_arr);
        buzz.patternSingle(long_3_ptrn, long_3_arr);
    }else{
        led_r.setOffSingle();
        buzz.setOffSingle();
    }
}

// Инициализация стола
bool bed_init(){
    if (digitalRead(sw_bed_high) == false){
        uint32_t now = millis();
        mot_BD.setSpeed(-600);
        while (millis() - now < 3000){
            mot_BD.runSpeed();
            //led_r.patternSingle(long_4_ptrn, long_4_arr);
            //buzz.patternSingle(long_4_ptrn, long_4_arr);
            if (digitalRead(sw_bed_high) == true){
                mot_BD.stop();
                led_r.setOffSingle();
                buzz.setOffSingle();
                return true;
            }
        }
        mot_BD.stop();
        return false;       
    }else{
        return true;
    }
};

// Движение стола на одну позицию
void bed_down(){
    mot_BD.setSpeed(500);
    // mot_BD.runToNewPosition(50 - mot_BD.currentPosition());
    mot_BD.setCurrentPosition(0);
    mot_BD.moveTo(15);
    while (mot_BD.currentPosition() != 15){
        mot_BD.run();
    }
    mot_BD.stop();
}

// Инициализация блока протяжки
bool feeder_init(){
    if (digitalRead(opt_label) == true){
        
        return false;
    }else{
        led_r.setOffSingle();
        buzz.setOffSingle();
        return true;
    }
};

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
    /*uint32_t now = millis();
    mot_FR.setSpeed(-600);
    while (millis() - now < 800){
        mot_FR.runSpeed();
    }
    mot_FR.stop();     
    if (digitalRead(opt_label) == false){
        return true;
    }      
    return false;*/
    mot_FR.setCurrentPosition(0);
    mot_FR.moveTo(-500);
    while (mot_FR.currentPosition() != -500){
        mot_FR.run();
    }
    mot_FR.stop();
    if (digitalRead(opt_label) == false){
        return true;
    }
    return false;
};

// Индикация состояния аварии
bool feeder_fault(bool fault){
    if (fault == 1){
        led_r.patternSingle(long_2_ptrn, long_2_arr);
        buzz.patternSingle(long_2_ptrn, long_2_arr);
    }else{
        led_r.setOffSingle();
        buzz.setOffSingle();
    }
}

// Включить зелёный светодиод
void led_g_on(){
    digitalWrite(led_green, HIGH);
}

// Выключить зелёный светодиод
void led_g_off(){
    digitalWrite(led_green, LOW);
}

// Включить зелёный светодиод
void led_r_on(){
    digitalWrite(led_red, HIGH);
}

// Выключить зелёный светодиод
void led_r_off(){
    digitalWrite(led_red, LOW);
}

void leds_buzz_update(){
    led_r.update();
    led_g.update();
    buzz.update();
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

void mot_BD_update(){
    mot_BD.runSpeed();
}