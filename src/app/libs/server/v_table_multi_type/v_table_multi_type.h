#ifndef TABLE_MULTI_TYPE_H
#define TABLE_MULTI_TYPE_H

#include "../v_hash_table/v_hash_table.h"

class VTableMultitype {
private:
    enum Type {
        INT, STRING, BOOLEAN
    };
    struct MultiValue {
        Type type = STRING;
        int nValue = 0;
        String sValue = "";
        boolean bValue = false;
    };
    VHashTable<MultiValue> *table;

    static String escapeJsonString(const String &s) {
        String out;
        out.reserve(s.length() + 4);

        for (size_t i = 0; i < s.length(); i++) {
            const char c = s[i];
            switch (c) {
                case '"':  out += "\\\""; break;
                case '\\': out += "\\\\"; break;
                case '\n': out += "\\n";  break;
                case '\r': out += "\\r";  break;
                case '\t': out += "\\t";  break;
                default:
                    if ((uint8_t)c < 0x20) {
                        // control chars — пропускаємо або замінюємо
                    } else {
                        out += c;
                    }
            }
        }
        return out;
    }

    static String stringifyItem(const MultiValue &value) {
        if (value.type == INT) {
            return String(value.nValue);
        }
        if (value.type == STRING) {
            return "\"" + escapeJsonString(value.sValue) + "\"";
        }
        return value.bValue?"true":"false";
    }

public:
    VTableMultitype() {
        table = new VHashTable<MultiValue>();
    }
    ~VTableMultitype() {
        delete table;
    }


    // забороняємо копіювання
    VTableMultitype(const VTableMultitype&) = delete;
    VTableMultitype& operator=(const VTableMultitype&) = delete;

    // дозволяємо переміщення (move)
    VTableMultitype(VTableMultitype&& other) noexcept {
        table = other.table;
        other.table = nullptr;
    }

    VTableMultitype& operator=(VTableMultitype&& other) noexcept {
        if (this == &other) return *this;
        if (table) delete table;
        table = other.table;
        other.table = nullptr;
        return *this;
    } // щоб спрацювало const VTableMultitype body = VTableMultitype::parseJson(bodyRaw);

    void putAll(const VTableMultitype &value) {
        value.table->forEach([this](const String &key, const MultiValue &val) {
            this->table->put(key,val);
        });
    }
    void putString(const String &key, const String &value) {
        MultiValue mv;
        mv.type = STRING;
        mv.sValue = value;
        table->put(key,mv);
    }
    String getString(const String &key) const{
        return table->get(key).sValue;
    }
    void putBoolean(const String &key, const boolean value) {
        MultiValue mv;
        mv.type = BOOLEAN;
        mv.bValue = value;
        table->put(key,mv);
    }
    boolean getBoolean(const String &key) const {
        return table->get(key).bValue;
    }
    void putInt(const String &key, const int value) {
        MultiValue mv;
        mv.type = INT;
        mv.nValue = value;
        table->put(key,mv);
    }
    int getInt(const String &key) const {
        return table->get(key).nValue;
    }

    // VTableMultitype(const VTableMultitype&) = delete;
    // VTableMultitype& operator=(const VTableMultitype&) = delete;

};

#endif //TABLE_MULTI_TYPE_H