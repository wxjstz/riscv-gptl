/*
 * This file is part of the coreboot project.
 *
 * Copyright 2018 SiFive, Inc
 * Copyright (C) 2019 HardenedLinux
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <spi.h>
#include <sd.h>
#include <clock.h>

#define SPI_DIR_RX							0
#define SPI_DIR_TX							1

#define SPI_ENDIAN_MSB					0
#define SPI_ENDIAN_LSB					1

#define SPI_CSMODE_AUTO				 0
#define SPI_CSMODE_HOLD				 2
#define SPI_CSMODE_OFF					3

#define SPI_PROTO_S						 0
#define SPI_PROTO_D						 1
#define SPI_PROTO_Q						 2

#define SD_CMD_GO_IDLE_STATE 0
#define SD_CMD_SEND_IF_COND 8
#define SD_CMD_STOP_TRANSMISSION 12
#define SD_CMD_SET_BLOCKLEN 16
#define SD_CMD_READ_BLOCK_MULTIPLE 18
#define SD_CMD_APP_SEND_OP_COND 41
#define SD_CMD_APP_CMD 55
#define SD_CMD_READ_OCR 58
#define SD_RESPONSE_IDLE 0x1
// Data token for commands 17, 18, 24
#define SD_DATA_TOKEN 0xfe

// SD card initialization must happen at 100-400kHz
#define SD_POWER_ON_FREQ_KHZ 400L
// SD cards normally support reading/writing at 20MHz
#define SD_POST_INIT_CLK_KHZ 20000L

// Command frame starts by asserting low and then high for first two clock edges
#define SD_CMD(cmd) (0x40 | (cmd))

/**
 * Send dummy byte (all ones).
 *
 * Used in many cases to read one byte from SD card, since SPI is a full-duplex
 * protocol and it is necessary to send a byte in order to read a byte.
 */
static inline uint8_t sd_dummy(volatile struct spi_ctrl *spi)
{
    return spi_txrx(spi, 0xFF);
}

static int
sd_cmd(volatile struct spi_ctrl *spi, uint8_t cmd, uint32_t arg,
       uint8_t crc)
{
    unsigned long n;
    uint8_t r;

    spi->csmode.mode = SPI_CSMODE_HOLD;
    sd_dummy(spi);
    spi_txrx(spi, cmd);
    spi_txrx(spi, arg >> 24);
    spi_txrx(spi, arg >> 16);
    spi_txrx(spi, arg >> 8);
    spi_txrx(spi, arg);
    spi_txrx(spi, crc);

    n = 1000;
    do {
        r = sd_dummy(spi);
        if (!(r & 0x80))
            break;
    }
    while (--n > 0);
    return r;
}

static inline void sd_cmd_end(volatile struct spi_ctrl *spi)
{
    sd_dummy(spi);
    spi->csmode.mode = SPI_CSMODE_AUTO;
}

static void
sd_poweron(volatile struct spi_ctrl *spi, unsigned int input_clk_khz)
{
    // Initialize SPI controller
    spi->fmt.raw_bits = ((spi_reg_fmt) {
                         .proto = SPI_PROTO_S,.dir = SPI_DIR_RX,.endian =
                         SPI_ENDIAN_MSB,.len = 8,}).raw_bits;
    spi->csdef |= 0x1;
    spi->csid = 0;

    spi->sckdiv = spi_min_clk_divisor(input_clk_khz, SD_POWER_ON_FREQ_KHZ);
    spi->csmode.mode = SPI_CSMODE_OFF;

    for (int i = 10; i > 0; i--) {
        // Set SD card pin DI high for 74+ cycles
        // SD CMD pin is the same pin as SPI DI, so CMD = 0xff means
        // assert DI high for 8 cycles.
        sd_dummy(spi);
    }

    spi->csmode.mode = SPI_CSMODE_AUTO;
}

/**
 * Reset card.
 */
static int sd_cmd0(volatile struct spi_ctrl *spi)
{
    int rc;
    rc = (sd_cmd(spi, SD_CMD(SD_CMD_GO_IDLE_STATE), 0, 0x95)
          != SD_RESPONSE_IDLE);
    sd_cmd_end(spi);
    return rc;
}

/**
 * Check for SD version and supported voltages.
 */
static int sd_cmd8(volatile struct spi_ctrl *spi)
{
    // Check for high capacity cards
    // Fail if card does not support SDHC
    int rc;
    rc = (sd_cmd(spi, SD_CMD(SD_CMD_SEND_IF_COND), 0x000001AA, 0x87)
          != SD_RESPONSE_IDLE);
    sd_dummy(spi);              /* command version; reserved */
    sd_dummy(spi);              /* reserved */
    rc |= ((sd_dummy(spi) & 0xF) != 0x1);       /* voltage */
    rc |= (sd_dummy(spi) != 0xAA);      /* check pattern */
    sd_cmd_end(spi);
    return rc;
}

/**
 * Send app command. Used as prefix to app commands (ACMD).
 */
static void sd_cmd55(volatile struct spi_ctrl *spi)
{
    sd_cmd(spi, SD_CMD(SD_CMD_APP_CMD), 0, 0x65);
    sd_cmd_end(spi);
}

/**
 * Start SDC initialization process.
 */
static int sd_acmd41(volatile struct spi_ctrl *spi)
{
    uint8_t r;
    do {
        sd_cmd55(spi);
        r = sd_cmd(spi, SD_CMD(SD_CMD_APP_SEND_OP_COND), 0x40000000, 0x77);
        sd_cmd_end(spi);
    }
    while (r == SD_RESPONSE_IDLE);
    return (r != 0x00);
}

/**
 * Read operation conditions register (OCR) to check for availability of block
 * addressing mode.
 */
static int sd_cmd58(volatile struct spi_ctrl *spi)
{
    // HACK: Disabled due to bugs. It is not strictly necessary to issue
    // this command if we only support SD cards that support SDHC mode.
    return 0;
    // int rc;
    // rc = (sd_cmd(spi, SD_CMD(SD_CMD_READ_OCR), 0, 0xFD) != 0x00);
    // rc |= ((sd_dummy(spi) & 0x80) != 0x80); /* Power up status */
    // sd_dummy(spi); /* Supported voltages */
    // sd_dummy(spi); /* Supported voltages */
    // sd_dummy(spi); /* Supported voltages */
    // sd_cmd_end(spi);
    // return rc;
}

/**
 * Set block addressing mode.
 */
static int sd_cmd16(volatile struct spi_ctrl *spi)
{
    int rc;
    rc = (sd_cmd(spi, SD_CMD(SD_CMD_SET_BLOCKLEN), 0x200, 0x15) != 0x00);
    sd_cmd_end(spi);
    return rc;
}

static uint8_t crc7(uint8_t prev, uint8_t in)
{
    // CRC polynomial 0x89
    uint8_t remainder = prev & in;
    remainder ^= (remainder >> 4) ^ (remainder >> 7);
    remainder ^= remainder << 4;
    return remainder & 0x7f;
}

static uint16_t crc16(uint16_t crc, uint8_t data)
{
    // CRC polynomial 0x11021
    crc = (uint8_t) (crc >> 8) | (crc << 8);
    crc ^= data;
    crc ^= (uint8_t) (crc >> 4) & 0xf;
    crc ^= crc << 12;
    crc ^= (crc & 0xff) << 5;
    return crc;
}

int sd_init(volatile struct spi_ctrl *spi)
{
    unsigned int input_clk_khz = clock_get_tlclk_khz();
    sd_poweron(spi, input_clk_khz);
    if (sd_cmd0(spi))
        return SD_INIT_ERROR_CMD0;
    if (sd_cmd8(spi))
        return SD_INIT_ERROR_CMD8;
    if (sd_acmd41(spi))
        return SD_INIT_ERROR_ACMD41;
    if (sd_cmd58(spi))
        return SD_INIT_ERROR_CMD58;
    if (sd_cmd16(spi))
        return SD_INIT_ERROR_CMD16;

    // Increase clock frequency after initialization for higher performance.
    spi->sckdiv = spi_min_clk_divisor(input_clk_khz, SD_POST_INIT_CLK_KHZ);
    return 0;
}

int
sd_copy(volatile struct spi_ctrl *spi, void *dst, uint32_t src_lba,
        size_t size)
{
    volatile uint8_t *p = dst;
    long i = size;
    int rc = 0;

    uint8_t c = 0;
    c = crc7(c, SD_CMD(SD_CMD_READ_BLOCK_MULTIPLE));
    c = crc7(c, src_lba >> 24);
    c = crc7(c, (src_lba >> 16) & 0xff);
    c = crc7(c, (src_lba >> 8) & 0xff);
    c = crc7(c, src_lba & 0xff);
    c = (c << 1) | 1;
    if (sd_cmd(spi, SD_CMD(SD_CMD_READ_BLOCK_MULTIPLE), src_lba, c) !=
        0x00) {
        sd_cmd_end(spi);
        return SD_COPY_ERROR_CMD18;
    }
    do {
        uint16_t crc, crc_exp;
        long n;

        crc = 0;
        n = 512;
        while (sd_dummy(spi) != SD_DATA_TOKEN);
        do {
            uint8_t x = sd_dummy(spi);
            *p++ = x;
            crc = crc16(crc, x);
        }
        while (--n > 0);

        crc_exp = ((uint16_t) sd_dummy(spi) << 8);
        crc_exp |= sd_dummy(spi);

        if (crc != crc_exp) {
            rc = SD_COPY_ERROR_CMD18_CRC;
            break;
        }
    }
    while (--i > 0);

    sd_cmd(spi, SD_CMD(SD_CMD_STOP_TRANSMISSION), 0, 0x01);
    sd_cmd_end(spi);
    return rc;
}
