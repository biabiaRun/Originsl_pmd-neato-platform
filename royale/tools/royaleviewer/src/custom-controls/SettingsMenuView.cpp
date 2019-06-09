/****************************************************************************\
 * Copyright (C) 2015 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include "SettingsMenuView.hpp"
#include <QApplication>
#include <QPainter>
#include <QPixmap>
#include <QDebug>
#include "DisplaySupport.hpp"
#include <QScroller>

SettingsMenuView::SettingsMenuView (royale::CameraAccessLevel accessLevel, QWidget *parent)
    : PMDView (parent)
    , delegate (NULL)
    , m_background (NULL)
    , selectedItem (NULL)
    , m_accessLevel (accessLevel)
{
    ui.setupUi (this);
    QObject::connect (ui.listWidget, SIGNAL (itemClicked (QListWidgetItem *)), this, SLOT (settingSelected (QListWidgetItem *)));

    m_checkboxes.clear();
    m_checkboxes.emplace ("Show Frustum", CheckBoxStruct (&SettingsMenuView::enableFrustumDisplay));
    m_checkboxes.emplace ("Lock View", CheckBoxStruct (&SettingsMenuView::enableLockView));
    m_checkboxes.emplace ("Flip vertically", CheckBoxStruct (&SettingsMenuView::enableFlipVertically));
    m_checkboxes.emplace ("Flip horizontally", CheckBoxStruct (&SettingsMenuView::enableFlipHorizontally));
    m_checkboxes.emplace ("Show FPS", CheckBoxStruct (&SettingsMenuView::enableShowFPS));
    m_checkboxes.emplace ("Show StreamID", CheckBoxStruct (&SettingsMenuView::enableShowStreamId));
    m_checkboxes.emplace ("Show the number of valid pixels", CheckBoxStruct (&SettingsMenuView::enableShowValidPixelsNumber));
    m_checkboxes.emplace ("Single frame recording", CheckBoxStruct (&SettingsMenuView::enableSingleFrameRecording));

#ifdef ROYALE_PLAYBACK_ONLY
    updateToolEntries (false, true, false, true);
#else
    updateToolEntries (false, true, false, false);
#endif

    ui.listWidget->setProperty ("separator", true);
    QScroller::grabGesture (ui.listWidget, QScroller::TouchGesture);
}

bool SettingsMenuView::singleFrameRecordingEnabled (bool playBack)
{
    if (playBack)
    {
        return true;
    }
    auto item = ui.listWidget->findItems ("Single frame recording", Qt::MatchExactly);
    if (item.empty())
    {
        return false;
    }
    return item.at (0)->checkState() == Qt::CheckState::Checked;
}

void SettingsMenuView::updateToolEntries (bool cameraAvailable, bool mode2D, bool modeMixed, bool playBack)
{
    m_entries.clear();
    m_entries.push_back ("Color Range");
    if (cameraAvailable)
    {
        m_entries.push_back ("Use Case (FPS)");
        m_entries.push_back ("Exposure Time");
        m_entries.push_back ("Filter level");
    }
    if (m_accessLevel >= royale::CameraAccessLevel::L2)
    {
        if (cameraAvailable && !modeMixed)
        {
            m_entries.push_back ("Frame Rate");
        }
        m_entries.push_back ("Parameters");
    }

    if (!mode2D)
    {
        m_entries.push_back ("Camera Preset");
        m_entries.push_back ("Auto Rotation");
    }
    m_entries.push_back ("Filter (Min/Max)");
    m_entries.push_back ("Data");
    m_entries.push_back ("Load File");

    ui.listWidget->clear();
    ui.listWidget->setResizeMode (QListView::Adjust);
    std::for_each (m_entries.begin(), m_entries.end(), [this] (std::string & item)
    {
        ui.listWidget->addItem (item.c_str());
    });
    int count = ui.listWidget->count();
    for (int i = 0; i < count; i++)
    {
        QListWidgetItem *item = ui.listWidget->item (i);
        item->setSizeHint (QSize (item->sizeHint().width(),
                                  static_cast<int> (DisplaySupport::sharedInstance()->pointsToPixels (40))
                                 ));
        item->setIcon (QIcon (QPixmap (DisplaySupport::sharedInstance()->asset ("icon-colorange"))));
    }
    ui.listWidget->setMinimumHeight (static_cast<int> (DisplaySupport::sharedInstance()->pointsToPixels (40) * static_cast<float> (count)));
    selectedItem = nullptr;

    for (auto &cb : m_checkboxes)
    {
        cb.second.visible = false;
    }

    if (!mode2D)
    {
        m_checkboxes["Show Frustum"].visible = true;
        if (modeMixed)
        {
            m_checkboxes["Lock View"].visible = true;
        }
    }
    if (cameraAvailable)
    {
        m_checkboxes["Flip vertically"].visible = true;
        m_checkboxes["Flip horizontally"].visible = true;
    }
    m_checkboxes["Show FPS"].visible = true;
    m_checkboxes["Show StreamID"].visible = true;
    if (m_accessLevel >= royale::CameraAccessLevel::L2)
    {
        m_checkboxes["Show the number of valid pixels"].visible = true;
    }
    m_checkboxes["Single frame recording"].visible = true;

    if (playBack)
    {
        m_checkboxes["Single frame recording"].visible = false;
    }

    for (auto cb : m_checkboxes)
    {
        if (cb.second.visible)
        {
            auto *item = new QListWidgetItem();
            item->setText (cb.first.c_str());
            item->setFlags (item->flags() | Qt::ItemIsUserCheckable);
            if (cb.second.checkState)
            {
                item->setCheckState (Qt::CheckState::Checked);
            }
            else
            {
                item->setCheckState (Qt::CheckState::Unchecked);
            }

            ui.listWidget->addItem (item);
        }
    }
}

void SettingsMenuView::settingSelected (QListWidgetItem *item)
{
    auto curCheckbox = m_checkboxes.find (item->text().toStdString().c_str());
    if (curCheckbox != m_checkboxes.end())
    {
        deselect();
        curCheckbox->second.checkState = !curCheckbox->second.checkState;
        if (curCheckbox->second.checkState)
        {
            item->setCheckState (Qt::CheckState::Checked);
        }
        else
        {
            item->setCheckState (Qt::CheckState::Unchecked);
        }

        emit (this->*curCheckbox->second.signalFunction) (item->checkState() == Qt::CheckState::Checked);
        return;
    }
    else
    {
        for (size_t i = 0; i < m_entries.size(); ++i)
        {
            QString s = m_entries[i].c_str();
            if (s == item->text())
            {
                if (delegate)
                {
                    if (selectedItem)
                    {
                        if (item->text() == selectedItem->text())
                        {
                            deselect();
                            delegate->settingSelected ("");
                            return;
                        }
                    }
                    selectedItem = item;
                    delegate->settingSelected (s);
                }
                break;
            }
        }
    }
}

void SettingsMenuView::deselect()
{
    selectedItem = NULL;
    ui.listWidget->clearSelection();
    ui.listWidget->clearFocus();
}

void SettingsMenuView::disableRRFRecording (bool val)
{
    auto item = ui.listWidget->findItems ("Single frame recording", Qt::MatchExactly);
    if (item.empty())
    {
        return;
    }
    if (val)
    {
        item.at (0)->setCheckState (Qt::CheckState::Checked);
    }
    else
    {
        item.at (0)->setCheckState (Qt::CheckState::Unchecked);
    }
    item.at (0)->setHidden (val);
}
