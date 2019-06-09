/****************************************************************************\
 * Copyright (C) 2017 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
\****************************************************************************/

#ifndef __ISPECTRE_HPP__
#define __ISPECTRE_HPP__

#include <type_traits>
#include <spectre/SpectreConfig.hpp>
#include "common/Field.hpp"
#include <spectre/Input.hpp>
#include <spectre/SpectreStatus.hpp>
#include <spectre/ResultType.hpp>
#include "common/CalibrationField.def"
#include "spectre/MemoryHolder.hpp"
#include "spectre/IConfiguration.hpp"

/** @defgroup spectre_group Spectre Public API
 *
 * @brief The public C++ API which should be used to access Spectre. Please refer to the ::spectre namespace which contains all relevant functions of the API.
 */

/**
 * @ingroup spectre_group
 *
 * @brief Contains the public API
 *
 * The public interface of libspectre provides functionalities to read calibration files, compute depth-data from
 * the sensor's raw data, and retrieve the results. It's main interface is available in the header spectre/ISpectre.hpp.
 * An instance of ISpectre suitable for a specific device can be obtained through the spectre::getSpectre method if
 * calibration data is available.
 *
 * A high-level overview on libspectre, its architecture, and relation to libcalibration / libprocessing can be found
 * in the Sect.
 */
namespace spectre
{
#define SPECTRE_ENUM_NAME(KEY,...) KEY,
    /// Calibration contents which can be requested through the public API
    enum class CalibrationField
    {
        SPECTRE_CALIBRATION_FIELDS (SPECTRE_ENUM_NAME)
        /*! Custom data (uint8_t) */ CUSTOM_FIELD,
        NUM_ENTRIES
    };
#undef SPECTRE_ENUM_NAME

    // Not considered as part of the public interface
    namespace details
    {
        template<CalibrationField F>
        struct field_types;

        template<>
        struct field_types<CalibrationField::CUSTOM_FIELD>
        {
            using value_type = uint8_t;
        };

#define SPECTRE_FIELD_TYPES(KEY,TYPE)                   \
        template<>                                      \
        struct field_types<CalibrationField::KEY>       \
        {                                               \
            using value_type = TYPE;                    \
        };
        SPECTRE_CALIBRATION_FIELDS (SPECTRE_FIELD_TYPES)
#undef SPECTRE_FIELD_TYPES
    }

    /// Hints for altering the behavior of the createSpectre factory method
    enum class SpectreFactoryHint
    {
        /**
         * Requests the default implementation of Spectre
         */
        AUTO,
        /**
         * Reserved for internal use
         */
        NO_HINT
    };

    /**
     * @brief ISpectre is Spectre's main entry point, encapsulating calibration handling, and depth-data processing.
     *
     * Each instance of ISpectre is permanently coupled to a specific calibration, and intended for depth-processing
     * of one raw data stream with specific settings. In contrast, all settings except the calibration can be changed
     * during the lifetime of the object.
     *
     * The calibration which has been used to create a ISpectre instance determines the expected width/height of the
     * input raw data. It will also return processed depth data with the same resolution. The used resolution can be
     * requested through the ISpectre::getWidth() and ISpectre::getHeight() methods.
     */
    class SPECTRE_API ISpectre
    {
    public:
        /// Underlying type of a public calibration field
        template<CalibrationField F>
        using CalibrationFieldType = typename ::spectre::details::field_types<F>::value_type;

        /**
        * @brief Gets the basic configuration for this instance
        *
        * The basic configuration defines the fundamental parameters of the measurement method used
        * to derive the raw-data:
        *
        * MODULATION_SCHEME
        * NUM_FREQS
        * FREQUENCY_1
        * FREQUENCY_2
        *
        * @return basic configuration for the current instance
        */
        virtual MemoryHolder<IBasicConfiguration> basicConfiguration() const = 0;

        /**
         * @brief Gets the extended configuration for this instance
         *
         *
         * @return extended configuration for this instance
         */
        virtual MemoryHolder<IExtendedConfiguration> extendedConfiguration() const = 0;

        /**
         * @brief Gets an extended configuration for a given basic configuration
         *
         * The returned valid extended configuration contains
         * Spectre's default for all parameters supported by the
         * current instance if the given basic configuration would
         * have been set.
         *
         * If the basic configuration is not valid, an empty MemoryHolder is returned.
         *
         *
         * @param basicConfiguration basic configuration
         *
         * @return extended configuration for the given basic configuration
         */
        virtual MemoryHolder<IExtendedConfiguration> extendedConfigurationFor (const IBasicConfiguration &basicConfiguration) const = 0;

        /**
         * @brief Requests a reconfiguration of the basic parameters of this instance
         *
         * A successful call of this function will apply all basic parameters in the configuration.
         * Additionally, all extended parameters which are not marked as persistent are reset to their
         * default values. Parameters will be reset to their respective default values if any of the
         * following conditions is met:
         * - the parameter is not available in the new basic configuration
         * - the parameter is available, but its value is not compatible with the new extended configuration
         *
         *
         * Calling this function might also change the available extended parameters. Call
         * extendedConfiguration() to get the actual list of available parameters.
         *
         * Note: Calling this function with a IBasicConfiguration not obtained from this
         * ISpectre instance invokes undefined behavior.
         *
         * @param basicConfig basic configuration
         *
         * @return status of operation
         */
        virtual SpectreStatus reconfigure (const IBasicConfiguration &basicConfig) = 0;

        /**
         * @brief Requests a reconfiguration of the extended parameters of this instance
         *
         *
         * Note: Calling this function with an IExtendedConfiguration not obtained from this
         * ISpectre instance *or* with an outdated instance prior to the latest call to
         * reconfigure(IBasicConfiguration) invokes undefined behavior.
         *
         *
         * @param extendedConfig extended configuration
         *
         * @return status of operation
         */
        virtual SpectreStatus reconfigure (const IExtendedConfiguration &extendedConfig) = 0;


        /**
         * @brief Queries a public calibration field from the loaded calibration
         *
         * If the field exists in the given calibration, it is copied to the passed
         * ArrayHolder<T> instance. Otherwise, an appropriate error code is returned.
         *
         * If the field type CalibrationField::CUSTOM_FIELD is selected, the second parameter
         * customIndex specifies which custom data shall be returned. For any other value of
         * CalibrationField, the customIndex is ignored.
         *
         * @param holder destination for copy operation
         * @param customIndex custom index ()
         *
         * @return status of operation
         */
        template<CalibrationField F>
        SpectreStatus copyCalibrationField (common::ArrayHolder<CalibrationFieldType<F> > &holder,
                                            int customIndex = 0) const;

        /**
         * @brief Does a complete processing of input data
         *
         * For calling it, the raw frames
         * for the intensity, each 4-phase raw frame set, and their respective exposure times
         * must be present. Additionally, the temperature is required.
         * After calling this method successfully, the return value
         * of result<T> is well-defined for each member of the Result
         * enumeration.
         *
         * Note: Spectre does not copy any data from the Input object,
         * and the caller is responsible for ensuring its validity
         * throughout this call.
         *
         * @param input input structure
         *
         * @return status of operation
         */
        virtual SpectreStatus processWhole (const Input<common::ArrayHolder> &input) = 0;

        /**
         * @see processWhole(const Input<ArrayHolder>&)
         */
        virtual SpectreStatus processWhole (const Input<common::ArrayReference> &input) = 0;

        /**
         * @brief Gets persistent initialization data for this Spectre instance
         *
         * The persistent intialization data can be used to speed up the creation of a new
         * Spectre instance. The data is platform dependent, and should only be used
         * on the same platform where it was obtained. Additionally, it is tied to the
         * Spectre version, and the calibration data.
         *
         * @return persistent initialization data
         */
        virtual common::ArrayHolder<uint8_t> persistentInitialization() const = 0;

        /**
         * @brief Checks if the persistent data has been changed
         *
         * Checks if the persistent data passed to this instance was
         * not valid for this Spectre version / calibration (or no
         * data was passed). In this case, use the persistentInitialization function
         * to retrieve data compatible with this Spectre version / calibration
         * combination
         *
         *
         * @return true if data has changed, false otherwise.
         */
        virtual bool hasPersistentDataChanged() const = 0;

        /**
         * Gets a mutable reference to the processing results for a given ResultType.
         *
         * The underlying data type of the reference array is given by
         * ResultTypeType.  The returned data is guaranteed to be
         * valid until any non-const operation is called on ISpectre, or
         * ISpectre is destroyed.
         *
         *
         * @return processed data
         */
        template<ResultType R>
        common::ArrayReference<ResultTypeType<R> > results();

        /**
         * @brief Gets a non-mutable reference to the processing results for a given ResultType.
         *
         * The underlying data type of the reference array is given by
         * ResultTypeType.  The returned data is guaranteed to be
         * valid until any non-const operation is called on ISpectre, or
         * ISpectre is destroyed.
         *
         *
         * @return processed data
         */
        template<ResultType R>
        common::ImmutableField<ResultTypeType<R> > results() const;

        /**
         * @brief Gets the expected width of the input raw-data expected by Spectre
         *
         *
         * @return expected input width
         */
        virtual uint32_t getInputWidth() const = 0;

        /**
         * @brief Gets the expected height of the input raw-data expected by Spectre
         *
         *
         * @return expected input height
         */
        virtual uint32_t getInputHeight() const = 0;

        /**
         * @brief Gets the width of the processed output depth-image
         *
         * The result is only valid after a frame has been processed by Spectre.
         * If it is called before a frame has been processed or during processing the
         * return value is undefined.
         *
         *
         * @return output width
         */
        virtual uint32_t getOutputWidth() const = 0;

        /**
         * @brief Gets the height of the processed output depth-image
         *
         * The result is only valid after a frame has been processed by Spectre.
         * If it is called before a frame has been processed or during processing the
         * return value is undefined.
         *
         *
         * @return output height
         */
        virtual uint32_t getOutputHeight() const = 0;

        /**
         * @brief Gets the width of the calibration
         *
         * The returned width is an upper-bound for results of
         * getInputWidth() and getOutputWidth()
         *
         *
         * @return calibrated width
         */
        virtual uint32_t getCalibratedWidth() const = 0;

        /**
         * @brief Gets the width of the calibration
         *
         * The returned width is an upper-bound for results of
         * getInputHeight() and getOutputHeight()
         *
         *
         * @return calibrated width
         */
        virtual uint32_t getCalibratedHeight() const = 0;

        /**
         * @brief Gets the unambiguous range of the current configuration
         *
         * @return unambiguous range
         */
        virtual float getUnambiguousRange() const = 0;

        /**
         * @brief Gets the number of threads used for depth-processing
         *
         * The returned value is the number of threads used for depth-processing
         * if the processWhole - method is called with the current configuration.
         *
         * If the configuration is changed, this value may also change.
         *
         * @return number of threads
         */
        virtual unsigned numThreads() const = 0;

    public:
        virtual ~ISpectre();
        ISpectre &operator= (const ISpectre &) = delete;
        ISpectre &operator= (ISpectre &&) = delete;
        ISpectre (const ISpectre &) = delete;
        ISpectre (ISpectre &&) = delete;

    protected:
        ISpectre() {}

    private:
        /**
         * Copies a calibration field of type uint32_t
         *
         * @param field field id to copy
         * @param holder target holder
         * @param customIdx custom index
         *
         * @return status of operation
         */
        virtual SpectreStatus copyCalibrationFieldImpl (CalibrationField field, common::ArrayHolder<uint32_t> &holder,
                int customIdx) const = 0;

        /**
         * Copies a calibration field of type uint16_t
         *
         * @param field field id to copy
         * @param holder target holder
         * @param customIdx custom index
         *
         * @return status of operation
         */
        virtual SpectreStatus copyCalibrationFieldImpl (CalibrationField field, common::ArrayHolder<uint16_t> &holder,
                int customIdx) const = 0;


        /**
         * Copies a calibration field of type uint8_t
         *
         * @param field field id to copy
         * @param holder target holder
         * @param customIdx custom index
         *
         * @return status of operation
         */
        virtual SpectreStatus copyCalibrationFieldImpl (CalibrationField field, common::ArrayHolder<uint8_t> &holder,
                int customIdx) const = 0;


        /**
         * Copies a calibration field of type float
         *
         * @param field field id to copy
         * @param holder target holder
         * @param customIdx custom index
         *
         * @return status of operation
         */
        virtual SpectreStatus copyCalibrationFieldImpl (CalibrationField field, common::ArrayHolder<float> &holder,
                int customIdx) const = 0;

        /**
         * Copies a calibration field of type char
         *
         * @param field field id to copy
         * @param holder target holder
         * @param customIdx custom index
         *
         * @return status of operation
         */
        virtual SpectreStatus copyCalibrationFieldImpl (CalibrationField field, common::ArrayHolder<char> &holder,
                int customIdx) const = 0;

        /**
         * Returns a processing result of type uint32_t
         *
         * @param r result type
         * @param results target holder
         */
        virtual void resultsImpl (ResultType r, common::ArrayReference<uint32_t> &results) = 0;

        /**
         * Returns a processing result of type float
         *
         * @param r result type
         * @param results target holder
         */
        virtual void resultsImpl (ResultType r, common::ArrayReference<float> &results) = 0;

        /**
         * Returns a processing result of type uint32_t
         *
         * @param r result type
         * @param results target holder
         */
        virtual void resultsImpl (ResultType r, common::ImmutableField<uint32_t> &results) const = 0;

        /**
         * Returns a processing result of type float
         *
         * @param r result type
         * @param results target holder
         */
        virtual void resultsImpl (ResultType r, common::ImmutableField<float> &results) const = 0;
    };

    /**
     * @brief Creates a new ISpectre instance
     *
     * On success dst is adjusted to point to the newly created ISpectre instance,
     * and its ownership is transferred to the caller. If an error occurred, *dst is
     * set to nullptr.
     *
     * @param calibBlob calibration data
     * @param dst destination
     * @param initBlob persistentInitialization data to speed up the process (optional)
     *
     * @return status of operation
     */
    SPECTRE_API SpectreStatus getSpectre (const common::AbstractField<uint8_t> &calibBlob, ISpectre **dst,
                                          const common::AbstractField<uint8_t> &initBlob = common::ArrayReference<uint8_t>());

    /**
     * @see getSpectre (const AbstractField<uint8_t> &, ISpectre **);
     *
     * This method allows to pass hints to alter the behavior of the factory method, if the standard
     * Spectre behavior is not sufficient.
     *
     *
     * @param calibBlob calibration data
     * @param hint hint to alter the method's behavior
     * @param dst destination
     * @param initBlob persistentInitialization data to speed up the process (optional)
     *
     * @return status of operation
     */
    SPECTRE_API SpectreStatus getSpectre (const common::AbstractField<uint8_t> &calibBlob, SpectreFactoryHint hint, ISpectre **dst,
                                          const common::AbstractField<uint8_t> &initBlob = common::ArrayReference<uint8_t>());


    /**
     * @brief Gets the version of Spectre
     *
     *
     * @return Spectre version
     */
    SPECTRE_API const char *getVersion();

    /**
     * @brief Gets the Git hash used to build this version
     *
     *
     * @return Git hash used to build this version
     */
    SPECTRE_API const char *getGitDescription();

    /**
     * @brief Gets a fake calibration used for auto exposure only processing
     *
     * With the fake calibration a Spectre version which only provides an auto exposure processing
     * will be created. Only the amplitude image, and the new exposure times are calculated by this instance.
     * No depth computation is done.
     * This Spectre instance only supports the following functions,
     * configuration: return a minimized config which contains only the necessary parameters,
     * reconfigure: allows the modification of the config,
     * processWhole: proceeds the auto exposure only processing and calls unscrambleReIm, calcAmplitude and
     * calcAutpExposure, and
     * results: returns the computed values for AMPLITUDES/EXPOSURE_TIMES, for all other result types a empty image.
     *
     * @param numCol width of image
     * @param numCol height of image
     *
     * @return ArrayHolder with the fake calibation
     */
    SPECTRE_API common::ArrayHolder<uint8_t> getAutoExposureOnlyCalibration (uint32_t numCol, uint32_t numRow);

}

#endif /*__ISPECTRE_HPP__*/
