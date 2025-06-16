#include "project.h" // Incluye los archivos de encabezado generados por PSoC Creator
#include "LiquidCrystal_I2C.h"
#include <cytypes.h>
#include <stdbool.h>
#include <math.h> // Necesario para funciones matemáticas como sqrt o pow si las usas para la rampa
#include <string.h> 
#include <stdio.h>

//----------------------------------definiendo variables---------------------------------------------//

#define RX_BUFFER_SIZE 64
#define STEPS_PER_REV       200UL    // Pasos por revolución de tu motor (ej. 200 para motor de 1.8 grados/paso)
#define MICROSTEPS_FACTOR   16UL     // Factor de microstepping (ej. 1 para full step, 16 para 1/16)
                                     // Esto debe coincidir con la configuración de tu driver de motor (DRV8825, A4988, etc.)
#define MM_PER_REV          8.0f     // Milímetros que avanza tu mecanismo por una revolución (ej. 8mm para un tornillo de plomo, o circunferencia de rueda si es un robot)
                                     // Si "distancia" es simplemente un número de vueltas o unidades abstractas, ajusta esto.
#define PULSE_WIDTH_US      2UL      // Duración mínima del pulso STEP en microsegundos (para asegurar que el driver lo detecte, ej. 2us)
#define RAMP_SEGMENT_STEPS 400 // Número de pasos dedicados a UNA fase de rampa (aceleración O desaceleración).
                               // Ajusta este valor:
                               // - Mayor valor: rampa más suave y larga.
                               // - Menor valor: rampa más brusca y corta.
                               // Deberás probar cuál es el mejor valor para tu motor y aplicación

char rxBuffer[RX_BUFFER_SIZE];

volatile uint8 rxBufferIdx = 0; // Índice actual del buffer
volatile uint8 newMessageFlag = 0; // Bandera que indica que se ha recibido un mensaje completo
char motorDirection = ' '; // 'L' para izquierda, 'R' para derecha
int motorRPM_parsed = 0;   // Renombrado para evitar conflicto si ya tienes motorRPM
int motorDistance_parsed = 0; 
int servo1Degrees = 0;
int servo2Degrees = 0;
uint32_t Addr = 0x27;
char lcdLineBuffer[17];

//-------------------------------------Interrupciones------------------------------------------------//
CY_ISR(UART_RX_ISR_Handler) // El nombre de la función C que maneja la interrupción
{
     uint32 readStatus = UART_ReadRxStatus(); // Lee el registro de estado para limpiar la interrupción

    if (readStatus & UART_RX_STS_FIFO_NOTEMPTY) // Si hay datos en el FIFO de RX
    {
        // *** CAMBIO AQUÍ: Usar la función API para leer el carácter recibido ***
        char receivedChar = UART_ReadRxData(); // Lee el byte recibido

        if (receivedChar == '\n') // Si el byte es un salto de línea (terminador de mensaje)
        {
            rxBuffer[rxBufferIdx] = '\0'; // Null-termina la cadena en el buffer
            newMessageFlag = 1;         // Marca la bandera para que el bucle principal procese
            rxBufferIdx = 0;            // Reinicia el índice del buffer para el próximo mensaje
        }
        else if (rxBufferIdx < (RX_BUFFER_SIZE - 1)) // Si no es salto de línea y hay espacio en el buffer
        {
            rxBuffer[rxBufferIdx++] = receivedChar; // Almacena el byte y avanza el índice
        }
    }
}
//----------------------------------------------funcion LCD--------------------------------------------------------//
void printLcd(int line,int col, const char* message){
    setCursor(col,line);
    char displayBuffer[17];
    int len = 0;
    while (message[len] != '\0' && len < 16) {
        displayBuffer[len] = message[len];
        len++;
    }
    for (int i = len; i < 16; i++) { displayBuffer[i] = ' ';}
    displayBuffer[16] = '\0'; 
    LCD_print(displayBuffer);  
}

//------------------------------------------Función de serial-------------------------------------------------//
void processMessage(char* message)
{
     char commandType = message[0];
    int parsedValue1, parsedValue2; // Usados para parsing genérico
    char parsedDirChar;             // Usado para la dirección del motor

    switch(commandType)
    {
        case 'M': // Comando de Motor: M,<dirección>,<RPM>,<distancia>
            // sscanf ahora busca 3 valores: un char y dos enteros
            if (sscanf(&message[2], "%c,%d,%d", &parsedDirChar, &motorRPM_parsed, &motorDistance_parsed) == 3)
            {
                // Convertir el carácter de dirección a booleano
                // Asumimos 'R' (Right) es true, 'L' (Left) es false
                bool dirBool = (parsedDirChar == 'R') ? true : false; 
                
                // --- ¡Llamada a la nueva función de control del motor! ---
                control_motor(dirBool, (uint16)motorRPM_parsed, (uint16)motorDistance_parsed);

                // --- Impresión de depuración por UART (opcional) ---
                UART_PutString("Motor Cmd Recibido: Dir=");
                UART_PutChar(parsedDirChar);
                UART_PutString(", RPM=");
                char tempStrRPM[10];
                sprintf(tempStrRPM, "%d, Dist=", motorRPM_parsed);
                UART_PutString(tempStrRPM);
                char tempStrDist[10];
                sprintf(tempStrDist, "%d\r\n", motorDistance_parsed);
                UART_PutString(tempStrDist);
            }
            else
            {
                UART_PutString("Error: Comando de motor malformado (esperado M,<dir>,<rpm>,<dist>).\r\n");
            }
            break;

        case 'S': // Comando de Servo: S,<grados_servo1>,<grados_servo2>
            // (Este caso permanece igual)
            if (sscanf(&message[2], "%d,%d", &servo1Degrees, &servo2Degrees) == 2)
            {
                // --- Aquí iría tu lógica de control de los servos ---
                UART_PutString("Servo Cmd Recibido: S1=");
                char tempStr1[10];
                sprintf(tempStr1, "%d, S2=", servo1Degrees);
                UART_PutString(tempStr1);
                char tempStr2[10];
                sprintf(tempStr2, "%d\r\n", servo2Degrees);
                UART_PutString(tempStr2);
            }
            else
            {
                UART_PutString("Error: Comando de servo malformado.\r\n");
            }
            break;

        default:
            UART_PutString("Error: Tipo de comando desconocido.\r\n");
            break;
    }
}



//------------------------------funciones para el movimiento del motor----------------------------------------------//

void delay_us(uint16 us) { CyDelayUs(us);}

uint16 calculate_delay(uint32 current_step_idx, uint32 total_steps, uint16 initial_delay, uint16 final_delay) {
    // Si no hay pasos, o si el retardo inicial es igual o menor al final (no hay rampa efectiva)
    if (total_steps == 0 || initial_delay <= final_delay) {
        return final_delay; // Opera a velocidad máxima o no hace nada si total_steps es 0
    }

    uint32 delay_diff = initial_delay - final_delay;
    uint32 effective_ramp_segment_steps = RAMP_SEGMENT_STEPS;

    // Ajusta la cantidad de pasos de la rampa si el movimiento total es muy corto.
    // Esto asegura que la rampa no exceda la mitad del movimiento total (para dejar espacio a la desaceleración).
    if (total_steps < (2 * RAMP_SEGMENT_STEPS)) {
        effective_ramp_segment_steps = total_steps / 2;
        // Asegurarse de que al menos haya un mínimo de pasos de rampa si el total es muy pequeño
        if (effective_ramp_segment_steps == 0 && total_steps > 0) {
            effective_ramp_segment_steps = 1; // Al menos 1 paso de rampa si hay movimiento
        }
    }
    
    // Si el movimiento total es tan corto que effective_ramp_segment_steps es 0,
    // o si la rampa no puede ser efectiva (por ejemplo, total_steps < 2),
    // simplemente mueve a la velocidad final.
    if (effective_ramp_segment_steps == 0) {
        return final_delay;
    }

    uint32 current_delay;

    // --- Fase de Aceleración ---
    // Si el paso actual está dentro del segmento inicial de rampa
    if (current_step_idx < effective_ramp_segment_steps) {
        // El retardo disminuye linealmente desde initial_delay hasta final_delay
        current_delay = initial_delay - (delay_diff * current_step_idx) / effective_ramp_segment_steps;
    }
    // --- Fase de Desaceleración ---
    // Si el paso actual está dentro del segmento final de rampa
    else if (current_step_idx >= (total_steps - effective_ramp_segment_steps)) {
        // Calcula el índice relativo dentro de la fase de desaceleración
        uint32 decel_step_idx = current_step_idx - (total_steps - effective_ramp_segment_steps);
        // El retardo aumenta linealmente desde final_delay hasta initial_delay
        current_delay = final_delay + (delay_diff * decel_step_idx) / effective_ramp_segment_steps;
    }
    // --- Fase de Velocidad Constante ---
    // Si el paso actual está entre las fases de aceleración y desaceleración
    else {
        // El motor se mueve a la velocidad máxima (retardo mínimo)
        current_delay = final_delay;
    }

    // Comprobaciones de seguridad para asegurar que el retardo no se salga de los límites
    // debido a la aritmética de enteros, especialmente si initial_delay y final_delay están muy cerca.
    if (current_delay > initial_delay) { // No debe ser más lento que el inicio de la rampa
        current_delay = initial_delay;
    }
    if (current_delay < final_delay) { // No debe ser más rápido que la velocidad máxima
        current_delay = final_delay;
    }

    return (uint16)current_delay;
}


void step_motor_ramped(uint32 steps_to_move, bool direction, uint16 initial_delay_us, uint16 min_delay_us) {
    DIR_Write(direction); // Establece la dirección (1 o 0)
    CyDelayUs(50);        // Pequeño retardo para que el pin de dirección se estabilice

    for (uint32 i = 0; i < steps_to_move; i++) {
        uint16 current_delay_period_us = calculate_delay(i, steps_to_move, initial_delay_us, min_delay_us);
        
        // Asegura que el pulso HIGH dure al menos PULSE_WIDTH_US y que no sea más de la mitad del período total
        uint16 pulse_on_time = PULSE_WIDTH_US; 
        if (pulse_on_time > (current_delay_period_us / 2)) {
            pulse_on_time = current_delay_period_us / 2; 
        }
        uint16 pulse_off_time = current_delay_period_us - pulse_on_time;

        STEP_Write(1); // Pulso HIGH para un paso
        delay_us(pulse_on_time); // Duración del pulso HIGH
        STEP_Write(0); // Pulso LOW (finaliza el paso)
        delay_us(pulse_off_time); // Duración del LOW (resto del período del paso)
    }
}

void control_motor(bool direction, uint16 target_rpm, uint16 distance_mm) {
    // 1. Calcular el número total de pasos (actuales, incluyendo microstepping) para la distancia
    // total_steps = (distancia_mm * (pasos_por_rev * factor_microstepping)) / mm_por_rev
    uint32 total_steps_for_distance = (uint32)(((float)distance_mm * (STEPS_PER_REV )) / MM_PER_REV);

    // 2. Calcular el 'min_delay_us' (retardo para la velocidad objetivo en RPM)
    // Pasos por segundo = (RPM * STEPS_PER_REV * MICROSTEPS_FACTOR) / 60
    // Retardo por paso (us) = 1,000,000 / (Pasos por segundo)
    // Retardo por paso (us) = 60,000,000 / (RPM * STEPS_PER_REV * MICROSTEPS_FACTOR)
    uint16 calculated_min_delay_us;
    if (target_rpm == 0) { // Si RPM es 0, no moverse o moverse muy lento
        calculated_min_delay_us = 60000; // Un retardo muy grande (60ms/paso)
    } else {
        // Asegúrate de usar uint32 para la multiplicación grande para evitar desbordamiento
        calculated_min_delay_us = (uint16)(60000000UL / ((uint32)target_rpm * STEPS_PER_REV * MICROSTEPS_FACTOR));
    }
    
    // Si el cálculo da un retardo muy pequeño (RPM muy altas), limitarlo para evitar problemas
    if (calculated_min_delay_us < (PULSE_WIDTH_US * 2)) { // Mínimo el doble del ancho de pulso
        calculated_min_delay_us = PULSE_WIDTH_US * 2;
    }

    // 3. Definir el 'initial_delay_us' (para la rampa de aceleración)
    // Normalmente, un retardo mucho mayor que min_delay_us para empezar lento
    uint16 initial_delay_us = calculated_min_delay_us * 5; // Empieza 5 veces más lento
    if (initial_delay_us > 100000) { // Limita el retardo inicial máximo a 100ms
        initial_delay_us = 100000;
    }
    if (initial_delay_us < calculated_min_delay_us) { // Asegura que initial sea mayor o igual que min
        initial_delay_us = calculated_min_delay_us;
    }

    // Llamar a la función de pasos con rampa usando los valores calculados
    step_motor_ramped(total_steps_for_distance, direction, initial_delay_us, calculated_min_delay_us);
}


int main(void) {
    //CyGlobalIntEnable;
    //I2C_Start();
    //for (;;) {
        
    //    step_motor_ramped(7250, true, 4000, 600); 
    //    CyDelay(2000); 
   
   //     step_motor_ramped(7250, false, 2000, 600);
    //    CyDelay(2000); 
    //}
     CyGlobalIntEnable; /* Habilita las interrupciones globales. */

    /* Coloca tu código de inicialización aquí. */
    UART_Start(); // Inicia el componente UART
    UART_SetRxInterruptMode(UART_RX_STS_FIFO_NOTEMPTY); // Configura la interrupción para FIFO no vacío
    
    // Este nombre 'isrRx' proviene de tu esquemático
    isrRx_StartEx(UART_RX_ISR_Handler); // Inicia y asocia el ISR con el manejador definido
    
    UART_PutString("PSoC 5LP Listo. Esperando comandos...\r\n");
    I2C_Start();
    LiquidCrystal_I2C_init(Addr, 16, 2, 0);
    begin();
    clear();
    printLcd(0,1,"Esperando comandos");

    for(;;) // Bucle infinito
    {
        /* Coloca tu código de aplicación principal aquí. */
        if (newMessageFlag)
        {
            CyGlobalIntDisable;
            newMessageFlag = 0;
            processMessage(rxBuffer);
            CyGlobalIntEnable;
        }
    }
    
}