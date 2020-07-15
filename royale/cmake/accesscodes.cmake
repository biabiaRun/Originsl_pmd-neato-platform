#****************************************************************************
# Copyright (C) 2019 pmdtechnologies ag & Infineon Technologies
#
# THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
# KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
# PARTICULAR PURPOSE.
#
#****************************************************************************

# SHA1 "walnuts are great"
set (ROYALE_ACCESS_CODE_LEVEL2 "d79dab562f13ef8373e906d919aec323a2857388" CACHE STRING "Access code for level 2" FORCE)
# SHA1 "we are heroes"
set (ROYALE_ACCESS_CODE_LEVEL3 "c715e2ca31e816b1ef17ba487e2a5e9efc6bbd7b" CACHE STRING "Access code for level 3" FORCE)
# SHA1 "I've been looking for freedom"
set (ROYALE_ACCESS_CODE_LEVEL4 "1a77786a05cc021099dd4b3610b19317fb12e72f" CACHE STRING "Access code for level 4" FORCE)

add_definitions(-DROYALE_ACCESS_CODE_LEVEL2="${ROYALE_ACCESS_CODE_LEVEL2}")
add_definitions(-DROYALE_ACCESS_CODE_LEVEL3="${ROYALE_ACCESS_CODE_LEVEL3}")
add_definitions(-DROYALE_ACCESS_CODE_LEVEL4="${ROYALE_ACCESS_CODE_LEVEL4}")
