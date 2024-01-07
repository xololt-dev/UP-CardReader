#include <iostream>
#include <cstdint>

#include <winscard.h>
#pragma comment(lib, "Winscard")

void dekodowanie(BYTE* wiadomosc) {
    // Odczyt nazwy kontaktu
    int i = 0;
    BYTE tempChar = wiadomosc[i];
    // Czytamy dopóki nie trafimy na 0xFF
    while ((int)tempChar != 255) {
        std::cout << tempChar;
        i++;
        tempChar = wiadomosc[i];
    }
    std::cout << " ";
    // Przechodzimy do odczytu numeru
    i = 18;
    tempChar = wiadomosc[i];
    // Czytamy dopóki nie trafimy na 0xFF
    while ((int)tempChar != 255) {
        // Trzeba zamienić kolejność znaków
        int displayNumber = (int)(tempChar & 0x0F);
        // Wyswietlic odpowiednio jako int lub char
        if (displayNumber < 10)
            std::cout << displayNumber;
        else std::cout << (char)(displayNumber + 55);
        // Długość numeru to max +xx xxx xx xx xx
        if (i >= 23) break;
        displayNumber = (int)((tempChar & 0xF0) >> 4);
        if (displayNumber < 10)
            std::cout << displayNumber;
        else std::cout << (char)(displayNumber + 55);
        // Przejscie do kolejnego znaku
        i++;
        tempChar = wiadomosc[i];
    }
    std::cout << "\n";
}