#ifndef _USER_CUSTS1_DEF_H_
#define _USER_CUSTS1_DEF_H_

#include "attm_db_128.h"

/*
 * Service UUID: 12345678-1234-1234-1234-1234567890ab
 * Little Endian으로 변환
 */
#define DEF_SVC1_UUID_128   {0xab, 0x90, 0x78, 0x56, 0x34, 0x12, \
                             0x34, 0x12, 0x34, 0x12, 0x34, 0x12, \
                             0x78, 0x56, 0x34, 0x12}

/*
 * Characteristic UUID: abcd1234-1234-1234-1234-1234567890ab
 * Little Endian으로 변환
 */
#define DEF_SVC1_SERIAL_UUID_128  {0xab, 0x90, 0x78, 0x56, 0x34, 0x12, \
                                   0x34, 0x12, 0x34, 0x12, 0x34, 0x12, \
                                   0x34, 0x12, 0xcd, 0xab}

#define DEF_SVC1_SERIAL_CHAR_LEN     8
#define DEF_SVC1_SERIAL_USER_DESC    "Device ID"

enum
{
    SVC1_IDX_SVC = 0,

    SVC1_IDX_SERIAL_CHAR,
    SVC1_IDX_SERIAL_VAL,
    SVC1_IDX_SERIAL_USER_DESC,

    CUSTS1_IDX_NB
};

#endif // _USER_CUSTS1_DEF_H_