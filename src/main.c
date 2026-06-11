#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/console/console.h>
#include <stdio.h>
#include <pwm_z42.h>

/* --- Engenharia de Frequência do HC-SR04 --- */
#define PRESCALER PS_16
#define TPM_PERIODO 50000 // 50.000 ticks * 2us = 100ms (10 leituras por segundo)
#define TPM_GATILHO 5     // 5 ticks * 2us = 10us (Gatilho mínimo do HC-SR04)

/* --- Variáveis Globais do Cronômetro (Interrupção) --- */
volatile uint16_t tempo_subida = 0;
volatile uint16_t duracao_ticks = 0;
volatile int nova_leitura = 0;
volatile int borda_esperada = 1; // 1 = esperando Subida, 0 = esperando Descida

/* --- A Rotina de Interrupção (ISR) do TPM1 --- */
void tpm1_isr(const void *arg) {
    // 1. Limpa as flags para avisar a placa que já vimos o "grito"
    TPM1->STATUS |= TPM_STATUS_CH0F_MASK;       
    TPM1->CONTROLS[0].CnSC |= TPM_CnSC_CHF_MASK;

    // 2. Salva o valor exato do cronômetro naquele instante
    uint16_t captura = TPM1->CONTROLS[0].CnV;

    if (borda_esperada == 1) {
        tempo_subida = captura;
        borda_esperada = 0; 
    } else {
        duracao_ticks = captura - tempo_subida; 
        nova_leitura = 1;   
        borda_esperada = 1; 
    }
}

int main(void) {
    k_msleep(500); // Pequena pausa para estabilizar o terminal serial
    printk("\n\n=== RADAR ULTRASSOM (HC-SR04) INICIADO ===\n");

    /* ==============================================================
     * 1. CONFIGURANDO O GATILHO (TRIGGER) - TPM2, Canal 0, Pino PTB2
     * ============================================================== */
    pwm_tpm_Init(TPM2, TPM_OSCERCLK, TPM_PERIODO, TPM_CLK, PRESCALER, EDGE_PWM);
    pwm_tpm_Ch_Init(TPM2, 0, TPM_PWM_H, GPIOB, 2);
    pwm_tpm_CnV(TPM2, 0, TPM_GATILHO); 


    /* ==============================================================
     * 2. CONFIGURANDO O CRONÔMETRO (ECHO) - TPM1, Canal 0, Pino PTB0
     * ============================================================== */
    pwm_tpm_Init(TPM1, TPM_OSCERCLK, 0xFFFF, TPM_CLK, PRESCALER, EDGE_PWM);
    pwm_tpm_Ch_Init(TPM1, 0, (TPM_CnSC_ELSA_MASK | TPM_CnSC_ELSB_MASK), GPIOB, 0);
    TPM1->CONTROLS[0].CnSC |= TPM_CnSC_CHIE_MASK;


    /* ==============================================================
     * 3. LIGANDO A INTERRUPÇÃO NO ZEPHYR (SO)
     * ============================================================== */
    // Movido de volta para cá para respeitar as regras de sintaxe da macro do Zephyr
    IRQ_CONNECT(18, 0, tpm1_isr, NULL, 0);
    irq_enable(18);


    /* ==============================================================
     * 4. LOOP INFINITO - Exibição dos Dados
     * ============================================================== */
    while (1) {
        if (nova_leitura == 1) {
            nova_leitura = 0; 

            float tempo_us = duracao_ticks * 2.0f;
            float distancia_cm = tempo_us / 58.0f;

            printk("Alvo detectado a: %.2f cm\n", (double)distancia_cm);
        }
        
        k_msleep(100); 
    }

    return 0;
}