/****************************************************************************\
 * Copyright (C) 2015 Infineon Technologies & pmdtechnologies ag
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 \****************************************************************************/

#pragma once

#include <vector>
#include <memory>
#include <royale/CallbackData.hpp>
#include <royale/String.hpp>
#include <royale/ProcessingFlag.hpp>
#include <processing/ProcessingParameterId.hpp>
#include <usecase/UseCaseDefinition.hpp>
#include <common/exceptions/InvalidValue.hpp>
#include <royale/CameraAccessLevel.hpp>

namespace royale
{
    namespace usecase
    {
        /**
         * The UseCase class contains the description for one particular configuration of the imager
         * which serves the user's target actions. Typically each camera module supports multiple use cases,
         * which need to be defined during module creation.
         */
        class UseCase
        {
        public:
            /*
             * A use case needs to have to have at least two information, a name which is used as an identifier,
             * and the definition about the sequence structure and frame rates. The callback data is defaulted
             * to depth, but may also be changed to raw.
             *
             * Processing parameters must be supplied by exactly one of parameterId or parameterMap.
             *
             * @param name must not be an empty string, otherwise an InvalidValue will be thrown
             * @param definition must not be nullptr, otherwise an InvalidValue will be thrown
             * @param parameterId a string which may be passed to the ProcessingParameterMapFactory
             * @param parameterMap processing parameters that should be used for this use case
             * @param cbData The CallbackData specifies for which callback data the usecase is designed (e.g. raw or depth)
             *        raw automatically means that the depth is also supported
             * @param level The access level specifies the level that is needed to activate this usecase
             */
            UseCase (const royale::String &name,
                     std::shared_ptr<UseCaseDefinition> definition,
                     const royale::processing::ProcessingParameterId &parameterId,
                     const royale::Vector<royale::ProcessingParameterMap> &parameterMap,
                     royale::CallbackData cbData = royale::CallbackData::Depth,
                     royale::CameraAccessLevel level = CameraAccessLevel::L1) :
                m_name (name),
                m_definition (std::move (definition)),
                m_parameterId (parameterId),
                m_parameterMap (parameterMap),
                m_callbackData (cbData),
                m_level (level)
            {
                if (m_name.empty())
                {
                    throw common::InvalidValue ("name must not be empty");
                }
                if (m_definition == nullptr)
                {
                    throw common::InvalidValue ("definition must not be empty");
                }
                m_definition->verifyClassInvariants();

                if (m_parameterMap.empty())
                {
                    if (m_parameterId.isSentinel())
                    {
                        throw common::InvalidValue ("neither parameter id nor parameter map specified");
                    }
                }
                else
                {
                    if (!m_parameterId.isSentinel())
                    {
                        throw common::InvalidValue ("both parameter id and parameter map specified");
                    }
                    if (m_definition->getStreamIds().size() != parameterMap.size())
                    {
                        throw common::InvalidValue ("number of parameter maps must match number of streams");
                    }
                }
            }

            /**
             * Old-style construction, takes a parameter map.
             *
             * @param name must not be an empty string, otherwise an InvalidValue will be thrown
             * @param definition must not be nullptr, otherwise an InvalidValue will be thrown
             * @param parameterMap processing parameters that should be used for this use case
             * @param cbData The CallbackData specifies for which callback data the usecase is designed (e.g. raw or depth)
             *        raw automatically means that the depth is also supported
             * @param level The access level specifies the level that is needed to activate this usecase
             */
            UseCase (const royale::String &name,
                     std::shared_ptr<UseCaseDefinition> definition,
                     const royale::Vector<royale::ProcessingParameterMap> &parameterMap,
                     royale::CallbackData cbData = royale::CallbackData::Depth,
                     royale::CameraAccessLevel level = CameraAccessLevel::L1) :
                UseCase (name, std::move (definition), {}, parameterMap, cbData, level)
            {
            }

            /**
             * Zwetschge-style construction, takes a parameter id.
             *
             * @param name must not be an empty string, otherwise an InvalidValue will be thrown
             * @param definition must not be nullptr, otherwise an InvalidValue will be thrown
             * @param parameterId a string which may be passed to the ProcessingParameterMapFactory
             * @param cbData The CallbackData specifies for which callback data the usecase is designed (e.g. raw or depth)
             *        raw automatically means that the depth is also supported
             * @param level The access level specifies the level that is needed to activate this usecase
             */
            UseCase (const royale::String &name,
                     std::shared_ptr<UseCaseDefinition> definition,
                     const royale::processing::ProcessingParameterId &parameterId,
                     royale::CallbackData cbData = royale::CallbackData::Depth,
                     royale::CameraAccessLevel level = CameraAccessLevel::L1) :
                UseCase (name, std::move (definition), parameterId, {}, cbData, level)
            {
            }

            /**
             * Returns the case sensitive name for this use case
             */
            const royale::String &getName() const
            {
                return m_name;
            }

            /**
             * Returns a const pointer to the UseCaseDefinition. The design does not allow to change
             * the UseCaseDefinition arbitrarily
             */
            const UseCaseDefinition *getDefinition() const
            {
                return m_definition.get();
            }

            /**
             * We need to get access to the UseCaseDefinition in the transition between v1 and v2.
             * This needs to be deleted once v2 platform is up.
             *
             * \todo XXX
             */
            UseCaseDefinition *getDefinition()
            {
                return m_definition.get();
            }

            /**
             * Returns the callback data that was specified for this usecase.
             * If the callback data equals CallbackData::Raw the processing will not be able
             * to handle this usecase.
             */
            const royale::CallbackData getCallbackData() const
            {
                return m_callbackData;
            }

            /**
             * Returns the parameter id that should be used to determine which external set of
             * processing parameter maps should be used.
             */
            const royale::processing::ProcessingParameterId &getProcessingParameterId() const
            {
                return m_parameterId;
            }

            /**
             * Returns the parameters that were specified for this usecase and
             * that will be used for processing the frames.
             */
            const royale::Vector<royale::ProcessingParameterMap> &getProcessingParameters() const
            {
                return m_parameterMap;
            }

            /**
             * Set the parameters that should be used for this use case.
             *
             * @param params ProcessingParameterMap that should be used for this use case
             */
            void setProcessingParameters (const royale::Vector<royale::ProcessingParameterMap> &params)
            {
                m_parameterMap = params;
            }

            /**
             * Returns the access level that was specified for this usecase.
             * Usecases with an access level greater than L2 might not be eye safe!
             * The Royale API layer will check the access level and hide usecases that
             * are not accessible to the application.
             */
            const royale::CameraAccessLevel getAccessLevel() const
            {
                return m_level;
            }

        private:
            royale::String                                          m_name;         //!< string-based identification
            std::shared_ptr<UseCaseDefinition>                      m_definition;   //!< specifies the parameters and sequence information
            royale::processing::ProcessingParameterId               m_parameterId;  //!< the m_parameterMap will be retrieved from an IProcessingParameterMapFactory
            royale::Vector<royale::ProcessingParameterMap>          m_parameterMap; //!< specifies which processing parameters should be used
            royale::CallbackData                                    m_callbackData; //!< specifies for which data type the usecase is intended
            royale::CameraAccessLevel                               m_level;        //!< specifies for which level the usecase should be displayed
        };

        /**
         * Typically a module supports multiple use cases, this container allows to
         * set more than one use case
         */
        using UseCaseList = std::vector<UseCase>;
    }
}
