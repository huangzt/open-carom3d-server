//
// Created by CGR on 16/05/2020.
//

#ifndef __OPEN_CAROM3D_SERVER_PLAYER_H__
#define __OPEN_CAROM3D_SERVER_PLAYER_H__

#include <string>
#include <core/util/types.h>
#include "account.h"

namespace business {

    class Account;

    class Player {
        Account &m_account;
        u32 m_id;
        std::wstring m_playerName;
        int m_points;//分数
        int m_level;//等级
        u8 m_maxpower = 150;//最大力度
        u8 m_minpower = 1;//最小力度
        u32 m_cueId = 3000;//球杆编号

    public:
        explicit Player(Account &account);

        Account &account() { return m_account; }
        u32 id() const { return m_id; }
        const wchar_t *name() const { return m_playerName.c_str(); }
        int points() const { return m_points; }
        int level() const { return m_level; }

        void setLevel(int mlevel) {
            m_level = mlevel;
        }

        // Getter for maxpower
        u8 maxpower() const {
            return m_maxpower;
        }

        // Setter for maxpower
        void setMaxPower(u8 maxpower) {
            m_maxpower = maxpower;
        }

        // Getter for minpower
        u8 minpower() const {
            return m_minpower;
        }

        // Setter for minpower
        void setMinPower(u8 minpower) {
            m_minpower = minpower;
        }

        // Getter for cueid
        u32 cueId() const {
            return m_cueId;
        }

        // Setter for cueid
        void setCueId(u32 cueId) {
            m_cueId = cueId;
        }
    };

}

#endif //__OPEN_CAROM3D_SERVER_PLAYER_H__
