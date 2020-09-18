// Copyright (c) 2005 - 2017 Settlers Freaks (sf-team at siedler25.org)
//
// This file is part of Return To The Roots.
//
// Return To The Roots is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Return To The Roots is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Return To The Roots. If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include "libendian/ConvertEndianess.h"
#include <boost/container/vector.hpp>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <type_traits>

class BinaryFile;

/// Klasse die einen Buffer zum Serialisieren verwaltet und entsprechende Methoden zum Lesen/Schreiben bereitstellt.
/// Implementiert einen FIFO (push fügt ans ende, pop entfernt am anfang)
class Serializer
{
    using Converter = libendian::ConvertEndianess<true>;

public:
    Serializer() = default;
    Serializer(const void* data, unsigned initial_size);

    virtual ~Serializer() = default;

    /// Aufräumen
    void Clear();

    unsigned GetPos() const { return pos_; }

    /// Getter
    unsigned GetLength() const { return length_; }

    /// Schreibzugriff auf die Länge
    void SetLength(unsigned length);

    unsigned GetBytesLeft() const;

    /// Zugriff auf die Rohdaten
    const unsigned char* GetData() const { return data_.data(); }

    /// Schreibzugriff auf die Rohdaten
    unsigned char* GetDataWritable(unsigned length)
    {
        EnsureSize(length);
        return data_.data();
    }

    /// Schreibt den Buffer in eine Datei
    void WriteToFile(BinaryFile& file) const;
    /// Liest den Buffer aus einer Datei
    virtual void ReadFromFile(BinaryFile& file);

    void PushRawData(const void* data, unsigned length);

    /// Sämtliche Integer
    void PushSignedInt(int32_t i) { Push(i); }
    void PushUnsignedInt(uint32_t i) { Push(i); }

    void PushSignedShort(int16_t i) { Push(i); }
    void PushUnsignedShort(uint16_t i) { Push(i); }

    void PushSignedChar(int8_t i) { Push(i); }
    void PushUnsignedChar(uint8_t i) { Push(i); }
    /// Push a variable sized uint32_t. Uses less space when value is small
    void PushVarSize(uint32_t i);

    void PushBool(bool b);

    void PushString(const std::string& str);
    void PushLongString(const std::string& str);

    // Lesemethoden

    /// Rohdaten kopieren
    void PopRawData(void* data, unsigned length);

    /// Sämtliche Integer
    int32_t PopSignedInt() { return Pop<int32_t>(); }
    uint32_t PopUnsignedInt() { return Pop<uint32_t>(); }

    int16_t PopSignedShort() { return Pop<int16_t>(); }
    uint16_t PopUnsignedShort() { return Pop<uint16_t>(); }

    int8_t PopSignedChar() { return Pop<int8_t>(); }
    uint8_t PopUnsignedChar() { return Pop<uint8_t>(); }
    uint32_t PopVarSize();

    bool PopBool();

    std::string PopString();
    std::string PopLongString();

protected:
    template<typename T>
    T Pop();

    template<typename T>
    void Push(T val);

    /// Erweitert ggf. Speicher um add_length
    void ExtendMemory(unsigned add_length) { EnsureSize(length_ + add_length); }

    /// Makes sure the internal buffer is at least length bytes long
    void EnsureSize(unsigned length);

private:
    /// data mit den Daten
    boost::container::vector<uint8_t> data_;
    /// Logische Länge
    unsigned length_ = 0;
    /// Schreib/Leseposition
    unsigned pos_ = 0;

    /// Checks if data of size len can be popped
    void CheckSize(unsigned len) const;
};

inline unsigned Serializer::GetBytesLeft() const
{
    assert(pos_ <= length_);
    return length_ - pos_;
}

inline void Serializer::EnsureSize(unsigned length)
{
    if(data_.size() < length)
    {
        size_t newSize = 8u;
        while(newSize < length)
            newSize *= 2u;
        data_.resize(newSize, boost::container::default_init);
    }
}

inline void Serializer::CheckSize(unsigned len) const
{
    if(GetBytesLeft() < len)
        throw std::range_error("Out of range during deserialization");
}

inline void Serializer::PushRawData(const void* data, unsigned length)
{
    if(length == 0)
        return;
    ExtendMemory(length);
    std::memcpy(&data_[length_], data, length);
    this->length_ += length;
}

inline void Serializer::PopRawData(void* data, unsigned length)
{
    if(length == 0)
        return;
    CheckSize(length);
    std::memcpy(data, &data_[pos_], length);
    pos_ += length;
}

template<typename T>
inline T Serializer::Pop()
{
    static_assert(std::is_trivial<T>::value && !std::is_pointer<T>::value, "Type must be a trivial, non-pointer type");
    T i;
    // Note: No casting allowed due to alignment
    PopRawData(&i, sizeof(i));
    return Converter::toNative(i);
}

template<typename T>
inline void Serializer::Push(T val)
{
    static_assert(std::is_trivial<T>::value && !std::is_pointer<T>::value, "Type must be a trivial, non-pointer type");
    val = Converter::fromNative(val);
    PushRawData(&val, sizeof(val));
}

template<>
inline void Serializer::Push(bool val)
{
    PushBool(val);
}

template<>
inline bool Serializer::Pop<bool>()
{
    return PopBool();
}
