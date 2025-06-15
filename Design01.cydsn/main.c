#include "project.h" // Incluye los archivos de encabezado generados por PSoC Creator
#include <cytypes.h>
#include <stdbool.h>
#include <math.h> // Necesario para funciones matemáticas como sqrt o pow si las usas para la rampa

// Definiciones de las funciones de control de pines
// PSoC Creator generará funciones como STEP_PIN_Write(), DIR_PIN_Write()
// y otros para tus pines de salida digital.

void delay_us(uint16 us) {
    CyDelayUs(us);
}

// Función para calcular el retardo en función del paso actual para una rampa
// Puedes experimentar con diferentes funciones matemáticas para obtener la rampa deseada.
// Aquí se presenta una función simple que simula una aceleración/desaceleración lineal
uint16 calculate_delay(uint16 current_step, uint16 total_steps, uint16 initial_delay, uint16 min_delay) {
    // Definimos una zona de aceleración y desaceleración
    uint16 acceleration_zone_steps = total_steps / 5; // Por ejemplo, el primer 25% para acelerar
    uint16 deceleration_zone_steps = total_steps / 5; // Y el último 25% para desacelerar

    if (current_step < acceleration_zone_steps) {
        // Aceleración: Reducir el retardo linealmente
        // El retardo irá de initial_delay a min_delay
        return initial_delay - ( (initial_delay - min_delay) * current_step / acceleration_zone_steps );
    } else if (current_step >= (total_steps - deceleration_zone_steps)) {
        // Desaceleración: Aumentar el retardo linealmente
        // El retardo irá de min_delay a initial_delay
        uint16 steps_in_decel_zone = total_steps - current_step;
        return min_delay + ( (initial_delay - min_delay) * (deceleration_zone_steps - steps_in_decel_zone) / deceleration_zone_steps );
    } else {
        // Velocidad constante en el medio
        return min_delay;
    }
}


void step_motor_ramped(uint16 num_steps, bool direction, uint16 initial_delay_us, uint16 min_delay_us) {
    // num_steps: número de pasos a mover
    // direction: true para una dirección, false para la otra
    // initial_delay_us: Retardo inicial (velocidad lenta)
    // min_delay_us: Retardo mínimo (velocidad máxima)

    DIR_Write(direction); // Establece la dirección
    CyDelayUs(50); // Pequeño retardo para que la dirección se establezca

    for (uint16 i = 0; i < num_steps; i++) {
        uint16 current_delay = calculate_delay(i, num_steps, initial_delay_us, min_delay_us);
        
        STEP_Write(1); // Pulso alto
        delay_us(current_delay / 2); // Retardo para mantener el pulso (la mitad del retardo total por ciclo)
        STEP_Write(0); // Pulso bajo
        delay_us(current_delay / 2); // Retardo entre pasos (la otra mitad)
    }
}

int main(void) {
    CyGlobalIntEnable; /* Habilita las interrupciones globales */

    /* Coloca aquí tu código de inicialización, por ejemplo: */
    // Initialize all necessary components like Pins, Timer, etc. from PSoC Creator.
    // For example, if you have a component named 'Pin_STEP' and 'Pin_DIR'
    // you might have functions generated like Pin_STEP_Start() and Pin_DIR_Start().
    // However, for simple digital output pins, PSoC Creator often handles initialization implicitly.

    for (;;) {
        // Mover el motor 800 pasos (4 revoluciones) con rampa
        // Empezar lento (2000 us = 2ms por ciclo de paso), acelerar a 500 us (0.5ms por ciclo)
        step_motor_ramped(7250, true, 4000, 600); 
        CyDelay(2000); // Espera 2 segundos

        // Mover el motor 800 pasos en la dirección opuesta con rampa
        step_motor_ramped(7250, false, 2000, 600);
        CyDelay(2000); // Espera 2 segundos
    }
}