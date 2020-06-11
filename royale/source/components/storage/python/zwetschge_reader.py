import argparse

import zwetschge_tool
from zwetschge_tool.zwetschge import reader

if __name__ == "__main__":
    """
    Allows loading a Zwetschge File to get information about its content
    """
    parser = argparse.ArgumentParser(description='pmd calibration software')
    parser.add_argument('--zwetschge_file', help='full path to Zwetschge file', default="", required=True)
    parser.add_argument('--calib_file_out', help='full path for the calibration file to write', default="", required=False)
    args = parser.parse_args()

    print('Zwetschge Tool version is: {}'.format(zwetschge_tool.__version__))

    zwetschge_data = reader.ZwetschgeData()
    zwetschge_data.load_zwetschge(args.zwetschge_file)

    if len(args.calib_file_out) !=0:
        zwetschge_data.store_calibration_data(args.calib_file_out)


