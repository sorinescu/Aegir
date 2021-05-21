#ifndef __APP_CONFIG_HPP__
#define __APP_CONFIG_HPP__

class AppConfig {
    void setDefaults();
    bool readV1();
public:
    AppConfig();

    void commit();
    void reset();
};

#endif // __APP_CONFIG_HPP__
