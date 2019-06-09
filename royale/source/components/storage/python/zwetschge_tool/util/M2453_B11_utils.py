from operator import xor
from ..zwetschge.data_types import SequentialRegisterBlock

def baseCrc(C,D):
    NewCRC = 0x0000

    # change to LSB addressing
    C = '{0:016b}'.format(C)[::-1]
    D = '{0:08b}'.format(D)[::-1]
    #start the XOR operation
    NewCRC  += xor(int(C[9-1],2), xor(int(D[5-1],2),xor(int(C[5-1],2),xor(int(D[1-1],2),int(C[1-1],2)))))
    NewCRC  += xor(int(C[10-1],2),xor(int(D[6-1],2),xor(int(C[6-1],2),xor(int(D[2-1],2),int(C[2-1],2))))) << 1
    NewCRC  += xor(int(C[11-1],2),xor(int(D[7-1],2),xor(int(C[7-1],2),xor(int(D[3-1],2),int(C[3-1],2))))) << 2 
    NewCRC  += xor(int(C[12-1],2),xor(int(D[1-1],2),xor(int(C[1-1],2),xor(int(D[8-1],2),xor(int(C[8-1],2),xor(int(D[4-1],2),int(C[4-1],2))))))) << 3
    NewCRC  += xor(int(C[13-1],2),xor(int(D[2-1],2),int(C[2-1],2))) << 4
    NewCRC  += xor(int(C[14-1],2),xor(int(D[3-1],2),int(C[3-1],2))) << 5
    NewCRC  += xor(int(C[15-1],2),xor(int(D[4-1],2),int(C[4-1],2))) << 6
    NewCRC  += xor(int(C[16-1],2),xor(int(D[5-1],2),xor(int(C[5-1],2),xor(int(D[1-1],2),int(C[1-1],2))))) << 7
    NewCRC  += xor(int(C[1-1],2),xor(int(D[1-1],2),xor(int(C[6-1],2),xor(int(D[6-1],2),xor(int(C[2-1],2),int(D[2-1],2)))))) << 8
    NewCRC  += xor(int(C[2-1],2),xor(int(D[2-1],2),xor(int(C[7-1],2),xor(int(D[7-1],2),xor(int(C[3-1],2),int(D[3-1],2)))))) << 9
    NewCRC  += xor(int(C[3-1],2),xor(int(D[3-1],2),xor(int(C[8-1],2),xor(int(D[8-1],2),xor(int(C[4-1],2),int(D[4-1],2)))))) << 10
    NewCRC  += xor(int(C[4-1],2),int(D[4-1],2)) << 11
    NewCRC  += xor(int(C[5-1],2),xor(int(D[5-1],2),xor(int(C[1-1],2),int(D[1-1],2)))) << 12
    NewCRC  += xor(int(C[6-1],2),xor(int(D[6-1],2),xor(int(C[2-1],2),int(D[2-1],2)))) << 13
    NewCRC  += xor(int(C[7-1],2),xor(int(D[7-1],2),xor(int(C[3-1],2),int(D[3-1],2)))) << 14
    NewCRC  += xor(int(C[8-1],2),xor(int(D[8-1],2),xor(int(C[4-1],2),int(D[4-1],2)))) << 15

    return NewCRC

def genSeqRegisterBlock(registers):
    SPI_startaddress = 0x9000
    CRC_startaddress = 0x9000
    CRC_stopaddress = 0x93FE
    index_max = CRC_stopaddress - CRC_startaddress + 1 #'we need to add also on the SPI Data a CRC therefore increase by 1'
    index  = 0
    SPI_length = CRC_stopaddress - SPI_startaddress + 1 #'we need to add also on the SPI Data a CRC therefore increase by 1'
    UC_length = CRC_startaddress - SPI_startaddress 
    crc = 0xFFFF
    
    'init byte Array with all zeros'
    CRC_data_byte = [0x0000]*index_max*2
    'init Array with all zeros'
    CRC_data = [0x0000]*index_max
    'init SPI Array with all zeros'
    SPI_data = [0x0000]*SPI_length
    
    sorted_out = []
    
    'fill array with values of SFR array'
    for reg in registers:
        if (reg[0] - CRC_startaddress) >= 0 and (reg[0] - CRC_startaddress) < index_max: 
            CRC_data [reg[0]- CRC_startaddress] = reg [1]
        else:
            sorted_out.append(reg)

    #for reg in sorted_out:
        #print ("Sort out : 0x%0.4x   0x%0.4x" % (reg[0], reg[1]))
            
    'change to bytes for CRC calculation'
    for data in CRC_data:
        CRC_data_byte [index] = data & 0x00FF
        CRC_data_byte [index+1] = (data & 0xFF00) >> 8
        index += 2
    
    'generate CRC'
    for data in CRC_data_byte:
        crc = baseCrc(crc,data)
    
    #write use case init into SPI data array
    for reg in registers:
        if (reg[0] - SPI_startaddress) >= 0 and (reg[0] - SPI_startaddress) < SPI_length: 
            SPI_data [reg[0]- SPI_startaddress] = reg [1]
    
    'add CRC to SPI_data' 
    SPI_data.append(crc)
    
    seqRegBlock = SequentialRegisterBlock(SPI_data, SPI_startaddress)
    
    return seqRegBlock

    
    
    
    