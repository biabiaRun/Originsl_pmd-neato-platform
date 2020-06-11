import os
import struct
from zlib import crc32

from . import data_types

OFFSET_CRC = 9
TOC_OFFSET_VERSION = 13
OFFSET_EMBEDDED_SPECIFIC = 16
OFFSET_PRODUCT_CODE = 26
OFFSET_SYSTEM_FREQUENCY = 42
OFFSET_REGISTER_MAPS = 46
OFFSET_CALIBRATION = 52
OFFSET_CALI_CRC = 58
OFFSET_NUMBER_OF_USE_CASES = 62
OFFSET_USE_CASES = 63
OFFSET_MODULE_SERIAL = 69


def unpack24(inArray, offset):
    return inArray[offset] + (inArray[offset+1] << 8) + (inArray[offset+2] << 16)

def unpack24p24s(inArray, offset):
    p = unpack24(inArray, offset)
    s = unpack24(inArray, offset+3)
    return p, s

def unpack24p24s16a(inArray, offset):
    p = unpack24(inArray, offset)
    s = unpack24(inArray, offset+3)
    a = struct.unpack('H', inArray[offset+6:offset+8])[0]
    return p, s, a


class ZwetschgeData:
    def __init__(self):
        self.zwetschgeData = None
        self.zwetschgeVersion = None
        self.productCode = None
        self.systemFrequency = None
        self.tableOfRegisterMaps = None
        self.calibInfo = None
        self.calibrationCrc = None
        self.calibrationData = None
        self.numberOfUseCases = None
        self.tableOfUseCases = None
        self.moduleSerial = None
        self.useCases = []
        self.crc = None
        self.tableOfModuleSuffix = None
        self.moduleSuffix = None
        self.useCaseData = None
        self.RegisterMapsData = None
        self.RegisterMapSettings = None

    def load_zwetschge(self, infile):
        """
        Reads the content of a Zwetschge File into the class ZwetschgeData

        :param infile: Filename to a Zwetschge File or bytearray containing Zwetschge Data
        :return:
        """
        if isinstance(infile, str):
            if os.path.isfile(infile):
                inFile = open(infile, "rb")
                self.zwetschgeData = bytearray(inFile.read())
        elif isinstance(infile, bytearray):
            self.zwetschgeData = infile
        else:
            raise FileNotFoundError('Not supported file format found!')

        if self.zwetschgeData[:9] != bytes("ZWETSCHGE", 'ascii'):
            zwetschgeFile = self.zwetschgeData[data_types.FLASH_OFFSET:]

        if self.zwetschgeData[:9] != bytes("ZWETSCHGE", 'ascii'):
            raise FileNotFoundError("Not a valid Zwetschge file")

        self.read_table_of_contents()

        self.get_calibration()

        self.get_use_cases()

        self.get_module_suffix()

        self.get_register_maps()

    def read_table_of_contents(self):
        """
        Reads the table of contents of Zwetschge Data into the class ZwetschgeData
        :return:
        """

        self.crc = struct.unpack('I', self.zwetschgeData[OFFSET_CRC:OFFSET_CRC+4])[0]

        self.zwetschgeVersion = unpack24(self.zwetschgeData, TOC_OFFSET_VERSION)
        if self.zwetschgeVersion != 0x147:
            raise Exception('Zwetschge version not supported!')

        self.tableOfModuleSuffix = unpack24p24s(self.zwetschgeData, OFFSET_EMBEDDED_SPECIFIC)

        self.productCode = self.zwetschgeData[OFFSET_PRODUCT_CODE:OFFSET_PRODUCT_CODE+16]

        self.systemFrequency = struct.unpack('I', self.zwetschgeData[OFFSET_SYSTEM_FREQUENCY:OFFSET_SYSTEM_FREQUENCY+4])[0]

        self.tableOfRegisterMaps = unpack24p24s(self.zwetschgeData, OFFSET_REGISTER_MAPS)

        self.calibInfo = unpack24p24s(self.zwetschgeData, OFFSET_CALIBRATION)

        self.calibrationCrc = struct.unpack('I', self.zwetschgeData[OFFSET_CALI_CRC:OFFSET_CALI_CRC+4])[0]

        self.numberOfUseCases = struct.unpack('B', self.zwetschgeData[OFFSET_NUMBER_OF_USE_CASES:OFFSET_NUMBER_OF_USE_CASES+1])[0]

        self.tableOfUseCases = unpack24p24s(self.zwetschgeData, OFFSET_USE_CASES)

        self.moduleSerial = self.zwetschgeData[OFFSET_MODULE_SERIAL:OFFSET_MODULE_SERIAL+19].decode()

        # verify table of content crc
        if crc32(self.zwetschgeData[TOC_OFFSET_VERSION:OFFSET_MODULE_SERIAL+19]) != self.crc:
            raise Exception('Table of content checksum mismatch!')


    def get_calibration(self):
        """
        Gets the calibration data out of Zwetschge data

        :return:
        """
        if isinstance(self.calibInfo, tuple):
            self.calibrationData = self.zwetschgeData[self.calibInfo[0] - data_types.FLASH_OFFSET:self.calibInfo[0] +
                                                                                       self.calibInfo[1] - data_types.FLASH_OFFSET]

            # verify calibration crc
            if crc32(self.calibrationData) != self.calibrationCrc:
                raise Exception('Calibration checksum mismatch!')

    def get_use_cases(self):
        """
        Gets the Use Cases out of Zwetschge data

        :return:
        """

        if isinstance(self.tableOfUseCases, tuple):
            if self.useCaseData is None:
                self.useCaseData = self.zwetschgeData[self.tableOfUseCases[0] - data_types.FLASH_OFFSET:
                                                      self.tableOfUseCases[0] - data_types.FLASH_OFFSET + self.tableOfUseCases[1]]

            touc_magic = self.useCaseData[0:5].decode()

            touc_crc = struct.unpack('I', self.useCaseData[5:9])[0]

            start_read_use_case = 9

            for num_use_case in range(self.numberOfUseCases):
                use_case = data_types.UseCase()
                pos_in = 0
                len_use_case = \
                struct.unpack('H', self.useCaseData[start_read_use_case + pos_in:start_read_use_case + pos_in + 2])[0]
                pos_in += 2

                use_case.seqRegBlockAddress = unpack24(self.useCaseData, start_read_use_case + pos_in)
                pos_in += 3
                use_case.seqRegBlockLen = unpack24(self.useCaseData, start_read_use_case + pos_in)
                pos_in += 3
                use_case.seqRegBlockImagerAddress = \
                struct.unpack('H', self.useCaseData[start_read_use_case + pos_in:start_read_use_case + pos_in + 2])[0]
                pos_in += 2

                use_case.imageSize.append(
                    struct.unpack('H', self.useCaseData[start_read_use_case + pos_in:start_read_use_case + pos_in + 2])[0])
                pos_in += 2
                use_case.imageSize.append(
                    struct.unpack('H', self.useCaseData[start_read_use_case + pos_in:start_read_use_case + pos_in + 2])[0])
                pos_in += 2

                use_case.guid = self.useCaseData[start_read_use_case + pos_in:start_read_use_case + pos_in + 16]
                pos_in += 16

                use_case.startFps = \
                struct.unpack('B', self.useCaseData[start_read_use_case + pos_in:start_read_use_case + pos_in + 1])[0]
                pos_in += 1
                use_case.fpsLimits.append(
                    struct.unpack('B', self.useCaseData[start_read_use_case + pos_in:start_read_use_case + pos_in + 1])[0])
                pos_in += 1
                use_case.fpsLimits.append(
                    struct.unpack('B', self.useCaseData[start_read_use_case + pos_in:start_read_use_case + pos_in + 1])[0])
                pos_in += 1

                use_case.processingParams = self.useCaseData[start_read_use_case + pos_in:start_read_use_case + pos_in + 16]
                pos_in += 16

                use_case.waitTime = unpack24(self.useCaseData, start_read_use_case + pos_in)
                pos_in += 3

                use_case.accessLevel = \
                struct.unpack('B', self.useCaseData[start_read_use_case + pos_in:start_read_use_case + pos_in + 1])[0]
                pos_in += 1

                len_name = \
                struct.unpack('B', self.useCaseData[start_read_use_case + pos_in:start_read_use_case + pos_in + 1])[0]
                pos_in += 1
                num_measurement_blocks = \
                struct.unpack('H', self.useCaseData[start_read_use_case + pos_in:start_read_use_case + pos_in + 2])[0]
                pos_in += 2
                num_frequencies = \
                struct.unpack('H', self.useCaseData[start_read_use_case + pos_in:start_read_use_case + pos_in + 2])[0]
                pos_in += 2
                num_timed_reg_list = \
                struct.unpack('H', self.useCaseData[start_read_use_case + pos_in:start_read_use_case + pos_in + 2])[0]
                pos_in += 2
                num_stream_ids = \
                struct.unpack('B', self.useCaseData[start_read_use_case + pos_in:start_read_use_case + pos_in + 1])[0]
                pos_in += 1
                num_exposure_groups = \
                struct.unpack('B', self.useCaseData[start_read_use_case + pos_in:start_read_use_case + pos_in + 1])[0]
                pos_in += 1
                num_raw_frame_sets = \
                struct.unpack('H', self.useCaseData[start_read_use_case + pos_in:start_read_use_case + pos_in + 2])[0]
                pos_in += 2
                len_reserved_block = \
                struct.unpack('B', self.useCaseData[start_read_use_case + pos_in:start_read_use_case + pos_in + 1])[0]
                pos_in += 1

                use_case.name = self.useCaseData[
                                start_read_use_case + pos_in:start_read_use_case + pos_in + len_name].decode()
                pos_in += len_name

                for i in range(num_measurement_blocks):
                    use_case.measurementBlocks.append(
                        struct.unpack('H', self.useCaseData[start_read_use_case + pos_in:start_read_use_case + pos_in + 2])[
                            0])
                    pos_in += 2

                for i in range(num_frequencies):
                    use_case.imagerFrequencies.append(
                        struct.unpack('I', self.useCaseData[start_read_use_case + pos_in:start_read_use_case + pos_in + 4])[
                            0])
                    pos_in += 4

                for i in range(num_timed_reg_list):
                    timed_reg_list_address = \
                    struct.unpack('H', self.useCaseData[start_read_use_case + pos_in:start_read_use_case + pos_in + 2])[0]
                    pos_in += 2
                    timed_reg_list_value = \
                    struct.unpack('H', self.useCaseData[start_read_use_case + pos_in:start_read_use_case + pos_in + 2])[0]
                    pos_in += 2
                    timed_reg_list_delay = \
                    struct.unpack('H', self.useCaseData[start_read_use_case + pos_in:start_read_use_case + pos_in + 2])[0]
                    pos_in += 2
                    use_case.timedRegList.append([timed_reg_list_address, timed_reg_list_value, timed_reg_list_delay])

                for i in range(num_stream_ids):
                    use_case.streamIds.append(
                        struct.unpack('H', self.useCaseData[start_read_use_case + pos_in:start_read_use_case + pos_in + 2])[
                            0])
                    pos_in += 2

                for i in range(num_exposure_groups):
                    exposure_time = \
                    struct.unpack('H', self.useCaseData[start_read_use_case + pos_in:start_read_use_case + pos_in + 2])[0]
                    pos_in += 2
                    exposure_limit_low = \
                    struct.unpack('H', self.useCaseData[start_read_use_case + pos_in:start_read_use_case + pos_in + 2])[0]
                    pos_in += 2
                    exposure_limit_high = \
                    struct.unpack('H', self.useCaseData[start_read_use_case + pos_in:start_read_use_case + pos_in + 2])[0]
                    pos_in += 2
                    use_case.exposureGroups.append([exposure_time, [exposure_limit_low, exposure_limit_high]])

                for i in range(num_raw_frame_sets):
                    raw_frame_count = \
                    struct.unpack('B', self.useCaseData[start_read_use_case + pos_in:start_read_use_case + pos_in + 1])[0]
                    pos_in += 1
                    raw_frame_freq = \
                    struct.unpack('I', self.useCaseData[start_read_use_case + pos_in:start_read_use_case + pos_in + 4])[0]
                    pos_in += 4
                    raw_frame_sequence_type = \
                    struct.unpack('B', self.useCaseData[start_read_use_case + pos_in:start_read_use_case + pos_in + 1])[0]
                    pos_in += 1
                    use_case.rawFrameSets.append([raw_frame_count, raw_frame_freq, raw_frame_sequence_type])

                use_case.reservedBlock = self.useCaseData[
                                         start_read_use_case + pos_in:start_read_use_case + pos_in + len_reserved_block]
                use_case.get_config(self.useCaseData)

                self.useCases.append(use_case)
                start_read_use_case += len_use_case

            # verify table of use case crc
            if crc32(self.useCaseData[9:start_read_use_case]) != touc_crc:
                raise Exception('Table of use case checksum mismatch!')

    def get_module_suffix(self):
        """
        Reads the Module Suffix from the Zwetschge Data

        :return:
        """
        if isinstance(self.tableOfModuleSuffix, tuple):
            self.moduleSuffix = self.zwetschgeData[self.tableOfModuleSuffix[0]-data_types.FLASH_OFFSET:self.tableOfModuleSuffix[0]-data_types.FLASH_OFFSET+self.tableOfModuleSuffix[1]]

    def get_register_maps(self):
        """
        Reads the register maps out of the Zwetschge Data

        :return:
        """
        if isinstance(self.tableOfRegisterMaps, tuple):
            if self.RegisterMapsData is None:
                self.RegisterMapsData = self.zwetschgeData[self.tableOfRegisterMaps[0] - data_types.FLASH_OFFSET:
                                                      self.tableOfRegisterMaps[0] - data_types.FLASH_OFFSET + self.tableOfRegisterMaps[1]]
        self.RegisterMapSettings = data_types.TableOfRegisterMaps()

        magic = self.RegisterMapsData[0:5].decode()
        pos_in = 5
        crc = struct.unpack('I', self.RegisterMapsData[pos_in:pos_in+4])[0]
        pos_in += 4
        version = unpack24(self.RegisterMapsData, pos_in)
        pos_in += 3
        fwPage1_super = unpack24p24s16a(self.RegisterMapsData, pos_in)
        pos_in += 8
        fwPage2_super = unpack24p24s16a(self.RegisterMapsData, pos_in)
        pos_in += 8

        len_init = struct.unpack('H', self.RegisterMapsData[pos_in:pos_in + 2])[0]
        pos_in += 2
        len_fwPage1 = struct.unpack('H', self.RegisterMapsData[pos_in:pos_in + 2])[0]
        pos_in += 2
        len_fwPage2 = struct.unpack('H', self.RegisterMapsData[pos_in:pos_in + 2])[0]
        pos_in += 2
        len_fwStart = struct.unpack('H', self.RegisterMapsData[pos_in:pos_in + 2])[0]
        pos_in += 2
        len_start = struct.unpack('H', self.RegisterMapsData[pos_in:pos_in + 2])[0]
        pos_in += 2
        len_stop = struct.unpack('H', self.RegisterMapsData[pos_in:pos_in + 2])[0]
        pos_in += 2

        self.RegisterMapSettings.init = []
        for i in range(len_init):
            self.RegisterMapSettings.init.append((struct.unpack('H', self.RegisterMapsData[pos_in:pos_in + 2])[0],
                                                  struct.unpack('H', self.RegisterMapsData[pos_in + 2:pos_in + 4])[0],
                                                  struct.unpack('H', self.RegisterMapsData[pos_in + 4:pos_in + 6])[0]))
            pos_in += 6

        self.RegisterMapSettings.fwPage1 = []
        for i in range(len_fwPage1):
            self.RegisterMapSettings.fwPage1.append((struct.unpack('H', self.RegisterMapsData[pos_in:pos_in + 2])[0],
                                                     struct.unpack('H', self.RegisterMapsData[pos_in + 2:pos_in + 4])[0],
                                                     struct.unpack('H', self.RegisterMapsData[pos_in + 4:pos_in + 6])[0]))
            pos_in += 6

        self.RegisterMapSettings.fwPage2 = []
        for i in range(len_fwPage2):
            self.RegisterMapSettings.fwPage2.append((struct.unpack('H', self.RegisterMapsData[pos_in:pos_in + 2])[0],
                                                     struct.unpack('H', self.RegisterMapsData[pos_in + 2:pos_in + 4])[0],
                                                     struct.unpack('H', self.RegisterMapsData[pos_in + 4:pos_in + 6])[0]))
            pos_in += 6

        self.RegisterMapSettings.fwStart = []
        for i in range(len_fwStart):
            self.RegisterMapSettings.fwStart.append((struct.unpack('H', self.RegisterMapsData[pos_in:pos_in + 2])[0],
                                                     struct.unpack('H', self.RegisterMapsData[pos_in + 2:pos_in + 4])[0],
                                                     struct.unpack('H', self.RegisterMapsData[pos_in + 4:pos_in + 6])[0]))
            pos_in += 6

        self.RegisterMapSettings.start = []
        for i in range(len_start):
            self.RegisterMapSettings.start.append((struct.unpack('H', self.RegisterMapsData[pos_in:pos_in + 2])[0],
                                                   struct.unpack('H', self.RegisterMapsData[pos_in + 2:pos_in + 4])[0],
                                                   struct.unpack('H', self.RegisterMapsData[pos_in + 4:pos_in + 6])[0]))
            pos_in += 6

        self.RegisterMapSettings.stop = []
        for i in range(len_stop):
            self.RegisterMapSettings.stop.append((struct.unpack('H', self.RegisterMapsData[pos_in:pos_in + 2])[0],
                                                  struct.unpack('H', self.RegisterMapsData[pos_in + 2:pos_in + 4])[0],
                                                  struct.unpack('H', self.RegisterMapsData[pos_in + 4:pos_in + 6])[0]))
            pos_in += 6

        # verify register maps crc
        if crc32(self.RegisterMapsData[9:pos_in]) != crc:
            raise Exception('Register maps checksum mismatch!')

    def store_calibration_data(self, outfile):
        outFile = open(outfile, "wb")
        outFile.write(self.calibrationData)
