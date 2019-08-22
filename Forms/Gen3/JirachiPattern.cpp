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

#include <QSettings>
#include "JirachiPattern.hpp"
#include "ui_JirachiPattern.h"
#include <Core/RNG/LCRNG.hpp>

JirachiPattern::JirachiPattern(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::JirachiPattern)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_QuitOnClose, false);
    setAttribute(Qt::WA_DeleteOnClose);

    setupModels();
}

JirachiPattern::~JirachiPattern()
{
    QSettings setting;
    setting.setValue("jirachiPattern/geometry", this->saveGeometry());

    delete ui;
}

void JirachiPattern::setupModels()
{
    model = new QStandardItemModel(ui->tableView);

    ui->textBoxSeed->setValues(InputType::Seed32Bit);

    model->setHorizontalHeaderLabels(QStringList() << tr("Pattern"));
    ui->tableView->setModel(model);

    QSettings setting;
    if (setting.contains("jirachiPattern/geometry")) this->restoreGeometry(setting.value("jirachiPattern/geometry").toByteArray());
}

void JirachiPattern::generate(u32 seed)
{
    QStringList result = getPatterns(seed).toSet().toList();

    if (result.isEmpty())
    {
        model->appendRow(new QStandardItem(tr("Spread Impossible")));
    }
    else
    {
        for (const auto &str : result)
        {
            model->appendRow(new QStandardItem(str));
        }
    }
}

QStringList JirachiPattern::getPatterns(u32 seed)
{
    QStringList results;

    QString pattern = QString::number(seed >> 30);
    data.append(seed);

    XDRNGR rng(seed);

    // Populate backwards data
    for (u8 m = 0; m < 25; m++)
    {
        seed = rng.nextUInt();
        data.append(seed);
        pattern = QString::number(seed >> 30) + "|" + pattern;
    }

    // Loop through 4 possible cases that would make a valid pattern
    for (int i = 0; i < 4; i++)
    {
        // Modify pattern with target, skip if invalid
        QString copy = getTarget(QString(pattern), i);
        if (!copy.isEmpty())
        {
            int index = copy.indexOf(QChar(':')) + 1;

            // Menu advances can't stop on 0 so skip
            u8 target = QString(copy.at(index)).toUInt();
            if (target != 0)
            {
                // From start, game advances frames until (prng >> 30) gives a 1, 2, and 3
                // (prng >> 30) being 0 just acts as a filler
                // Map remaining numbers from 1, 2, 3 without target for later use
                QVector<u8> remain = { 1, 2, 3 };
                remain.removeAll(target);

                QVector<bool> obtain(4, false);
                obtain[target] = true;

                // Determine if spread is possible
                // Need to work backwards to see if going forward with 1, 2, and 3 lands on our target
                bool valid = true;
                for (int x = ((copy.length() - index + 2) / 2); x < data.size(); x++)
                {
                    u8 temp = data.at(x) >> 30;
                    if (temp == target)
                    {
                        // Check if remaining numbers haven't occured yet
                        if (!obtain.at(remain.at(0)) || !obtain.at(remain.at(1)))
                        {
                            // Spread impossible
                            valid = false;
                            break;
                        }
                    }

                    obtain[temp] = true;

                    // Check to see if pattern passes
                    if (obtain.at(1) && obtain.at(2) && obtain.at(3))
                    {
                        break;
                    }
                }

                // This part actually skips when a pattern is impossible
                if (valid)
                {
                    results.append(copy.replace("|", " | "));
                }
            }
        }
    }

    return results;
}

QString JirachiPattern::getTarget(QString in, int index)
{
    // (prng >> 30 == 0) then advance 1
    // (prng >> 30 == 2) then advance 2, (next prng >> 25 > 41) then advance 1 more
    // (prng >> 30 == 1) or (prng >> 30 == 3) then advance 3

    // Return an empty string if math doesn't fit
    switch (index)
    {
        case 0:
            if ((data.at(1) >> 30) == 0) // 6 advances total
            {
                in = in.mid(0, in.length() - 13) + "T:" + in.mid(in.length() - 13);
            }
            else
            {
                return QString();
            }
            break;
        case 1:
            if ((data.at(3) >> 30) == 1 || (data.at(3) >> 30) == 3) // 8 advances total
            {
                in = in.mid(0, in.length() - 17) + "T:" + in.mid(in.length() - 17);
            }
            else
            {
                return QString();
            }
            break;
        case 2:
            if ((data.at(3) >> 30) == 2 && (data.at(1) >> 25) <= 41) // 7 advances total
            {
                in = in.mid(0, in.length() - 15) + "T:" + in.mid(in.length() - 15);
            }
            else
            {
                return QString();
            }
            break;
        case 3:
            if ((data.at(3) >> 30) == 2 && (data.at(1) >> 25) > 41) // 8 advances total
            {
                in = in.mid(0, in.length() - 17) + "T:" + in.mid(in.length() - 17);
            }
            else
            {
                return QString();
            }
            break;
    }
    return in;
}

void JirachiPattern::on_pushButtonGenerate_clicked()
{
    model->removeRows(0, model->rowCount());
    data.clear();
    generate(ui->textBoxSeed->getUInt());
}
