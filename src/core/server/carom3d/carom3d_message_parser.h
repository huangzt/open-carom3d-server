//
// Created by CGR on 13/05/2020.
//

#ifndef __OPEN_CAROM3D_SERVER_MESSAGE_PARSER_H__
#define __OPEN_CAROM3D_SERVER_MESSAGE_PARSER_H__

#include <vector>
#include "action.h"
#include "crypto.h"

namespace core {

    struct ParsedDataResultInfo {
        std::vector<ActionData> parsedActions;
        unsigned int parsedTotalLen;
        bool parseSuccess;          // 解析是否成功
        unsigned int errorActionId; // 发生错误的 ActionID
        unsigned int expectedLen;   // 期望的数据长度
        unsigned int actualLen;     // 实际的数据长度
        
        ParsedDataResultInfo() 
            : parsedTotalLen(0)
            , parseSuccess(true)
            , errorActionId(0)
            , expectedLen(0)
            , actualLen(0) {
        }
    };

    class Carom3DMessageParser {
    public:
        ParsedDataResultInfo parseMessageData(CryptoContext &cryptoCtx, unsigned char *data, unsigned int dataLen);
    };

}

#endif //__OPEN_CAROM3D_SERVER_MESSAGE_PARSER_H__
