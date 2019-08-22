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

#ifndef RESEARCHERMODEL_HPP
#define RESEARCHERMODEL_HPP

#include <Models/TableModel.hpp>
#include <Util/ResearcherFrame.hpp>

class ResearcherModel : public TableModel<ResearcherFrame>
{
    Q_OBJECT

public :
    ResearcherModel(QObject *parent, bool is64Bit);
    void setFlag(bool is64Bit);
    void setHex(const QVector<bool> &hex);
    int columnCount(const QModelIndex &index = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QModelIndex search(const QString &string, u64 result, int row);

private:
    bool flag;
    QVector<bool> hex;

    QStringList header1 =
    {
        tr("Frame"), tr("64Bit"), tr("32Bit High"), tr("32Bit Low"), tr("16Bit High"), tr("16Bit Low"), tr("Custom 1"), tr("Custom 2"), tr("Custom 3"),
        tr("Custom 4"), tr("Custom 5"), tr("Custom 6"), tr("Custom 7"), tr("Custom 8"), tr("Custom 9"), tr("Custom 10"), "%3", "%25", "%100", "/656",
        tr("HBit"), tr("LBit")
    };
    QStringList header2 =
    {
        tr("Frame"), tr("32Bit"), tr("16Bit High"), tr("16Bit Low"), tr("Custom 1"), tr("Custom 2"), tr("Custom 3"), tr("Custom 4"), tr("Custom 5"),
        tr("Custom 6"), tr("Custom 7"), tr("Custom 8"), tr("Custom 9"), tr("Custom 10"), "%3", "%25", "%100", "/656", tr("HBit"), tr("LBit")
    };

    static u64 get64Bit(const ResearcherFrame &f) { return f.getFull64(); }
    static u64 get32BitHigh(const ResearcherFrame &f) { return f.getHigh32(); }
    static u64 get32BitLow(const ResearcherFrame &f) { return f.getLow32(); }
    static u64 get32(const ResearcherFrame &f) { return f.getFull32(); }
    static u64 get16BitHigh(const ResearcherFrame &f) { return f.getHigh16(); }
    static u64 get16BitLow(const ResearcherFrame &f) { return f.getLow16(); }

};

#endif // RESEARCHERMODEL_HPP
