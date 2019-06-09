/****************************************************************************\
 * Copyright (C) 2017 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
\****************************************************************************/

#ifndef __ICONFIGURATION_HPP__
#define __ICONFIGURATION_HPP__

#include "common/Field.hpp"
#include "spectre/Parameter.hpp"
#include <spectre/SpectreStatus.hpp>
#include <spectre/ResultType.hpp>
#include "common/Variant.hpp"
#include "spectre/Deleter.hpp"
#include <spectre/SpectreConfig.hpp>

namespace spectre
{
    class IConfiguration;
    namespace details
    {
        SPECTRE_API void freeIConfiguration (IConfiguration *);
    }

    /**
     * @brief An encapsulation of runtime configurable parts of a Spectre instance
     *
     *
     * Note: Applying a IConfiguration instance to an ISpectre instance other than the one from where it was
     * obtained leads to undefined behavior.
     */
    class IConfiguration
    {
    public:
        /**
         * @brief Gets a list of all parameters contained in this configuration.
         *
         * The validity of the returned parameter list is bound to the lifetime of
         * the Configuration instance.
         *
         *
         * @return list of all parameters
         */
        virtual SPECTRE_API common::AbstractField<Parameter> &getParameters() = 0;

        /**
         * @brief Gets an immutable list of all parameters contained in this configuration.
         *
         * The validity of the returned parameter list is bound to the lifetime of
         * the Configuration instance.
         *
         *
         * @return list of all parameters
         */

        virtual SPECTRE_API common::ImmutableField<Parameter> getParameters() const = 0;

        /**
         * @brief Gets a parameter by its name
         *
         * If the requested Parameter is not part of the Configuration, an invalid Parameter instance is returned.
         *
         * @param name name of the parameter
         *
         * @return Parameter instance for this name, or an invalidated instance
         */
        virtual SPECTRE_API Parameter getParameterByName (const char *name) const = 0;

        /**
         * @brief Gets a parameter by its enum key
         *
         * If the requested Parameter is not part of the Configuration, an invalid Parameter instance is returned.
         *
         * @param key parameter key
         *
         * @return Parameter instance for this name, or an invalidated instance
         */
        virtual SPECTRE_API Parameter getParameterByKey (common::ParameterKey key) const = 0;

        /**
         * @brief Sets a Parameter to a new value
         *
         * If the passed Parameter is not part of the Configuration, the operation will fail.
         *
         * @param p parameter to set
         *
         * @return true on success, false otherwise
         */
        virtual SPECTRE_API bool setParameter (const Parameter &p) = 0;

        /**
         * @brief Sets a list of Parameters to a new value
         *
         * If a passed Parameter is not part of the Configuration, the operation will fail.
         * However, it is not atomic and all parameters before the failing one are set.
         * In this case, SpectreStatus::additional() will contain the key of the failing enum.
         *
         * @param pars parameters to set
         *
         * @return status of operation
         */
        virtual SPECTRE_API SpectreStatus setParameters (const common::ImmutableField<Parameter> &pars) = 0;


    protected:
        IConfiguration (const IConfiguration &) = default;
        IConfiguration &operator= (const IConfiguration &) = default;
        IConfiguration &operator= (IConfiguration &&) = delete;
        IConfiguration (IConfiguration &&) = delete;
        IConfiguration() = default;
        virtual ~IConfiguration() = default;

        friend void spectre::details::freeIConfiguration (IConfiguration *);
    };

    /**
     * @brief An encapsulation of the basic processing characteristics of an ISpectre instance configurable at runtime
     *
     * To get an instance of this class call ISpectre::basicConfiguration(). The returned configuration
     * reflects the state of ISpectre at the time of its creation. It contains all configurable basic
     * parameters of ISpectre taking the calibration into account. After changing the desired parameters
     * the changed configuration is applied by the ISpectre::reconfigure() method, so that the changes
     * are in effect.
     **/
    class IBasicConfiguration : public IConfiguration
    {
    protected:
        IBasicConfiguration (const IBasicConfiguration &) = default;
        IBasicConfiguration &operator= (const IBasicConfiguration &) = default;
        IBasicConfiguration &operator= (IBasicConfiguration &&) = delete;
        IBasicConfiguration (IBasicConfiguration &&) = delete;
        IBasicConfiguration() = default;
        virtual ~IBasicConfiguration() = default;
    };

    /**
     * @brief An encapsulation of the extended processing characteristics of an ISpectre instance configurable at runtime
     *
     * To get an instance of this class call
     * ISpectre::extendedConfiguration(). The returned configuration
     * reflects the state of ISpectre at the time of its creation. It
     * contains all configurable extended parameters and the output
     * redirections. In doing so it takes both the calibration and the
     * current basic configuration of the ISpectre instance into
     * account. After changing the desired parameters the changed
     * configuration is applied by the ISpectre::reconfigure() method,
     * so that the changes are in effect.
     **/
    class IExtendedConfiguration : public IConfiguration
    {
    public:
        template<typename T>
        class OutputRedirector;

        /**
         * Sets an output redirection for a specific ResultType R
         *
         * The output redirection contains an array reference, which will be used
         * as destination for the final depth computation results. The referenced array
         * must be of type ResultTypeType<R>, and large enough to hold all values.
         * This is checked during ISpectre::reconfigure() only if SPECTRE_ENABLE_ADDITIONAL_CHECKS
         * is enabled.
         *
         * @param redir redirection to set
         *
         */
        template<ResultType R>
        SPECTRE_API void setOutputRedirection (const OutputRedirector<ResultTypeType<R>> &redir);

        /**
         * Gets the current output redirection for a specific ResultType R
         *
         *
         * @return output redirection of the ResultType
         */
        template<ResultType R>
        SPECTRE_API OutputRedirector<ResultTypeType<R> > getOutputRedirection() const;

        /// Captures all possible redirections
        using RedirVariant = common::Variant<OutputRedirector<float>, OutputRedirector<uint32_t>>;


    protected:
        IExtendedConfiguration (const IExtendedConfiguration &) = default;
        IExtendedConfiguration &operator= (const IExtendedConfiguration &) = default;
        IExtendedConfiguration &operator= (IExtendedConfiguration &&) = delete;
        IExtendedConfiguration (IExtendedConfiguration &&) = delete;
        IExtendedConfiguration() = default;
        virtual ~IExtendedConfiguration() = default;

    private:
        virtual void setOutputRedirectionImpl (ResultType r, const OutputRedirector<float> &redir) = 0;
        virtual void setOutputRedirectionImpl (ResultType r, const OutputRedirector<uint32_t> &redir) = 0;
        virtual void getOutputRedirectionImpl (ResultType r, OutputRedirector<float> &redir) const = 0;
        virtual void getOutputRedirectionImpl (ResultType r, OutputRedirector<uint32_t> &redir) const = 0;
    };


    /// Configuration of an output redirection
    /**
     * This class can be used to redirect a selected computation
     * result of the depth-processing into external memory not hold by
     * Spectre.
     *
     * The template parameter T determines the type of the computation result.
     * It must be equal with ResultTypeType<R>, where R is the ResultType which should be redirected.
     */
    template<typename T>
    class IExtendedConfiguration::OutputRedirector
    {
    public:
        /**
         * Activates the output redirection to the specified location
         *
         * The location must be large enough to hold the output. This is checked
         * during ISpectre::reconfigure().
         *
         * @param dst destination
         */
        SPECTRE_API void setRedirection (const common::ArrayReference<T> &dst);

        /**
         * Clears the redirection.
         * After ISpectre::reconfigure() the Spectre instance will use internal memory to store
         * the specific results.
         */
        SPECTRE_API void clearRedirection();

        /**
         * @brief Gets if a redirection is active
         *
         *
         * @return true if a redirection is set, false other
         */
        SPECTRE_API bool hasRedirection() const;

        /**
         * @brief Gets the redirection target
         *
         * The result of this operation is only well-defined if
         * isRedirection() returns true.
         *
         *
         * @return target of the redirection
         */
        SPECTRE_API common::ArrayReference<T> target() const;

        /**
         * @brief Creates a new OutputRedirector instance for a ResultType
         *
         *
         * @return output redirector for ResultType R
         */
        template<ResultType R>
        SPECTRE_API static OutputRedirector<ResultTypeType<R> > create();

        template<typename U>
        friend class OutputRedirector;

    private:
        SPECTRE_API OutputRedirector();

    private:
        common::ArrayReference<T> m_dst;
        bool m_isRedirect;

        friend class Configuration;
    };

    template<typename T>
    SPECTRE_API std::ostream &operator<< (std::ostream &os, const IExtendedConfiguration::OutputRedirector<T> &redir);
    SPECTRE_API std::ostream &operator<< (std::ostream &os, const IExtendedConfiguration &c);
    SPECTRE_API std::ostream &operator<< (std::ostream &os, const IBasicConfiguration &c);

    namespace details
    {
        template<>
        struct Deleter<IConfiguration>
        {
            static void free (IConfiguration *ptr)
            {
                freeIConfiguration (ptr);
            }
        };

        template<>
        struct Deleter<IExtendedConfiguration>
        {
            static void free (IConfiguration *ptr)
            {
                freeIConfiguration (ptr);
            };
        };

        template<>
        struct Deleter<IBasicConfiguration>
        {
            static void free (IConfiguration *ptr)
            {
                freeIConfiguration (ptr);
            };
        };


    }  // details


}  // spectre


#endif /*__ICONFIGURATION_HPP__*/
