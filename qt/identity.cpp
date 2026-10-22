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

#include "qt/identity.h"

Identity::Identity()
{
    static bool first {true};
    if (first)
    {
        qRegisterMetaType<Identity>("Identity");
        first = false;
    }
}

int Identity::compare(const Identity& i1, const Identity& i2, CompareField cmp)
{
    #define COMPARE_ITEM(ITEM1, ITEM2) \
        ((ITEM1 > ITEM2) ? 1 : ((ITEM1 < ITEM2) ? -1 : 0))

    if (CompareField::Id == cmp)
    {
        return QUuidEx::compare(i1.id, i2.id);
    }
    else if (CompareField::Index == cmp)
    {
        return COMPARE_ITEM(i1.index, i2.index);
    }
    else if (CompareField::Name == cmp)
    {
        return QString::compare(i1.name, i2.name);
    }
    else if (CompareField::IdIndex == cmp)
    {
        if (int res = QUuidEx::compare(i1.id, i2.id))
            return res;
        return COMPARE_ITEM(i1.index, i2.index);
    }
    else if (CompareField::IdName == cmp)
    {
        if (int res = QUuidEx::compare(i1.id, i2.id))
            return res;
        return QString::compare(i1.name, i2.name);
    }
    else if (CompareField::IndexId == cmp)
    {
        if (int res = COMPARE_ITEM(i1.index, i2.index))
            return res;
        return QUuidEx::compare(i1.id, i2.id);
    }
    else if (CompareField::IndexName == cmp)
    {
        if (int res = COMPARE_ITEM(i1.index, i2.index))
            return res;
        return QString::compare(i1.name, i2.name);
    }
    else if (CompareField::NameId == cmp)
    {
        if (int res = QString::compare(i1.name, i2.name))
            return res;
        return QUuidEx::compare(i1.id, i2.id);
    }
    else if (CompareField::NameIndex == cmp)
    {
        if (int res = QString::compare(i1.name, i2.name))
            return res;
        return COMPARE_ITEM(i1.index, i2.index);
    }
    return -1;

    #undef COMPARE_ITEM
}

bool Identity::isNull() const
{
    return (id.isNull() && (index < 0) && name.isEmpty());
}

QString Identity::toString() const
{
    QString str = "{";
    bool nextComma = false;
    if (!id.isNull())
    {
        str += "id: " + digest4(id);
        nextComma = true;
    }
    if (index > -1)
    {
        if (nextComma) str += ", ";
        str += "index: " + QString::number(index);
        nextComma = true;
    }
    if (!name.isEmpty())
    {
        if (nextComma) str += ", ";
        str += "name: " + name;
    }
    str += "}";
    return str;
}
