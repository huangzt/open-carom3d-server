//
// Created by CGR on 14/05/2020.
//

#include <core/server/client_session.h>
#include "login_action.h"
#include <business/service/account_service.h>
#include <business/entity/player.h>

#include <mysql/jdbc.h> // MySQL Connector/C++ 的头文件

#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <algorithm>
#include <iostream>

namespace business { namespace management {

#pragma pack(push, 1)
    struct MSLoginResult {
        unsigned long result;
        wchar_t playerName[21];
        unsigned long totalPoints;
        unsigned long unk;
        unsigned long accountLevel;
        // Probably cue and jewels stats
        unsigned long _unk;
        unsigned long _unk1;
        unsigned char _unk2[6];
        unsigned long _unk3;
        unsigned char _unk4;
        unsigned char _unk5;
        unsigned char _unk6[8];
        unsigned short _unk7;
        //
        wchar_t ip[16];
        unsigned long unk2;
        unsigned long unk3;
        unsigned long unk4;
        unsigned long unk5;
        unsigned long unk6;
        unsigned long unk7;
    };
#pragma pack(pop)

    void sendMSLoginResult(u32 result, User& user) {
        Player* player = user.player();

        MSLoginResult loginResult = {
            result,
            L"",
            (unsigned long)player->points(), 0, (unsigned long)0x3C,
            0, 0, {0, 0, 0, 0, 0, 0}, 0, 0, 0,
            {0, 0, 0, 0, 0, 0, 0, 0},
            0,
            L"127.0.0.1",
            0, 0, 0, 0, 0, 0
        };

        ::wcsncpy(loginResult.playerName, player->name(), 20);
        loginResult.playerName[20] = L'\x00';

        ActionData actionData(0x05, (unsigned char*)&loginResult, sizeof(loginResult));

        user.sendAction(actionData);
    }

    void sendMSLoginResult(u32 result, User& user, const wchar_t* accountName) {
        MSLoginResult loginResult = {
            result,
            L"",
            (unsigned long)0, 0, (unsigned long)0x3C,
            0, 0, {0, 0, 0, 0, 0, 0}, 0, 0, 0,
            {0, 0, 0, 0, 0, 0, 0, 0},
            0,
            L"127.0.0.1",
            0, 0, 0, 0, 0, 0
        };

        ::wcsncpy(loginResult.playerName, accountName, 20);
        loginResult.playerName[20] = L'\x00';

        ActionData actionData(0x05, (unsigned char*)&loginResult, sizeof(loginResult));
        user.sendAction(actionData);
    }

    bool LoginAction::validate(const ActionData &action) {
        int size = action.data().size();
        int _size = sizeof(LoginData);
        return size == _size;
    }

    // 定义一个类来存储账号信息
    class AccountInfo {
    public:
        std::wstring account;
        std::wstring password;
        u8 maxpower;
        u8 minpower;
        u32 cueId;

        // 默认构造函数
        AccountInfo()
            : account(L""), password(L""), maxpower(150), minpower(1), cueId(3000) {}

        // 带参数的构造函数
        AccountInfo(const std::wstring& name, const std::wstring& pwd, u8 max, u8 min, u32 cue)
            : account(name), password(pwd), maxpower(max), minpower(min), cueId(cue) {}
    };

    // 将 std::string 转换为 std::wstring
    std::wstring s2ws(const std::string& str) {
        std::wstring wstr(str.begin(), str.end());
        return wstr;
    }

    // 从数据库中验证账号信息
    void getAccountInDB(const std::wstring& accountId, AccountInfo& accountInfo) {
        try {
            std::string configFilePath = "resources/mysql.txt";

            // 打开配置文件
            std::ifstream file(configFilePath);
            if (!file.is_open()) {
                std::cerr << "Error: Could not open config file " << configFilePath << std::endl;
                return;
            }

            std::string line;
            if (!std::getline(file, line)) {
                std::cerr << "Error: Could not read from config file " << configFilePath << std::endl;
                return;
            }
            file.close();

            // 解析连接信息
            std::istringstream iss(line);
            std::string url, user, password;
            if (!std::getline(iss, url, ',') || !std::getline(iss, user, ',') || !std::getline(iss, password)) {
                std::cerr << "Error: Invalid config file format" << std::endl;
                return;
            }

            // 创建 MySQL 驱动实例
            sql::Driver* driver = get_driver_instance();

            // 连接到 MySQL 数据库
            std::unique_ptr<sql::Connection> con(driver->connect(url, user, password));
            // std::unique_ptr<sql::Connection> con(driver->connect("tcp://192.168.14.12:3306", "root", "password"));
            con->setSchema("carom3d");

            // 创建 SQL 预处理语句
            std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement(
                "SELECT account, password, maxpower, minpower, cueId FROM sys_account WHERE account = ?"));

            // 设置 SQL 参数（账号）
            pstmt->setString(1, std::string(accountId.begin(), accountId.end()));

            // 执行查询
            std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

            // 检查是否找到匹配的账号
            if (res->next()) {
                // 获取数据库中的账号信息
                accountInfo.account = s2ws(res->getString("account"));
                accountInfo.password = s2ws(res->getString("password"));
                accountInfo.maxpower = res->getUInt("maxpower");
                accountInfo.minpower = res->getUInt("minpower");
                accountInfo.cueId = res->getUInt("cueId");
            }
            else {
                // 账号不存在
                std::cerr << "Error: Account does not exist" << std::endl;
            }
        }
        catch (sql::SQLException& e) {
            std::cerr << "SQLException: " << e.what();
            std::cerr << " (MySQL error code: " << e.getErrorCode();
            std::cerr << ", SQLState: " << e.getSQLState() << ")" << std::endl;
        }
    }

    static std::map<std::wstring, AccountInfo> loadAccountsFromFile() {
        std::map<std::wstring, AccountInfo> accounts;
        std::wifstream file(L"resources/accounts.txt");
        std::wstring line;

        // 跳过第一行（注释行）
        std::getline(file, line);

        while (std::getline(file, line)) {
            std::wistringstream iss(line);
            std::wstring account, password, maxStr, minStr, cueStr;

            // 通过逗号分隔: 账户名、密码、最大力度、最小力度和球杆编码
            if (std::getline(iss, account, L',') &&
                std::getline(iss, password, L',') &&
                std::getline(iss, maxStr, L',') &&
                std::getline(iss, minStr, L',') &&
                std::getline(iss, cueStr)) {

                // 初始化默认值
                u8 maxForce = 150;  // 可以根据需求设置默认最大力度
                u8 minForce = 1;  // 可以根据需求设置默认最小力度
                u32 cueCode = 3000; // 可以根据需求设置默认球杆编码

                // 判断长度，若有内容则转换为整数
                if (!maxStr.empty()) {
                    maxForce = static_cast<u8>(std::stoi(maxStr));
                }
                if (!minStr.empty()) {
                    minForce = static_cast<u8>(std::stoi(minStr));
                }
                if (!cueStr.empty()) {
                    cueCode = static_cast<u32>(std::stoi(cueStr));
                }

                // 创建 AccountInfo 对象并添加到 map 中
                accounts[account] = AccountInfo(account, password, maxForce, minForce, cueCode);
            }
        }
        return accounts;
    }

    // 主逻辑：验证账号信息并处理
    void LoginAction::execute(const ActionData &action, User &user, const LoginData *data) {
        printf("\n\nPlayer logged in: %ws, %ws\n\n", data->username, data->password);

        // 转换账号为小写
        std::wstring accountId = data->username;
        std::transform(accountId.begin(), accountId.end(), accountId.begin(),
            [](unsigned char c) { return std::tolower(c); });

        // 存储查询结果的 AccountInfo 对象
        AccountInfo accountInfo;

        // 从数据库中验证账号信息
        getAccountInDB(accountId, accountInfo);

        // 检查是否找到了账户
        if (accountInfo.account.empty()) {
            // 账号不存在的情况
            printf("Error: Account does not exist\n");
            // 账号不存在，返回相应结果
            sendMSLoginResult(0x02, user, data->username);
            return;
        }
        else if (accountInfo.password != data->password) {
            // 账号存在，但密码不正确
            printf("Error: Incorrect password\n");
            // 密码不正确，返回相应结果
            sendMSLoginResult(0x03, user, data->username);
            return;
        }
        else {
            // 账号存在且密码正确
            // 在这里，您可以进行后续的处理，例如创建并返回代表该账户的对象等
            printf("Login successful!\n");
        }

        //// 从文件中加载账户信息
        //auto accounts = loadAccountsFromFile();

        //// 在加载的账户中搜索
        //auto it = accounts.find(accountId);

        //// 检查是否找到了账户
        //if (it == accounts.end()) {
        //    // 账号不存在的情况
        //    printf("Error: Account does not exist\n");
        //    // 账号不存在，返回相应结果
        //    sendMSLoginResult(0x02, user, data->username);
        //    return;
        //}
        //else if (it->second.password != data->password) {
        //    // 账号存在，但密码不正确
        //    printf("Error: Incorrect password\n");
        //    // 密码不正确，返回相应结果
        //    sendMSLoginResult(0x03, user, data->username);
        //    return;
        //}
        //else {
        //    // 账号存在且密码正确
        //    // 在这里，您可以进行后续的处理，例如创建并返回代表该账户的对象等
        //    printf("Login successful!\n");
        //    // 进行其他逻辑，例如：
        //    // AccountInfo accountInfo = it->second;
        //    // ...进一步处理 accountInfo
        //}

        Account* account = AccountService::getInstance().logAccount(data->username, data->password);
        Player* player;
        if(nullptr == account) {
            account = AccountService::getInstance().createAccount(data->username, data->password);
            player = AccountService::getInstance().createPlayerFromAccount(*account);
        }
        else {
            player = AccountService::getInstance().getPlayer(*account);
        }

        player->setMinPower(accountInfo.minpower);
        player->setMaxPower(accountInfo.maxpower);
        player->setCueId(accountInfo.cueId);
        player->setLevel(55);

        account->setPlayer(player);

        user.setAccount(account);
        user.setPlayer(player);

        //0x02 账号不存在
        //0x03 密码不正确
        sendMSLoginResult(0x00, user);
    }

}}
