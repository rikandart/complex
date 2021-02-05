#include <QtCore>

// установление битов в number в обратном порядке
// для 12-битного числа
auto reversebits = [](quint16 number)->quint16{
    quint16 mask1_1 = 0b111111000000, mask1_2 = 0b000000111111,
            mask2_1 = 0b111000111000, mask2_2 = 0b000111000111,
            mask3_1 = 0b100100100100, mask3_2 = 0b001001001001, mask3_3 = 0b010010010010;
    number = (number & mask1_1) >> 6 | (number & mask1_2) << 6;
    number = (number & mask2_1) >> 3 | (number & mask2_2) << 3;
    number = (number & mask3_1) >> 2 | (number & mask3_2) << 2 | (number & mask3_3);
    return number;
};
