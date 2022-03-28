#include <Arduino.h>
#include <Debouncer.h>
#include <global.h>
#include <functions.h>

Debouncer cutter_out(cutter_output, duration_ms);

int comm = 0;
int state = 10;
bool flag_run = 0;
bool debug = 0;

void loop() {
  //mot_BD_update();
  cutter_out.update();
  leds_buzz_update();
  
  if (debug){
  //Отладка
  // Состояние элементов управления
  Serial.println("reset_btn: " + String(digitalRead(butt_res)) + "; sw_cont: " + String(digitalRead(sw_cont)) + 
  "; sw_Z_high: " + String(digitalRead(sw_bed_high)) + "; sw_Z_low: " + String(digitalRead(sw_bed_low)) + 
  "; opt_label: " + String(digitalRead(opt_label)) +  "; opt_carr: " + String(digitalRead(opt_carr)));
  
  /*if (Serial.available() > 0){
    comm = Serial.read();
    if (comm == '1' && flag_run == 0) {
      flag_run = 1;
      state = 1;
      Serial.println("Start move");
    }   
    if (comm == '5') {
      digitalWrite(coil, HIGH);
      Serial.println("Coil ON");
    }
    if (comm == '3') {
      state = 5;
      Serial.println("Start FR");
    }   
    if (comm == '0') {
      digitalWrite(coil, LOW);
      Serial.println("Coil OFF");
    }
    if (comm == 'p') {
      Serial2.write("~PP");
      Serial.println("Print pause");
    }
    if (comm == 's') {
      Serial2.write("~PS");
      Serial.println("Print start");
    }
  }*/
  }else{
  switch (state) {

    // 1 Включение, 1 длинный сигнал. Проверка, что стол в верхнем положении, нет этикетки в тракте подачи, каретка в исходном состоянии, контейнер установлен. 
    // Успешно - переход в 11. Не успешно - переход в 3.
    case 10:
      if (check_all() == true){
        Serial.println("System initialized!");
        led_g_on();
        Serial.println("Waiting for a signal from the printer...");
        flag_run = 0;
        state = 120;
      }
      if (check_all() == false){
        led_r_on();
        Serial.println("Bed initialization...");
        state = 30;
      }
      break;
    
    // 3 Если стол не в верхнем положении - движение стола вверх до концевика. Если стол не вернулся в начальное положение за t время - 4 длинных мигания красного, 4 длинных сигнала.
    // Нажатие кнопки - повторное движение, проверка.
    // Успешно - переход в 11. Не упешно - повтор.
    case 30:
      if (bed_init() == true){
        Serial.println("Bed initialized!");
        
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
      }
      Serial.println("Bed initialization...");
      state = 30;
      break;
    
    // 5 Если контейнер не установлен - 1 длинный красного, 1 длинный сигнал.
    // Успешно - переход в 7. Не успешно - повтор.
    case 50:
      if (cont_init() == true){
        Serial.println("Container inserted!");
        
        flag_run = 0;
        state = 70;        
      }else{
        state = 55;
      }
      break;
    case 55:
      Serial.println("Please insert an empty container...");
      while (digitalRead(sw_cont) == false){
        //indication
      }
      state = 50;
      break;
    
    // 7 Если есть этикетка в тракте подачи - 2 длинных мигания красного, 2 длинных сигнала. Нажатие кнопки - повторная проверка
    // Успешно - переход в 9. Не успешно - повтор.
    case 70:
      if (feeder_init() == true){
        Serial.println("Feeder initialized!");
        Serial.println("Carriage initialization...");
        flag_run = 0;
        state = 90; 
      }else{
        state = 75;
      }
      break;
    
    case 75:
      Serial.println("Please remove the label from the feeder...");
      while (digitalRead(opt_label) == true){
        // indication
      }
      state = 70;
      break;
    
    // 9 Если каретка не в исходном состоянии - попытка провернуть до исходного. Если не получается за t время - 3 длинных мигания красного, 3 длинных сигнала.
    // Нажатие кнопки - повторное движение, проверка.
    // Успешно - переход в 11. Не успешно - переход в 10.
    case 90:
      if (carr_init() == true){
        Serial.println("Carriage initialized!");
        led_r_off();
        led_g_on();
        Serial.println("Waiting for a signal from the printer...");
        flag_run = 0;
        state = 120; 
      }else{
        state = 100;
      }
      break;
    
    // Попытка повторной инициализации каретки по нажатию кнопки
    case 100:
      Serial.println("Carriage fault!");
      while (digitalRead(butt_res) == true){
        //indication
      }
      Serial.println("Carriage initialization...");
      state = 90;
      break;     

    // 12 Состояние готовности к работе. Свечение зелёного. Однократно 3 коротких сигнала. Ожидание сигнала с принтера.
    // Сигнал с принтера - переход в 13
    case 120:      
      if (cutter_out.edge()){
        if (cutter_out.falling() && flag_run == 0){
          led_g_off();
          flag_run = 1;
          state = 130;
        }
      }
      if (Serial.available() > 0){
        comm = Serial.read();
        if (comm == 'n' && flag_run == 0){
          led_g_off();
          flag_run = 1;
          state = 130;
        }
        if (comm == 'p') {
          Serial2.print("~PP");
          Serial.println("Print pause");
        }
        if (comm == 's') {
          Serial2.print("~PS");
          Serial.println("Print start");
        }
      }
      if (digitalRead(sw_cont) == false){
        led_g_off();
        state = 280;
      }
      break;

    // 13 Движение стола на 1 поз. вниз.
    // Переход в 15
    case 130:
      Serial.println("Bed 1 pos. down");
      bed_down();
      state = 150;      
      break;

    // 15 Опустить блок протяжки
    // Переход в 17
    case 150:
      Serial.println("Feeder down");
      feeder_down();
      state = 170;
      break;

    // 17 Протянуть этикетку. Этикетка освободила датчик за t время?
    // Успешно - переход в 19. Не успешно - переход в 7
    case 170:
      Serial.println("Feeder move");
      if (feeder_move() == true){
        state = 190;
      }else{
        feeder_up();
        Serial.println("Feeder fault!");       
        state = 70;
      }
      break;

    // 19 Поднять блок протяжки
    // Переход в 21
    case 190:
      Serial.println("Feeder up");
        feeder_up();
        state = 210;
      break;

    // 21 Движение каретки. Каретка отработала за t время?
    // Успешно - переход в 23. Не успешно - переход в 7
    case 210:
      Serial.println("Carriage move");
      if (carr_move() == true){
        Serial.println("Done!");
        state = 230;
      }else{
        Serial.println("Carriage error!");
        state = 100;
      }
      break;
    
    // 23 Контейнер заполнен?
    // Заполнен - переход в 25. Не заполнен - команда продолжить печать, переход 11
    case 230:        
      if (digitalRead(sw_bed_low) == false){        
        // Задержка перед отправкой команды на принтер
        delay(300);
        Serial2.print("~PS");
        flag_run = 0;
        state = 120;
      }
      if (digitalRead(sw_bed_low) == true){
        Serial.println("Container is full!");
        state = 250;
      }
      break;

    // 25 Ожидание извлечение полного контейнера. 4 коротких мигания красного, 4 коротких сигнала
    // Извлечён - ждать
    case 250:
      Serial.println("Please remove the container");
      state = 260;
      break;

    case 260:  
      if (cont_remove() == true){
        state = 280;
      }
      break;

    // 30 Ожидание установки.
    // Если установлен и нажата кнопка - переход 33
    case 280:
      Serial.println("Please insert an empty container");
      state = 300;
      break;

    // 30 Ожидание установки.
    // Если установлен и нажата кнопка - переход 33
    case 300:  
      if (cont_insert() == true){
        Serial.println("Container inserted");
        state = 30;
      }
      
    // 50 Состояние аварии
    case 500:
      break;
  }
  }
}

