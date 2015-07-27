﻿// $Id: Serializer.h 9359 2014-04-25 15:37:22Z FloSoft $
//
// Copyright (c) 2005 - 2011 Settlers Freaks (sf-team at siedler25.org)
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
#ifndef SERIALIZER_H_INCLUDED
#define SERIALIZER_H_INCLUDED

#pragma once

#include "libUtilDefines.h"

#ifdef _WIN32
    #include <winsock2.h>
#else
    #include <arpa/inet.h>
#endif // _WIN32

#include <string>
#include <cstring>

class BinaryFile;

/// Klasse die einen Buffer zum Serialisieren verwaltet und entsprechende Methoden zum Lesen/Schreiben bereitstellt.
/// Implementiert einen FIFO (push fügt ans ende, pop entfernt am anfang)
class Serializer
{
    public:
        Serializer(void)
            : data(0), buffer_length(0), length(0), pos(0)
        {
        }

        Serializer(const void* const data, const unsigned initial_size)
            : data(new unsigned char[initial_size]), buffer_length(initial_size), length(initial_size), pos(0)
        {
            memcpy(this->data, data, initial_size);
        }

        Serializer(const Serializer& two)
            : data(new unsigned char[two.buffer_length]), buffer_length(two.buffer_length), length(two.length), pos(two.pos)
        {
            memcpy(data, two.data, length);
        }

        virtual ~Serializer()
        {
            Clear();
        }

        /// Aufräumen
        inline void Clear()
        {
            delete[] data;
            data = 0;
            buffer_length = 0;
            length = 0;
            pos = 0;
        }

        unsigned GetPos() const
        {
            return pos;
        }

        /// Getter
        unsigned GetLength() const
        {
            return length;
        }

        /// Zugriff auf die Rohdaten
        const unsigned char* GetData(void) const
        {
            return data;
        }

        /// Schreibt den Buffer in eine Datei
        void WriteToFile(BinaryFile& file);
        /// Liest den Buffer aus einer Datei
        virtual void ReadFromFile(BinaryFile& file);


        /// Kopiermethoden

        /// Rohdaten kopieren
        inline void PushRawData(const void* const data, const unsigned length)
        {
            ExtendMemory(length);
            memcpy(&this->data[this->length], data, length);
            this->length += length;
        }

        /// Sämtliche Integer
        inline void PushSignedInt(signed int i)
        {
            Push(i);
        }
        inline void PushUnsignedInt(unsigned int i)
        {
            Push(i);
        }

        inline void PushSignedShort(signed short int i)
        {
            Push(i);
        }
        inline void PushUnsignedShort(unsigned short int i)
        {
            Push(i);
        }

        inline void PushSignedChar(signed char i)
        {
            Push(i);
        }
        inline void PushUnsignedChar(unsigned char i)
        {
            Push(i);
        }

        inline void PushBool(bool b)
        {
            PushUnsignedChar(b ? 1 : 0);
        }

        inline void PushString(const std::string& str)
        {
            PushUnsignedInt(str.length());
            for(unsigned i = 0; i < str.length(); ++i)
                PushSignedChar(str[i]);
        }

        // Lesemethoden

        /// Copy all data
        inline void ToBuffer(unsigned char* const buffer)
        {
            memcpy(buffer, data, length);
        }

        /// Rohdaten kopieren
        inline void PopRawData(void* const data, const unsigned length)
        {
            assert(pos + length <= this->length);

            memcpy(data, &this->data[pos], length);
            pos += length;
        }

        /// Sämtliche Integer
        inline signed int PopSignedInt()
        {
            return Pop<signed int>();
        }
        inline unsigned int PopUnsignedInt()
        {
            return Pop<unsigned int>();
        }

        inline signed short PopSignedShort()
        {
            return Pop<signed short>();
        }
        inline unsigned short PopUnsignedShort()
        {
            return Pop<unsigned short>();
        }

        inline signed char PopSignedChar()
        {
            return Pop<signed char>();
        }

        inline unsigned char PopUnsignedChar()
        {
            return Pop<unsigned char>();
        }

        inline bool PopBool()
        {
            return ((PopUnsignedChar() == 1) ? true : false);
        }

        inline std::string PopString()
        {
            std::string str;
            str.resize(PopUnsignedInt());

            assert(pos + str.size() <= length);

            for(unsigned i = 0; i < str.length(); ++i)
                str[i] = PopSignedChar();

            return str;
        }

    protected:
        Serializer& operator=(const Serializer& other)
        {
            if(this == &other)
                return *this;

            Clear();

            data = new unsigned char[other.buffer_length];
            buffer_length = other.buffer_length;
            length = other.length;
            pos = other.pos;
            memcpy(data, other.data, length);

            return *this;
        }

        unsigned int checkByteOrder(unsigned int i)
        {
            return htonl(i);
        }

        signed int checkByteOrder(signed int i)
        {
            return htonl(i);
        }

        unsigned short checkByteOrder(unsigned short i)
        {
            return htons(i);
        }

        signed short checkByteOrder(signed short i)
        {
            return htons(i);
        }

        unsigned char checkByteOrder(unsigned char i)
        {
            return i;
        }

        signed char checkByteOrder(signed char i)
        {
            return i;
        }

        template<typename T>
        T Pop()
        {
            assert(pos + sizeof(T) <= length);

            T i = checkByteOrder( *reinterpret_cast<T*>(&data[pos]) );
            pos += sizeof(T);

            return i;
        }

        template<typename T>
        void Push(const T i)
        {
            ExtendMemory(sizeof(T));
            *reinterpret_cast<T*>(&data[length]) = checkByteOrder(i);
            this->length += sizeof(T);
        }

        /// Schreibzugriff auf die Rohdaten
        unsigned char* GetDataWritable(void)
        {
            return data;
        }

        /// Schreibzugriff auf die Länge
        void SetLength(const unsigned int length)
        {
            this->length = length;
        }

        /// vergrößert den Speicher auf die nächst höhere 2er potenz zur Länge @p length.
        inline void Realloc(const unsigned int length)
        {
            if(this->buffer_length == 0)
                this->buffer_length = 64;

            // speicher vergrößern
            while(this->buffer_length < length)
                this->buffer_length *= 2;

            if(data == 0)
            {
                // neu anlegen
                data = new unsigned char[this->buffer_length];
                memset(data, 0, sizeof(unsigned char)*this->buffer_length);
            }
            else
            {
                // umkopieren (vergrößern)
                unsigned char* ndata = new unsigned char[this->buffer_length];
                memset(ndata + this->length, 0, sizeof(unsigned char)*(this->buffer_length - this->length));
                memcpy(ndata, data, this->length);

                delete[] data;
                data = ndata;
            }
        }

        /// Erweitert ggf. Speicher um add_length
        inline void ExtendMemory(const unsigned add_length)
        {
            if(length + add_length > this->buffer_length)
                Realloc(length + add_length);
        }

    private:
        /// data mit den Daten
        unsigned char* data;
        /// Länge des datas
        unsigned buffer_length;
        /// Logische Länge
        unsigned length;
        /// Schreib/Leseposition
        unsigned pos;
};

#endif // !SERIALIZER_H_INCLUDED
