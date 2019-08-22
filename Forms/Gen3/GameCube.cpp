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

#include <QClipboard>
#include <QSettings>
#include "GameCube.hpp"
#include "ui_GameCube.h"
#include <Core/Gen3/Generator3.hpp>
#include <Core/Gen3/IVSearcher3.hpp>
#include <Core/Util/Nature.hpp>
#include <Core/Util/Power.hpp>
#include <Core/Util/Translator.hpp>
#include <Forms/Gen3/GameCubeRTC.hpp>
#include <Forms/Gen3/ProfileManager3.hpp>

GameCube::GameCube(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GameCube)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_QuitOnClose, false);

    ui->labelSearcherShadow->setVisible(false);
    ui->comboBoxSearcherShadow->setVisible(false);
    ui->labelGeneratorShadow->setVisible(false);
    ui->comboBoxGeneratorShadow->setVisible(false);
    ui->labelGeneratorType->setVisible(false);
    ui->comboBoxGeneratorType->setVisible(false);

    updateProfiles();
    setupModels();

    qRegisterMetaType<QVector<Frame3>>("QVector<Frame3>");
}

GameCube::~GameCube()
{
    QSettings setting;
    setting.beginGroup("gamecube");
    setting.setValue("profile", ui->comboBoxProfiles->currentIndex());
    setting.setValue("geometry", this->saveGeometry());
    setting.endGroup();

    delete ui;
}

void GameCube::updateProfiles()
{
    profiles = { Profile3(tr("None"), Game::Gales, 12345, 54321, Language::English) };
    for (const auto &profile : Profile3::loadProfileList())
    {
        if (profile.getVersion() & Game::GC)
        {
            profiles.append(profile);
        }
    }

    ui->comboBoxProfiles->clear();
    for (const auto &profile : profiles)
    {
        ui->comboBoxProfiles->addItem(profile.getProfileName());
    }

    QSettings setting;
    int val = setting.value("gamecube/profile", 0).toInt();
    if (val < ui->comboBoxProfiles->count())
    {
        ui->comboBoxProfiles->setCurrentIndex(val);
    }
}

void GameCube::setupModels()
{
    generatorModel = new Stationary3Model(ui->tableViewGenerator);
    searcherModel = new Searcher3Model(ui->tableViewSearcher, Method::XDColo);
    generatorMenu = new QMenu(ui->tableViewGenerator);
    searcherMenu = new QMenu(ui->tableViewSearcher);

    ui->tableViewGenerator->setModel(generatorModel);
    ui->tableViewSearcher->setModel(searcherModel);

    ui->textBoxGeneratorSeed->setValues(InputType::Seed32Bit);
    ui->textBoxGeneratorTID->setValues(InputType::TIDSID);
    ui->textBoxGeneratorSID->setValues(InputType::TIDSID);
    ui->textBoxGeneratorStartingFrame->setValues(InputType::Frame32Bit);
    ui->textBoxGeneratorMaxResults->setValues(InputType::Frame32Bit);
    ui->textBoxGeneratorDelay->setValues(InputType::Frame32Bit);

    ui->textBoxSearcherTID->setValues(InputType::TIDSID);
    ui->textBoxSearcherSID->setValues(InputType::TIDSID);

    ui->comboBoxGeneratorNature->setup(Nature::getNatures());
    ui->comboBoxSearcherNature->setup(Nature::getNatures());

    ui->comboBoxGeneratorHiddenPower->setup(Power::getPowers());
    ui->comboBoxSearcherHiddenPower->setup(Power::getPowers());

    ui->comboBoxGeneratorMethod->addItem(tr("XD/Colo"), Method::XDColo);
    ui->comboBoxGeneratorMethod->addItem(tr("Gales"), Method::XD);
    ui->comboBoxGeneratorMethod->addItem(tr("Colo"), Method::Colo);
    ui->comboBoxGeneratorMethod->addItem(tr("Channel"), Method::Channel);
    ui->comboBoxSearcherMethod->addItem(tr("XD/Colo"), Method::XDColo);
    ui->comboBoxSearcherMethod->addItem(tr("Gales"), Method::XD);
    ui->comboBoxSearcherMethod->addItem(tr("Colo"), Method::Colo);
    ui->comboBoxSearcherMethod->addItem(tr("Channel"), Method::Channel);

    ui->comboBoxGeneratorGenderRatio->setItemData(0, 0);
    ui->comboBoxGeneratorGenderRatio->setItemData(1, 127);
    ui->comboBoxGeneratorGenderRatio->setItemData(2, 191);
    ui->comboBoxGeneratorGenderRatio->setItemData(3, 63);
    ui->comboBoxGeneratorGenderRatio->setItemData(4, 31);
    ui->comboBoxGeneratorGenderRatio->setItemData(5, 1);
    ui->comboBoxGeneratorGenderRatio->setItemData(6, 2);

    ui->comboBoxSearcherGenderRatio->setItemData(0, 0);
    ui->comboBoxSearcherGenderRatio->setItemData(1, 127);
    ui->comboBoxSearcherGenderRatio->setItemData(2, 191);
    ui->comboBoxSearcherGenderRatio->setItemData(3, 63);
    ui->comboBoxSearcherGenderRatio->setItemData(4, 31);
    ui->comboBoxSearcherGenderRatio->setItemData(5, 1);
    ui->comboBoxSearcherGenderRatio->setItemData(6, 2);

    QAction *outputTXTGenerator = generatorMenu->addAction(tr("Output Results to TXT"));
    QAction *outputCSVGenerator = generatorMenu->addAction(tr("Output Results to CSV"));

    connect(outputTXTGenerator, &QAction::triggered, this, [ = ]() { ui->tableViewGenerator->outputModelTXT(); });
    connect(outputCSVGenerator, &QAction::triggered, this, [ = ]() { ui->tableViewGenerator->outputModelCSV(); });

    QAction *copySeedToClipboard = searcherMenu->addAction(tr("Copy Seed to Clipboard"));
    QAction *seedToTime = searcherMenu->addAction(tr("Generate times for seed"));
    QAction *outputTXTSearcher = searcherMenu->addAction(tr("Output Results to TXT"));
    QAction *outputCSVSearcher = searcherMenu->addAction(tr("Output Results to CSV"));

    connect(copySeedToClipboard, &QAction::triggered, this, &GameCube::copySeedToClipboard);
    connect(seedToTime, &QAction::triggered, this, &GameCube::seedToTime);
    connect(outputTXTSearcher, &QAction::triggered, this, [ = ]() { ui->tableViewSearcher->outputModelTXT(); });
    connect(outputCSVSearcher, &QAction::triggered, this, [ = ]() { ui->tableViewSearcher->outputModelCSV(); });

    QSettings setting;
    if (setting.contains("gamecube/geometry")) this->restoreGeometry(setting.value("gamecube/geometry").toByteArray());
}

void GameCube::updateProgress(const QVector<Frame3> &frames, int progress)
{
    searcherModel->addItems(frames);
    ui->progressBar->setValue(progress);
}

void GameCube::refreshProfiles()
{
    emit alertProfiles(3);
}

void GameCube::on_comboBoxProfiles_currentIndexChanged(int index)
{
    if (index < 0)
    {
        return;
    }

    auto profile = profiles.at(index);
    QString tid = QString::number(profile.getTID());
    QString sid = QString::number(profile.getSID());

    ui->textBoxGeneratorTID->setText(tid);
    ui->textBoxGeneratorSID->setText(sid);
    ui->textBoxSearcherTID->setText(tid);
    ui->textBoxSearcherSID->setText(sid);
    ui->labelProfileTIDValue->setText(tid);
    ui->labelProfileSIDValue->setText(sid);
    ui->labelProfileGameValue->setText(profile.getVersionString());
}

void GameCube::on_pushButtonGenerate_clicked()
{
    generatorModel->clearModel();

    u32 seed = ui->textBoxGeneratorSeed->getUInt();
    u32 startingFrame = ui->textBoxGeneratorStartingFrame->getUInt();
    u32 maxResults = ui->textBoxGeneratorMaxResults->getUInt();
    u16 tid = ui->textBoxGeneratorTID->getUShort();
    u16 sid = ui->textBoxGeneratorSID->getUShort();
    u32 offset = 0;
    if (ui->checkBoxGeneratorDelay->isChecked())
    {
        offset = ui->textBoxGeneratorDelay->getUInt();
    }

    u8 genderRatio = ui->comboBoxGeneratorGenderRatio->currentData().toUInt();
    Generator3 generator(maxResults, startingFrame, seed, tid, sid, offset, genderRatio);
    FrameCompare compare(ui->comboBoxGeneratorGender->currentIndex(), ui->comboBoxGeneratorAbility->currentIndex(),
                         ui->checkBoxGeneratorShinyOnly->isChecked(), ui->checkBoxGeneratorDisableFilters->isChecked(),
                         ui->ivFilterGenerator->getLower(), ui->ivFilterGenerator->getUpper(), ui->comboBoxGeneratorNature->getChecked(),
                         ui->comboBoxGeneratorHiddenPower->getChecked(), QVector<bool>());

    Method method = static_cast<Method>(ui->comboBoxGeneratorMethod->currentData().toInt());
    generator.setup(method);

    if (method == Method::XD || method == Method::Colo)
    {
        generator.setShadowTeam(ui->comboBoxGeneratorShadow->currentIndex(), ui->comboBoxGeneratorType->currentIndex());
    }

    QVector<Frame3> frames = generator.generate(compare);
    generatorModel->addItems(frames);
}

void GameCube::on_pushButtonSearch_clicked()
{
    searcherModel->clearModel();
    searcherModel->setMethod(static_cast<Method>(ui->comboBoxSearcherMethod->currentData().toInt()));

    ui->pushButtonSearch->setEnabled(false);
    ui->pushButtonCancel->setEnabled(true);

    u16 tid = ui->textBoxSearcherTID->getUShort();
    u16 sid = ui->textBoxSearcherSID->getUShort();

    u8 genderRatio = ui->comboBoxSearcherGenderRatio->currentData().toUInt();
    FrameCompare compare(ui->comboBoxSearcherGender->currentIndex(), ui->comboBoxSearcherAbility->currentIndex(),
                         ui->checkBoxSearcherShinyOnly->isChecked(), false,
                         ui->ivFilterSearcher->getLower(), ui->ivFilterSearcher->getUpper(), ui->comboBoxSearcherNature->getChecked(),
                         ui->comboBoxSearcherHiddenPower->getChecked(), QVector<bool>());
    Searcher3 searcher(tid, sid, genderRatio, compare);

    searcher.setup(static_cast<Method>(ui->comboBoxSearcherMethod->currentData().toInt()));
    if (searcher.getFrameType() == Method::XD || searcher.getFrameType() == Method::Colo)
    {
        searcher.setupNatureLock(ui->comboBoxSearcherShadow->currentIndex());
    }

    QVector<u8> min = ui->ivFilterSearcher->getLower();
    QVector<u8> max = ui->ivFilterSearcher->getUpper();

    int maxProgress = 1;
    for (u8 i = 0; i < 6; i++)
    {
        maxProgress *= max.at(i) - min.at(i) + 1;
    }

    ui->progressBar->setValue(0);
    ui->progressBar->setMaximum(maxProgress);

    auto *search = new IVSearcher3(searcher, min, max);

    connect(search, &IVSearcher3::finished, this, [ = ] { ui->pushButtonSearch->setEnabled(true); ui->pushButtonCancel->setEnabled(false); });
    connect(search, &IVSearcher3::updateProgress, this, &GameCube::updateProgress);
    connect(ui->pushButtonCancel, &QPushButton::clicked, search, &IVSearcher3::cancelSearch);

    search->startSearch();
}

void GameCube::on_comboBoxGeneratorMethod_currentIndexChanged(int /*index*/)
{
    Method method = static_cast<Method>(ui->comboBoxGeneratorMethod->currentData().toInt());
    ui->comboBoxGeneratorShadow->clear();

    if (method == Method::XD)
    {
        QStringList s = Translator::getSpecies(
        {
            334, 24, 354, 12, 113, 301, 85, 149, 51, 355, 125, 83, 55, 88, 58,
            316, 316, 316, 107, 106, 97, 115, 131, 165, 108, 337, 219, 126, 82,
            296, 310, 105, 303, 52, 122, 177, 299, 322, 46, 17, 204, 127, 62, 261,
            57, 280, 78, 20, 315, 302, 373, 123, 273, 273, 273, 86, 285, 143, 361,
            338, 21, 363, 363, 363, 167, 121, 220, 114, 49, 100, 37, 70
        });

        s[15] += tr(" (Citadark)");
        s[16] += tr(" (Initial)");
        s[17] += tr(" (Phenac)");
        s[52] += tr(" (Citadark)");
        s[53] += tr(" (Initial)");
        s[54] += tr(" (Phenac)");
        s[61] += tr(" (Citadark)");
        s[62] += tr(" (Initial)");
        s[63] += tr(" (Phenac)");

        ui->comboBoxGeneratorShadow->addItems(s);
        ui->comboBoxGeneratorShadow->setVisible(true);
        ui->labelGeneratorShadow->setVisible(true);

        QVector<int> secondShadows = { 0, 2, 3, 4, 14, 20, 22, 26, 34, 41, 49, 50, 57, 71 };
        ui->comboBoxGeneratorType->setVisible(secondShadows.contains(ui->comboBoxGeneratorShadow->currentIndex()));
        ui->labelGeneratorType->setVisible(secondShadows.contains(ui->comboBoxGeneratorShadow->currentIndex()));
    }
    else if (method == Method::Colo)
    {
        QStringList s = Translator::getSpecies({ 207, 214, 296, 179, 198, 212, 175, 217 });
        s[3] += tr(" (E-Reader)");
        s[5] += tr(" (E-Reader)");
        s[6] += tr(" (E-Reader)");
        ui->comboBoxGeneratorShadow->addItems(s);
        ui->comboBoxGeneratorShadow->setVisible(true);
        ui->labelGeneratorShadow->setVisible(true);
        ui->comboBoxGeneratorType->setVisible(false);
        ui->labelGeneratorType->setVisible(false);
    }
    else
    {
        ui->comboBoxGeneratorShadow->setVisible(false);
        ui->labelGeneratorShadow->setVisible(false);
        ui->comboBoxGeneratorType->setVisible(false);
        ui->labelGeneratorType->setVisible(false);
    }
}

void GameCube::on_comboBoxGeneratorShadow_currentIndexChanged(int index)
{
    Method version = static_cast<Method>(ui->comboBoxGeneratorMethod->currentData().toInt());
    if (ui->comboBoxGeneratorShadow->isVisible() && version == Method::XD)
    {
        QVector<int> secondShadows = { 0, 2, 3, 4, 14, 20, 22, 26, 34, 41, 49, 50, 57, 71 };
        ui->comboBoxGeneratorType->setVisible(secondShadows.contains(index));
        ui->labelGeneratorType->setVisible(secondShadows.contains(index));
    }
    else
    {
        ui->comboBoxGeneratorType->setVisible(false);
        ui->labelGeneratorType->setVisible(false);
    }
}

void GameCube::on_comboBoxSearcherMethod_currentIndexChanged(int /*index*/)
{
    Method method = static_cast<Method>(ui->comboBoxSearcherMethod->currentData().toInt());
    ui->comboBoxSearcherShadow->clear();

    if (method == Method::XD)
    {
        QStringList s = Translator::getSpecies(
        {
            334, 24, 354, 12, 113, 301, 85, 149, 51, 355, 125, 83, 55, 88, 58,
            316, 316, 316, 107, 106, 97, 115, 131, 165, 108, 337, 219, 126, 82,
            296, 310, 105, 303, 52, 122, 177, 299, 322, 46, 17, 204, 127, 62, 261,
            57, 280, 78, 20, 315, 302, 373, 123, 273, 273, 273, 86, 285, 143, 361,
            338, 21, 363, 363, 363, 167, 121, 220, 114, 49, 100, 37, 70
        });

        s[15] += tr(" (Citadark)");
        s[16] += tr(" (Initial)");
        s[17] += tr(" (Phenac)");
        s[52] += tr(" (Citadark)");
        s[53] += tr(" (Initial)");
        s[54] += tr(" (Phenac)");
        s[61] += tr(" (Citadark)");
        s[62] += tr(" (Initial)");
        s[63] += tr(" (Phenac)");

        ui->comboBoxSearcherShadow->addItems(s);
        ui->comboBoxSearcherShadow->setVisible(true);
        ui->labelSearcherShadow->setVisible(true);
    }
    else if (method == Method::Colo)
    {
        QStringList s = Translator::getSpecies({ 207, 214, 296, 179, 198, 212, 175, 217 });
        s[3] += tr(" (E-Reader)");
        s[5] += tr(" (E-Reader)");
        s[6] += tr(" (E-Reader)");
        ui->comboBoxSearcherShadow->addItems(s);
        ui->comboBoxSearcherShadow->setVisible(true);
        ui->labelSearcherShadow->setVisible(true);
    }
    else
    {
        ui->comboBoxSearcherShadow->setVisible(false);
        ui->labelSearcherShadow->setVisible(false);
    }
}

void GameCube::on_tableViewGenerator_customContextMenuRequested(const QPoint &pos)
{
    if (generatorModel->rowCount() == 0)
    {
        return;
    }

    generatorMenu->popup(ui->tableViewGenerator->viewport()->mapToGlobal(pos));
}

void GameCube::on_tableViewSearcher_customContextMenuRequested(const QPoint &pos)
{
    if (searcherModel->rowCount() == 0)
    {
        return;
    }

    searcherMenu->popup(ui->tableViewSearcher->viewport()->mapToGlobal(pos));
}

void GameCube::seedToTime()
{
    QModelIndex index = ui->tableViewSearcher->currentIndex();
    u32 seed = searcherModel->data(searcherModel->index(index.row(), 0), Qt::DisplayRole).toString().toUInt(nullptr, 16);
    auto *rtc = new GameCubeRTC(seed);
    rtc->show();
    rtc->raise();
}

void GameCube::copySeedToClipboard()
{
    QModelIndex index = ui->tableViewSearcher->currentIndex();
    QApplication::clipboard()->setText(searcherModel->data(searcherModel->index(index.row(), 0), Qt::DisplayRole).toString());
}

void GameCube::on_pushButtonProfileManager_clicked()
{
    auto *manager = new ProfileManager3();
    connect(manager, &ProfileManager3::updateProfiles, this, &GameCube::refreshProfiles);
    manager->show();
}
