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

#ifndef EGGSETTINGS_HPP
#define EGGSETTINGS_HPP

#include <QWidget>
#include <Core/Util/Global.hpp>

namespace Ui
{
    class EggSettings;
}

class EggSettings : public QWidget
{
    Q_OBJECT

signals:
    void toggleInheritance(bool);

public:
    explicit EggSettings(QWidget *parent = nullptr);
    ~EggSettings() override;
    QVector<u8> getParent1() const;
    QVector<u8> getParent2() const;

private:
    Ui::EggSettings *ui;

private slots:
    void on_pushButtonSwapParents_clicked();
    void on_checkBoxShowInteritance_clicked(bool checked);

};

#endif // EGGSETTINGS_HPP
