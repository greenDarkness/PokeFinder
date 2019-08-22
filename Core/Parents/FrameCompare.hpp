/*
 * This file is part of PokéFinder
 * Copyright (C) 2017-2019 by Admiral_Fish, bumba, and EzPzStreamz
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef FRAMECOMPARE_HPP
#define FRAMECOMPARE_HPP

#include <QVector>
#include <Core/Parents/Frame.hpp>

class FrameCompare
{

public:
    FrameCompare() = default;
    FrameCompare(int gender, int ability, bool shiny, bool skip, const QVector<u8> &min, const QVector<u8> &max,
                 const QVector<bool> &natures, const QVector<bool> &powers, const QVector<bool> &encounters);
    bool comparePID(const Frame &frame) const;
    bool compareIVs(const Frame &frame) const;
    bool compareNature(const Frame &frame) const;
    bool compareHiddenPower(const Frame &frame) const;
    bool compareSlot(const Frame &frame) const;
    bool compareGender(const Frame &frame) const;
    bool compareFrame(const Frame &frame) const;

private:
    QVector<u8> min;
    QVector<u8> max;
    u8 gender{};
    u8 ability{};
    QVector<bool> natures;
    QVector<bool> powers;
    QVector<bool> encounters;
    bool shiny{};
    bool skip{};

};

#endif // FRAMECOMPARE_HPP
