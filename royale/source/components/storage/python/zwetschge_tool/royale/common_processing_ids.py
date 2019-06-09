#!/usr/bin/python3

# Copyright (C) 2019 Infineon Technologies & pmdtechnologies ag
#
# THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
# KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
# PARTICULAR PURPOSE.

"""UUIDs corresponding to those defined in Royale's CommonProcessingParameters.hpp."""

from zwetschge_tool.royale.uuidlike_identifier import castUuidlikeIdentifier

# This id and the associated map are a default set of settings for 9-phase use cases.
uuid2Freq = castUuidlikeIdentifier('common id 2 freq')

# This id and the associated map are a default set of settings for 5-phase use cases.
uuid1Freq = castUuidlikeIdentifier('common id 1 freq')

# This id is a default for mixed modes with the first stream being 9-phase and the second stream being 5-phase.
uuidMixedEsHt = castUuidlikeIdentifier('common id 2 1 fq')

# This id is a default for mixed modes with the first stream being 5-phase and the second stream being 9-phase.
uuidMixedHtEs = castUuidlikeIdentifier('common id 1 2 fq')

uuidLowNoiseExtendedParam    = castUuidlikeIdentifier('LowNoiseExtended')
uuidVideoExtendedParam       = castUuidlikeIdentifier('VideoExtended   ')
uuidVideoHalfParam           = castUuidlikeIdentifier('VideoHalf       ')
uuidVideoParam               = castUuidlikeIdentifier('Video           ')
uuidFastAcquisitionParam     = castUuidlikeIdentifier('FastAcquisition ')
uuidVeryFastAcquisitionParam = castUuidlikeIdentifier('VFastAcquisition')
