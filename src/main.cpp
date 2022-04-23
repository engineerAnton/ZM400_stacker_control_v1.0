#include <Arduino.h>
#include <Debouncer.h>
#include <global.h>
#include <functions.h>

Debouncer cutter_out(cutter_output, duration_ms);

int comm = 0;
int state = 5;
bool flag_run = 0;
bool debug = 0;

void loop() {
  cutter_out.update();
  leds_buzz_update();
  
  if (debug){
  //Отладка
  // Состояние элементов управления
  Serial.println("reset_btn: " + String(digitalRead(butt_res)) + "; sw_cont: " + String(digitalRead(sw_cont)) + 
  "; sw_Z_high: " + String(digitalRead(sw_bed_high)) + "; sw_Z_low: " + String(digitalRead(sw_bed_low)) + 
  "; opt_label: " + String(digitalRead(opt_label)) +  "; opt_carr: " + String(digitalRead(opt_carr)));
    
  }else{
  switch (state) {
    case 5:    
      while (digitalRead(butt_res) == true){
        start(true);
      }
      Serial.println("System initializing...");
      start(false);
      state = 10;
      break;

    // 10 Включение. Проверка, что стол в верхнем положении, нет этикетки в тракте подачи, каретка в исходном состоянии, контейнер установлен. 
    // Успешно - переход в 120. Не успешно - переход в 30.
    case 10:
      if (check_all() == true){
        Serial.println("System initialized!");
        led_g_on();
        Serial.println("Waiting for a signal from the printer...");
        flag_run = 0;
        state = 120;
      }
      if (check_all() == false){
        led_g_off();
        Serial.println("Bed initialization...");
        state = 30;
      }
      break;
    
    // 30 Если стол не в верхнем положении - движение стола вверх до концевика. Если стол не вернулся в начальное положение за t время - 4 длинных мигания красного, 4 длинных сигнала.
    // Нажатие кнопки - повторное движение, проверка.
    // Успешно - переход в 50. Не упешно - 40.
    case 30:
      if (bed_init() == true){
        Serial.println("Bed initialized!");
        Serial.println("Checking if a container exists...");
        flag_run = 0;       
        state = 50;
      }
      if (bed_init() == false){    
        state = 40;
      }
      break;
    
    // Попытка повторной инициализации стола по нажатию кнопки  
    case 40:
      Serial.println("Bed fault!");
      while (digitalRead(butt_res) == true){
        // indication
        bed_fault(true);
      }
      Serial.println("Bed initialization...");
      bed_fault(false);
      state = 30;
      break;
    
    // 50 Если контейнер не установлен.
    // Успешно - переход в 70. Не успешно - 55.
    case 50:
      if (cont_init() == true){
        Serial.println("Container inserted!");
        Serial.println("Feeder initialization...");
        flag_run = 0;
        state = 70;        
      }
      if (cont_init() == false){
        state = 55;
      }
      break;
    
    // Просьба установить пустой контейнер
    case 55:
      Serial.println("Please insert an empty container...");
      while (digitalRead(sw_cont) == false){
        //indication
        cont_fault(true);
      }
      Serial.println("Checking if a container exists...");
      cont_fault(false);
      state = 50;
      break;
    
    // 70 Наличие этикетки в тракте подачи.
    // Нет - переход в 90. Есть - 75.
    case 70:
      if (feeder_init() == true){
        Serial.println("Feeder initialized!");
        Serial.println("Carriage initialization...");
        flag_run = 0;
        state = 90; 
      }
      if (feeder_init() == false){
        state = 75;
      }
      break;
    
    // Повторная проверка наличия этикетки
    case 75:
      Serial.println("Please remove the label from the feeder...");
      while (digitalRead(opt_label) == true){
        // indication
        feeder_fault(true);
      }
      Serial.println("Feeder initialization...");
      feeder_fault(false);
      state = 70;
      break;
    
    // 90 Если каретка не в исходном состоянии - попытка провернуть до исходного.
    // Успешно - переход в 120. Не успешно - переход в 100.
    case 90:
      if (carr_init() == true){
        Serial.println("Carriage initialized!");
        led_g_on();
        Serial.println("Waiting for a signal from the printer...");
        flag_run = 0;
        state = 120; 
      }
      if (carr_init() == false){
        state = 100;
      }
      break;
    
    // Попытка повторной инициализации каретки по нажатию кнопки
    case 100:
      Serial.println("Carriage fault!");
      while (digitalRead(butt_res) == true){
        //indication
        carr_fault(true);
      }
      Serial.println("Carriage initialization...");
      carr_fault(false);
      state = 90;
      break;     

    // 120 Состояние готовности к работе. Ожидание сигнала с принтера и отслеживание момента изъятия контейнера.
    // Сигнал с принтера - переход в 130
    case 120:      
      if (cutter_out.edge()){
        if (cutter_out.falling() && flag_run == 0){
          flag_run = 1;          
          state = 130;
        }
      }
      if (Serial.available() > 0){
        comm = Serial.read();
        if (comm == 'n' && flag_run == 0){
          flag_run = 1;
          state = 130;
        }
        if (comm == 'p') {
          digitalWrite(pause_butt, HIGH);
          delay(100);
          digitalWrite(pause_butt, LOW);
          Serial.println("PAUSE button");
        }
        /*if (comm == 'o') {
          Serial2.println("~PP");
          Serial.println("Print pause o");
        }
        if (comm == 's') {
          Serial2.println("~PS");
          Serial.println("Print start");
        }*/
      }
      if (digitalRead(sw_cont) == false){
        state = 280;
      }
      break;

    // 130 Движение стола на 1 поз. вниз.
    // Переход в 150
    case 130:
      Serial.println("Bed 1 pos. down");
      bed_down();
      state = 150;      
      break;

    // 150 Опустить блок протяжки
    // Переход в 170
    case 150:
      Serial.println("Feeder down");
      feeder_down();
      state = 170;
      break;

    // 170 Протянуть этикетку. Этикетка освободила датчик за t время?
    // Успешно - переход в 190. Не успешно - переход в 70
    case 170:
      Serial.println("Feeder move");
      if (feeder_move() == true){
        state = 190;
      }else{
        feeder_up();
        led_g_off();
        Serial.println("Feeder fault!");       
        state = 70;
      }
      break;

    // 190 Поднять блок протяжки
    // Переход в 210
    case 190:
      Serial.println("Feeder up");
        feeder_up();
        state = 210;
      break;

    // 210 Движение каретки. Каретка отработала за t время?
    // Успешно - переход в 230. Не успешно - переход в 100
    case 210:
      Serial.println("Carriage move");
      if (carr_move() == true){
        Serial.println("Done!");
        state = 230;
      }else{
        led_g_off();
        Serial.println("Carriage fault!");
        state = 100;
      }
      break;
    
    // 230 Контейнер заполнен?
    // Заполнен - переход в 260. Не заполнен - команда продолжить печать, переход 120
    case 230:        
      if (digitalRead(sw_bed_low) == false){        
        // Имитация нажатия кнопки "PAUSE"
        digitalWrite(pause_butt, HIGH);
        delay(100);
        digitalWrite(pause_butt, LOW);
        flag_run = 0;
        Serial.println("Waiting for a signal from the printer...");
        state = 120;
      }
      if (digitalRead(sw_bed_low) == true){
        led_g_off();
        Serial.println("Container is full!");
        Serial.println("Please remove the container");
        state = 260;
      }
      break;

    // 260 Ожидание извлечение полного контейнера.
    // Извлечён - ждать
    case 260:  
      if (cont_full() == true){
        state = 300;
      }
      break;

    // 280 Ожидание установки.
    // Если установлен и нажата кнопка - переход 300
    case 280:
      if (digitalRead(butt_res) == true){
        Serial.println("Please insert an empty container");
        state = 300;
      }      
      break;

    // 300 Ожидание установки.
    // Если установлен и нажата кнопка - переход 30
    case 300:  
      if (cont_insert() == true){
        Serial.println("Container inserted");
        state = 30;
      }
  }
  }
}

