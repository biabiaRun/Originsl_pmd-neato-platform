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

#include <imager/ImagerBase.hpp>
#include <imager/MeasurementBlock.hpp>

namespace royale
{
    namespace imager
    {
        /**
        * This base class covers all the logic required for dealing with measurement blocks.
        * Based on a given use case definition it can decide whether a given use case is feasible
        * or not (see verifyRawFrameAssignment). If the use case fits to the current layout of the
        * imager (see concrete implementation of ImagerMeasurementBlockBase::createMeasurementBlockList)
        * the method generateRawFrameAssignment can be used to prepare the measurement
        * block list (m_mbList) and the assignment of raw frames to measurement blocks (m_rfAssignment)
        * so the specialized imager only needs to fill the objects in the
        * measurement block list (m_mbList) in its implementation of Imager::prepareUseCase.
        */
        class ImagerMeasurementBlockBase : public ImagerBase
        {
        public:
            ImagerMeasurementBlockBase (const std::shared_ptr<royale::hal::IBridgeImager> &bridge,
                                        size_t defaultMeasurementBlockCount,
                                        size_t defaultMeasurementBlockCapacity);
            virtual ~ImagerMeasurementBlockBase() = default;

            std::vector<std::size_t> getMeasurementBlockSizes() const override;

        protected:
            /**
            * For each raw frame of the given useCase the timespan of the start of the raw frame
            * to the start of the next raw frame is calculated. This may include the raw frame rates
            * which are used for throttling as well as the eye safety delay.
            * \todo ROYAL-1724 the eye safety delay is not yet supported and for now this method will never be called
            *       having such a delay set as this is prevented by the preceeding verification of the use case.
            *
            * \param   useCase   The useCase contains the list of ImagerRawFrame objects as well as required
            *                    meta information for calculation of the raw frame timings.
            * \return  list of raw frame times, in the same order as the ImagerRawFrame objects are
            *          returned when calling UseCaseDefinition::getRawFrames.
            */
            const std::vector < double > generateRawFrameTimings (const ImagerUseCaseDefinition &useCase) const;

            /**
            * Calculates the raw frame time.
            *
            * This is the time from when the sequencer starts to configure the imager for the specified sequence entry
            * until the time that the sequencer is finished with processing the complete raw frame so that
            * the imager is ready to proceed with the next raw frame. This time includes startup, line readout,
            * but not the frame rate counter.
            *
            * See Mira devspec and framerate calculation sheet for more detailed information on that topic.
            * Names of variables and constants used by this method should be aligned to the framerate excel sheet.
            *
            * WARNING: This method is implemented only for a certain base register configuration
            * and system clock. It is not allowed to use it with deviant settings.
            *
            * \param   useCase              The useCase contains required information
            *                               such as ROI, binning and special pixels.
            * \param   expoTime             The exposure time of the raw frame.
            * \param   modFreq              The modulation frequency of the raw frame.
            * \param   isFirstRawFrame      If true, assume frame is first in the sequence when computing the timings.
            * \param   [out] rawFrameTime   The time in seconds the complete raw frame takes. If a raw frame rate
            *                               is specified the raw frame time is increased to fit the raw frame rate.
            *                               If the specified raw frame rate is not feasibile the raw frame time value
            *                               will be greater than the maximum possible raw frame time. This will be
            *                               indicated by the return value of this method.
            * \return  True if the configuration is feasible and the desired raw frame rate can be met, false otherwise.
            */
            virtual bool calcRawFrameRateTime (
                const ImagerUseCaseDefinition &useCase,
                uint32_t expoTime,
                uint32_t modFreq,
                bool isFirstRawFrame,
                double &rawFrameTime) const = 0;

            /**
            * Calculate and assign the measurement block (MB) based framerate counter value.
            *
            * If the imager supports a measurement block based framerate counter, the implementation of this method
            * should calculate the counter value that matches the given mbTargetTime and set the frameRateCounter member
            * of the measurement block from m_mbList that matches the identifier mbId.
            * The default implementation assumes that no measurement block based framerate counter is supported and
            * will always assign zero to the frameRateCounter member.
            *
            * \param   mbId              Identifies the measurement block within the list (m_mbList) of measurement blocks.
            * \param   mbTargetTime      The time in units of seconds the framerate counter should run if the counter starts
            *                            at the start of a measurement. If the imager's framerate counter starts at the end
            *                            of a measurement subtracting the parameter mbMeasurementTime will give the desired
            *                            delay time for the counter. Will be ignored by the default implementation of this method.
            * \param   mbMeasurementTime The time in units of seconds the measurement of the specified MB will take to run.
            *                            This time includes possible delays in between the single raw frames, but
            *                            not the delay that is added to the very last raw frame of the MB.
            *                            This value can be used to calculate the idle time for imagers that do not have
            *                            the counter starting at the start of a measurement but at the end of the measurement.
            */
            virtual void prepareMeasurementBlockTargetTime (MeasurementBlockId mbId,
                    const double mbTargetTime,
                    const double mbMeasurementTime);

            /**
            * Depending on the use case the imager can decide for a specific sequencer layout.
            * Imagers without a firmware will return always the same layout, but imagers using
            * a firmware that supports mixed mode can change this layout during runtime.
            * The default implementation of this method will return a list with m_defaultMeasurementBlockCount
            * elements, each element being a MeasurementBlock capable of containing
            * m_defaultMeasurementBlockCapacity sequence entries.
            *
            * \param   useCase   The useCase is not used by the default implementation but can be used
            *                    by specializied implementations to make a decision about which sequencer layout to use.
            * \return  A list of measurement blocks, size and measurement block capacity are initialized
            *          to match the sequencer layout that fits best to the given use case.
            */
            virtual std::vector<MeasurementBlock> createMeasurementBlockList (const ImagerUseCaseDefinition &useCase) const;

            /**
            * This method generates a map that assigns the raw frames of a given use case
            * to a certain measurement block. In case the concrete imager supports only a single measurement block
            * all raw frames will be assigned to this one. If multiple measurement blocks are available
            * raw frames will be assigned to a first measurement block MB(n) as long as the number of raw frames
            * is not exceeding the measurement block's raw frame capacity.
            * In this case the following raw frames will be assigned to a second measurement block MB(n+1).
            * Linked raw frames (marked by the isEndOfLinkedRawFrames flag) will never be spreaded over
            * multiple measurement blocks. If they doesn't fit completely into a measurement block MB(n) a next
            * measurement block MB(n+1) will be used for them.
            *
            * This method alters the content of the m_mbList member by setting the cycles member
            * of the MeasurementBlock objects of m_mbList as well as by resizing the sequence vector to match
            * the generated raw frame assignment.
            *
            * \param useCase       The useCase the raw frame assignment should be created for.
            */
            void generateRawFrameAssignment (const ImagerUseCaseDefinition &useCase);

            /**
            * Validates if it is possible to create an assignment for the raw frames of a given
            * use case to the measurement block / sequencer layout the imager uses for executing this use case.
            *
            * \param useCase       The useCase the raw frame assignment should be validated for.
            * \return              True if the use case is feasible, false otherwise.
            */
            bool verifyRawFrameAssignment (const ImagerUseCaseDefinition &useCase) const;

            /**
            * Find the contiguous measurement block sequence that is safe for reconfiguration
            * with the longest duration and return its duration. This method expects to have
            * ImagerMeasurementBlockBase::generateRawFrameAssignment called first to update
            * the m_mbList member.
            *
            * \param useCase       To get the raw frame information and to generate their timings.
            * \return              Time in milliseconds
            */
            uint32_t getMaxSafeReconfigTimeMilliseconds (const ImagerUseCaseDefinition &useCase) const;

            const size_t m_defaultMeasurementBlockCount;    //!< The imager's natively supported measurement block count
            const size_t m_defaultMeasurementBlockCapacity; //!< The imager's natively supported measurement block capacity

            /**
            * During execution of a new use case (\see {Imager::executeUseCase}) each
            * raw frame must be assigned to a certain MeasurementBlock. If the concrete imager
            * is not supporting multiple measurement blocks then it is assigning all raw frames
            * to one common MeasurementBlock. The implementation relies on an ordering so that
            * for an increasing position of the raw frame in its container class
            * the MeasurementBlockId must not decrease.
            * E.g. (1,1) - (2,2) would be valid, but (1,1) - (2,0) would be an invalid map entry.
            */
            std::map<size_t, MeasurementBlockId> m_rfAssignment;

            /**
            * Represents the logical organization of the configuration container.
            * Depending on the capabilities of the concrete imager this may be a
            * list of one or more measurement blocks, each capable of containing one
            * or more raw frames.
            */
            std::vector<MeasurementBlock> m_mbList;

        private:
            /**
            * This method takes a raw frame to measurement block mapping and decides for two specific
            * measurement block IDs if the assignment is equal.
            *
            * For repeating measurement blocks it is necessary to decide whether two measurement blocks are
            * equal. As the use case itself does not provide this information (as it does not know about
            * measurement blocks), the imager must evaluate equality of measurement blocks by itself.
            *
            * \param useCase       To get the raw frame information and to generate their timings.
            * \param rfAssignment  A not necessarily complete assignment of raw frames to measurement blocks.
            * \param first         Index of a first measurement block.
            * \param second        Index of a second measurement block.
            * \return              True if the second measurement block can be replaced by repeating the first one.
            */
            bool isRawFrameAssignmentToMeasurementBlockEqual (const ImagerUseCaseDefinition &useCase,
                    const std::map<size_t, MeasurementBlockId> &rfAssignment,
                    const MeasurementBlockId first,
                    const MeasurementBlockId second) const;

            /**
            * Removes the entries of the map that have a specific measurement block identifier as their value.
            *
            * \param rfAssignment  The raw frame to measurement block assignment that should be modified.
            * \param currentMb     Index of the measurement block for which the assignment should be removed.
            */
            static void removeMappingForMeasurementBlock (std::map<size_t, MeasurementBlockId> &rfAssignment,
                    MeasurementBlockId currentMb);

            /**
            * This is the repeat count limit for all imagers having multiple measurement blocks.
            * Any imager using measurement blocks (createMeasurementBlockList() returns a list of size > 1)
            * will use this value. A repeat count of 1 would mean the measurement block will only be
            * executed once and it cannot be repeated.
            */
            static const size_t MB_REPEAT_LIMIT;
        };
    }
}
