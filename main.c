#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <avr/wdt.h>
#include <avr/pgmspace.h>
#include <string.h>
#include <stdio.h>

#define F_CPU 8000000UL
#define BAUD 9600

#include <util/setbaud.h>

volatile uint32_t m_byte_counter=0;
volatile char m_buf[32];
volatile uint8_t m_command_ready = 0;

ISR(USART_RX_vect)
{
    uint8_t c;
    c = UDR0;
    m_buf[m_byte_counter] = c;
    if(m_byte_counter < sizeof(m_buf) - 2)
    {
	    m_byte_counter++;
    }

    if(c == '\n' || c == '\r')
    {
        m_command_ready = 1;
        m_buf[m_byte_counter] = 0;
        m_byte_counter = 0;
    }
    
	//send received data back
    // no need to wait for empty send buffer
    UDR0 = c;
}

void uart_putchar(char c, FILE *stream) {
    if (c == '\n') {
        uart_putchar('\r', stream);
    }
    loop_until_bit_is_set(UCSR0A, UDRE0);
    UDR0 = c;
}

char uart_getchar(FILE *stream) {
    loop_until_bit_is_set(UCSR0A, RXC0);
    return UDR0;
}

FILE uart_output = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);
FILE uart_input = FDEV_SETUP_STREAM(NULL, uart_getchar, _FDEV_SETUP_READ);

void uart_init(void)
{
    UBRR0H = UBRRH_VALUE;
    UBRR0L = UBRRL_VALUE;

#if USE_2X
    UCSR0A |= _BV(U2X0);
#else
    UCSR0A &= ~(_BV(U2X0));
#endif

    UCSR0C = _BV(UCSZ01) | _BV(UCSZ00);
    UCSR0B = _BV(RXEN0) | _BV(TXEN0) | _BV(RXCIE0);
    
    stdout = &uart_output;
    stdin  = &uart_input;

	sei();
}

void init_io(void)
{
    DDRC |= _BV(PC5);
    DDRC |= _BV(PC4);
    DDRC |= _BV(PC3);
    DDRC |= _BV(PC2);
    DDRC |= _BV(PC1);
    DDRC |= _BV(PC0);
    DDRB |= _BV(PB1);
    DDRB |= _BV(PB2);
}

void relay_out(uint8_t channel, uint8_t state)
{
    if(channel > 7)
        return;
    switch (channel)
    {
        case 0:
        {
            if(state)
                PORTC |= _BV(PC5);
            else
                PORTC &= ~_BV(PC5);
            break;
        }
        case 1:
        {
            if(state)
                PORTC |= _BV(PC4);
            else
                PORTC &= ~_BV(PC4);
            break;
        }
        case 2:
        {
            if(state)
                PORTC |= _BV(PC3);
            else
                PORTC &= ~_BV(PC3);
            break;
        }
        case 3:
        {
            if(state)
                PORTC |= _BV(PC2);
            else
                PORTC &= ~_BV(PC2);
            break;
        }
        case 4:
        {
            if(state)
                PORTC |= _BV(PC1);
            else
                PORTC &= ~_BV(PC1);
            break;
        }
        case 5:
        {
            if(state)
                PORTC |= _BV(PC0);
            else
                PORTC &= ~_BV(PC0);
            break;
        }
        case 6:
        {
            if(state)
                PORTB |= _BV(PB1);
            else
                PORTB &= ~_BV(PB1);
            break;
        }
        case 7:
        {
            if(state)
                PORTB |= _BV(PB2);
            else
                PORTB &= ~_BV(PB2);
            break;
        }
    }
}

int main (void)
{
    init_io();
    uart_init();
    puts("AVR online");
    while (1)
    {
        // Execute command
        if(m_command_ready)
        {
            m_command_ready = 0;
            //puts("\nGot:"); 
            //puts(m_buf); 
            if(m_buf[0] == 'R' || m_buf[0] == 'r')
            {
                // ASCII to DEC
                uint8_t channel = m_buf[1] - 48;
                if(channel > 0 && channel < 9 && m_buf[2] == '=')
                {
                    relay_out(channel - 1, m_buf[3] - 48);
                }
            }
        }
       
        /*
        _delay_ms(5000);
        puts("loop");
        */
    }
    
    return (0);
}
