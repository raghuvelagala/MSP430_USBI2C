#ifndef USCI_LIB
#define USCI_LIB

#define SDA_PIN 0x02     // UCB0SDA pin on PORT3
#define SCL_PIN 0x04     // UCB0SCL pin on PORT3

void TI_USCI_I2C_receive(unsigned char byteCount, unsigned char *field);
void TI_USCI_I2C_transmit(unsigned char byteCount, unsigned char *field);

/* send start condition after transfer finishes instead of stop condition */
void TI_USCI_I2C_restart(void);
// avoid sending stop condition after next transfer
void TI_USCI_I2C_hold(void);

/* Call from USCIAB0TX_VECTOR: */
void TI_USCI_I2C_rx_tx_isr(void);

#endif
