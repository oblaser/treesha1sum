/*
author          Oliver Blaser
date            22.12.2024
copyright       GPL-3.0 - Copyright (c) 2024 Oliver Blaser
*/

/*
    This SHA1 implementation is based on https://github.com/clibs/sha1 and https://github.com/vog/sha1.

    Examples for how to use this implementation can be found in this repo `/test/unit/sha1.cpp`.
*/

#ifndef IG_MIDDLEWARE_SHA1_H
#define IG_MIDDLEWARE_SHA1_H

#include <cstddef>
#include <cstdint>
#include <istream>
#include <string>
#include <vector>


class SHA1
{
public:
    static constexpr size_t digestSize = 20;

public:
    SHA1() { reset(); }

    explicit SHA1(const char* str)
    {
        reset();
        update(str);
    }

    explicit SHA1(const std::string& str)
    {
        reset();
        update(str);
    }

    SHA1(const uint8_t* data, size_t count)
    {
        reset();
        update(data, count);
    }

    explicit SHA1(const std::vector<uint8_t>& data)
    {
        reset();
        update(data);
    }

    // no ctor for `std::istream& istream` because the arg can't be const

    void reset()
    {
        m_digest[0] = 0x67452301;
        m_digest[1] = 0xEFCDAB89;
        m_digest[2] = 0x98BADCFE;
        m_digest[3] = 0x10325476;
        m_digest[4] = 0xC3D2E1F0;

        m_buffer.clear();

        m_nTransformations = 0;

        m_finalDone = false;
    }

    void update(const char* str);
    void update(const std::string& str);
    void update(const uint8_t* data, size_t count) { update(std::string(data, data + count)); }
    void update(const std::vector<uint8_t>& data) { update(data.data(), data.size()); }
    void update(std::istream& istream);

    std::string final() const;

    std::string digest() const;

    operator std::string() const { return digest(); }

private:
    mutable uint32_t m_digest[digestSize / 4];
    mutable std::string m_buffer;
    mutable uint64_t m_nTransformations;
    mutable bool m_finalDone;

    void m_transform(uint32_t* block) const;
};


#endif // IG_MIDDLEWARE_SHA1_H
