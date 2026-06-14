#ifndef UART_H
#define UART_H

#include <stdint.h>

#define UART_BAUD   115200U

void uart_init(void);
void uart_send_byte(uint8_t b);
void uart_send_frame(uint16_t rpm, uint8_t duty, uint8_t state);

/* CRC-8/MAXIM (Dallas 1-Wire): poly = 0x31, init = 0x00 */
uint8_t crc8(const uint8_t *data, uint8_t len);

#endif /* UART_H */
