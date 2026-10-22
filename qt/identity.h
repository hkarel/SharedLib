/* clang-format off */
/*****************************************************************************
  The MIT License

  Copyright © 2026 Pavel Karelin (hkarel), <hkarel@yandex.ru>

  Permission is hereby granted, free of charge, to any person obtaining
  a copy of this software and associated documentation files (the
  "Software"), to deal in the Software without restriction, including
  without limitation the rights to use, copy, modify, merge, publish,
  distribute, sublicense, and/or sell copies of the Software, and to
  permit persons to whom the Software is furnished to do so, subject to
  the following conditions:

  The above copyright notice and this permission notice shall be included
  in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*****************************************************************************/

#pragma once

#include "qt/quuidex.h"
#include <QtCore>
#include <tuple>

/**
  Структура используется для идентификации различных примитивов. Идентификация
  может осуществляться по одному или паре полей:  id,  index,  name.  За выбор
  полей для идентификации примитива отвечает пользователь API
*/
struct Identity
{
    Identity();
    Identity(const Identity&) = default;

    Identity(const QUuidEx& id   ) : id(id)       {}
    Identity(const int      index) : index(index) {}
    Identity(const QString& name ) : name(name)   {}
    Identity(const char*    name ) : name(name)   {}

    Identity(const QUuidEx& id, int index) : id(id), index(index) {}

    Identity(const QUuidEx& id, const QString& name) : id(id), name(name) {}
    Identity(const QUuidEx& id, const char*    name) : id(id), name(name) {}

    Identity(int index, const QUuidEx& id) : id(id), index(index) {}

    Identity(int index, const QString& name) : index(index), name(name)   {}
    Identity(int index, const char*    name) : index(index), name(name)   {}

    Identity(const QString& name, const QUuidEx& id) : id(id), name(name) {}
    Identity(const char*    name, const QUuidEx& id) : id(id), name(name) {}

    Identity(const QString& name, int index) : index(index), name(name)   {}
    Identity(const char*    name, int index) : index(index), name(name)   {}

    QUuidEx id;
    qint32  index = {-1};
    QString name;

    // Определяет поля по которым нужно выполнять сравнение/поиск/идентификацию
    enum class CompareField : quint32
    {
        Id        = 0, // По полю id
        Index     = 1, // По полю index
        Name      = 2, // По полю name
        IdIndex   = 3, // По полям id,index
        IdName    = 4, // По полям id,name
        IndexId   = 5, // По полям index,id
        IndexName = 6, // По полям index,name
        NameId    = 7, // По полям name,id
        NameIndex = 8, // По полям name,index
    };

    static int compare(const Identity& identity1, const Identity& identity2,
                       CompareField);

    template<typename T, CompareField compareField = CompareField::Index>
    struct Compare
    {
        int operator() (const T* item1, const T* item2) const
        {
            return compare(item1->identity, item2->identity, compareField);
        }

        int operator() (const Identity* identity, const T* item2) const
        {
            return compare(*identity, item2->identity, compareField);
        }

        int operator() (const std::tuple<Identity, CompareField>* item1, const T* item2) const
        {
            return compare(std::get<0>(*item1), item2->identity, std::get<1>(*item1));
        }
    };

    bool isNull() const;
    QString toString() const;
};

Q_DECLARE_METATYPE(Identity)
