/****************************************************************************\
 * Copyright (C) 2017 pmdtechnologies ag & Infineon Technologies
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#pragma once

#include <pal/IStorageReadRandom.hpp>
#include <pal/IStorageWriteFullOverwrite.hpp>

#include <QtWidgets>
#include <royale.hpp>

#include <memory>

namespace spiFlashTool
{
    namespace storage
    {
        /**
         * Callback to report the status during a long-running operation.
         *
         * \todo ROYAL-1316 design the RoyaleAPI for this
         */
        class IProgressReportListener
        {
        public:
            virtual ~IProgressReportListener() = default;

            /**
             * To send the UI a message that says "Reading page 1 of 200", then the arguments
             * would be "Reading page", 1, 200.
             */
            virtual void reportProgress (const std::string &operation, std::size_t x, std::size_t y) = 0;
        };

        /**
        * Used if SPIStorage's constructor is called with progressListener == nullptr.
        */
        class DummyProgressListener : public IProgressReportListener
        {
        public:
            void reportProgress (const std::string &operation, std::size_t x, std::size_t y) override
            {
                (void) operation;
                (void) x;
                (void) y;
            }
        };

        class SPIStorageBase : public royale::pal::IStorageReadRandom,
            public royale::pal::IStorageWriteFullOverwrite
        {
        public:
            SPIStorageBase (IProgressReportListener *progressListener = nullptr) :
                m_progressListener{ progressListener }
            {
                if (m_progressListener == nullptr)
                {
                    m_progressListener = &m_dummyProgressListener;
                }
            };

            void readStorage (std::size_t startAddr, std::vector<uint8_t> &recvBuffer) override = 0;
            void writeStorage (const std::vector<uint8_t> &buffer) override = 0;

            virtual royale::Vector<royale::Pair<royale::String, uint64_t>> getEFuseRegisters() = 0;
        protected:
            /**
            * This is never empty, if nullptr was passed to the constructor then it contains a
            * dummy object.
            */
            IProgressReportListener *m_progressListener;

            DummyProgressListener m_dummyProgressListener;
        };
    }
}
