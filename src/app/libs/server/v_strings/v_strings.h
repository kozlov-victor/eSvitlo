#ifndef V_STRINGS_H
#define V_STRINGS_H

#include <Arduino.h>
#include "../v_array_list/v_array_list.h"

struct StringSegment {
    const String* source;  // вказівник на оригінальний рядок
    int start;             // початок сегмента
    int end;               // кінець сегмента (не включно)

    String toString() const {
        return source->substring(start, end); // при потребі створює String
    }

    int length() const {
        return end - start;
    }

    char operator[](int i) const {
        return (*source)[start + i];
    }

    boolean isEmpty() const {
        return length() == 0;
    }

};

class VStrings {
public:

    static VArrayList<StringSegment>* splitBy(const String& source, const char delimiter) {
        auto* result = new VArrayList<StringSegment>();
        int start = 0;
        const int len = source.length();
        for (int i = 0; i < len; i++) {
            if (source[i] == delimiter) {
                if (i > start) {
                    result->add({&source, start, i});
                }
                start = i + 1;
            }
        }
        if (start < len) {
            result->add({&source, start, len});
        }
        return result;
    }

    static VArrayList<StringSegment>* splitBy(const StringSegment& source, const char delimiter) {
        auto* result = new VArrayList<StringSegment>();

        int segmentStart = source.start;

        for (int i = source.start; i < source.end; i++) {
            if ((*source.source)[i] == delimiter) {
                if (i > segmentStart) { // ігноруємо пусті сегменти
                    result->add({source.source, segmentStart, i});
                }
                segmentStart = i + 1;
            }
        }

        // додаємо останній сегмент
        if (segmentStart < source.end) {
            result->add({source.source, segmentStart, source.end});
        }

        return result;
    }



    static String replaceAll(const String &source, char delimiter, const String &replace) {
        int len = source.length();

        // рахуємо, скільки замін потрібно
        int count = 0;
        for (int i = 0; i < len; i++) {
            if (source[i] == delimiter) count++;
        }

        // підсумковий розмір: оригінал + кількість замін * довжина replacement
        int resultLen = len + count * replace.length();
        char* buffer = new char[resultLen + 1]; // +1 для '\0'

        int bufIndex = 0;
        for (int i = 0; i < len; i++) {
            if (source[i] == delimiter) {
                // копіюємо replacement
                for (int j = 0; j < replace.length(); j++) {
                    buffer[bufIndex++] = replace[j];
                }
            } else {
                buffer[bufIndex++] = source[i];
            }
        }

        buffer[bufIndex] = '\0';

        String result(buffer);
        delete[] buffer;

        return result;
    }

    static String uriEncode(const String &str) {
        String encoded = "";

        for (int i = 0; i < str.length(); i++) {
            const unsigned char c = str[i];

            // дозволені символи
            if ( ('a' <= c && c <= 'z') ||
                 ('A' <= c && c <= 'Z') ||
                 ('0' <= c && c <= '9') ||
                 c == '-' || c == '_' || c == '.' || c == '!' ||
                 c == '~' || c == '*' || c == '\'' || c == '(' || c == ')') {
                     encoded += (char)c;
                 } else {
                     // кодуємо у UTF-8
                     // Arduino String вже у UTF-8, тому просто беремо байт
                     encoded += '%';
                     char hex[3];
                     sprintf(hex, "%02X", c);
                     encoded += hex;
                 }
        }

        return encoded;
    }
};

#endif