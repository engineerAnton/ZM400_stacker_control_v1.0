#define led_green A5
#define cutter_output 32
#define led_red A9
#define butt_res A10
#define buzzer 40
//#define buzzer 42
#define sw_cont 3
#define sw_bed_high 2
#define sw_bed_low 18
#define opt_label 18
#define opt_carr 15

// Защита от статики (антидребезг)
#define duration_ms 50

// Назначение контактов драйвера мотора блока протяжки этикетки
#define mot_FR_step A0
#define mot_FR_dir A1
#define mot_FR_en 38
#define mot_FR_steps 200

// Назначение контактов драйвера мотора укладчика
#define mot_CR_step A6
#define mot_CR_dir A7
#define mot_CR_en A2
#define mot_CR_steps 200

#define mot_CR_step A6
#define mot_CR_dir A7
#define mot_CR_en A2
#define mot_CR_steps 200

// Назначение контактов драйвера мотора оси Z
#define mot_BD_step 46
#define mot_BD_dir 48
#define mot_BD_en A8
#define mot_BD_steps 200

//Управление электромагнитом
#define coil 8

#if !defined(CLOSE)
#define CLOSE 1
#endif // CLOSE

#if !defined(OPEN)
#define OPEN 0
#endif // OPEN

