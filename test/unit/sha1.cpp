/*
author          Oliver Blaser
date            22.12.2024
copyright       GPL-3.0 - Copyright (c) 2024 Oliver Blaser
*/

/*
    build:
    $ g++ -Wall -Werror=reorder -Werror=format -I ../../src/ ../../src/middleware/sha1.cpp sha1.cpp -o sha1
*/

#include <array>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <vector>

#include "middleware/sha1.h"


using std::cout;
using std::endl;


int main()
{
    int r = 0;

    const std::vector<uint8_t> bin = { 0x10, 0x20, 0x30, 0x0A, 0x0B, 0xCC, 0xDD, 0xEE, 0xFF };

    SHA1 sha1_million_a;
    for (int i = 0; i < 1000000 / 200; ++i)
    {
        sha1_million_a.update("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                              "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                              "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                              "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
    }

    const SHA1 sha1_bin_3(bin.data(), 3);

    SHA1 sha1_tmp;
    sha1_tmp.update("abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu");

    // https://emn178.github.io/online-tools/sha1.html
    std::vector<std::array<std::string, 2>> testVector = {
        { "da39a3ee5e6b4b0d3255bfef95601890afd80709", SHA1() },
        { "da39a3ee5e6b4b0d3255bfef95601890afd80709", SHA1("") },
        { "a9993e364706816aba3e25717850c26c9cd0d89d", SHA1("abc") },
        { "84983e441c3bd26ebaae4aa1f95129e5e54670f1", SHA1("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq") },
        { "a49b2446a02c645bf419f995b67091253a04a259", sha1_tmp.digest() },
        { "34aa973cd4c4daa4f61eeb2bdbad27316534016f", sha1_million_a.final() },
        { "16312751ef9307c3fd1afbcb993cdc80464ba0f1", SHA1("the quick brown fox jumps over the lazy dog") },
        { "2cbd0727187241f9a1b366c498c334229f6c913f", SHA1(bin) },
        { "b203c5a0c19f15f173698158e08f83ca07638574", sha1_bin_3 },
    };

    sha1_tmp.reset();
    sha1_tmp.update("asdf");
    sha1_tmp.update("1234");
    testVector.push_back({ "f58cf5e7e10f195e21b553096d092c763ed18b0e", sha1_tmp });

    for (size_t i = 0; i < testVector.size(); ++i)
    {
        const auto& tmp = testVector[i];

        if (tmp[0] == tmp[1]) { cout << std::setw(2) << i << "  " << tmp[0] << " != " << tmp[1] << " \033[92mOK\033[39m" << endl; }
        else
        {
            cout << std::setw(2) << i << "  " << tmp[0] << " != " << tmp[1] << " \033[91mFAILED\033[39m" << endl;
            r = 1;
        }
    }

    if (r == 0) { cout << "\033[92mOK\033[39m" << endl; }
    else { cout << "\033[91mFAILED\033[39m" << endl; }

    return r;
}
