#include "stm32l476xx.h"
#include "delay.h"
#include "spi.h"
#include "nrf24.h"
#include <string.h>
#include <stdio.h>

// --- CONFIGURATION ---
uint8_t RxAddress[] = {0xEE, 0xDD, 0xCC, 0xBB, 0xAA};
uint8_t RxData[32];

// --- FONCTIONS UART (BARE-METAL) ---

void uart_init(void) {
    // 1. Activation des horloges : USART2 (APB1) et GPIOA (AHB2)
    RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN;
    RCC->AHB2ENR  |= RCC_AHB2ENR_GPIOAEN;

    // 2. Configuration de PA2 (TX) et PA3 (RX) en mode Fonction Alterne (10)
    GPIOA->MODER &= ~(GPIO_MODER_MODE2 | GPIO_MODER_MODE3);
    GPIOA->MODER |= (GPIO_MODER_MODE2_1 | GPIO_MODER_MODE3_1);

    // 3. Configuration de la fonction alterne AF7 (pour USART2)
    // Chaque broche a 4 bits dans AFR. PA2 = bits 8-11, PA3 = bits 12-15.
    GPIOA->AFR[0] &= ~((0xF << 8) | (0xF << 12)); // Nettoyage
    GPIOA->AFR[0] |=  ((7 << 8) | (7 << 12));    // AF7 = 0111 binaire

    // 4. Configuration du Baud Rate (Vitesse)
    // Pour 9600 baud avec une horloge de 4MHz : 4 000 000 / 9 600 = 416.66
    USART2->BRR = 417;

    // 5. Activation du périphérique (Transmission, Réception, UART Enable)
    USART2->CR1 |= (USART_CR1_TE | USART_CR1_RE | USART_CR1_UE);
}

void uart_send_char(char c) {
    // Attendre que le registre de transmission soit vide (TXE)
    while (!(USART2->ISR & USART_ISR_TXE));
    USART2->TDR = c;
}

void uart_send_string(char* str) {
    while (*str) {
        uart_send_char(*str++);
    }
}

// --- INITIALISATION LED ---

void gpio_led_init(void) {
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
    GPIOA->MODER &= ~GPIO_MODER_MODE5;
    GPIOA->MODER |= GPIO_MODER_MODE5_0; // Mode Sortie
}

// --- PROGRAMME PRINCIPAL ---

int main(void) {
    // 1. Initialisations de base
    delay_init(4000000); // Horloge à 4MHz
    gpio_led_init();
    uart_init();        // L'UART est maintenant prêt
    spi_init();
    NRF24_Init();

    // 2. Configuration du nRF24 en mode Récepteur
    NRF24_RxMode(RxAddress, 10); // Canal 10

    uart_send_string("\r\n--- SYSTEME RX PRET ---\r\n");
    uart_send_string("En attente de donnees du compteur...\r\n");

    while (1) {
        // 3. Vérification de la disponibilité des données (Pipe 1)
        if (isDataAvailable(1)) {

            // 4. Lecture du message radio
            memset(RxData, 0, 32); // Nettoyer le buffer avant lecture
            NRF24_Receive(RxData);

            // 5. Affichage sur la console PC
            uart_send_string("Recu par Radio : [");
            uart_send_string((char*)RxData);
            uart_send_string("]\r\n");

            // 6. Signal visuel : Clignotement de la LED PA5
            GPIOA->ODR ^= GPIO_ODR_OD5;
        }

        // 7. Petit délai pour ne pas saturer le processeur
        delay(100); // 10ms (en microsecondes selon ta lib delay)
    }
}
