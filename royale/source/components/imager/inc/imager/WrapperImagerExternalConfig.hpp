/****************************************************************************\
 * Copyright (C) 2018 Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#pragma once

#include <config/IImagerExternalConfig.hpp>

namespace royale
{
    namespace imager
    {
        /**
        * A utility class implementing IImagerExternalConfig's interface.
        *
        * The data members are public, so that the code constructing the IImagerExternalConfig can
        * access them, and then pass the object to users that can only access them via the
        * IImagerExternalConfig interface.
        */
        class WrapperImagerExternalConfig : public IImagerExternalConfig
        {
        public:
            IMAGER_EXPORT WrapperImagerExternalConfig () = default;

            IMAGER_EXPORT WrapperImagerExternalConfig (TimedRegisterList initializationMap,
                    TimedRegisterList fwPage1,
                    TimedRegisterList fwPage2,
                    TimedRegisterList fwStartMap,
                    TimedRegisterList startMap,
                    TimedRegisterList stopMap,
                    std::vector<UseCaseData> useCases) :
                m_initializationMap (std::move (initializationMap)),
                m_fwHeader1 (
            {
                0, 0, 0
            }),
            m_fwHeader2 ({0, 0, 0}),
            m_fwPage1 (std::move (fwPage1)),
            m_fwPage2 (std::move (fwPage2)),
            m_fwStartMap (std::move (fwStartMap)),
            m_startMap (std::move (startMap)),
            m_stopMap (std::move (stopMap)),
            m_useCases (std::move (useCases))
            {
            }

            IMAGER_EXPORT WrapperImagerExternalConfig (TimedRegisterList initializationMap,
                    SequentialRegisterHeader fwHeader1,
                    SequentialRegisterHeader fwHeader2,
                    TimedRegisterList fwStartMap,
                    TimedRegisterList startMap,
                    TimedRegisterList stopMap,
                    std::vector<UseCaseData> useCases) :
                m_initializationMap (std::move (initializationMap)),
                m_fwHeader1 (std::move (fwHeader1)),
                m_fwHeader2 (std::move (fwHeader2)),
                m_fwPage1 (),
                m_fwPage2 (),
                m_fwStartMap (std::move (fwStartMap)),
                m_startMap (std::move (startMap)),
                m_stopMap (std::move (stopMap)),
                m_useCases (std::move (useCases))
            {
            }

            IMAGER_EXPORT ~WrapperImagerExternalConfig () override = default;

            const TimedRegisterList &getInitializationMap() const override
            {
                return m_initializationMap;
            }

            SequentialRegisterHeader getFirmwareHeader1() const override
            {
                return m_fwHeader1;
            }

            SequentialRegisterHeader getFirmwareHeader2() const override
            {
                return m_fwHeader1;
            }

            const TimedRegisterList &getFirmwarePage1() const override
            {
                return m_fwPage1;
            }

            const TimedRegisterList &getFirmwarePage2() const override
            {
                return m_fwPage2;
            }

            const TimedRegisterList &getFirmwareStartMap() const override
            {
                return m_fwStartMap;
            }

            const TimedRegisterList &getStartMap() const override
            {
                return m_startMap;
            }

            const TimedRegisterList &getStopMap() const override
            {
                return m_stopMap;
            }

            const std::vector<UseCaseData> &getUseCaseList() const override
            {
                return m_useCases;
            }

            // members are public because this is only exposed via the IImagerExternalConfig interface
        public:
            TimedRegisterList m_initializationMap;
            SequentialRegisterHeader m_fwHeader1;
            SequentialRegisterHeader m_fwHeader2;
            TimedRegisterList m_fwPage1;
            TimedRegisterList m_fwPage2;
            TimedRegisterList m_fwStartMap;
            TimedRegisterList m_startMap;
            TimedRegisterList m_stopMap;
            std::vector<UseCaseData> m_useCases;
        };
    }
}
