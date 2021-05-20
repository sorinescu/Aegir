#ifndef __APP_CONFIG_HPP__
#define __APP_CONFIG_HPP__

class AppConfig {
    bool _autocommit;

    void setDefaults();
    bool readV1();
public:
    AppConfig();

    void setAutocommit(bool value) { _autocommit = value ; }
    void commit();
    void reset();
};

#endif // __APP_CONFIG_HPP__
