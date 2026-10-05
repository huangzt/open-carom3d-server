//
// Created by CGR on 14/05/2020.
//

#include "carom3d_message_parser.h"

namespace core {

    ParsedDataResultInfo Carom3DMessageParser::parseMessageData(CryptoContext &cryptoCtx, unsigned char *data, unsigned int dataLen) {
        ParsedDataResultInfo info;
        int i = 0;
        unsigned int parsedLen = 0;
        unsigned int unparsedLen = dataLen;
        while (unparsedLen >= 8) {
            unsigned char *d = &data[parsedLen];
            int actionId = *(int *) &d[0];
            unsigned int actionDataLen = *(unsigned int *) &d[4];
            cryptoCtx.crypt((unsigned char *) &actionId, 4);
            cryptoCtx.crypt((unsigned char *) &actionDataLen, 4);
            
            // Check if data length is sufficient
            if ((unparsedLen - 8) < actionDataLen) {
                // Parse failed: incomplete data
                info.parseSuccess = false;
                info.errorActionId = actionId;
                info.expectedLen = actionDataLen + 8;
                info.actualLen = unparsedLen;
                
                printf("[WARN] Message parse failed - ActionID: 0x%x (%d), Expected: %d bytes, Actual: %d bytes\n", 
                       actionId, actionId, info.expectedLen, info.actualLen);
                return info;
            }
            
            d += 8;
            cryptoCtx.crypt(d, actionDataLen);
            info.parsedActions.push_back(ActionData(actionId, d, actionDataLen));
            parsedLen += 8 + actionDataLen;
            unparsedLen = dataLen - parsedLen;
            cryptoCtx.update();
        }
        info.parsedTotalLen = parsedLen;
        return info;
    }

}
