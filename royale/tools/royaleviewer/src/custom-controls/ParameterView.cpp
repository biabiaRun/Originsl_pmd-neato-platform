/****************************************************************************\
 * Copyright (C) 2015 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#include "ParameterView.hpp"
#include "DisplaySupport.hpp"
#include <common/exceptions/OutOfBounds.hpp>
#include <QPainter>
#include <QCheckBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QScroller>

using namespace royale;

ParameterView::ParameterView (QWidget *parent) : PMDStreamView (parent)
{
    ui.setupUi (this);

    ui.parameterTable->setVerticalScrollMode (QAbstractItemView::ScrollPerPixel);
    QScroller::grabGesture (ui.parameterTable->viewport(), QScroller::TouchGesture);

    setFocusPolicy (Qt::StrongFocus);
}

void ParameterView::reset()
{
    ui.parameterTable->setRowCount (0);
    m_flagMapping.clear();
}

void ParameterView::setCurrentParameters (ProcessingParameterVector parameters)
{
    ui.parameterTable->setRowCount (static_cast<int> (parameters.size()));
    int currentRow = 0;
    for (auto currentParameter = parameters.begin(); currentParameter != parameters.end(); ++currentParameter, ++currentRow)
    {
        QTableWidgetItem *twi = new QTableWidgetItem();
        QString cellText = QString::fromStdString (getProcessingFlagName (currentParameter->first).toStdString());
        twi->setText (cellText);
        twi->setFlags (Qt::ItemIsEnabled);
        ui.parameterTable->setItem (currentRow, 0, twi);

        QString twTooltip = cellText + "\n";
        QWidget *tw;
        switch (currentParameter->second.variantType())
        {
            case VariantType::Bool:
                {
                    QCheckBox *tmpWidget = new QCheckBox();
                    tmpWidget->setChecked (currentParameter->second.getBool());
                    QObject::connect (tmpWidget, SIGNAL (clicked (void)), this, SLOT (parameterChanged (void)));
                    tw = (QWidget *) tmpWidget;
                    twTooltip += "true/false";

                    break;
                }
            case VariantType::Int:
                {
                    QSpinBox *tmpWidget = new QSpinBox();
                    tmpWidget->setMinimum (currentParameter->second.getIntMin ());
                    tmpWidget->setMaximum (currentParameter->second.getIntMax());
                    twTooltip += QString ("Min : ") + QString::number (tmpWidget->minimum());
                    twTooltip += QString (" Max : ") + QString::number (tmpWidget->maximum());
                    tmpWidget->setMinimumHeight (tmpWidget->sizeHint().height() * 2);

                    tmpWidget->setValue (currentParameter->second.getInt());
                    QObject::connect (tmpWidget, SIGNAL (valueChanged (int)), this, SLOT (parameterChanged (void)));
                    QObject::connect (tmpWidget, SIGNAL (editingFinished (void)), this, SLOT (parameterChanged (void)));
                    tw = (QWidget *) tmpWidget;

                    break;
                }
            case VariantType::Float:
                {
                    QDoubleSpinBox *tmpWidget = new QDoubleSpinBox();
                    tmpWidget->setMinimum (currentParameter->second.getFloatMin());
                    tmpWidget->setMaximum (currentParameter->second.getFloatMax());
                    twTooltip += QString ("Min : ") + QString::number (tmpWidget->minimum());
                    twTooltip += QString (" Max : ") + QString::number (tmpWidget->maximum());
                    tmpWidget->setMinimumHeight (tmpWidget->sizeHint().height() * 2);

                    tmpWidget->setSingleStep (0.001);
                    tmpWidget->setDecimals (3);
                    tmpWidget->setValue (currentParameter->second.getFloat());
                    QObject::connect (tmpWidget, SIGNAL (valueChanged (double)), this, SLOT (parameterChanged (void)));
                    QObject::connect (tmpWidget, SIGNAL (editingFinished (void)), this, SLOT (parameterChanged (void)));
                    tw = (QWidget *) tmpWidget;

                    break;
                }
        }

        m_flagMapping[tw] = *currentParameter;
        tw->setToolTip (twTooltip);
        twi->setToolTip (twTooltip);
        ui.parameterTable->setCellWidget (currentRow, 1, tw);
    }

    ui.parameterTable->verticalHeader()->setSectionResizeMode (QHeaderView::ResizeToContents);
    ui.parameterTable->horizontalHeader()->setSectionResizeMode (0, QHeaderView::ResizeToContents);
}

void ParameterView::parameterChanged()
{
    QWidget *senderWidget = (QWidget *) QObject::sender();

    if (m_flagMapping.find (senderWidget) == m_flagMapping.end())
    {
        return;
    }

    senderWidget->blockSignals (true);

    Pair<ProcessingFlag, Variant> &changedParameter = m_flagMapping[senderWidget];

    bool valueChanged = false;

    try
    {
        switch (changedParameter.second.variantType())
        {
            case VariantType::Bool:
                {
                    QCheckBox *tmpWidget = (QCheckBox *) senderWidget;
                    if (changedParameter.second.getBool() != tmpWidget->isChecked())
                    {
                        changedParameter.second.setBool (tmpWidget->isChecked());
                        valueChanged = true;
                    }

                    break;
                }
            case VariantType::Int:
                {
                    QSpinBox *tmpWidget = (QSpinBox *) senderWidget;
                    if (changedParameter.second.getInt() != tmpWidget->value())
                    {
                        changedParameter.second.setInt (tmpWidget->value());
                        valueChanged = true;
                    }

                    break;
                }
            case VariantType::Float:
                {
                    QDoubleSpinBox *tmpWidget = (QDoubleSpinBox *) senderWidget;
                    if (changedParameter.second.getFloat() != static_cast<float> (tmpWidget->value()))
                    {
                        changedParameter.second.setFloat (static_cast<float> (tmpWidget->value()));
                        valueChanged = true;
                    }

                    break;
                }
        }
    }
    catch (const common::OutOfBounds &)
    {
        // The parameter value is outside the bounds set by the processing.
        // The changedParameter will be unchanged, so force the UI back to
        // the parameter's current value.
        senderWidget->blockSignals (false);
        setValue (changedParameter);
        return;
    }

    senderWidget->blockSignals (false);

    if (valueChanged)
    {
        emit changePipelineParameter (changedParameter);
    }
}

void ParameterView::setValue (royale::Pair<royale::ProcessingFlag, royale::Variant> parameter)
{
    for (auto curMapping : m_flagMapping)
    {
        if (curMapping.second.first == parameter.first)
        {
            QWidget *curWidget = curMapping.first;
            curWidget->blockSignals (true);

            switch (parameter.second.variantType())
            {
                case VariantType::Bool:
                    {
                        QCheckBox *tmpWidget = static_cast<QCheckBox *> (curWidget);
                        tmpWidget->setChecked (parameter.second.getBool());
                        break;
                    }
                case VariantType::Int:
                    {
                        QSpinBox *tmpWidget = static_cast<QSpinBox *> (curWidget);
                        tmpWidget->setValue (parameter.second.getInt());
                        break;
                    }
                case VariantType::Float:
                    {
                        QDoubleSpinBox *tmpWidget = static_cast<QDoubleSpinBox *> (curWidget);
                        tmpWidget->setValue (parameter.second.getFloat());
                        break;
                    }
            }

            curWidget->blockSignals (false);
        }
    }
}
