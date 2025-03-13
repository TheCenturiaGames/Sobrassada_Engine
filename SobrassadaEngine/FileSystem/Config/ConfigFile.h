#pragma once

class ConfigFile
{
public:
    ConfigFile() = default;
    virtual ~ConfigFile() = default;

    virtual void Load() = 0;
    virtual void Save() const = 0;
};
