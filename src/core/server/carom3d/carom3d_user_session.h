//
// Created by CGR on 13/05/2020.
//

#ifndef __OPEN_CAROM3D_SERVER_CAROM3D_USER_SESSION_H__
#define __OPEN_CAROM3D_SERVER_CAROM3D_USER_SESSION_H__

#include <vector>

#include <ctime>
#include <core/server/server_context.h>
#include <core/server/client_session.h>
#include <nettools/ntConnection.h>
#include "crypto.h"
#include "action.h"

namespace core {

    class Server;

    class Carom3DUserSession : public ClientSession {

    protected:
        CryptoContext m_inDataCryptoCtx;
        CryptoContext m_outDataCryptoCtx;
        std::vector<ActionData> m_pendingActions;
        std::vector<u8> m_unparsedData;
        time_t m_lastMessageTime;

    public:
        Carom3DUserSession(nettools::ntConnection& ntConnection, Server& server)
            : ClientSession(ntConnection, server) {
            update();
        }

        CryptoContext& inDataCryptoCtx() { return m_inDataCryptoCtx; }
        CryptoContext& outDataCryptoCtx() { return m_outDataCryptoCtx; }

        const std::vector<ActionData>& pendingActions() const { return m_pendingActions; }

        void clearPendingActions() {
            if(!m_pendingActions.empty())
                m_pendingActions.clear();
        }

        void appendActions(const std::vector<ActionData> &actions) {
            m_pendingActions.insert(m_pendingActions.end(), actions.begin(), actions.end());
        }

        nettools::NT_ERROR sendAction(const ActionData& action) {
            unsigned int totalLen = 8 + action.data().size();
            
            // Check packet size
            if (totalLen > MAX_PACKET_SIZE) {
                printf("[ERROR] Packet too large - ActionID: 0x%x (%d), Size: %d, Limit: %d\n", 
                       action.id(), action.id(), totalLen, MAX_PACKET_SIZE);
                return nettools::NTERR_OTHER;
            }
            
            unsigned char data[MAX_PACKET_SIZE];
            *(int*)&data[0] = action.id();
            *(unsigned int*)&data[4] = action.data().size();
            unsigned int actionDataLen = action.data().size();
            if(actionDataLen > 0) {
                ::memcpy(&data[8], action.data().data(), actionDataLen);
            }
            m_outDataCryptoCtx.crypt(data, totalLen);
            m_outDataCryptoCtx.update();
            
            // Send data and check return value
            nettools::NT_ERROR result = m_ntClient.send(m_ntClient.socket(), data, totalLen);
            if (result != nettools::NTERR_SUCCESS) {
                printf("[ERROR] Send failed - SessionID: %d, ActionID: 0x%x, ErrorCode: %d\n", 
                       m_sessionId, action.id(), result);
                return result;
            }
            
            return nettools::NTERR_SUCCESS;
        }

        void update() {
            m_lastMessageTime = ::time(nullptr);
        }

        time_t lastMessageTime() const {
            return m_lastMessageTime;
        }

    };

}

#endif //__OPEN_CAROM3D_SERVER_USER_SESSION_H__
