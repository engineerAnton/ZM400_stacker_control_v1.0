#include <Arduino.h>
#include <Debouncer.h>
//#include <A4988.h>
#include <global.h>
#include <functions.h>

Debouncer debouncer(cutter_output, duration_ms);

// MillisTimer carr_tim = MillisTimer(3000);

int comm = 0;
byte state = 1;
bool flag_run = 0;



void loop() {
  debouncer.update(); 
  
  //Отладка
  // Состояние элементов управления
  /*Serial.println("reset_btn: " + String(digitalRead(reset_btn)) + "; sw_cont: " + String(digitalRead(sw_cont)) + 
  "; sw_Z_high: " + String(digitalRead(sw_Z_high)) + "; sw_Z_low: " + String(digitalRead(sw_Z_low)) + 
  "; opt_label: " + String(digitalRead(opt_label)) +  "; opt_carr: " + String(digitalRead(opt_carr)));
*/
  /*
  if (Serial.available() > 0){
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
  }

  // Рабочий цикл
  if (debouncer.edge()){
    if (debouncer.falling() && flag_run == 0){
      flag_run = 1;
      state = 1;
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
    // 3 Если контейнер не установлен - 1 длинный красного, 1 длинный сигнал.
    // Успешно - переход в 5. Не успешно - повтор.
    case 3:
      if (cont_init() == true){
        Serial.println("Container inserted!");
        state = 5;        
      }
      break;
    // 5 Если есть этикетка в тракте подачи - 2 длинных мигания красного, 2 длинных сигнала. Нажатие кнопки - повторная проверка
    // Успешно - переход в 7. Не успешно - повтор.
    case 5:
      if (feeder_init() == true){
        Serial.println("Feeder initialized!");
        state = 7; 
      }
      break;
      //Serial.println("State 5");
    // 7 Если каретка не в исходном состоянии - попытка провернуть до исходного. Если не получается за t время - 3 длинных мигания красного, 3 длинных сигнала.
    // Нажатие кнопки - повторное движение, проверка.
    // Успешно - переход в 9. Не успешно - повтор.
    case 7:
      if (carr_init() == true){
        Serial.println("Carriage initialized!");
        state = 9; 
      }
      if (carr_init() == false){
        Serial.println("Carriage error!");
        state = 8;
      } 
      break;
    
    // Попытка повторной инициализации каретки по нажатию кнопки  
    case 8:
      if (digitalRead(butt_res) == false){
        state = 7;
      }
      break;  
    // 9 Если стол не в верхнем положении - движение стола вверх до концевика. Если стол не вернулся в начальное положение за t время - 4 длинных мигания красного, 4 длинных сигнала.
    // Нажатие кнопки - повторное движение, проверка.
    // Успешно - переход в 11. Не упешно - повтор.
    case 9:
      if (bed_init() == true){
        Serial.println("Bed initialized!");
        state = 11; 
      }
      if (bed_init() == false){
        Serial.println("Bed error!");
        state = 10;
      } 
      break;
    
    // Попытка повторной инициализации стола по нажатию кнопки  
    case 10:
      if (digitalRead(butt_res) == false){
        state = 9;
      }
      break; 
    // 11 Состояние готовности к работе. Свечение зелёного. Однократно 3 коротких сигнала. Ожидание сигнала с принтера.
    // Сигнал с принтера - переход в 13
    //case 11:
    // 13 Движение стола на 1 поз. вниз.
    // Переход в 15

    // 15 Опустить блок протяжки
    // Переход в 17

    // 17 Протянуть этикетку. Этикетка освободила датчик за t время?
    // Успешно - переход в 19. Не успешно - печать на паузу, переход в 5

    // 19 Движение каретки. Каретка отработала за t время?
    // Успешно - переход в 21. Не успешно - печать на паузу, переход в 7

    // 21 Поднять блок протяжки
    // Переход в 23 запуск печати следующей этикетки

    // 23 Контейнер заполнен?
    // Заполнен - переход в 25. Не заполнен - команда продолжить печать, переход 13

    // 25 Ожидание извлечение полного контейнера. 4 коротких мигания красного, 4 коротких сигнала
    // Извлечён - ждать 

    // 30 Ожидание установки.
    // Если установлен и нажата кнопка - переход 33

    // 33 Ожидание нажатия кнопки
    // Если нажата кнопка - переход 3

    // 50 Состояние аварии
  }
  // Конечный автомат
  /*switch (state) {
    



        case 1:
          Serial.println("Z axis down");
          mot_Z.rotate(-20);
          state = 5;
          break;
        case 5:
          digitalWrite(coil, HIGH);
          Serial.println("Coil ON");
          delay(200);
          state = 10;
          break;  
        case 10:          
          Serial.println("Start FR");
          mot_FR.rotate(550);
          state = 15;
          break;        
        case 15:
          Serial.println("Start CR");
          mot_CR.rotate(360);
          state = 20;
          break;
        case 20:
          digitalWrite(coil, LOW);
          Serial.println("Coil OFF");          
          flag_run = 0;
          Serial2.write("~PS");
          state = 0;
          break;
  }*/
}

