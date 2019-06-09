/****************************************************************************\
* Copyright (C) 2015 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/

#pragma once

#include <royale/Pair.hpp>
#include <royale/String.hpp>
#include <royale/Vector.hpp>
#include <usecase/ExposureGroup.hpp>
#include <usecase/RawFrameSet.hpp>
#include <usecase/Stream.hpp>
#include <usecase/UseCaseIdentifier.hpp>

#include <memory>

namespace royale
{
    namespace usecase
    {
        //! If a use case is verified and fails verification, an error flag can be set
        enum class VerificationStatus
        {
            SUCCESS = 0,
            FRAMERATE,
            PHASE,
            DUTYCYCLE,
            MODULATION_FREQUENCY,
            EXPOSURE_TIME,
            REGION,
            NO_CALIBRATION,
            INVALID_FRAME_COMPOSITION,
            STREAM_COUNT,
            UNDEFINED = 0x7fffff01
        };

        enum class ExposureGray
        {
            Off = 0,
            On = 1
        };

        enum class IntensityPhaseOrder
        {
            IntensityLastPhase = 0,
            IntensityFirstPhase = 1
        };

        /**
         * The information about the sequence structure, frame rates, modulation and exposure for a
         * UseCase.
         *
         * This class has subclasses which aid the construction of the UseCaseDefinition, but every
         * instance must cope with being sliced (copied using the base class's copy assignment
         * function) without losing the definition.
         */
        class UseCaseDefinition
        {
        public:
            /**
             * Default ctor. The only users of this are the ones that will overwrite the contents by
             * assigning a complete UseCaseDefinition.
             */
            ROYALE_API explicit UseCaseDefinition ();

            /**
             * Constructor to be invoked by each derived class to ensure that the mandatory rate
             * limit is defined.  This needs to correspond to the mandatory exposure limits in the
             * streams' exposure groups.
             *
             * This sets the minimum allowed rate to 1.  If the maximum rate is less than the
             * minimum then verifyClassInvariants() will throw.
             *
             * The target frame rate is set to the maxRate.
             *
             * @param maxRate maximum allowed rate for this particular use case
             */
            ROYALE_API explicit UseCaseDefinition (uint16_t maxRate);

            ROYALE_API bool operator== (const UseCaseDefinition &rhs) const;
            ROYALE_API bool operator!= (const UseCaseDefinition &rhs) const;

            /**
             * Consistency checks for data being as expected.  For example, at least one stream, at
             * least one frame group per stream, all frame groups in each stream have the same
             * structure.
             *
             * The constructors in this class create a UCD with no streams, which would fail this
             * test.  This function is only expected to be called after the subclass' constructor
             * has added the streams and frames.
             *
             * @throws LogicError if a test fails
             */
            ROYALE_API void verifyClassInvariants() const;

            /**
             * Set the target rate of the clock that RawFrameSet::Alignment::CLOCK_ALIGNED frames
             * should aim for in [Hz].
             */
            ROYALE_API void setTargetRate (uint16_t rate);

            //! Set the target exposure time for all modulated raw frame sets in a given stream [micro seconds]
            ROYALE_API void setExposureTime (uint32_t exposureTime, StreamId s);

            //! Set a dutycycle in the range of (0, 100) for a specific raw frame set [percent] (A nullptr for rawFrameSet means for all raw frame sets)
            ROYALE_API void setDutyCycle (double dutyCycle, const RawFrameSet *rawFrameSet = nullptr);

            /**
             * Set the target exposure time for all exposure groups [micro seconds]
             *
             * Expects exposure times for each exposure group in the usecase,
             * in the order given by getExposureGroups().
             *
             * @param exposureTimes vector of new exposure times in microseconds
             */
            ROYALE_API void setExposureTimes (const royale::Vector<uint32_t> &exposureTimes);

            /**
             * Get the target exposure time for all exposure groups [micro seconds]
             *
             * Exposure times are for each each exposure group in the usecase,
             * in the order given by getExposureGroups().
             *
             * @return vector of exposure times in microseconds
             */
            ROYALE_API royale::Vector<uint32_t> getExposureTimes() const;

            /**
             * The equivalent of getExposureTimes()[set.getExposureGroupIndex()].
             *
             * @param set a raw frame set which is part of this UseCaseDefinition
             * @return the exposure in microseconds
             */
            ROYALE_API uint32_t getExposureTimeForRawFrameSet (const RawFrameSet &set) const;

            /**
             * Get the exposure limits for the modulated RFSs in a stream, the result is returned as
             * a pair [minExposureTime maxExposureTime of times in [microseconds].
             *
             * This function returns the intersection of the limits for each modulated RawFrameSet,
             * which is the range which is accepted by setExposureTime(uint32_t, StreamId).  Any
             * gray RFSs are ignored.
             */
            ROYALE_API const royale::Pair<uint32_t, uint32_t> getExposureLimits (StreamId s) const;

            /**
             * Get the exposure limits for all exposure groups [micro seconds]
             *
             * Limits returned are for each each exposure group in the usecase,
             * in the order given by getExposureGroups().
             *
             * @return vector of exposure limits in microseconds as a pair [minExposureTime maxExposureTime]
             */
            ROYALE_API const royale::Vector<royale::Pair<uint32_t, uint32_t>> getExposureLimits() const;

            /**
             * Get the target rate [in Hz] of the clock for RawFrameSet::Alignment::CLOCK_ALIGNED
             * frames.  For non-mixed mode use cases there will normally be one CLOCK_ALIGNED frame,
             * but mixed mode use cases are likely to have several.
             */
            ROYALE_API uint16_t getTargetRate() const;

            //! Get the maximal rate for this particular use case [hz]
            ROYALE_API uint16_t getMaxRate () const;

            //! Get the minimal rate for this particular use case [hz]
            ROYALE_API uint16_t getMinRate() const;

            /**
             * Get all raw frame sets which are available in this use case definition
             *
             * This is regardless to which stream the RFS belongs (raw frame sets may be shared between streams,
             * or not be associated with any stream in the usecase).
             * The vector can be indexed by the indices returned by getRawFrameSetIndices().
             *
             * @return const reference to vector of raw frame sets
             */
            ROYALE_API const royale::Vector<RawFrameSet> &getRawFrameSets() const;

            //! Get all stream IDs available for this use case definition
            ROYALE_API royale::Vector<StreamId> getStreamIds() const;

            /**
             * Get raw frame set indices for a given frame group of the given stream ID
             *
             * The indices returned can be used as index into the vector returned by getRawFrameSets().
             * Order is that of the frame group.
             *
             * Many callers of this function always pass zero as the groupIdx regardless of which
             * group is being processed, as the information needed by that caller is consistent for
             * all groups in the stream. For example, getExposureIndicesForStream() is a convenience
             * wrapper that doesn't need the groupIdx, because all framegroups in a stream must have
             * the same exposure groups.
             *
             * @return vector of indices
             */
            ROYALE_API royale::Vector<std::size_t> getRawFrameSetIndices (StreamId s, std::size_t groupIdx) const;

            /**
             * Get the exposure group indices for any FrameGroup in the Stream.
             *
             * This returns a vector where each element is an index in to the vector returned by
             * getExposureGroups(); it can also be used as an index in to the vector returned by
             * CapturedUseCase::getExposureGroups().
             * The vector corresponds to one frame group in the stream (as returned by getRawFrameSetIndices()),
             * i.e. there is one element for each raw frame set in the group. There may be duplicate
             * exposure group indices if exposure groups are reused within the frame group.
             *
             * All FrameGroups in a given stream would return the same vector, so there's no need to
             * supply a groupIdx.
             */
            ROYALE_API royale::Vector<std::size_t> getExposureIndicesForStream (StreamId s) const;

            /**
             * Convert the index of a raw frame set to the indices of its raw frames.
             *
             * For example, with a 1+4 Use Case, getSequenceIndicesForRawFrameSet(0) returns {0} and
             * getSequenceIndicesForRawFrameSet(1) returns {1, 2, 3, 4}.
             *
             * @param set this retrieves information corresponding to getRawFrameSets[set]
             * @return where these frames would be in a depth-first traversal of the RawFrameSets
             */
            ROYALE_API royale::Vector<uint16_t> getSequenceIndicesForRawFrameSet (std::size_t set) const;

            //! Total number of RawFrames in all RawFrameSets of the usecase
            ROYALE_API std::size_t getRawFrameCount() const;

            //! Number of frame groups in the given stream
            ROYALE_API std::size_t getFrameGroupCount (StreamId s) const;

            /**
             * Get the width and height of the region of interest.  These are the dimensions
             * of the data sent from the Access to the Processing component. The image is centered
             * around the lens center which is defined in the CameraModule
             *
             * @param columns   size x of the image
             * @param rows      size y of the image
             */
            ROYALE_API void getImage (uint16_t &columns, uint16_t &rows) const;

            /**
             *  Set the image size which should be used. The image size is centered around
             *  the specified lens center of the CameraModule
             *
             *  @param columns   size x of the image
             *  @param rows     size y of the image
             */
            ROYALE_API void setImage (uint16_t columns, uint16_t rows);

            /**
             * Get a human readable name of the type of use case, for use in debugging and log
             * files.  This name is expected to only contain printable ASCII characters.
             *
             * This can be a string such as "UseCaseEightPhase", and might not be a unique
             * identifier; a single device may have multiple use cases with identical values for
             * getTypeName().
             *
             * Names such as "MODE_5_45FPS" are not part of UseCaseDefinition at all - those are in
             * the UseCase class, and returned by UseCase::getName().
             */
            ROYALE_API const royale::String &getTypeName() const;

            /**
             * The identifier may be a GUID/UUID, and is not required to be printable as ASCII.
             *
             * A module config must either use identifiers for all of its UseCaseDefinitions, or
             * use identifiers for none of them. This includes any additional use cases loaded from
             * an external config, for example from Zwetschge; the external config format may mean
             * that it can only be used with identifiers.
             *
             * A device that uses identifiers must not have two use cases with identical
             * identifiers, and must not have any use cases with a sentinel identifier.
             *
             * For a device that does not use identifiers, all UCD's getIdentifier() methods must
             * return UseCaseIdentifier's sentinel value (the default-initialized value).
             */
            ROYALE_API UseCaseIdentifier getIdentifier() const;

            /**
             * Fetch the stream with the given streamId.
             * The stream must already exist.
             *
             * @param id the stream id.
             * @return the stream.
             */
            ROYALE_API const std::shared_ptr<Stream> getStream (StreamId id) const;

            //! Get all exposure groups which are available in this use case definition
            ROYALE_API const royale::Vector<ExposureGroup> &getExposureGroups() const;

            //! Returns true if the spread spectrum feature is turned on for this use case
            ROYALE_API bool getSSCEnabled() const;

        protected:
            /**
             *  Create a new stream and add it to m_streams.
             *  Mixed-mode use cases generally have more than one stream, while non-mixed only have
             *  one stream with a single frame group.
             *
             *  Zero is not a valid ID, but if this is called with id == 0 a stream id will be
             *  automatically allocated.
             *
             *  @param id the stream id.
             *  @return the stream.
             */
            std::shared_ptr<Stream> createStream (StreamId id = 0);

            /**
             *  Create a new exposure group and add it to m_exposureGroups.
             *
             *  @param name the name of the exposure group
             *  @param limits minimum and maximum exposure
             *  @param exposure default exposure
             *  @return exposure group index.
             */
            ExposureGroupIdx createExposureGroup (const royale::String &name, royale::Pair<uint32_t, uint32_t> limits, uint32_t exposure);

            /**
             * Constructs and returns a RawFrameSet with the typical arguments for grayscale sets
             * (MODFREQ_AUTO, GRAYSCALE, expoOn ? DC_AUTO : DC_0, exposureGroup).
             *
             * Most UseCaseDefinition subclasses have a gray RFS with these common parameters.
             */
            static RawFrameSet createGrayRFS (ExposureGroupIdx exposureGroup, ExposureGray expoOn, uint32_t modulationFrequency = RawFrameSet::MODFREQ_AUTO ());

            /**
             * Creates one stream containing all of the raw frame sets passed in the argument.  This
             * is a convenience function for non-mixed mode use cases, where all RawFrameSets are in
             * a single stream.
             *
             * All the RawFrameSets must already include references to their ExposureGroups.  This
             * function will handle the alignment settings (it will set the first to
             * RawFrameSet::Alignment::CLOCK_ALIGNED and the rest to START_ALIGNED).
             *
             * This will throw a LogicError if m_streams is not empty when called.
             *
             * This will throw a LogicError if m_rawFrameSet is not empty when called.
             */
            void constructNonMixedUseCase (royale::Vector<RawFrameSet> rawFrameSets);

            /**
             * Convenience function for constructing mixed-mode use cases.  Given a set (groupSets)
             * of RawFrameSets, adds them to m_rawFrameSet and to the given stream.  Their exposure
             * groups will be automatically added to the stream's exposure groups.
             *
             * The groupSets parameter is taken by value to make a copy of the original sets, as the
             * caller is expected to construct one instance of the groupSets as a template and then
             * use it to call this function for each repetition of the group.
             *
             * Alignment CLOCK_ALIGNED sets the first RFS to CLOCK_ALIGNED, and the rest to START.
             * START sets all the RFS' alignments to START.
             * STOP sets all the RFS' alignments to STOP.
             *
             * If the optional appendPrevious parameter is true, the frames are added to the last
             * existing frame group instead of creating a new frame group.  This is intended for
             * constructing interleaved-mode groups - given a frame group that's split in to ES1 and
             * ES2, the first call to constructFrameGroup would have the ES1 frame sets in groupSets
             * and appendPrevious false.  The second call would have the ES2 frame sets and
             * appendPrevious true.
             *
             * @param stream which stream to add these frames to
             * @param groupSets the frames for a FrameGroup (or some of them, if using the optional parameter)
             * @param alignment placement of the entire groupSets w.r.t the clock
             * @param appendPrevious optional parameter to assist with constructing interleaved use cases
             * @return the group that the frames were added to
             */
            void constructFrameGroup (std::shared_ptr<Stream> stream, royale::Vector<RawFrameSet> groupSets, RawFrameSet::Alignment alignment, bool appendPrevious = false);

        protected:
            royale::String                                 m_typeName;
            UseCaseIdentifier                              m_identifier;
            uint16_t                                       m_targetRate;
            royale::Vector<RawFrameSet>                    m_rawFrameSet;
            uint16_t                                       m_imageColumns;     //!< The number of columns the target image should have
            uint16_t                                       m_imageRows;        //!< The number of rows the target image should have
            bool                                           m_enableSSC;        //!< turn spread spectrum feature (spread spectrum clock) on or off for modulated raw frames
            royale::Vector<std::shared_ptr<Stream>>        m_streams;
            royale::Vector<ExposureGroup>                  m_exposureGroups;

        private:
            /**
             * This is the maximal allowed rate for this use case.  It's set by the constructor, and
             * only changed by the copy assignment operator completely overwriting the current UCD.
             */
            uint16_t                                       m_maxRate;
            /**
             * This is the minimal allowed rate for this use case. Similar to m_maxRate it's
             * non-const to allow the default implementation of the copy assignment operator,
             * but will always be the single value set in UCD's constructors.
             */
            uint16_t                                       m_minRate;
        };
    }
}
