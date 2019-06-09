/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <cstdint>
#include <vector>

namespace royale
{
    namespace imager
    {
        using MeasurementBlockId = std::size_t;

        /**
        * This struct is used by the imagers as intermediate data storage
        * for the register configuration of the imager's configuration
        * container. The imager is not writing directly to registers because
        * this additional indirection allows to decouple the raw frame configuration
        * from the concrete layout of the configuration container.
        * Note: \see {ImagerMeasurementBlockBase::m_rfAssignment} and
        * \see {ImagerMeasurementBlockBase::m_mbList}
        */
        struct MeasurementBlock
        {
            explicit MeasurementBlock (std::size_t capacity) :
                sequence (capacity),
                maxSequenceLength (capacity),
                cycles (1u),
                frameRateCounter (0u),
                safeForReconfig (false)
            {};

            /**
            * A SequenceEntry is a generic struct comprising 4 members,
            * representing the register values for a single raw frame entry
            * of the imager's sequencer. Note that for the M2452 imager the
            * ps and pllset members are merged together into one register
            * value (\see {SequenceGeneratorM2452}).
            */
            struct SequenceEntry
            {
                uint16_t expo;       //!< A exposure time register value
                uint16_t fr;         //!< A framerate counter register value
                uint16_t ps;         //!< A phaseshifter configuration register value
                uint16_t pllset;     //!< A PLL LUT assignment register value

                /**
                * Marks a fr counter value to be equivalent to zero.
                * The framerate counter starts with the beginning of a new raw frame and
                * the subsequent raw frame will not start until the counter value reaches zero.
                * If the framerate counter value is zero the subsequent raw frame will start
                * immediately after the readout of the previous raw frame finished (the framerate
                * counter will not add any idle time after the raw frame). It is allowed
                * for a generation routine to create MeasurementBlock::SequenceEntry objects
                * with a frame rate counter value that reaches zero at the same time the readout
                * of the raw frame finishes - and therefore having the same effect as using a
                * raw frame counter value of zero. However, in this case the generation routine
                * must also provide this information to allow the register transfer layer that
                * translates the MeasurementBlock::SequenceEntry objects to concrete register
                * settings to further optimize its output.
                */
                bool     fr_valEqZero;
            };

            /**
            * A list of SequenceEntry objects that is allowed to have a size
            * of 1 to maxSequenceLength elements. The actual size of this list
            * corresponds to the number of active sequence entries of this measurement block.
            */
            std::vector<SequenceEntry> sequence;

            /**
            * The measurement block is a logical unit constructed once during
            * imager instantiation, thus having a certain capacity of SequenceEntry
            * objects available via the MeasurementBlock's "sequence" member.
            * However, during the imager's lifetime the actual number of active sequence
            * entries will change depending on the use case, but it must not exceed
            * maxSequenceLength elements.
            */
            const std::size_t maxSequenceLength;

            /**
            * During assignment of raw frames to measurement blocks
            * (\see {ImagerMeasurementBlockBase::generateRawFrameAssignment})
            * equal measurement blocks are detected and in case they are equal
            * just one measurement block entry will be used, but it will be
            * repeatedly executed. This member tells how many times the
            * measurement block should be executed. By default this member is
            * set to '1' which means that this measurement block is executed one time.
            */
            uint16_t cycles;

            /**
            * Each measurement block can have a framerate counter value assigned.
            * If this counter is zero the framerate is fully defined by the framerate
            * counter values of the sequence entries. If this values is non-zero it is
            * recommended to set the framerate counter value of the last sequence entry
            * to zero. Not all imagers support a measurement block based framerate counter,
            * in this case its value is ignored but should be set to zero anyway. In this
            * case it is mandatory to have the framerate counter value of the last sequence
            * entry set to a non-zero value to run at a well-defined frame rate.
            * Having a separate counter for a measurement block framerate is required for
            * supporting the low-power feature of the imager.
            */
            uint32_t frameRateCounter;

            /**
            * For each measurement block (MB) it can be specified if it is safe for it to react on a
            * reconfiguration trigger. Imagers (w/o safe reconfig support) could have this flag
            * set to false for all MBs. Imagers supporting multiple MBs can set this flag
            * for each MB to define the associated sequences of MBs for which a change of their
            * configuration must happen simultaneously. An associated sequence could be
            * a 9-phase measurement which is splitted up to 2 MBs.
            * For this only the last MB of such a sequence will have set this flag to true.
            * Or it could be an interleaved mode sequence which puts a 5-phase measurement
            * within a 9-phase measurement, resulting in 3 MBs used, but only the last one
            * having this flag set to true.
            */
            bool safeForReconfig;
        };
    }
}
