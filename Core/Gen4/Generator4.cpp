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

#include "Generator4.hpp"
#include <Core/RNG/LCRNG.hpp>
#include <Core/Util/EncounterSlot.hpp>

Generator4::Generator4()
{
    maxResults = 100000;
    initialFrame = 1;
    initialSeed = 0;
    tid = 12345;
    sid = 54321;
    psv = tid ^ sid;
}

Generator4::Generator4(u32 maxResults, u32 initialFrame, u32 initialSeed, u16 tid, u16 sid, u32 offset, Method type, u8 genderRatio)
{
    this->maxResults = maxResults;
    this->initialFrame = initialFrame;
    this->initialSeed = initialSeed;
    this->tid = tid;
    this->sid = sid;
    psv = tid ^ sid;
    this->offset = offset;
    frameType = type;
    this->genderRatio = genderRatio;
}

void Generator4::setEncounter(const EncounterArea4 &value)
{
    encounter = value;
}

QVector<Frame4> Generator4::generate(const FrameCompare &compare) const
{
    switch (frameType)
    {
        case Method::Method1:
            return generateMethod1(compare);
        case Method::MethodJ:
            switch (encounterType)
            {
                case Encounter::Stationary:
                    return generateMethodJStationary(compare);
                default:
                    return generateMethodJWild(compare);
            }
        case Method::MethodK:
            switch (encounterType)
            {
                case Encounter::Stationary:
                    return generateMethodKStationary(compare);
                default:
                    return generateMethodKWild(compare);
            }
        case Method::ChainedShiny:
            return generateChainedShiny(compare);
        case Method::WondercardIVs:
            return generateWondercardIVs(compare);
        default:
            return QVector<Frame4>();
    }
}

QVector<Frame4> Generator4::generateMethod1(const FrameCompare &compare) const
{
    QVector<Frame4> frames;
    Frame4 frame(tid, sid, psv);

    PokeRNG rng(initialSeed, initialFrame - 1 + offset);
    QVector<u16> rngList(maxResults + 4);
    for (u16 &x : rngList)
    {
        x = rng.nextUShort();
    }

    // Method 1 [SEED] [PID] [PID] [IVS] [IVS]

    for (u32 cnt = 0; cnt < maxResults; cnt++)
    {
        u16 high = rngList.at(cnt + 1);
        u16 low = rngList.at(cnt);
        u16 iv1 = rngList.at(cnt + 2);
        u16 iv2 = rngList.at(cnt + 3);

        frame.setPID(high, low, genderRatio);
        frame.setIVs(iv1, iv2);

        if (compare.compareFrame(frame))
        {
            frame.setSeed(rngList.at(cnt));
            frame.setFrame(cnt + initialFrame);
            frames.append(frame);
        }
    }

    return frames;
}

QVector<Frame4> Generator4::generateMethodJStationary(const FrameCompare &compare) const
{
    QVector<Frame4> frames;
    Frame4 frame(tid, sid, psv);

    PokeRNG rng(initialSeed, initialFrame - 1 + offset);
    u32 max = initialFrame + maxResults;
    u32 pid;
    u16 low, high;

    u8 buffer = 0;

    switch (leadType)
    {
        case Lead::CuteCharmFemale:
            buffer = 0;
            break;
        case Lead::CuteCharm25M:
            buffer = 0xC8;
            break;
        case Lead::CuteCharm50M:
            buffer = 0x96;
            break;
        case Lead::CuteCharm75M:
            buffer = 0x4B;
            break;
        case Lead::CuteCharm875M:
            buffer = 0x32;
            break;
        default:
            break;
    }

    for (u32 cnt = initialFrame; cnt < max; cnt++)
    {
        PokeRNG go(rng.nextUInt());
        frame.setSeed(go.getSeed() >> 16);

        switch (leadType)
        {
            case Lead::None:
                // Get hunt nature
                frame.setNature(go.nextUShort() / 0xa3e);

                if (!compare.compareNature(frame))
                {
                    continue;
                }

                // Begin search for valid pid
                do
                {
                    low = go.nextUShort();
                    high = go.nextUShort();
                    pid = (high << 16) | low;
                }
                while (pid % 25 != frame.getNature());
                frame.setPID(pid, genderRatio);

                break;
            case Lead::Synchronize:
                if ((go.nextUShort() >> 15) == 0) // Successful synch
                {
                    frame.setNature(synchNature);
                }
                else // Failed synch
                {
                    frame.setNature(go.nextUShort() / 0xa3e);
                }

                if (!compare.compareNature(frame))
                {
                    continue;
                }

                // Begin search for valid pid
                do
                {
                    low = go.nextUShort();
                    high = go.nextUShort();
                    pid = (high << 16) | low;
                }
                while (pid % 25 != frame.getNature());
                frame.setPID(pid, genderRatio);

                break;
            default: // Default to cover all cute charm cases
                if ((go.nextUShort() / 0x5556) != 0) // Successful cute charm
                {
                    // Get nature
                    frame.setNature(go.nextUShort() / 0xa3e);

                    if (!compare.compareNature(frame))
                    {
                        continue;
                    }

                    // Cute charm doesn't hunt for a valid PID, just uses buffer and target nature
                    frame.setPID(buffer + frame.getNature(), genderRatio);
                }
                else // Failed cute charm
                {
                    // Get nature
                    frame.setNature(go.nextUShort() / 0xa3e);

                    if (!compare.compareNature(frame))
                    {
                        continue;
                    }

                    // Begin search for valid pid
                    do
                    {
                        low = go.nextUShort();
                        high = go.nextUShort();
                        pid = (high << 16) | low;
                    }
                    while (pid % 25 != frame.getNature());
                    frame.setPID(pid, genderRatio);
                }

                break;
        }

        u16 iv1 = go.nextUShort();
        u16 iv2 = go.nextUShort();

        frame.setIVs(iv1, iv2);

        if (compare.compareFrame(frame))
        {
            frame.setFrame(cnt);
            frames.append(frame);
        }
    }

    return frames;
}

QVector<Frame4> Generator4::generateMethodJWild(const FrameCompare &compare) const
{
    QVector<Frame4> frames;
    Frame4 frame(tid, sid, psv);

    PokeRNG rng(initialSeed, initialFrame - 1 + offset);
    u32 max = initialFrame + maxResults;
    u32 pid, hunt = 0;
    u16 low, high;

    u8 buffer = 0;
    u8 thresh = encounterType == Encounter::OldRod ? 25 : encounterType == Encounter::GoodRod ? 50 : encounterType == Encounter::SuperRod ? 75 : 0;

    switch (leadType)
    {
        case Lead::CuteCharmFemale:
            buffer = 0;
            break;
        case Lead::CuteCharm25M:
            buffer = 0xC8;
            break;
        case Lead::CuteCharm50M:
            buffer = 0x96;
            break;
        case Lead::CuteCharm75M:
            buffer = 0x4B;
            break;
        case Lead::CuteCharm875M:
            buffer = 0x32;
            break;
        default:
            break;
    }

    for (u32 cnt = initialFrame; cnt < max; cnt++)
    {
        PokeRNG go(rng.nextUInt());
        frame.setSeed(go.getSeed() >> 16);

        switch (encounterType)
        {
            case Encounter::Grass:
                frame.setEncounterSlot(EncounterSlot::jSlot(go.getSeed() >> 16, encounterType));
                if (!compare.compareSlot(frame))
                {
                    continue;
                }

                frame.setLevel(encounter.calcLevel(frame.getEncounterSlot()));
                hunt = 0;
                break;
            case Encounter::Surfing:
                frame.setEncounterSlot(EncounterSlot::jSlot(go.getSeed() >> 16, encounterType));
                if (!compare.compareSlot(frame))
                {
                    continue;
                }

                frame.setLevel(encounter.calcLevel(frame.getEncounterSlot(), go.nextUShort()));
                hunt = 1;
                break;
            case Encounter::OldRod:
            case Encounter::GoodRod:
            case Encounter::SuperRod:
                if (((go.getSeed() >> 16) / 656) >= thresh)
                {
                    continue;
                }

                frame.setEncounterSlot(EncounterSlot::jSlot(go.nextUShort(), encounterType));
                if (!compare.compareSlot(frame))
                {
                    continue;
                }

                frame.setLevel(encounter.calcLevel(frame.getEncounterSlot(), go.nextUShort()));
                hunt = 1;
                break;
            default:
                break;
        }

        switch (leadType)
        {
            case Lead::None:
                // Get hunt nature
                frame.setNature(go.nextUShort() / 0xa3e);

                if (!compare.compareNature(frame))
                {
                    continue;
                }

                // Begin search for valid pid
                do
                {
                    low = go.nextUShort();
                    high = go.nextUShort();
                    pid = (high << 16) | low;
                    hunt += 2;
                }
                while (pid % 25 != frame.getNature());
                frame.setPID(pid, genderRatio);

                break;
            case Lead::Synchronize:
                hunt++;
                if ((go.nextUShort() >> 15) == 0) // Successful synch
                {
                    frame.setNature(synchNature);
                }
                else // Failed synch
                {
                    frame.setNature(go.nextUShort() / 0xa3e);
                }

                if (!compare.compareNature(frame))
                {
                    continue;
                }

                // Begin search for valid pid
                do
                {
                    low = go.nextUShort();
                    high = go.nextUShort();
                    pid = (high << 16) | low;
                    hunt += 2;
                }
                while (pid % 25 != frame.getNature());
                frame.setPID(pid, genderRatio);

                break;
            default: // Default to cover all cute charm cases
                if ((go.nextUShort() / 0x5556) != 0) // Successful cute charm
                {
                    // Get nature
                    frame.setNature(go.nextUShort() / 0xa3e);

                    if (!compare.compareNature(frame))
                    {
                        continue;
                    }

                    // Cute charm doesn't hunt for a valid PID, just uses buffer and target nature
                    frame.setPID(buffer + frame.getNature(), genderRatio);
                    hunt = 1;
                }
                else // Failed cute charm
                {
                    hunt++;

                    // Get nature
                    frame.setNature(go.nextUShort() / 0xa3e);

                    if (!compare.compareNature(frame))
                    {
                        continue;
                    }

                    // Begin search for valid pid
                    do
                    {
                        low = go.nextUShort();
                        high = go.nextUShort();
                        pid = (high << 16) | low;
                        hunt += 2;
                    }
                    while (pid % 25 != frame.getNature());
                    frame.setPID(pid, genderRatio);
                }

                break;
        }

        u16 iv1 = go.nextUShort();
        u16 iv2 = go.nextUShort();

        frame.setIVs(iv1, iv2);

        if (compare.compareFrame(frame))
        {
            frame.setFrame(cnt);
            frame.setOccidentary(hunt + cnt);
            frames.append(frame);
        }
    }

    return frames;
}

QVector<Frame4> Generator4::generateMethodKStationary(const FrameCompare &compare) const
{
    QVector<Frame4> frames;
    Frame4 frame(tid, sid, psv);

    PokeRNG rng(initialSeed, initialFrame - 1 + offset);
    u32 max = initialFrame + maxResults;
    u32 pid;
    u16 low, high;

    u8 buffer = 0;

    switch (leadType)
    {
        case Lead::CuteCharmFemale:
            buffer = 0;
            break;
        case Lead::CuteCharm25M:
            buffer = 0xC8;
            break;
        case Lead::CuteCharm50M:
            buffer = 0x96;
            break;
        case Lead::CuteCharm75M:
            buffer = 0x4B;
            break;
        case Lead::CuteCharm875M:
            buffer = 0x32;
            break;
        default:
            break;
    }

    for (u32 cnt = initialFrame; cnt < max; cnt++)
    {
        PokeRNG go(rng.nextUInt());
        frame.setSeed(go.getSeed() >> 16);

        switch (leadType)
        {
            case Lead::None:
                // Get hunt nature
                frame.setNature(go.nextUShort() % 25);

                if (!compare.compareNature(frame))
                {
                    continue;
                }

                // Begin search for valid pid
                do
                {
                    low = go.nextUShort();
                    high = go.nextUShort();
                    pid = (high << 16) | low;
                }
                while (pid % 25 != frame.getNature());
                frame.setPID(pid, genderRatio);

                break;
            case Lead::Synchronize:
                if ((go.nextUShort() & 1) == 0) // Successful synch
                {
                    frame.setNature(synchNature);
                }
                else // Failed synch
                {
                    frame.setNature(go.nextUShort() % 25);
                }

                if (!compare.compareNature(frame))
                {
                    continue;
                }

                // Begin search for valid pid
                do
                {
                    low = go.nextUShort();
                    high = go.nextUShort();
                    pid = (high << 16) | low;
                }
                while (pid % 25 != frame.getNature());
                frame.setPID(pid, genderRatio);

                break;
            default: // Default to cover all cute charm cases
                if ((go.nextUShort() % 3) != 0) // Successfull cute charm
                {
                    // Get hunt nature
                    frame.setNature(go.nextUShort() % 25);

                    if (!compare.compareNature(frame))
                    {
                        continue;
                    }

                    frame.setPID(buffer + frame.getNature(), genderRatio);
                }
                else // Failed cutecharm
                {
                    // Get hunt nature
                    frame.setNature(go.nextUShort() % 25);

                    if (!compare.compareNature(frame))
                    {
                        continue;
                    }

                    // Begin search for valid pid
                    do
                    {
                        low = go.nextUShort();
                        high = go.nextUShort();
                        pid = (high << 16) | low;
                    }
                    while (pid % 25 != frame.getNature());

                    frame.setPID(pid, genderRatio);
                }

                break;
        }

        u16 iv1 = go.nextUShort();
        u16 iv2 = go.nextUShort();

        frame.setIVs(iv1, iv2);

        if (compare.compareFrame(frame))
        {
            frame.setFrame(cnt);
            frames.append(frame);
        }
    }

    return frames;
}

QVector<Frame4> Generator4::generateMethodKWild(const FrameCompare &compare) const
{
    QVector<Frame4> frames;
    Frame4 frame(tid, sid, psv);

    PokeRNG rng(initialSeed, initialFrame - 1 + offset);
    u32 max = initialFrame + maxResults;
    u32 pid, hunt = 0;
    u16 low, high;

    u8 buffer, thresh = 0, rate = encounter.getEncounterRate();
    if (encounterType == Encounter::OldRod)
    {
        thresh = leadType == Lead::SuctionCups ? 90 : 25;
    }
    else if (encounterType == Encounter::GoodRod)
    {
        thresh = leadType == Lead::SuctionCups ? 100 : 50;
    }
    else if (encounterType == Encounter::SuperRod)
    {
        thresh = leadType == Lead::SuctionCups ? 100 : 75;
    }

    switch (leadType)
    {
        case Lead::CuteCharmFemale:
            buffer = 0;
            break;
        case Lead::CuteCharm25M:
            buffer = 0xC8;
            break;
        case Lead::CuteCharm50M:
            buffer = 0x96;
            break;
        case Lead::CuteCharm75M:
            buffer = 0x4B;
            break;
        case Lead::CuteCharm875M:
            buffer = 0x32;
            break;
        default:
            break;
    }

    for (u32 cnt = initialFrame; cnt < max; cnt++)
    {
        PokeRNG go(rng.nextUInt());
        frame.setSeed(go.getSeed() >> 16);

        switch (encounterType)
        {
            case Encounter::Grass:
                frame.setEncounterSlot(EncounterSlot::kSlot(go.getSeed() >> 16, encounterType));
                if (!compare.compareSlot(frame))
                {
                    continue;
                }

                frame.setLevel(encounter.calcLevel(frame.getEncounterSlot()));
                hunt = 1;
                break;
            case Encounter::Surfing:
                frame.setEncounterSlot(EncounterSlot::kSlot(go.getSeed() >> 16, encounterType));
                if (!compare.compareSlot(frame))
                {
                    continue;
                }

                frame.setLevel(encounter.calcLevel(frame.getEncounterSlot(), go.nextUShort()));
                hunt = 2;
                break;
            case Encounter::OldRod:
            case Encounter::GoodRod:
            case Encounter::SuperRod:
                if (((go.getSeed() >> 16) % 100) >= thresh)
                {
                    continue;
                }

                frame.setEncounterSlot(EncounterSlot::kSlot(go.nextUShort(), encounterType));
                if (!compare.compareSlot(frame))
                {
                    continue;
                }

                frame.setLevel(encounter.calcLevel(frame.getEncounterSlot()));
                go.advanceFrames(1);
                hunt = 3;
                break;
            case Encounter::RockSmash:
                // Blank(or maybe item) ???
                if (((go.nextUShort()) % 100) >= rate)
                {
                    continue;
                }

                frame.setEncounterSlot(EncounterSlot::kSlot(go.nextUShort(), encounterType));
                if (!compare.compareSlot(frame))
                {
                    continue;
                }

                frame.setLevel(encounter.calcLevel(frame.getEncounterSlot(), go.nextUShort()));
                hunt = 2;
                break;
            case Encounter::HeadButt: // TODO
            case Encounter::BugCatchingContest: // TODO
            default:
                break;
        }

        switch (leadType)
        {
            case Lead::None:
            case Lead::SuctionCups:
                // Get hunt nature
                frame.setNature(go.nextUShort() % 25);

                if (!compare.compareNature(frame))
                {
                    continue;
                }

                // Begin search for valid pid
                do
                {
                    low = go.nextUShort();
                    high = go.nextUShort();
                    pid = (high << 16) | low;
                    hunt += 2;
                }
                while (pid % 25 != frame.getNature());
                frame.setPID(pid, genderRatio);

                break;
            case Lead::Synchronize:
                hunt++;

                if ((go.nextUShort() & 1) == 0) // Successful synch
                {
                    frame.setNature(synchNature);
                }
                else // Failed synch
                {
                    frame.setNature(go.nextUShort() % 25);
                }

                if (!compare.compareNature(frame))
                {
                    continue;
                }

                // Begin search for valid pid
                do
                {
                    low = go.nextUShort();
                    high = go.nextUShort();
                    pid = (high << 16) | low;
                }
                while (pid % 25 != frame.getNature());
                frame.setPID(pid, genderRatio);

                break;
            default: // Default to cover all cute charm cases
                if ((go.nextUShort() % 3) != 0) // Successfull cute charm
                {
                    // Get hunt nature
                    frame.setNature(go.nextUShort() % 25);

                    if (!compare.compareNature(frame))
                    {
                        continue;
                    }

                    frame.setPID(buffer + frame.getNature(), genderRatio);
                    hunt = 1;
                }
                else // Failed cutecharm
                {
                    hunt++;

                    // Get hunt nature
                    frame.setNature(go.nextUShort() % 25);

                    if (!compare.compareNature(frame))
                    {
                        continue;
                    }

                    // Begin search for valid pid
                    do
                    {
                        low = go.nextUShort();
                        high = go.nextUShort();
                        pid = (high << 16) | low;
                    }
                    while (pid % 25 != frame.getNature());

                    frame.setPID(pid, genderRatio);
                }

                break;
        }

        u16 iv1 = go.nextUShort();
        u16 iv2 = go.nextUShort();

        frame.setIVs(iv1, iv2);

        if (compare.compareFrame(frame))
        {
            frame.setFrame(cnt);
            frames.append(frame);
        }
    }

    return frames;
}

QVector<Frame4> Generator4::generateChainedShiny(const FrameCompare &compare) const
{
    QVector<Frame4> frames;
    Frame4 frame(tid, sid, psv);

    PokeRNG rng(initialSeed, initialFrame - 1 + offset);
    QVector<u16> rngList(maxResults + 18);
    for (u16 &x : rngList)
    {
        x = rng.nextUShort();
    }

    u16 low, high;
    for (u32 cnt = initialFrame; cnt < maxResults; cnt++)
    {
        low = chainedPIDLow(rngList.at(cnt + 1), rngList.at(cnt + 15), rngList.at(cnt + 14), rngList.at(cnt + 13), rngList.at(cnt + 12), rngList.at(cnt + 11), rngList.at(cnt + 10),
                            rngList.at(cnt + 9), rngList.at(cnt + 8), rngList.at(cnt + 7), rngList.at(cnt + 6), rngList.at(cnt + 5), rngList.at(cnt + 4), rngList.at(cnt + 3));
        high = chainedPIDHigh(rngList.at(2 + cnt), low, tid, sid);

        frame.setPID(high, low, genderRatio);
        frame.setIVs(rngList.at(cnt + 16), rngList.at(cnt + 17));

        if (compare.compareFrame(frame))
        {
            frame.setSeed(rngList.at(cnt));
            frame.setFrame(cnt + initialFrame);
            frames.append(frame);
        }
    }

    return frames;
}

QVector<Frame4> Generator4::generateWondercardIVs(const FrameCompare &compare) const
{
    QVector<Frame4> frames;
    Frame4 frame(tid, sid, psv);

    PokeRNG rng(initialSeed, initialFrame - 1 + offset);
    QVector<u16> rngList(maxResults + 2);
    for (u16 &x : rngList)
    {
        x = rng.nextUShort();
    }

    // Wondercard IVs [SEED] [IVS] [IVS]

    for (u32 cnt = 0; cnt < maxResults; cnt++)
    {
        frame.setIVs(rngList.at(cnt), rngList.at(cnt + 1));

        if (compare.compareIVs(frame))
        {
            frame.setSeed(rngList.at(cnt));
            frame.setFrame(cnt + initialFrame);
            frames.append(frame);
        }
    }

    return frames;
}

u16 Generator4::chainedPIDLow(u16 low, u16 call1, u16 call2, u16 call3, u16 call4, u16 call5, u16 call6, u16 call7, u16 call8, u16 call9, u16 call10, u16 call11, u16 call12, u16 call13) const
{
    return (low & 7) | (call13 & 1) << 3 | (call12 & 1) << 4 | (call11 & 1) << 5 | (call10 & 1) << 6 |
           (call9 & 1) << 7 | (call8 & 1) << 8 | (call7 & 1) << 9 | (call6 & 1) << 10 | (call5 & 1) << 11 |
           (call4 & 1) << 12 | (call3 & 1) << 13 | (call2 & 1) << 14 | (call1 & 1) << 15;
}

u16 Generator4::chainedPIDHigh(u16 high, u16 low, u16 tid, u16 sid) const
{
    return (((low ^ tid ^ sid) & 0xFFF8) | (high & 7));
}
