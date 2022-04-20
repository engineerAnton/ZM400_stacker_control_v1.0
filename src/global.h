#define led_green A5        // AUX2.3
#define cutter_output 32    // J1.24
#define led_red A9          // AUX2.4
#define butt_res A10        // AUX2.5
#define buzzer 40           // AUX2.6
//#define buzzer 42
#define sw_cont 3           // X-
#define sw_bed_high 2       // X+
#define sw_bed_low 18       // Z-
#define opt_label 14        // Y-
#define opt_carr 15         // Y+

// serial 2                 // AUX4.17
                            // AUX4.18

// Защита от статики (антидребезг)
#define duration_ms 50

// Назначение контактов драйвера мотора блока протяжки этикетки
#define mot_FR_step A0      // X
#define mot_FR_dir A1
#define mot_FR_en 38
#define mot_FR_steps 200

// Назначение контактов драйвера мотора каретки
#define mot_CR_step A6      // Z
#define mot_CR_dir A7
#define mot_CR_en A2
#define mot_CR_steps 200

#define mot_CR_step A6
#define mot_CR_dir A7
#define mot_CR_en A2
#define mot_CR_steps 200

// Назначение контактов драйвера мотора привода стола
#define mot_BD_step 46      // Y
#define mot_BD_dir 48
#define mot_BD_en A8
#define mot_BD_steps 200

//Управление электромагнитом
#define coil 8              // J28

#if !defined(CLOSE)
#define CLOSE 1
#endif // CLOSE

#if !defined(OPEN)
#define OPEN 0
#endif // OPEN

