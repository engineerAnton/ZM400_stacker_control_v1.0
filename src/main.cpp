#include <Arduino.h>
#include <Debouncer.h>
#include <global.h>
#include <functions.h>

Debouncer cutter_out(cutter_output, duration_ms);

int comm = 0;
byte state = 1;
bool flag_run = 0;

void loop() {
  //mot_BD_update();
  cutter_out.update();
  leds_buzz_update();
  /*
  //Отладка
  // Состояние элементов управления
  Serial.println("reset_btn: " + String(digitalRead(reset_btn)) + "; sw_cont: " + String(digitalRead(sw_cont)) + 
  "; sw_Z_high: " + String(digitalRead(sw_Z_high)) + "; sw_Z_low: " + String(digitalRead(sw_Z_low)) + 
  "; opt_label: " + String(digitalRead(opt_label)) +  "; opt_carr: " + String(digitalRead(opt_carr)));
  */
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

  switch (state) {

    // 1 Включение, 1 длинный сигнал. Проверка, что стол в верхнем положении, нет этикетки в тракте подачи, каретка в исходном состоянии, контейнер установлен. 
    // Успешно - переход в 11. Не успешно - переход в 3.
    case 1:
      if (check_all() == true){
        Serial.println("System initialized!");
        state = 11;
      }
      if (check_all() == false){
        state = 3;
      }
      break;
    
    // 3 Если стол не в верхнем положении - движение стола вверх до концевика. Если стол не вернулся в начальное положение за t время - 4 длинных мигания красного, 4 длинных сигнала.
    // Нажатие кнопки - повторное движение, проверка.
    // Успешно - переход в 11. Не упешно - повтор.
    case 3:
      if (bed_init() == true){
        Serial.println("Bed initialized!");        
        flag_run = 0;       
        state = 5;
      }
      if (bed_init() == false){
        Serial.println("Bed error!");
        state = 4;
      }
      break;
    
    // Попытка повторной инициализации стола по нажатию кнопки  
    case 4:
      if (digitalRead(butt_res) == false){
        state = 3;
      }
      break; 
    
    // 5 Если контейнер не установлен - 1 длинный красного, 1 длинный сигнал.
    // Успешно - переход в 7. Не успешно - повтор.
    case 5:
      if (cont_init() == true){
        Serial.println("Container inserted!");
        flag_run = 0;
        state = 7;        
      }
      break;
    
    // 7 Если есть этикетка в тракте подачи - 2 длинных мигания красного, 2 длинных сигнала. Нажатие кнопки - повторная проверка
    // Успешно - переход в 9. Не успешно - повтор.
    case 7:
      if (feeder_init() == true){
        Serial.println("Feeder initialized!");
        flag_run = 0;
        state = 9; 
      }
      break;
    
    // 9 Если каретка не в исходном состоянии - попытка провернуть до исходного. Если не получается за t время - 3 длинных мигания красного, 3 длинных сигнала.
    // Нажатие кнопки - повторное движение, проверка.
    // Успешно - переход в 11. Не успешно - переход в 10.
    case 9:
      if (carr_init() == true){
        Serial.println("Carriage initialized!");
        flag_run = 0;
        state = 11; 
      }
      if (carr_init() == false){
        Serial.println("Carriage error!");
        state = 10;
      } 
      break;
    
    // Попытка повторной инициализации каретки по нажатию кнопки  
    case 10:
      if (digitalRead(butt_res) == false){
        state = 9;
      }
      break;
    
    // Вывод сообщения
    case 11:
      Serial.println("Waiting for a signal from the printer...");
      state = 12;
      break;

    // 12 Состояние готовности к работе. Свечение зелёного. Однократно 3 коротких сигнала. Ожидание сигнала с принтера.
    // Сигнал с принтера - переход в 13
    case 12:
      led_g_on();
      if (cutter_out.edge()){
        if (cutter_out.falling() && flag_run == 0){
          led_g_off();
          flag_run = 1;
          state = 13;
        }
      }
      if (Serial.available() > 0){
        comm = Serial.read();
        if (comm == 'n' && flag_run == 0){
          led_g_off();
          flag_run = 1;
          state = 13;
        }
      }
      if (digitalRead(sw_cont) == false){
        led_g_off();
        state = 28;
      }
      break;

    // 13 Движение стола на 1 поз. вниз.
    // Переход в 15
    case 13:
      Serial.println("Bed 1 pos. down");
      bed_down();
      Serial.println("Rotate");
      state = 15;      
      break;

    // 15 Опустить блок протяжки
    // Переход в 17
    case 15:
      Serial.println("Feeder down");
      feeder_down();
      state = 17;
      break;

    // 17 Протянуть этикетку. Этикетка освободила датчик за t время?
    // Успешно - переход в 19. Не успешно - переход в 7
    case 17:
      Serial.println("Feeder move");
      if (feeder_move() == true){
        state = 19;
      }else{
        feeder_up();
        Serial.println("Feeder fault!");       
        state = 7;
      }
      break;

    // 19 Поднять блок протяжки
    // Переход в 21
    case 19:
      Serial.println("Feeder up");
        feeder_up();
        state = 21;
      break;

    // 21 Движение каретки. Каретка отработала за t время?
    // Успешно - переход в 23. Не успешно - переход в 7
    case 21:
      Serial.println("Carriage move");
      if (carr_move() == true){
        Serial.println("Done!");
        state = 23;
      }else{
        Serial.println("Carriage error!");
        state = 9;
      }
      break;
    
    // 23 Контейнер заполнен?
    // Заполнен - переход в 25. Не заполнен - команда продолжить печать, переход 11
    case 23:        
      if (digitalRead(sw_bed_low) == false){
        flag_run = 0;
        // Задержка перед отправкой команды на принтер
        delay(200);
        Serial2.println("~PS");
        state = 11;
      }
      if (digitalRead(sw_bed_low) == true){
        Serial.println("Container is full!");
        state = 25;
      }
      break;

    // 25 Ожидание извлечение полного контейнера. 4 коротких мигания красного, 4 коротких сигнала
    // Извлечён - ждать
    case 25:
      Serial.println("Please remove the container");
      state = 26;
      break;

    case 26:  
      if (cont_remove() == true){
        state = 28;
      }
      break;

    // 30 Ожидание установки.
    // Если установлен и нажата кнопка - переход 33
    case 28:
      Serial.println("Please insert an empty container");
      state = 30;
      break;

    // 30 Ожидание установки.
    // Если установлен и нажата кнопка - переход 33
    case 30:  
      if (cont_insert() == true){
        Serial.println("Container inserted");
        state = 3;
      }
      
    // 50 Состояние аварии
    case 50:
      break;
  }
}

