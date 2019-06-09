#!/usr/bin/python3

"""For converting Python to Lena (not Zwetschge), the input files being
converted define a single variable, full_cfg, as follows.

For the ZwetschgeTool, this is used from the device file, either by putting it
in that file or loading it as demonstrated in ExampleDevice.py and
ExampleFlashImage.py.

There's a subtle difference between the registers for ExampleDevice and
ExampleFlashImage.  Because the registers for ExampleFlashImage must be
sequential, the value 0x1234 is in registed 0x9002 for the FlashImage.

Both Zwetschge and Lena support timings, although they're not used in the .py
files that are supplied to py2lena.
"""

# ExampleDevice.py imports this and uses it as data for the TimedRegisterList.
# For this it doesn't need the zwetschge.data_types, as any list of (int, int)
# or (int, int, int) types can be used.
full_cfg = [
        (0x9000, 0x0001),
        (0x9001, 0x0002, 50 * 1000),
        (0xaaaa, 0x1234),
        # ... (circa 2000 more entries) ...
        ];


from zwetschge_tool.zwetschge.data_types import SequentialRegisterBlock

# ExampleFlashImage.py imports this and uses it as the block of sequential
# registers.  For this the SequentialRegisterBlock from zwetschge.data_types
# should be used.
full_cfg_as_sequential_register_block = SequentialRegisterBlock (
        [0x0001, 0x0002, 0x1234],
        0x9000)
