/****************************************************************************\
* Copyright (C) 2017 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <config/ExternalConfig.hpp>
#include <hal/INonVolatileStorage.hpp>
#include <imager/WrapperImagerExternalConfig.hpp>
#include <pal/IStorageReadRandom.hpp>

#include <memory>

namespace royale
{
    namespace storage
    {
        /**
         * A format for the non-volatile storage that holds the both the calibration data and the
         * setup registers for a flash-defined imager.
         */
        class StorageFormatZwetschge
        {
        public:
            ROYALE_API explicit StorageFormatZwetschge (std::shared_ptr<royale::pal::IStorageReadRandom> storageAccess);
            ROYALE_API virtual ~StorageFormatZwetschge() = default;

            /**
             * Returns the IImagerExternalConfig, UseCase list and INonVolatileStorage.
             *
             * The INonVolatileStorage returned will have access to the IStorageReadRandom passed to
             * StorageFormatZwetschge's constructor, and its getCalibrationData() method will
             * require I/O.  If the data should be cached then the caller of this function must
             * handle that.
             *
             *  \param convertSeqRegMaps convert all sequential register maps into timed register lists
             *  \param keepSeqAddressInfo if convertSeqRegMaps is true this toggles if the addresses from the sequential
             *                            register map header are kept in place
             */
            ROYALE_API royale::config::ExternalConfig getExternalConfig (bool convertSeqRegMaps = false,
                    bool keepSeqAddressInfo = false);


            ROYALE_API uint32_t getZwetschgeCrc();

            /**
             * Holder for one of this format's "24p + 24s" address and size pairs
             *
             * Public so that static helper functions in StorageFormatZwetschge.cpp can use it.
             */
            struct AddrAndSize;

        private:
            /** A parsed copy of the main index, containing AddrAndSize info for the other data */
            struct TableOfContents;

            /**
             * Given the details of where to find data in the storage, read it from the hardware.
             * This is a wrapper for IStorageReadRandom:readStorage, creating a buffer of the
             * appropriate size.
             *
             * Will throw if it encounters an I/O error, but does not know about CRCs or otherwise
             * do any validation except for a sanity check on the block size.
             */
            std::vector<uint8_t> readBlock (const AddrAndSize &target);

            /**
             * Read the TableOfContents from its specificiation-defined location in the flash.
             */
            struct TableOfContents getToC();

            /**
             * Subset of ExternalConfig returned from getUseCaseList.
             */
            struct UseCasePartOfConfig
            {
                std::vector<royale::imager::IImagerExternalConfig::UseCaseData> imagerUseCaseList;
                std::vector<royale::usecase::UseCase> royaleUseCaseList;
            };

            /**
             * Returns the structures expected from IImagerExternalConfig::getUseCaseList(), and
             * also the structures required for the processing and core parts of Royale.
             *
             *  \param toc the table of contents that should be used
             *  \param convertSeqRegMaps convert all sequential register maps into timed register lists
             *  \param keepSeqAddressInfo if convertSeqRegMaps is true this toggles if the addresses from the sequential
             *                            register map header are kept in place
             */
            ROYALE_API UseCasePartOfConfig getUseCaseList (const TableOfContents &toc, bool convertSeqRegMaps,
                    bool keepSeqAddressInfo = false);

            /**
             * Read the TableOfRegisterMaps for the given location, returns an IImagerExternalConfig
             * which has no use cases.
             */
            std::shared_ptr<royale::imager::WrapperImagerExternalConfig> getTableOfRegisterMaps (const AddrAndSize &target);

            /**
             * Uses the information from the ToC to create an INonVolatileStorage instance. Reading
             * the calibration data itself is deferred until getCalibrationData() is called, but all
             * data that's available from the ToC, such as the module id, doesn't need further I/O.
             *
             * The handling of missing or corrupt calibration matches the documentation for
             * ExternalConfig::calibration and StorageFormatZwetschge::getExternalConfig.
             */
            std::shared_ptr<royale::hal::INonVolatileStorage> getCalibrationProxy (const TableOfContents &toc);

            std::shared_ptr<pal::IStorageReadRandom> m_bridge;
        };
    }
}
