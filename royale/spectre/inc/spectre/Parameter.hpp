/****************************************************************************\
 * Copyright (C) 2017 pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
\****************************************************************************/

#ifndef __PARAMETER_HPP__
#define __PARAMETER_HPP__

#include <spectre/SpectreConfig.hpp>
#include "common/Variant.hpp"
#include <cstdint>
#include "common/ParameterKey.hpp"

namespace spectre
{
    /**
     * @brief Evaluates to the type R if the ParameterKey P identifies a frequency
     *
     * @param P ParameterKey value to check
     * @param R type to evaluate to
     */
    template<common::ParameterKey P, typename R>
    using IsFrequencyParameter = typename std::enable_if < P == common::ParameterKey::FREQUENCY_1 || P == common::ParameterKey::FREQUENCY_2, R >::type;

#define SPECTRE_CREATE_TLIST(TYPE,...) TYPE,
    /// Instantiation of Variant template used for Parameters
    using ParameterVariant = common::Variant<SPECTRE_PARAMETER_TYPES (SPECTRE_CREATE_TLIST) uintptr_t>;
#undef SPECTRE_CREATE_TLIST

    /**
     * @brief Enumeration of types used by parameters inside Spectre.
     *
     * To get or set the Variant values the associated C++ type from the table below has to be used.
     *
     * | Enumeration Value | C++ type |
     * |------------------------------|
     * | BOOL_PARAMETER    | bool     |
     * | FLOAT_PARAMETER   | float    |
     * | UINT32_PARAMETER  | uint32_t |
     * | INT32_PARAMETER   | int32_t  |
     */
#define SPECTRE_CREATE_TLIST(TYPE,DESC) DESC##_PARAMETER,
    enum class ParameterType : unsigned { SPECTRE_PARAMETER_TYPES (SPECTRE_CREATE_TLIST) };
#undef SPECTRE_CREATE_TLIST

    /**
     * @brief Encapsulation of a parameter which can be used to configure Spectre
     *
     * Each Parameter instance represents a configuration option of Spectre. Depending on
     * the Parameter its value can be specified using different data types which are all
     * encapsulated in a ParameterVariant. Use the function Parameter::type() to get
     * the expected type.
     *
     * Parameters can have a range of valid values. Use Parameter::hasBounds() to determine
     * if a range is present. Values outside of the range cannot be set.
     *
     * If the function Parameter::isValid() returns false this instance is invalid. In most
     * cases that means that the instance was retrieved through Configuration::getParameterByKey(),
     * or Configuration::getParameterByName() but the requested Parameter is not supported by
     * the actual Spectre instance.
     */
    class Parameter final
    {
    private:
        class ParameterImpl *m_impl;

    public:
        /**
         * @brief Gets the name of the parameter
         *
         *
         * @return name of parameter
         */
        SPECTRE_API const char *name() const;

        /**
         * @brief Gets a brief description of the parameter
         *
         *
         * @return brief description
         */
        SPECTRE_API const char *description() const;

        /**
         * @brief Gets the value of the parameter
         *
         * The value has the type returned by type().
         *
         * @return value of the parameter
         */
        SPECTRE_API ParameterVariant getValue() const;

        /**
         * @brief Sets a new value of the parameter
         *
         * This method checks if the value to set has the same type as retrieved by
         * the type() function. If hasBounds() is true, the method also checks
         * if the value lies inside the compact interval specified by the bounds.
         *
         * @param val value to set
         *
         * @return true if the value has been set, false otherwise
         */
        SPECTRE_API bool setValue (const ParameterVariant &val);

        /**
         * @brief Gets the upper bound of the compact interval of valid parameter values
         *
         * If hasBounds() is false for this parameter, the return value of this function is
         * undefined. The returned bound is of the same type as the value of this enum.
         *
         * @return upper bound
         */
        SPECTRE_API ParameterVariant upperBound() const;

        /**
         * @brief Gets the lower bound of the compact interval of valid parameter values
         *
         * If hasBounds() is false for this parameter, the return value of this function is
         * undefined. The returned bound is of the same type as the value of this enum.
         *
         * @return lower bound
         */
        SPECTRE_API ParameterVariant lowerBound() const;

        /**
         * @brief Gets the type of the Parameter
         *
         * The value of this Parameter, and its bounds (if it has any) is of the returned type.
         *
         *
         * @return type of the Parameter
         */
        SPECTRE_API ParameterType type() const;

        /**
         * @brief Gets if Parameter has bounds
         *
         *
         * @return true if it has bounds, false otherwise
         */
        SPECTRE_API bool hasBounds() const;

        /**
         * @brief Gets the enum key associated to this parameter
         *
         *
         * @return enum key
         */
        SPECTRE_API common::ParameterKey key() const;


        /**
         * Destructor
         */
        SPECTRE_API ~Parameter();

        /**
         * @brief Constructor
         *
         * @param impl implementation class
         */
        SPECTRE_API explicit Parameter (ParameterImpl *impl);

        /**
         * @brief Default constructor
         *
         * The returned object is in an invalid state, and not associated to any specific Spectre Parameter.
         */
        SPECTRE_API Parameter();

        /**
         * @brief Copy constructor
         *
         * @param other source object to copy
         */
        SPECTRE_API Parameter (const Parameter &other);

        /**
         * @brief Copy move constructor
         *
         * @param other source object to move
         */
        SPECTRE_API Parameter (Parameter &&other);

        /**
         * @brief Assignment operator
         *
         * @param other source to assign
         */
        SPECTRE_API Parameter &operator= (const Parameter &other);

        /**
         * @brief Move assignment operator
         *
         * @param other source to move
         */
        SPECTRE_API Parameter &operator= (Parameter &&other);

        /**
         * @brief Gets the underlying implementation class
         *
         *
         * @return underlying implementation class
         */
        SPECTRE_API const ParameterImpl &impl() const;

        /**
         * @brief Checks if the instance is valid, and associated to an actual Spectre parameter
         *
         *
         * @return true if the instance is valid, false otherwise
         */
        SPECTRE_API bool isValid() const;

        /**
         * @brief Checks if the parameter is marked as persistent
         *
         * Spectre will try to retain the value of persistent parameters if possible,
         * that is, if they are available in the new configuration.
         *
         * @return true if the parameter is persistent
         */
        SPECTRE_API bool persistent() const;

        /**
         * @brief Sets the persistent value of this parameter
         *
         * @param perst new persistent value
         */
        SPECTRE_API void setPersistent (bool perst);
    };

    /**
     * @brief Converts a parameter into a human-readable format
     *
     * @param p parameter to convert
     * @param os output stream
     *
     * @return os
     */
    SPECTRE_API std::ostream &operator<< (std::ostream &os, const Parameter &p);
}

#endif /*__PARAMETER_HPP__*/
