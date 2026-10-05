//
// Created by CGR on 09/06/2020.
//

#include <vector>
#include <ctime>
#include "carom3d_protocol.h"
#include "carom3d_user_session.h"
#include "action.h"
#include <business/entity/user.h>
#include <business/entity/player.h>


namespace core {

    Carom3DProtocol::Carom3DProtocol() {
        m_userActionMap = nullptr;
    }

    ClientSession* Carom3DProtocol::createSession(nettools::ntConnection& ntClient, Server& server) {
        return new Carom3DUserSession(ntClient, server);
    }

    void Carom3DProtocol::onMessageReceived(ClientSession& session) {
        Carom3DUserSession& user = (Carom3DUserSession&)session;
        user.update();

        u32 dataLen = user.pendingReadDataSize();
        ParsedDataResultInfo parsedData = m_messageParser.parseMessageData(user.inDataCryptoCtx(), user.pendingReadData(), user.pendingReadDataSize());

        // Check if parsing was successful
        if (!parsedData.parseSuccess) {
            printf("[ERROR] MessageReceived parse failed - SessionID: %d, ActionID: 0x%x, Expected: %d, Actual: %d\n", 
                   user.sessionId(), parsedData.errorActionId, 
                   parsedData.expectedLen, parsedData.actualLen);
            
            // Clear all pending read data to prevent error data accumulation
            // Note: This will discard incomplete packets, but prevents memory leaks
            user.discardReadPendingData(user.pendingReadDataSize());
            
            // TODO: Consider marking connection for closure, or adding error counter
            // Frequent parse failures may indicate malicious client or network issues
            return;
        }

        unsigned int parsedLen = parsedData.parsedTotalLen;
        user.discardReadPendingData(parsedLen);

        user.appendActions(parsedData.parsedActions);
        processUserActions(user);
    }

    void Carom3DProtocol::onMessageSent(ClientSession& session) {
        Carom3DUserSession& user = (Carom3DUserSession&)session;

        u32 dataLen = user.pendingReadDataSize();
        ParsedDataResultInfo parsedData = m_messageParser.parseMessageData(user.inDataCryptoCtx(), user.pendingReadData(), user.pendingReadDataSize());

        // Check if parsing was successful
        if (!parsedData.parseSuccess) {
            printf("[ERROR] MessageSent parse failed - SessionID: %d, ActionID: 0x%x, Expected: %d, Actual: %d\n", 
                   user.sessionId(), parsedData.errorActionId, 
                   parsedData.expectedLen, parsedData.actualLen);
            
            // Clear all pending read data to prevent error data accumulation
            // Note: This will discard incomplete packets, but prevents memory leaks
            user.discardReadPendingData(user.pendingReadDataSize());
            
            // TODO: Consider marking connection for closure, or adding error counter
            // Frequent parse failures may indicate malicious client or network issues
            return;
        }

        unsigned int parsedLen = parsedData.parsedTotalLen;
        user.discardReadPendingData(parsedLen);

        user.appendActions(parsedData.parsedActions);
        processUserActions(user);
    }

    void Carom3DProtocol::closeSession(ClientSession& session) {
        delete &session;
    }

    void Carom3DProtocol::setUserActionMap(std::map<int, Action*>* actionMap) {
        m_userActionMap = actionMap;
    }

    void Carom3DProtocol::processUserActions(Carom3DUserSession& user) {
        if(nullptr == m_userActionMap)
            return;

        const std::vector<ActionData>& actions = user.pendingActions();
        if(actions.empty())
            return;

        for(auto actionData : actions) {
            onUserAction(user, actionData);
        }
        user.clearPendingActions();
    }

    void Carom3DProtocol::onUserAction(Carom3DUserSession& session, const ActionData& actionData) {
        Action* action = (*m_userActionMap)[actionData.id()];
        if(action != nullptr) {
            if(action->validate(actionData)) {
                action->execute(actionData, session);
                // Filter some messages that affect debugging
                // if (actionData.id() == 105 || actionData.id() == 107)
                // {
                //     return;
                // }
                // onHandledUserAction(session, actionData);
            }
        }
        // else {
        //     onUnhandledUserAction(session, actionData);
        // }
    }

    void Carom3DProtocol::onUnhandledUserAction(Carom3DUserSession& session, const ActionData& actionData) {
        //TODO: Invalid Action
        int a = actionData.id();
        printf("Unhandled action: %x - %d - %zu\n", a, a, actionData.data().size());
    }
    
    void Carom3DProtocol::onHandledUserAction(Carom3DUserSession& session, const ActionData& actionData) {
        business::User& user = (business::User&)session;
        if (user.player())
        {
            printf("onHandledUserAction: %S - %x - %d - %zu\n",
                user.player()->name(),
                actionData.id(),
                actionData.id(),
                actionData.data().size());

            printf("xData: \n");
			int i = 0;
            for (u8 byte : actionData.data()) {
                printf("%x ", byte);
            }
            /*printf("\n");
            for (u8 byte : actionData.data()) {
                printf("%d:%x,", i, byte);
                i++;
            }*/
            printf("\n");
            printf("cData: \n");
            for (u8 byte : actionData.data()) {
                printf("%c ", (byte >= 0x20 && byte <= 0x7F) ? byte : ' ');
            }
            /*printf("\n");
            i = 0;
            for (u8 byte : actionData.data()) {
                printf("%d:%c,", i, (byte >= 0x20 && byte <= 0x7F) ? byte : ' ');
                i++);
            }*/
            printf("\n\n");
        }
    }

}