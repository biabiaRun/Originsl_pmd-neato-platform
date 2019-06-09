/****************************************************************************\
* Copyright (C) 2016 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <assert.h>

#include <algorithm>
#include <memory>
#include <limits>

#include <linux/i2c-dev.h>
#include <linux/i2c.h>

#include <I2cAccessImpl.hpp>

#define I2CACCESSIMPL_DEBUG 0
#define PRINT_MAX_DEBUG_BYTES     16

using namespace platform;
using namespace royale::pal;

typedef std::vector<uint8_t> charVector;
typedef std::vector<struct i2c_msg> msgVector;

namespace {

const size_t kMaxMsgSize = 8192; /* magic number from kernel 4.14 */
const size_t kMaxMsgs = I2C_RDWR_IOCTL_MAX_MSGS;

size_t getAddrSize(I2cAddressMode mode)
{
    switch(mode) {
    case I2cAddressMode::I2C_NO_ADDRESS:
        return 0;
    case I2cAddressMode::I2C_8BIT:
        return 1;
    case I2cAddressMode::I2C_16BIT:
        return 2;
    default:
        return 3; /* error value */
    }

}

inline void serializeAddress(size_t addrSize, uint16_t regAddr,
        charVector::iterator dst)
{
    if(!addrSize)
        return;
    else if(addrSize == 2)
        regAddr = htons(regAddr);
    else
        assert(false); /* invalid addr size */

    auto addrPtr = reinterpret_cast<charVector::value_type*>(&regAddr);
    std::copy(addrPtr, addrPtr + addrSize, dst);
}

#if I2CACCESSIMPL_DEBUG
void printBytes(uint8_t* buf, size_t len, bool newline)
{
    for(int i=0; i<len; ++i) {
        if(i > PRINT_MAX_DEBUG_BYTES) {
            printf("...");
            break;
        }
        printf("%02x ", buf[i]);
    }
    if(newline)
        printf("\n");
}
#endif

int doRdwr(int fd, msgVector &msgs)
{
#if I2CACCESSIMPL_DEBUG
    for(size_t i=0; i<msgs.size(); ++i) {
        if(!(msgs[i].flags & I2C_M_RD)) {
            printf("I2C Write %d; Addr: 0x%04x Flags: 0x%04x Len: 0x%04x Data: ",
                    (int)i, (unsigned)msgs[i].addr, (unsigned)msgs[i].flags,
                    (unsigned)msgs[i].len);
            printBytes(msgs[i].buf, msgs[i].len, true);
        }
    }
#endif

    struct i2c_rdwr_ioctl_data ctlData = {
            .msgs = msgs.data(),
            .nmsgs = static_cast<__u32>(msgs.size())
    };

    int ret = ioctl(fd, I2C_RDWR, &ctlData);

#if I2CACCESSIMPL_DEBUG
    if(ret < 0) {
        fprintf(stderr, "I2C transfer fail ret=%d errno=%d\n",
                ret, errno);
    } else {
        for(size_t i=0; i<msgs.size(); ++i) {
            if(msgs[i].flags & I2C_M_RD) {
                if(msgs[i].flags & I2C_M_RD) {
                    printf("I2C Read %d; Addr: 0x%04x Flags: 0x%04x Len: 0x%04x Data: ",
                            (int)i, (unsigned)msgs[i].addr, (unsigned)msgs[i].flags,
                            (unsigned)msgs[i].len);
                    printBytes(msgs[i].buf, msgs[i].len, true);
                }
            }
        }
        printf("I2C transfer ok.\n");
    }
#endif
    return ret;
}

} /* namespace anon */

namespace platform {

I2cAccessImpl::I2cAccessImpl(const char* devNode)
{
    m_fd = open(devNode, O_RDWR);
    if(m_fd < 0) {
        fprintf(stderr, "Failed to open I2C device (%s) errno: %d\n",
                devNode, errno);
        return;
    }
}

I2cAccessImpl::~I2cAccessImpl()
{
    if(m_fd > 0)
        close(m_fd);
}

void I2cAccessImpl::readI2c (uint8_t devAddr,
                             I2cAddressMode addrMode,
                             uint16_t regAddr,
                             std::vector<uint8_t> &buffer)
{
    if(m_fd < 0 || buffer.empty())
        return;

    int ret;
    size_t curPos = 0;
    size_t left = buffer.size();
    size_t addrSize;
    if((addrSize = getAddrSize(addrMode)) > 2)
        /* sanity error */
        return;

    size_t noPayloadMsgs = (left + kMaxMsgSize - 1) / kMaxMsgSize;

    charVector addrBuf(addrSize);
    msgVector msgs(std::min(kMaxMsgs, noPayloadMsgs + (addrSize ? 1 : 0)));
    while(left)
    {
        size_t curMsgsPos = 0;
        while(left && curMsgsPos < kMaxMsgs) {
            /* First do I2C write with internal device address. */
            if(addrSize && curMsgsPos == 0) {
                serializeAddress(addrSize,
                        regAddr + curPos,
                        addrBuf.begin());

                struct i2c_msg msg = {
                        .addr = devAddr,
                        .flags = 0,
                        .len = static_cast<uint16_t>(addrSize),
                        .buf = addrBuf.data(),
                };
                msgs[curMsgsPos++] = msg;
            }

            /* Compose I2C read message for reading payload into. */
            size_t consumed = std::min(left, kMaxMsgSize);
            struct i2c_msg msg = {
                    .addr = devAddr,
                    .flags = I2C_M_RD,
                    .len = static_cast<__u16>(consumed),
                    .buf = buffer.data() + curPos,
            };
            msgs[curMsgsPos++] = msg;

            curPos += consumed;
            left -= consumed;
        }

        ret = doRdwr(m_fd, msgs);
        if(ret < 0)
            break;
    }
}

void I2cAccessImpl::writeI2c (uint8_t devAddr,
                              I2cAddressMode addrMode,
                              uint16_t regAddr,
                              const std::vector<uint8_t> &buffer)
{
    if(m_fd < 0 || buffer.empty())
        return;

    int ret;
    size_t curPos = 0;
    size_t left = buffer.size();
    size_t addrSize;
    if((addrSize = getAddrSize(addrMode)) > 2)
        /* sanity error */
        return;

    size_t msgSize = std::min(left + addrSize, kMaxMsgSize);
    size_t noMsgs = std::min(kMaxMsgs,
                (left + msgSize - 1) / msgSize);

    charVector tmpBuf(noMsgs * msgSize);
    msgVector msgs(noMsgs);
    while(left)
    {
        size_t curTmpPos = 0;
        size_t curMsgsPos = 0;
        while(left && curMsgsPos < kMaxMsgs) {
            size_t consumed = std::min(kMaxMsgSize - addrSize, left);
            __u8 *thisMsg = tmpBuf.data() + curTmpPos;

            /* copy register address into tmpBuf */
            if(addrSize) {
                serializeAddress(addrSize,
                        regAddr + static_cast<uint16_t>(curPos),
                        tmpBuf.begin() + curTmpPos);
                curTmpPos += addrSize;
            }

            /* copy payload into tmpBuf */
            std::copy(buffer.begin() + curPos,
                    buffer.begin() + curPos + consumed,
                    tmpBuf.begin() + curTmpPos);


            struct i2c_msg msg = {
                    .addr = devAddr,
                    .flags = 0,
                    .len = static_cast<uint16_t>(consumed + addrSize),
                    .buf = thisMsg,
            };
            msgs[curMsgsPos++] = msg;

            curPos += consumed;
            curTmpPos += consumed;
            left -= consumed;
        }

        ret = doRdwr(m_fd, msgs);
        if(ret)
            break;
    }
}

void I2cAccessImpl::setBusSpeed (uint32_t bps)
{
    /* not available at runtime */
    (void)(bps);
}

std::size_t I2cAccessImpl::maximumDataSize ()
{
    /* This implementation supports any arbitrary size,
     * not caring about possible internal device address overflow. */
    return std::numeric_limits<size_t>::max();
}

} /* namespace platform */
