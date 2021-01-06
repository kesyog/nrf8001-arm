/* Copyright (c) 2014, Nordic Semiconductor ASA
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/** @file
@brief Implementation of the ACI transport layer module
*/

#ifndef __arm__
#include <SPI.h>
#endif
#include <stdbool.h>
#if INCLUDE_DEBUG_STATEMENTS
#include <debug.h>
#endif
#include "spi.h"
#include "hal_platform.h"
#include "hal_aci_tl.h"
#include "aci_queue.h"
#include "io_support.h"

#if ( !defined(__SAM3X8E__) && !defined(__PIC32MX__) && !defined(__arm__))
#include <avr/sleep.h>
#endif
/*
PIC32 supports only MSbit transfer on SPI and the nRF8001 uses LSBit
Use the REVERSE_BITS macro to convert from MSBit to LSBit
The outgoing command and the incoming event needs to be converted
*/
//Board dependent defines
#if defined (__AVR__)
    //For Arduino add nothing
#elif defined(__PIC32MX__)
    //For ChipKit as the transmission has to be reversed, the next definitions have to be added
    #define REVERSE_BITS(byte) (((reverse_lookup[(byte & 0x0F)]) << 4) + reverse_lookup[((byte & 0xF0) >> 4)])
    static const uint8_t reverse_lookup[] = { 0, 8,  4, 12, 2, 10, 6, 14,1, 9, 5, 13,3, 11, 7, 15 };
#endif

static void m_aci_data_print(hal_aci_data_t *p_data);
static void m_aci_event_check(void);
#ifndef __arm__
static void m_aci_isr(void);
#endif
static void m_aci_pins_set(aci_pins_t *a_pins_ptr);
static inline void m_aci_reqn_disable (void);
static inline void m_aci_reqn_enable (void);
static void m_aci_q_flush(void);
static bool m_aci_spi_transfer(hal_aci_data_t * data_to_send, hal_aci_data_t * received_data);

static uint8_t        spi_readwrite(uint8_t aci_byte);

static bool           aci_debug_print = false;

aci_queue_t    aci_tx_q;
aci_queue_t    aci_rx_q;

static aci_pins_t	 *a_pins_local_ptr;

void m_aci_data_print(hal_aci_data_t *p_data)
{
#ifndef __arm__
  const uint8_t length = p_data->buffer[0];
  Serial.print(length, DEC);
  Serial.print(" :");
  for (uint8_t i=0; i<=length; i++)
  {
    Serial.print(p_data->buffer[i], HEX);
    Serial.print(F(", "));
  }
  Serial.println(F(""));
#endif
}

#ifndef __arm__
/*
  Interrupt service routine called when the RDYN line goes low. Runs the SPI transfer.
*/
static void m_aci_isr(void)
{
  hal_aci_data_t data_to_send;
  hal_aci_data_t received_data;

  // Receive from queue
  if (!aci_queue_dequeue_from_isr(&aci_tx_q, &data_to_send))
  {
    /* queue was empty, nothing to send */
    data_to_send.status_byte = 0;
    data_to_send.buffer[0] = 0;
  }

  // Receive and/or transmit data
  m_aci_spi_transfer(&data_to_send, &received_data);

  if (!aci_queue_is_full_from_isr(&aci_rx_q) && !aci_queue_is_empty_from_isr(&aci_tx_q))
  {
    m_aci_reqn_enable();
  }

  // Check if we received data
  if (received_data.buffer[0] > 0)
  {
    if (!aci_queue_enqueue_from_isr(&aci_rx_q, &received_data))
    {
      /* Receive Buffer full.
         Should never happen.
         Spin in a while loop.
      */
      while(1);
    }

    // Disable ready line interrupt until we have room to store incoming messages
    if (aci_queue_is_full_from_isr(&aci_rx_q))
    {
      detachInterrupt(a_pins_local_ptr->interrupt_number);
    }
  }

  return;
}
#endif

/*
  Checks the RDYN line and runs the SPI transfer if required.
*/
static void m_aci_event_check(void)
{
  hal_aci_data_t data_to_send;
  hal_aci_data_t received_data;

  // No room to store incoming messages
  if (aci_queue_is_full(&aci_rx_q))
  {
    return;
  }

  // If the ready line is disabled and we have pending messages outgoing we enable the request line
  if (HIGH == digitalRead(a_pins_local_ptr->rdyn_pin))
  {
    if (!aci_queue_is_empty(&aci_tx_q))
    {
      m_aci_reqn_enable();
    }

    return;
  }

  // Receive from queue
  if (!aci_queue_dequeue(&aci_tx_q, &data_to_send))
  {
#if INCLUDE_DEBUG_STATEMENTS
    log_info("aci_queue_dequeue returned empty queue\r\n");
#endif
    /* queue was empty, nothing to send */
    data_to_send.status_byte = 0;
    data_to_send.buffer[0] = 0;
  }

  // Receive and/or transmit data
#if INCLUDE_DEBUG_STATEMENTS
  //log_info("m_aci_spi_transfer invoked control loop\r\n");
#endif
  m_aci_spi_transfer(&data_to_send, &received_data);

  /* If there are messages to transmit, and we can store the reply, we request a new transfer */
  if (!aci_queue_is_full(&aci_rx_q) && !aci_queue_is_empty(&aci_tx_q))
  {
    m_aci_reqn_enable();
  }

  // Check if we received data
  if (received_data.buffer[0] > 0)
  {
    if (!aci_queue_enqueue(&aci_rx_q, &received_data))
    {
      /* Receive Buffer full.
         Should never happen.
         Spin in a while loop.
      */
      while(1);
    }
  }

  return;
}

/** @brief Point the low level library at the ACI pins specified
 *  @details
 *  The ACI pins are specified in the application and a pointer is made available for
 *  the low level library to use
 */
static void m_aci_pins_set(aci_pins_t *a_pins_ptr)
{
  a_pins_local_ptr = a_pins_ptr;
}

static inline void m_aci_reqn_disable (void)
{
  digitalWrite(a_pins_local_ptr->reqn_pin, 1);
}

static inline void m_aci_reqn_enable (void)
{
  digitalWrite(a_pins_local_ptr->reqn_pin, 0);
}

static void m_aci_q_flush(void)
{
#ifndef __arm__
  noInterrupts();
#endif
  /* re-initialize aci cmd queue and aci event queue to flush them*/
  aci_queue_init(&aci_tx_q);
  aci_queue_init(&aci_rx_q);
#ifndef __arm__
  interrupts();
#endif
}

static bool m_aci_spi_transfer(hal_aci_data_t * data_to_send, hal_aci_data_t * received_data)
{
  uint8_t byte_cnt;
  uint8_t byte_sent_cnt;
  uint8_t max_bytes;

  m_aci_reqn_enable();

#if INCLUDE_DEBUG_STATEMENTS
  printf("data_to_send->buffer[0] = %02x\r\n", data_to_send->buffer[0]);
  printf("data_to_send->buffer[1] = %02x\r\n", data_to_send->buffer[1]);
#endif

  // Send length, receive header
  byte_sent_cnt = 0;
  received_data->status_byte = spi_readwrite(data_to_send->buffer[byte_sent_cnt++]);
  // Send first byte, receive length from slave
  received_data->buffer[0] = spi_readwrite(data_to_send->buffer[byte_sent_cnt++]);
  if (0 == data_to_send->buffer[0])
  {
    max_bytes = received_data->buffer[0];
  }
  else
  {
    // Set the maximum to the biggest size. One command byte is already sent
    max_bytes = (received_data->buffer[0] > (data_to_send->buffer[0] - 1))
                                          ? received_data->buffer[0]
                                          : (data_to_send->buffer[0] - 1);
  }

  if (max_bytes > HAL_ACI_MAX_LENGTH)
  {
    max_bytes = HAL_ACI_MAX_LENGTH;
  }

#if INCLUDE_DEBUG_STATEMENTS
  //printf("m_aci_spi_transfer, max_bytes = %d\r\n", max_bytes);
#endif
  // Transmit/receive the rest of the packet
  for (byte_cnt = 0; byte_cnt < max_bytes; byte_cnt++)
  {
#if INCLUDE_DEBUG_STATEMENTS
    //printf("%x:", data_to_send->buffer[byte_sent_cnt]);
#endif
    received_data->buffer[byte_cnt+1] =  spi_readwrite(data_to_send->buffer[byte_sent_cnt++]);
  }
#if INCLUDE_DEBUG_STATEMENTS
  //printf("\r\n");
#endif

  // RDYN should follow the REQN line in approx 100ns
  m_aci_reqn_disable();

  return (max_bytes > 0);
}

void hal_aci_tl_debug_print(bool enable)
{
	aci_debug_print = enable;
}

void hal_aci_tl_pin_reset(void)
{
    if (UNUSED != a_pins_local_ptr->reset_pin)
    {
        pinMode(a_pins_local_ptr->reset_pin, OUTPUT);

        if ((REDBEARLAB_SHIELD_V1_1     == a_pins_local_ptr->board_name) ||
            (REDBEARLAB_SHIELD_V2012_07 == a_pins_local_ptr->board_name))
        {
            //The reset for the Redbearlab v1.1 and v2012.07 boards are inverted and has a Power On Reset
            //circuit that takes about 100ms to trigger the reset
            digitalWrite(a_pins_local_ptr->reset_pin, 1);
            delay(100);
            digitalWrite(a_pins_local_ptr->reset_pin, 0);
        }
        else
        {
#if INCLUDE_DEBUG_STATEMENTS
            log_info("attempting reset toggle\r\n");
#endif
            digitalWrite(a_pins_local_ptr->reset_pin, 1);
            digitalWrite(a_pins_local_ptr->reset_pin, 0);
            digitalWrite(a_pins_local_ptr->reset_pin, 1);
        }
    }
}

bool hal_aci_tl_event_peek(hal_aci_data_t *p_aci_data)
{
#ifdef __arm__
  m_aci_event_check();
#else
  if (!a_pins_local_ptr->interface_is_interrupt)
  {
    m_aci_event_check();
  }
#endif


  if (aci_queue_peek(&aci_rx_q, p_aci_data))
  {
    return true;
  }

  return false;
}

bool hal_aci_tl_event_get(hal_aci_data_t *p_aci_data)
{
#ifdef __arm__
  if (!aci_queue_is_full(&aci_rx_q))
  {
    m_aci_event_check();
  }
#else
  if (!a_pins_local_ptr->interface_is_interrupt && !aci_queue_is_full(&aci_rx_q))
  {
    m_aci_event_check();
  }
#endif

#ifndef __arm__
  bool was_full = aci_queue_is_full(&aci_rx_q);
#endif

  if (aci_queue_dequeue(&aci_rx_q, p_aci_data))
  {
    if (aci_debug_print)
    {
#ifndef __arm__
      Serial.print(" E");
#endif
      m_aci_data_print(p_aci_data);
    }

#ifndef __arm__
    if (was_full && a_pins_local_ptr->interface_is_interrupt)
	  {
      /* Enable RDY line interrupt again */
      attachInterrupt(a_pins_local_ptr->interrupt_number, m_aci_isr, LOW);
    }
#endif

    /* Attempt to pull REQN LOW since we've made room for new messages */
    if (!aci_queue_is_full(&aci_rx_q) && !aci_queue_is_empty(&aci_tx_q))
    {
      m_aci_reqn_enable();
    }

    return true;
  }

  return false;
}

void hal_aci_tl_init(aci_pins_t *a_pins, bool debug)
{
  aci_debug_print = debug;

  /* Needs to be called as the first thing for proper intialization*/
  m_aci_pins_set(a_pins);

  /*
  The SPI lines used are mapped directly to the hardware SPI
  MISO MOSI and SCK
  Change here if the pins are mapped differently

  The SPI library assumes that the hardware pins are used
  */

#ifndef __arm__
  SPI.begin();
  //Board dependent defines
  #if defined (__AVR__)
    //For Arduino use the LSB first
    SPI.setBitOrder(LSBFIRST);
  #elif defined(__PIC32MX__)
    //For ChipKit use MSBFIRST and REVERSE the bits on the SPI as LSBFIRST is not supported
    SPI.setBitOrder(MSBFIRST);
  #endif
  SPI.setClockDivider(a_pins->spi_clock_divider);
  SPI.setDataMode(SPI_MODE0);
#endif

  /* Initialize the ACI Command queue. This must be called after the delay above. */
  aci_queue_init(&aci_tx_q);
  aci_queue_init(&aci_rx_q);

  //Configure the IO lines
  pinMode(a_pins->rdyn_pin,		INPUT_PULLUP);
  pinMode(a_pins->reqn_pin,		OUTPUT);

  if (UNUSED != a_pins->active_pin)
  {
    pinMode(a_pins->active_pin,	INPUT);
  }
  /* Pin reset the nRF8001, required when the nRF8001 setup is being changed */
  hal_aci_tl_pin_reset();

  /* Set the nRF8001 to a known state as required by the datasheet*/

#ifndef __arm__
  digitalWrite(a_pins->miso_pin, 0);
  digitalWrite(a_pins->mosi_pin, 0);
  digitalWrite(a_pins->reqn_pin, 1);
  digitalWrite(a_pins->sck_pin,  0);
#endif
  digitalWrite(a_pins->reqn_pin, 1);

  delay(30); //Wait for the nRF8001 to get hold of its lines - the lines float for a few ms after the reset

  /* Attach the interrupt to the RDYN line as requested by the caller */
#ifndef __arm__
  if (a_pins->interface_is_interrupt)
  {
    // We use the LOW level of the RDYN line as the atmega328 can wakeup from sleep only on LOW
    attachInterrupt(a_pins->interrupt_number, m_aci_isr, LOW);
  }
#endif
}

bool hal_aci_tl_send(hal_aci_data_t *p_aci_cmd)
{
  const uint8_t length = p_aci_cmd->buffer[0];
  bool ret_val = false;

  if (length > HAL_ACI_MAX_LENGTH)
  {
    return false;
  }

  ret_val = aci_queue_enqueue(&aci_tx_q, p_aci_cmd);
  if (ret_val)
  {
    if(!aci_queue_is_full(&aci_rx_q))
    {
      // Lower the REQN only when successfully enqueued
      m_aci_reqn_enable();
    }

    if (aci_debug_print)
    {
#ifndef __arm__
      Serial.print("C"); //ACI Command
#endif
      m_aci_data_print(p_aci_cmd);
    }
  }

  return ret_val;
}

static uint8_t spi_readwrite(const uint8_t aci_byte)
{
	//Board dependent defines
#if defined (__AVR__)
  //For Arduino the transmission does not have to be reversed
  return SPI.transfer(aci_byte);
#elif defined(__PIC32MX__)
  //For ChipKit the transmission has to be reversed
  uint8_t tmp_bits;
  tmp_bits = SPI.transfer(REVERSE_BITS(aci_byte));
	return REVERSE_BITS(tmp_bits);
#elif defined(__arm__)
  uint8_t send_byte = aci_byte;
  uint8_t ret_byte;
  transmit_SPI(NRF8001_SPI, (uint8_t *) &send_byte, &ret_byte, 1);
#if INCLUDE_DEBUG_STATEMENTS
  //log_info("transmit_SPI with write = %x, read = %x\r\n", send_byte, ret_byte);
#endif
    return ret_byte;
#endif
}

bool hal_aci_tl_rx_q_empty (void)
{
  return aci_queue_is_empty(&aci_rx_q);
}

bool hal_aci_tl_rx_q_full (void)
{
  return aci_queue_is_full(&aci_rx_q);
}

bool hal_aci_tl_tx_q_empty (void)
{
  return aci_queue_is_empty(&aci_tx_q);
}

bool hal_aci_tl_tx_q_full (void)
{
  return aci_queue_is_full(&aci_tx_q);
}

void hal_aci_tl_q_flush (void)
{
  m_aci_q_flush();
}
