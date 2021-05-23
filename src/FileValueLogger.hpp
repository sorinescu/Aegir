#ifndef __FILE_VALUE_LOGGER_HPP__
#define __FILE_VALUE_LOGGER_HPP__

#include <FS.h>
#include <LittleFS.h>

#define FILE_VALUE_LOGGER_BLOCK_SIZE 512

template <typename ValueType>
class FileValueLogger
{
    File _file;
    size_t _write_ptr;

public:
    FileValueLogger() : _file(nullptr), _write_ptr(0)
    {
    }

    ~FileValueLogger()
    {
        _file.close();
    }

    void open(const char *path)
    {
        if (_file)
            return;

        _file = LittleFS.open(path, "w+");
        if (!_file)
        {
            Serial.printf("failed to open '%s'\n", path);
        }
    }

    size_t size() const
    {
        return _file.size() / sizeof(ValueType);
    }

    ValueType at(size_t idx)
    {
        size_t read_ptr = idx * sizeof(ValueType);
        if (_file.position() != read_ptr)
            _file.seek(read_ptr);

        ValueType val = 0;
        size_t read = _file.readBytes((char *)&val, sizeof(ValueType));
        if (read != sizeof(ValueType))
        {
            Serial.println("failed to read data");
            return ValueType(0);
        }
        return val;
    }

    void append(ValueType val)
    {
        if (_file.position() != _write_ptr)
            _file.seek(_write_ptr);
        size_t written = _file.write((uint8_t *)&val, sizeof(ValueType));
        _write_ptr = _file.position();
        if (written != sizeof(ValueType))
        {
            Serial.println("failed to write data");
        }

        if (_write_ptr % FILE_VALUE_LOGGER_BLOCK_SIZE == 0)
        {
            Serial.printf("Flushing %d bytes to LittleFS; file size %d bytes\n", FILE_VALUE_LOGGER_BLOCK_SIZE, _write_ptr);

            _file.flush();
            
            FSInfo fsinfo;
            LittleFS.info(fsinfo);
            Serial.println("LittleFS details:");
            Serial.printf("totalBytes: %d\nusedBytes: %d\nblockSize: %d\n", fsinfo.totalBytes, fsinfo.usedBytes, fsinfo.blockSize);
        }
    }
};

#endif // __FILE_VALUE_LOGGER_HPP__
