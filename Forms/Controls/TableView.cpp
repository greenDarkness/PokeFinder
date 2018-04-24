/*
 * This file is part of PokéFinder
 * Copyright (C) 2017 by Admiral_Fish, bumba, and EzPzStreamz
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

#include "TableView.hpp"

void TableView::resizeEvent(QResizeEvent *event)
{
    QTableView::resizeEvent(event);

    int w = event->size().width() - event->oldSize().width();

    if (w > 10 || w < -10)
    {
        QHeaderView *header = horizontalHeader();
        int val = horizontalHeader()->width();
        int width = val / header->count();

        for (int column = 0; column < header->count(); column++)
        {
            header->setSectionResizeMode(column, QHeaderView::Interactive);
            header->resizeSection(column, width);
        }

        int j = 0;
        for (int i = width * header->count(); i < val; i++)
        {
            header->resizeSection(j, header->sectionSize(j) + 1);
            j++;
        }
    }
}
