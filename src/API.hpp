#ifndef __AEGIR_API_HPP__
#define __AEGIR_API_HPP__

class API
{
public:
    API(): _isStarted(false) {}
    void start();
    bool isStarted() { return _isStarted; }
private:
    bool _isStarted;
};

#endif
