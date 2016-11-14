/***************************************************************************
 *   Copyright ?? 2012 Aleix Pol Gonzalez <aleixpol@blue-systems.com>       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#ifndef UTILS_H
#define UTILS_H

#include <functional>

template <typename T, typename Q, typename _UnaryOperation>
static T kTransform(const Q &input, _UnaryOperation op)
{
    T ret;
    ret.reserve(input.size());
    for(const auto& v : input) {
        ret += op(v);
    }
    return ret;
}

template <typename T, typename Q, typename _UnaryOperation>
static T kFilter(const Q &input, _UnaryOperation op)
{
    T ret;
    for(const auto& v : input) {
        if (op(v))
            ret += v;
    }
    return ret;
}

#endif
