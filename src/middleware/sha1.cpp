/*
author          Oliver Blaser
date            22.12.2024
copyright       GPL-3.0 - Copyright (c) 2024 Oliver Blaser
*/

#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <istream>
#include <sstream>
#include <string>
#include <vector>

#include "sha1.h"


namespace {

static constexpr size_t blockSize = 64;
static constexpr size_t blockSize32 = blockSize / 4;

uint32_t rol(const uint32_t value, const size_t bits) { return (value << bits) | (value >> (32 - bits)); }

uint32_t blk(const uint32_t* block, const size_t i) { return rol(block[(i + 13) & 0x0F] ^ block[(i + 8) & 0x0F] ^ block[(i + 2) & 0x0F] ^ block[i], 1); }

void r0(const uint32_t* block, const uint32_t v, uint32_t& w, const uint32_t x, const uint32_t y, uint32_t& z, const size_t i)
{
    z += ((w & (x ^ y)) ^ y) + block[i] + 0x5A827999 + rol(v, 5);
    w = rol(w, 30);
}

void r1(uint32_t* block, const uint32_t v, uint32_t& w, const uint32_t x, const uint32_t y, uint32_t& z, const size_t i)
{
    block[i] = blk(block, i);
    z += ((w & (x ^ y)) ^ y) + block[i] + 0x5A827999 + rol(v, 5);
    w = rol(w, 30);
}

void r2(uint32_t* block, const uint32_t v, uint32_t& w, const uint32_t x, const uint32_t y, uint32_t& z, const size_t i)
{
    block[i] = blk(block, i);
    z += (w ^ x ^ y) + block[i] + 0x6ED9EBA1 + rol(v, 5);
    w = rol(w, 30);
}

void r3(uint32_t* block, const uint32_t v, uint32_t& w, const uint32_t x, const uint32_t y, uint32_t& z, const size_t i)
{
    block[i] = blk(block, i);
    z += (((w | x) & y) | (w & x)) + block[i] + 0x8F1BBCDC + rol(v, 5);
    w = rol(w, 30);
}

void r4(uint32_t* block, const uint32_t v, uint32_t& w, const uint32_t x, const uint32_t y, uint32_t& z, const size_t i)
{
    block[i] = blk(block, i);
    z += (w ^ x ^ y) + block[i] + 0xCA62C1D6 + rol(v, 5);
    w = rol(w, 30);
}

void buffer_to_block(const std::string& buffer, uint32_t* block)
{
    /* Convert the std::string (byte buffer) to a uint32_t array (MSB) */
    for (size_t i = 0; i < blockSize32; ++i)
    {
        // clang-format off
        block[i] = (((uint32_t)(buffer[4 * i + 3]) & 0xFF))       |
                   (((uint32_t)(buffer[4 * i + 2]) & 0xFF) << 8)  |
                   (((uint32_t)(buffer[4 * i + 1]) & 0xFF) << 16) |
                   (((uint32_t)(buffer[4 * i + 0]) & 0xFF) << 24);
        // clang-format on
    }
}

} // namespace



void SHA1::update(const char* str)
{
    std::istringstream iss(str);
    update(iss);
}

void SHA1::update(const std::string& str)
{
    std::istringstream iss(str);
    update(iss);
}

void SHA1::update(std::istream& istream)
{
    while (true)
    {
        char readBuffer[blockSize];

        istream.read(readBuffer, blockSize - m_buffer.size());
        m_buffer.append(readBuffer, (std::size_t)istream.gcount());

        if (m_buffer.size() != blockSize) { break; }

        uint32_t block[blockSize32];
        buffer_to_block(m_buffer, block);
        m_transform(block);
        m_buffer.clear();
    }
}

std::string SHA1::final() const
{
    const uint64_t nBits = (m_nTransformations * blockSize + m_buffer.size()) * 8;

    // padding
    m_buffer += (char)0x80;
    const size_t bufferStartSize = m_buffer.size();
    while (m_buffer.size() < blockSize) { m_buffer += (char)0x00; }

    uint32_t block[blockSize32];
    buffer_to_block(m_buffer, block);

    if (bufferStartSize > (blockSize - 8))
    {
        m_transform(block);
        for (size_t i = 0; i < (blockSize32 - 2); i++) { block[i] = 0; }
    }

    // append number of message bits
    block[blockSize32 - 1] = (uint32_t)nBits;
    block[blockSize32 - 2] = (uint32_t)(nBits >> 32);
    m_transform(block);

    m_finalDone = true;

    return digest();
}

std::string SHA1::digest() const
{
    if (!m_finalDone) { return final(); }
    else
    {
        std::ostringstream result;
        for (size_t i = 0; i < (sizeof(m_digest) / sizeof(m_digest[0])); ++i)
        {
            result << std::hex << std::setfill('0') << std::setw(8);
            result << m_digest[i];
        }

        return result.str();
    }
}

void SHA1::m_transform(uint32_t* block) const
{
    uint32_t a = m_digest[0];
    uint32_t b = m_digest[1];
    uint32_t c = m_digest[2];
    uint32_t d = m_digest[3];
    uint32_t e = m_digest[4];

    r0(block, a, b, c, d, e, 0);
    r0(block, e, a, b, c, d, 1);
    r0(block, d, e, a, b, c, 2);
    r0(block, c, d, e, a, b, 3);
    r0(block, b, c, d, e, a, 4);
    r0(block, a, b, c, d, e, 5);
    r0(block, e, a, b, c, d, 6);
    r0(block, d, e, a, b, c, 7);
    r0(block, c, d, e, a, b, 8);
    r0(block, b, c, d, e, a, 9);
    r0(block, a, b, c, d, e, 10);
    r0(block, e, a, b, c, d, 11);
    r0(block, d, e, a, b, c, 12);
    r0(block, c, d, e, a, b, 13);
    r0(block, b, c, d, e, a, 14);
    r0(block, a, b, c, d, e, 15);
    r1(block, e, a, b, c, d, 0);
    r1(block, d, e, a, b, c, 1);
    r1(block, c, d, e, a, b, 2);
    r1(block, b, c, d, e, a, 3);
    r2(block, a, b, c, d, e, 4);
    r2(block, e, a, b, c, d, 5);
    r2(block, d, e, a, b, c, 6);
    r2(block, c, d, e, a, b, 7);
    r2(block, b, c, d, e, a, 8);
    r2(block, a, b, c, d, e, 9);
    r2(block, e, a, b, c, d, 10);
    r2(block, d, e, a, b, c, 11);
    r2(block, c, d, e, a, b, 12);
    r2(block, b, c, d, e, a, 13);
    r2(block, a, b, c, d, e, 14);
    r2(block, e, a, b, c, d, 15);
    r2(block, d, e, a, b, c, 0);
    r2(block, c, d, e, a, b, 1);
    r2(block, b, c, d, e, a, 2);
    r2(block, a, b, c, d, e, 3);
    r2(block, e, a, b, c, d, 4);
    r2(block, d, e, a, b, c, 5);
    r2(block, c, d, e, a, b, 6);
    r2(block, b, c, d, e, a, 7);
    r3(block, a, b, c, d, e, 8);
    r3(block, e, a, b, c, d, 9);
    r3(block, d, e, a, b, c, 10);
    r3(block, c, d, e, a, b, 11);
    r3(block, b, c, d, e, a, 12);
    r3(block, a, b, c, d, e, 13);
    r3(block, e, a, b, c, d, 14);
    r3(block, d, e, a, b, c, 15);
    r3(block, c, d, e, a, b, 0);
    r3(block, b, c, d, e, a, 1);
    r3(block, a, b, c, d, e, 2);
    r3(block, e, a, b, c, d, 3);
    r3(block, d, e, a, b, c, 4);
    r3(block, c, d, e, a, b, 5);
    r3(block, b, c, d, e, a, 6);
    r3(block, a, b, c, d, e, 7);
    r3(block, e, a, b, c, d, 8);
    r3(block, d, e, a, b, c, 9);
    r3(block, c, d, e, a, b, 10);
    r3(block, b, c, d, e, a, 11);
    r4(block, a, b, c, d, e, 12);
    r4(block, e, a, b, c, d, 13);
    r4(block, d, e, a, b, c, 14);
    r4(block, c, d, e, a, b, 15);
    r4(block, b, c, d, e, a, 0);
    r4(block, a, b, c, d, e, 1);
    r4(block, e, a, b, c, d, 2);
    r4(block, d, e, a, b, c, 3);
    r4(block, c, d, e, a, b, 4);
    r4(block, b, c, d, e, a, 5);
    r4(block, a, b, c, d, e, 6);
    r4(block, e, a, b, c, d, 7);
    r4(block, d, e, a, b, c, 8);
    r4(block, c, d, e, a, b, 9);
    r4(block, b, c, d, e, a, 10);
    r4(block, a, b, c, d, e, 11);
    r4(block, e, a, b, c, d, 12);
    r4(block, d, e, a, b, c, 13);
    r4(block, c, d, e, a, b, 14);
    r4(block, b, c, d, e, a, 15);

    m_digest[0] += a;
    m_digest[1] += b;
    m_digest[2] += c;
    m_digest[3] += d;
    m_digest[4] += e;

    ++m_nTransformations;
}
