#include "stm32f4xx.h"

// ===== UART (USART2 on PA2=TX, PA3=RX) =====
void UART2_Init(uint32_t baud) {
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

    // PA2 (TX), PA3 (RX) -> AF7 (USART2)
    GPIOA->MODER &= ~((3 << (2*2)) | (3 << (3*2)));
    GPIOA->MODER |=  (2 << (2*2)) | (2 << (3*2)); // AF mode
    GPIOA->AFR[0] &= ~((0xF << (2*4)) | (0xF << (3*4)));
    GPIOA->AFR[0] |= (7 << (2*4)) | (7 << (3*4));

    // Note: APB1 clock assumptions same as before
    USART2->BRR = SystemCoreClock / 2 / baud; // APB1 = SystemCoreClock/2 typical
    USART2->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
}

void UART2_SendChar(char c) {
    while (!(USART2->SR & USART_SR_TXE));
    USART2->DR = (uint8_t)c;
}

char UART2_RecvChar() {
    while (!(USART2->SR & USART_SR_RXNE));
    return (char)USART2->DR;
}

// ===== SPI (SPI1 on PA5=SCK, PA6=MISO, PA7=MOSI) =====
// Using PB6 as CS (active low) and PB7 as DC (data/command). Adjust pins if needed.
void SPI1_Init(void) {
    // Enable clocks
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN;
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

    // Configure PA5, PA6, PA7 -> AF5 (SPI1)
    // Set to AF mode:
    GPIOA->MODER &= ~((3 << (5*2)) | (3 << (6*2)) | (3 << (7*2)));
    GPIOA->MODER |=  (2 << (5*2)) | (2 << (6*2)) | (2 << (7*2));
    // AF5 = SPI1
    GPIOA->AFR[0] &= ~((0xF << (5*4)) | (0xF << (6*4)) | (0xF << (7*4)));
    GPIOA->AFR[0] |= (5 << (5*4)) | (5 << (6*4)) | (5 << (7*4));

    // Set alternate function pins to high speed (optional but common for SPI)
    GPIOA->OSPEEDR |= (3 << (5*2)) | (3 << (6*2)) | (3 << (7*2));

    // Configure PB6 (CS) and PB7 (DC) as outputs, push-pull
    GPIOB->MODER &= ~((3 << (6*2)) | (3 << (7*2)));
    GPIOB->MODER |=  (1 << (6*2)) | (1 << (7*2)); // General purpose output
    GPIOB->OTYPER &= ~((1<<6) | (1<<7)); // push-pull
    GPIOB->OSPEEDR |= (3 << (6*2)) | (3 << (7*2));
    // Default: CS high (inactive), DC low
    GPIOB->BSRR = (1 << (6+16)) | (1 << (7)); // set PB6=0? wait: BSRR set bit -> set, bit+16 -> reset
    // simpler: set CS high, DC low:
    GPIOB->BSRR = (1 << 6);           // set PB6 = 1 (CS inactive)
    GPIOB->BSRR = (1 << (7+16));      // reset PB7 = 0 (DC low)

    // Configure SPI1:
    // Master, Software NSS management, set SSI, baud rate = fPCLK/16, 8-bit, CPOL=0 CPHA=0 (mode 0)
    SPI1->CR1 = SPI_CR1_MSTR    // Master mode
             | SPI_CR1_SSM     // Software slave management
             | SPI_CR1_SSI     // Internal slave select = 1
             | (3 << 3);       // BR[2:0] = 011 => fPCLK/16 (approx moderate speed)
    // 8-bit data (DFF=0), MSB first (default)
    // Enable SPI
    SPI1->CR1 |= SPI_CR1_SPE;
}

// Assert CS (active low)
static inline void SPI_CS_Assert(void)  { GPIOB->BSRR = (1 << (6+16)); } // reset PB6 -> 0
// Deassert CS
static inline void SPI_CS_Release(void) { GPIOB->BSRR = (1 << 6); }       // set PB6 -> 1

// DC controls
static inline void SPI_DC_Data(void)    { GPIOB->BSRR = (1 << 7); }       // PB7 = 1
static inline void SPI_DC_Command(void) { GPIOB->BSRR = (1 << (7+16)); }  // PB7 = 0

// Transmit one byte over SPI (blocking)
void SPI1_TransmitByte(uint8_t b) {
    // Wait until TXE (data register empty)
    while (!(SPI1->SR & SPI_SR_TXE));
    *((__IO uint8_t *)&SPI1->DR) = b; // write byte to data register (8-bit)
    // Wait until transmission complete (TXE=1 and BSY=0)
    while (!(SPI1->SR & SPI_SR_TXE));
    while (SPI1->SR & SPI_SR_BSY);
}

// Send a buffer with CS toggled once for full buffer
void SPI1_SendBuffer(const uint8_t *buf, int len) {
    SPI_CS_Assert();
    for (int i = 0; i < len; ++i) {
        SPI1_TransmitByte(buf[i]);
    }
    SPI_CS_Release();
}

// Small helper: send a single data byte (DC=1)
void SPI_SendDataByte(uint8_t b) {
    SPI_CS_Assert();
    SPI_DC_Data();
    SPI1_TransmitByte(b);
    SPI_CS_Release();
}

// Send an ASCII string as data bytes (CS toggled per string)
void SPI_SendString(const char *s) {
    SPI_CS_Assert();
    SPI_DC_Data();
    while (*s) {
        SPI1_TransmitByte((uint8_t)*s++);
    }
    SPI_CS_Release();
}

// ===== Main =====
int main(void) {
    SystemInit();
    UART2_Init(9600);
    SPI1_Init();

    // simple prompt
    const char *p = "Ready\r\n";
    for (const char *q = p; *q; ++q) UART2_SendChar(*q);

    // echo loop — receive characters and forward to SPI as data
    while (1) {
        char c = UART2_RecvChar();   // blocking receive
        UART2_SendChar(c);           // echo back to PC

        // If you want to accumulate into strings until newline, you could buffer here.
        // For simplicity we send each received character as SPI data:
        SPI_SendDataByte((uint8_t)c);
    }
}
