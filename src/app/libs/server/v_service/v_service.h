#ifndef V_SERVICE_H
#define V_SERVICE_H

#define Service(ClassName) \
    public: \
    static ClassName& instance() { \
        static ClassName __instance = ClassName(); \
        return __instance; \
    }

#endif //V_SERVICE_H