# Header Use case  Date: 10/12/2018 Time: 3:38 nachm., file name: ES_limits_gen_B1x_V107.xlsm
# RawFrames: 5
# Frame Rate: 15 fps
# Tint_max: 0,1ms @ 60,24MHz (Goff) / 1,5ms @ 60,24MHz (P4) /   
# Tint_start: 0,1ms @ 60,24MHz (Goff) / 1,5ms @ 60,24MHz (P4) /   
# Header Thresholds:
# Opt_power = 530 mW  @ 25[%] Duty Cycle / Alpha = 7 / Correction Factor = 122
# Margin Eye = 1,7 / Margin Skin = 1 / Wave length = 935 nm
# Power conversion Factor = 0,8 (W/A) / Shunt Resistor = 0,068Ohm / PMOS test = FALSE / Plausibility sample =  0x1
# Margin Init = 1,03 / plausibility power HH = 730 mW / plausibility power HL = 330 mW / plausibility power LH = 40 mW / plausibility power LL= -40 mW
# Module ID 0x0300 / 0x0000 / 0x0000 / 0x0000 / 0x0000 / 0x0000 / 0x0000 / 0x0004 / PMD Selene
#-----------------------------------------------------------------------------------------------
# 									Configuration Infos:
#	F_ref: 24.0 MHz		Illu-DC: 25.00			Code-Lenght: 4			SSC active: 1
#	LUT: 0				Illu-Freq: 60.24 MHz	SSC_freq: 9536.64 Hz	SSC_dev: 374404.19 Hz
#-----------------------------------------------------------------------------------------------
full_cfg = [
	(0x9000, 0x1788),	 # S00_EXPOTIME 0.100 ms @ 60.24 MHz (Grey-Scale)
	(0x9001, 0x0000),	 # S00_FRAMETIME
	(0x9002, 0x6C1F),	 # S01_EXPOTIME 1.500 ms @ 60.24 MHz 
	(0x9003, 0x0000),	 # S01_FRAMETIME
	(0x9004, 0x6C1F),	 # S02_EXPOTIME 1.500 ms @ 60.24 MHz 
	(0x9005, 0x0000),	 # S02_FRAMETIME
	(0x9006, 0x6C1F),	 # S03_EXPOTIME 1.500 ms @ 60.24 MHz 
	(0x9007, 0x0000),	 # S03_FRAMETIME
	(0x9008, 0x6C1F),	 # S04_EXPOTIME 1.500 ms @ 60.24 MHz 
	(0x9009, 0x0000),	 # S04_FRAMETIME
	(0x9080, 0x1788),	 # S00_EXPOTIMEMAX 0.100 ms @ 60.24 MHz (Grey-Scale)
	(0x9081, 0x0000),	 # S00_FRAMETIMEMAX
	(0x9082, 0x00A0),	 # S00_ILLUCFG Gray-Scale @ 60.24 MHz (LUT0)
	(0x9083, 0x00A0),	 # S00_SENSORCFG (Greyscale)
	(0x9084, 0x8000),	 # S00_ROCFG (Reconfig allowed)
	(0x9085, 0x6C1F),	 # S01_EXPOTIMEMAX 1.500 ms @ 60.24 MHz 
	(0x9086, 0x0000),	 # S01_FRAMETIMEMAX
	(0x9087, 0x0000),	 # S01_ILLUCFG 0° @ 60.24 MHz (LUT0)
	(0x9088, 0x0000),	 # S01_SENSORCFG
	(0x9089, 0x0000),	 # S01_ROCFG (Reconfig forbidden)
	(0x908A, 0x6C1F),	 # S02_EXPOTIMEMAX 1.500 ms @ 60.24 MHz 
	(0x908B, 0x0000),	 # S02_FRAMETIMEMAX
	(0x908C, 0x0020),	 # S02_ILLUCFG 90° @ 60.24 MHz (LUT0)
	(0x908D, 0x0000),	 # S02_SENSORCFG
	(0x908E, 0x0000),	 # S02_ROCFG (Reconfig forbidden)
	(0x908F, 0x6C1F),	 # S03_EXPOTIMEMAX 1.500 ms @ 60.24 MHz 
	(0x9090, 0x0000),	 # S03_FRAMETIMEMAX
	(0x9091, 0x0040),	 # S03_ILLUCFG 180° @ 60.24 MHz (LUT0)
	(0x9092, 0x0000),	 # S03_SENSORCFG
	(0x9093, 0x0000),	 # S03_ROCFG (Reconfig forbidden)
	(0x9094, 0x6C1F),	 # S04_EXPOTIMEMAX 1.500 ms @ 60.24 MHz 
	(0x9095, 0x0000),	 # S04_FRAMETIMEMAX
	(0x9096, 0x0060),	 # S04_ILLUCFG 270° @ 60.24 MHz (LUT0)
	(0x9097, 0x0000),	 # S04_SENSORCFG
	(0x9098, 0x8000),	 # S04_ROCFG (Reconfig allowed)
	(0x91C0, 0x0592),	 # CSICFG
	(0x91C1, 0xDF00),	 # ROICOL
	(0x91C2, 0xAB00),	 # ROIROW
	(0x91C3, 0x0008),	 # ROS
	(0x91C4, 0x0008),	 # EXPCFG0
	(0x91C5, 0x0020),	 # EXPCFG1
	(0x91C6, 0x8008),	 # PSOUT
	(0x91CD, 0x0009),	 # RAW_CFG
	(0x91CE, 0x249F),	 # RAW_FRAMETIME
	(0x91CF, 0x0005),	 # MB_CFG0 (5 sequences to execute)
	(0x91D3, 0x0C35),	 # MB0_FRAMETIME (15 fps)
	(0x91DB, 0x0008),	 # MBSEQ0_CFG0 (MB0 Repetitions = 1)
	(0x91EA, 0x16A1),	 # PLL_MODLUT0_CFG0 (for Fmod = 60.24 MHz)
	(0x91EB, 0x1EB8),	 # PLL_MODLUT0_CFG1 (for Fmod = 60.24 MHz)
	(0x91EC, 0x0005),	 # PLL_MODLUT0_CFG2 (for Fmod = 60.24 MHz)
	(0x91ED, 0x0C01),	 # PLL_MODLUT0_CFG3 (for Fmod = 60.24 MHz)
	(0x91EE, 0xFCBF),	 # PLL_MODLUT0_CFG4 (for Fmod = 60.24 MHz)
	(0x91EF, 0x04A7),	 # PLL_MODLUT0_CFG5 (for Fmod = 60.24 MHz)
	(0x91F0, 0x000D),	 # PLL_MODLUT0_CFG6 (for Fmod = 60.24 MHz)
	(0x91F1, 0x0022),	 # PLL_MODLUT0_CFG7 (for Fmod = 60.24 MHz)
	(0x9220, 0x0003),	 # SENSOR_LENGHT_CODE0 (length = 4)
	(0x9221, 0x000C),	 # SENSOR_CODE0_0 (code = 1100)
	(0x9244, 0x0003),	 # ILLU_LENGHT_CODE0 (length = 4)
	(0x9245, 0x0008),	 # ILLU_CODE0_0 (code = 1000)
	(0x9268, 0x0001),	 # DIGCLKDIV_S0_PLL0 (for Fmod = 60.24 MHz)
	(0x9278, 0x0001),	 # DLLREGDELAY_S0_PLL0 (for Fmod = 60.24 MHz)
	(0x9288, 0x0255),	 # Filter Stage initializaiton Value 611
	(0x9289, 0x0259),	 # Filter Stage initializaiton Value 610
	(0x928A, 0x0259),	 # Filter Stage initializaiton Value 69 
	(0x928B, 0x0255),	 # Filter Stage initializaiton Value 68 
	(0x928C, 0x0259),	 # Filter Stage initializaiton Value 67 
	(0x928D, 0x0259),	 # Filter Stage initializaiton Value 66 
	(0x928E, 0x0255),	 # Filter Stage initializaiton Value 65 
	(0x928F, 0x0259),	 # Filter Stage initializaiton Value 64 
	(0x9290, 0x0259),	 # Filter Stage initializaiton Value 63 
	(0x9291, 0x0255),	 # Filter Stage initializaiton Value 62 
	(0x9292, 0x0259),	 # Filter Stage initializaiton Value 61 
	(0x9293, 0x0259),	 # Filter Stage initializaiton Value 60 
	(0x9294, 0x0123),	 # Filter Stage initializaiton Value 57 
	(0x9295, 0x0135),	 # Filter Stage initializaiton Value 56 
	(0x9296, 0x0123),	 # Filter Stage initializaiton Value 55 
	(0x9297, 0x0135),	 # Filter Stage initializaiton Value 54 
	(0x9298, 0x0123),	 # Filter Stage initializaiton Value 53 
	(0x9299, 0x0135),	 # Filter Stage initializaiton Value 52 
	(0x929A, 0x0123),	 # Filter Stage initializaiton Value 51 
	(0x929B, 0x0135),	 # Filter Stage initializaiton Value 50 
	(0x929C, 0x0040),	 # Filter Stage initializaiton Value 47 
	(0x929D, 0x0087),	 # Filter Stage initializaiton Value 46 
	(0x929E, 0x0040),	 # Filter Stage initializaiton Value 45 
	(0x929F, 0x0040),	 # Filter Stage initializaiton Value 44 
	(0x92A0, 0x0087),	 # Filter Stage initializaiton Value 43 
	(0x92A1, 0x0080),	 # Filter Stage initializaiton Value 42 
	(0x92A2, 0x0080),	 # Filter Stage initializaiton Value 41 
	(0x92A3, 0x0080),	 # Filter Stage initializaiton Value 40 
	(0x92A4, 0x0040),	 # Filter Stage initializaiton Value 37 
	(0x92A5, 0x0040),	 # Filter Stage initializaiton Value 36 
	(0x92A6, 0x0040),	 # Filter Stage initializaiton Value 35 
	(0x92A7, 0x0040),	 # Filter Stage initializaiton Value 34 
	(0x92A8, 0x0040),	 # Filter Stage initializaiton Value 33 
	(0x92A9, 0x0040),	 # Filter Stage initializaiton Value 32 
	(0x92AA, 0x0040),	 # Filter Stage initializaiton Value 31 
	(0x92AB, 0x0040),	 # Filter Stage initializaiton Value 30 
	(0x92AC, 0x0040),	 # Filter Stage initializaiton Value 27 
	(0x92AD, 0x0040),	 # Filter Stage initializaiton Value 26 
	(0x92AE, 0x0040),	 # Filter Stage initializaiton Value 25 
	(0x92AF, 0x0040),	 # Filter Stage initializaiton Value 24 
	(0x92B0, 0x0040),	 # Filter Stage initializaiton Value 23 
	(0x92B1, 0x0040),	 # Filter Stage initializaiton Value 22 
	(0x92B2, 0x0040),	 # Filter Stage initializaiton Value 21 
	(0x92B3, 0x0040),	 # Filter Stage initializaiton Value 20 
	(0x92B4, 0x0040),	 # Filter Stage initializaiton Value 17 
	(0x92B5, 0x0040),	 # Filter Stage initializaiton Value 16 
	(0x92B6, 0x0040),	 # Filter Stage initializaiton Value 15 
	(0x92B7, 0x0040),	 # Filter Stage initializaiton Value 14 
	(0x92B8, 0x0040),	 # Filter Stage initializaiton Value 13 
	(0x92B9, 0x0040),	 # Filter Stage initializaiton Value 12 
	(0x92BA, 0x0040),	 # Filter Stage initializaiton Value 11 
	(0x92BB, 0x0040),	 # Filter Stage initializaiton Value 10 
	(0x92BC, 0x0040),	 # Filter Stage initializaiton Value 07 
	(0x92BD, 0x0040),	 # Filter Stage initializaiton Value 06 
	(0x92BE, 0x0040),	 # Filter Stage initializaiton Value 05 
	(0x92BF, 0x0040),	 # Filter Stage initializaiton Value 04 
	(0x92C0, 0x0040),	 # Filter Stage initializaiton Value 03 
	(0x92C1, 0x0040),	 # Filter Stage initializaiton Value 02 
	(0x92C2, 0x0040),	 # Filter Stage initializaiton Value 01 
	(0x92C3, 0x0040),	 # Filter Stage initializaiton Value 00 
	(0x92C4, 0x5D37),	 # TH0
	(0x92C5, 0x9C7F),	 # TH1
	(0x92C6, 0xD4B9),	 # TH2
	(0x92C7, 0xFFEF),	 # TH3
	(0x92C8, 0x4C37),	 # TH4
	(0x92C9, 0x705E),	 # TH5
	(0x92CA, 0x9180),	 # TH6
	(0x92CB, 0x00A0),	 # TH7
	(0x92CC, 0x2E22),	 # TH8
	(0x92CD, 0x4439),	 # TH9
	(0x92CE, 0x594F),	 # TH10
	(0x92CF, 0x0062),	 # TH11
	(0x92D0, 0x1815),	 # TH12
	(0x92D1, 0x1A19),	 # TH13
	(0x92D2, 0x1C1B),	 # TH14
	(0x92D3, 0x001D),	 # TH15
	(0x92D4, 0x2D25),	 # TH16
	(0x92D5, 0x4C44),	 # TH17
	(0x92D6, 0x6954),	 # TH18
	(0x92D7, 0x0071),	 # TH19
	(0x92D8, 0x4932),	 # TH20
	(0x92D9, 0x755E),	 # TH21
	(0x92DA, 0x9F89),	 # TH22
	(0x92DB, 0x00B3),	 # TH23
	(0x92DC, 0x0F0A),	 # TH24
	(0x92DD, 0x1914),	 # TH25
	(0x92DE, 0x221D),	 # TH26
	(0x92DF, 0x2B26),	 # TH27
	(0x92E0, 0x3530),	 # TH28
	(0x92E1, 0x003A),	 # TH29
	(0x92E2, 0x0500),	 # CMPMUX
	(0x92E3, 0x08EE),	 # HH
	(0x92E4, 0x088E),	 # HL
	(0x92E5, 0x0849),	 # LH
	(0x92E6, 0x0836),	 # LL
	(0x92E7, 0x0001),	 # PlausiCheck ; Idle Mode enable 
	(0x92E8, 0x00D0),	 # Idle Counter
	(0x92E9, 0x0300),	 # Module ID 0
	(0x92EA, 0x0000),	 # Module ID 1
	(0x92EB, 0x0000),	 # Module ID 2
	(0x92EC, 0x0000),	 # Module ID 3
	(0x92ED, 0x0000),	 # Module ID 4
	(0x92EE, 0x0000),	 # Module ID 5
	(0x92EF, 0x0000),	 # Module ID 6
	(0x92F0, 0x0004),	 # Module ID 7
	(0x92F1, 0x1AEA),	 # CRC
	(0x9401, 0x0002),	 # Mode
	(0xA001, 0x0007),	 # DMUX1
	(0xA008, 0x1513),	 # PADGPIOCFG1
	(0xA00C, 0x0135),	 # PADMODCFG
	(0xA039, 0x1AA1),	 # PLL_SYSLUT_CFG0 (for fref = 24.00 MHz)
	(0xA03A, 0xAAAB),	 # PLL_SYSLUT_CFG1 (for fref = 24.00 MHz)
	(0xA03B, 0x000A),	 # PLL_SYSLUT_CFG2 (for fref = 24.00 MHz)
	(0xA03C, 0x0000),	 # PLL_SYSLUT_CFG3 (for fref = 24.00 MHz)
	(0xA03D, 0x03C0),	 # PLL_SYSLUT_CFG4 (for fref = 24.00 MHz)
	(0xA03E, 0x0000),	 # PLL_SYSLUT_CFG5 (for fref = 24.00 MHz)
	(0xA03F, 0x0017),	 # PLL_SYSLUT_CFG6 (for fref = 24.00 MHz)
]