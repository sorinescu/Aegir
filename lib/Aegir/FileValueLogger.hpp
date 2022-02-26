#ifndef __FILE_VALUE_LOGGER_HPP__
#define __FILE_VALUE_LOGGER_HPP__

#include <FS.h>
#include <LittleFS.h>
#include "AppendableLogger.hpp"

#define FILE_VALUE_LOGGER_BLOCK_SIZE 512

template <typename ValueType>
class FileValueLogger: public AppendableLogger<ValueType>
{
    File _file;
    char *_path;
    bool _read_mode;

    void doOpen(const char *mode)
    {
        if (_file)
        {
            Serial.printf("Closing '%s'\n", _file.name());
            _file.close();
        }
        _file = LittleFS.open(_path, mode);
        if (!_file)
        {
            Serial.printf("failed to open '%s'\n", _path);
        }
        Serial.printf("Opened '%s' in mode '%s'\n", _file.name(), mode);
    }

    void ensureReadMode()
    {
        if (!_read_mode || !_file)
        {
            doOpen("r");
            _read_mode = true;
        }
    }

    void ensureAppendMode()
    {
        if (_read_mode || !_file)
        {
            doOpen("a"); // append
            _read_mode = false;
        }
    }

public:
    FileValueLogger() : _file(nullptr), _path(nullptr), _read_mode(false)
    {
    }

    ~FileValueLogger()
    {
        _file.close();
        if (_path)
            free(_path);
    }

    void open(const char *path)
    {
        if (_path)
            return;
        _path = strdup(path);

        doOpen("w"); // truncate before write
        _read_mode = false;
    }

    size_t size() override
    {
        // ensureReadMode();
        Serial.printf("File '%s' has size %d\n", _file.name(), _file.size());
        return _file.size() / sizeof(ValueType);
    }

    ValueType at(size_t idx) override
    {
        ensureReadMode();

        size_t read_ptr = idx * sizeof(ValueType);
        if (read_ptr + sizeof(ValueType) > _file.size())
            return 0;

        if (_file.position() != read_ptr)
            _file.seek(read_ptr);

        Serial.printf("Reading from %d in '%s'\n", _file.position(), _file.name());

        ValueType val = 0;
        size_t read = _file.readBytes((char *)&val, sizeof(ValueType));
        if (read != sizeof(ValueType))
        {
            Serial.println("failed to read data");
            return ValueType(0);
        }
        return val;
    }

    void append(ValueType val) override
    {
        ensureAppendMode();

        Serial.printf("Writing at %d in '%s'\n", _file.position(), _file.name());
        size_t written = _file.write((uint8_t *)&val, sizeof(ValueType));
        Serial.printf("Wrote %d bytes to '%s'\n", written, _file.name());

        if (written != sizeof(ValueType))
        {
            Serial.println("failed to write data");
        }

        if (_file.position() % FILE_VALUE_LOGGER_BLOCK_SIZE == 0)
        {
            Serial.printf("Flushing %d bytes to LittleFS; file size %d bytes\n", FILE_VALUE_LOGGER_BLOCK_SIZE, _file.size());

            _file.close();

            FSInfo fsinfo;
            LittleFS.info(fsinfo);
            Serial.println("LittleFS details:");
            Serial.printf("totalBytes: %d\nusedBytes: %d\nblockSize: %d\n", fsinfo.totalBytes, fsinfo.usedBytes, fsinfo.blockSize);
        }
    }
};

#endif // __FILE_VALUE_LOGGER_HPP__
