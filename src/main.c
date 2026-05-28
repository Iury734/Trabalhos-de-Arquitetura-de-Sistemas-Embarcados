#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

// Macros para facilitar a inversão de lógica caso sua placa seja Active-Low bruto.
// Se as cores apagarem quando deveriam ligar, inverta: LIGADO 0 e DESLIGADO 1.
#define LED_LIGADO 1
#define LED_DESLIGADO 0

// O mapeamento definitivo baseado nos seus testes físicos:
static const struct gpio_dt_spec Led_Vermelho = GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios); // led2 é o Vermelho
static const struct gpio_dt_spec Led_Verde = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);    // led0 é o Verde
static const struct gpio_dt_spec Led_Azul = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);     // led1 é o Azul

// Definição dos estados do semáforo na nova ordem solicitada
typedef enum {
    Estado_Verde,
    Estado_Amarelo,
    Estado_Vermelho
} Estado_Semaforo;

void main(void)
{
    // 1. Verificação se os dispositivos (pinos) estão prontos
    if (!gpio_is_ready_dt(&Led_Vermelho) || 
        !gpio_is_ready_dt(&Led_Verde) || 
        !gpio_is_ready_dt(&Led_Azul)) {
        return;
    }

    // 2. Configuração dos pinos como saída e inicialização em estado inativo (desligados)
    gpio_pin_configure_dt(&Led_Vermelho, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&Led_Verde, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&Led_Azul, GPIO_OUTPUT_INACTIVE);

    // 3. Inicializa a máquina de estados no Verde (primeira cor)
    Estado_Semaforo Estado_Atual = Estado_Verde;

    // Loop infinito do RTOS
    while (1) {
        
        // 4. Máquina de Estados para o Semáforo
        switch (Estado_Atual) {
            
            case Estado_Verde:
                // Liga apenas o Verde
                gpio_pin_set_dt(&Led_Vermelho, LED_DESLIGADO);
                gpio_pin_set_dt(&Led_Verde, LED_LIGADO);
                gpio_pin_set_dt(&Led_Azul, LED_DESLIGADO);
                
                k_msleep(3000); // Fica verde por 3 segundos
                
                // Transição: Verde -> Amarelo
                Estado_Atual = Estado_Amarelo;
                break;

            case Estado_Amarelo:
                // Liga Vermelho e Verde juntos para formar Amarelo (Garantindo que Azul está fora)
                gpio_pin_set_dt(&Led_Vermelho, LED_LIGADO);
                gpio_pin_set_dt(&Led_Verde, LED_LIGADO);
                gpio_pin_set_dt(&Led_Azul, LED_DESLIGADO);
                
                k_msleep(1000); // Fica amarelo por 1 segundo
                
                // Transição: Amarelo -> Vermelho
                Estado_Atual = Estado_Vermelho;
                break;

            case Estado_Vermelho:
                // Liga apenas o Vermelho
                gpio_pin_set_dt(&Led_Vermelho, LED_LIGADO);
                gpio_pin_set_dt(&Led_Verde, LED_DESLIGADO);
                gpio_pin_set_dt(&Led_Azul, LED_DESLIGADO);
                
                k_msleep(3000); // Fica vermelho por 3 segundos
                
                // Transição: Vermelho -> Verde
                Estado_Atual = Estado_Verde;
                break;
        }
    }
}