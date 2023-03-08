#include <vector>

namespace SOE
{
    static unsigned char key[] = { 0x17, 0xbd, 0x08, 0x6b, 0x1b, 0x94 ,0xf0, 0x2f ,0xf0 ,0xec ,0x53 ,0xd7 ,0x63 ,0x58 ,0x9b ,0x5f };

    static void rc4_crypt(unsigned char* data, size_t data_len)
    {
        unsigned char S[256];
        unsigned char K[256];

        // Initialisation de S et K
        for (int i = 0; i < 256; ++i) {
            S[i] = i;
            K[i] = key[i % sizeof(key)];
        }

        // Mélange de S
        int j = 0;
        for (int i = 0; i < 256; ++i) {
            j = (j + S[i] + K[i]) % 256;
            std::swap(S[i], S[j]);
        }

        // Génération de la suite chiffrante
        int i = 0;
        j = 0;
        for (size_t n = 0; n < data_len; ++n) {
            i = (i + 1) % 256;
            j = (j + S[i]) % 256;
            std::swap(S[i], S[j]);
            unsigned char f = S[(S[i] + S[j]) % 256];
            data[n] ^= f;
        }
    }
}