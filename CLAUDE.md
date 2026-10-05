# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

An open-source reimplementation of the official Carom3D (billiards game) server, written in C++14. The stock Carom3D 5.31 client (`carom.exe`) connects to it after the player points `caromengopening.neoact.com` and `carom3d.gameis.com` to the server's IP in their hosts file.

## Working conventions (from `.cursor/rules`)

- Reply in Simplified Chinese.
- Do not create test files unless the user explicitly agrees. The project has no test suite.
- Always check that commands run from the correct working directory. CMake sources live in `project/`, not the repo root.

## Build & run

The target is **Windows-only** (MSVC / Visual Studio 2022, CMake ≥ 3.15). It links against MySQL Connector C++ 9.0 (or 8.0), with paths hardcoded to `C:/Program Files/MySQL/MySQL Connector C++ 9.0/` in `project/CMakeLists.txt`. You can't build it natively on macOS or Linux.

```bat
.\build_simple.bat      :: recommended; picks Debug/Release, copies DLLs
.\build_ninja.bat       :: Ninja-based alternative
.\clean_build.bat       :: wipe build outputs
.\run_server.bat        :: finds project\out\build\x64-{Release,Debug}\open_carom3d_server.exe and runs it
```

You can also run CMake directly, which is what `.vscode/tasks.json` does:
```bat
cmake -S project -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Debug
```

- **Every new `.cpp` must be added by hand** to the explicit `add_executable` list in `project/CMakeLists.txt`. There is no globbing.
- A post-build step copies `resources/` and the MySQL/OpenSSL DLLs next to the exe. The server reads `resources/...` through **relative paths**, so run it from the exe's directory.
- On Linux deployments the Windows exe runs under Wine in Docker (`docker/docker-compose.yml`, image `scottyhardy/docker-wine`, `network_mode: host`). `fixes/wine-crash-fix.patch` contains null-safe `wprintf` fixes for player-name logging that crashed under Wine.

## Runtime configuration

- `resources/host.txt`: the first line is the IP advertised to clients. It defaults to `127.0.0.1`.
- `loadGameServerConfigs()` in `src/server_starter.cpp` hardcodes the game servers: Novice on 9883 and Free on 9887. Other tiers on 9884–9889 are commented out. The management/login server listens on 9882.
- The HTTP server in `src/core/server/http/`, which serves profile, banner, spotlight and picture pages from `resources/*.html|png`, is compiled in, but its startup in `runHTTPServer` is commented out.
- Accounts and players live in memory, in the static maps in `business/service/account_service.cpp`. `carom3d.sql` and `resources/mysql.txt` exist, and MySQL is linked, but no current source file in `src/` references MySQL.

## Architecture

There are two layers. `src/core` is a generic networking and protocol layer. `src/business` contains the Carom3D game logic. The include roots are `src/` and `src/core/lib`, so includes look like `<business/...>`, `<core/...>`, `<nettools/...>`.

**Threading**: `main` starts one thread that calls `ServerService::startGameServer` for each game server config. Each `GameServer` gets its own thread that polls in a loop every 10 ms. That thread then runs the `ManagementServer` loop. The main thread just reads stdin until `quit`. The services are global singletons (`XService::getInstance()`) that hold global state, and nothing in them is locked across server threads.

**Core layer** (`src/core`):
- `lib/nettools`: raw socket server/connection (`ntServer`, `ntConnection`).
- `server/server_context.*` (`core::Server`): owns the `ntServer` and a `sessionId → ClientSession*` map. It forwards connect/receive/send/disconnect events to a `MessagingProtocol`.
- `server/carom3d/`: the Carom3D wire protocol. Each session has its own `crypto` context. `Carom3DMessageParser` decrypts and splits incoming bytes into `ActionData{id, bytes}`. `Carom3DProtocol` looks up `id` in an `std::map<int, Action*>` and calls `Action::execute`. Unknown ids go to `onUnhandledUserAction`, which hex-dumps them and is useful for reverse-engineering new packets.
- `carom3d/util/action_builder.h`: `ActionBuilder(id).add(...).build()` serializes outgoing packets as raw little-endian struct bytes. Wide strings are written as UTF-16 with a terminating null.

**Business layer** (`src/business`):
- `management_server/`: the login/lobby server. It handles client version, login and the server list, with one class per action in `actions/`.
- `game_server/`: `GameServer : Carom3DServer`. The packet-id → action table is the static `actions` map at the top of `game_server.cpp`, with ids `#define`d there. Inbound handlers live in `recv_actions.h` as `GameServerAction<TPayloadStruct>` subclasses: the payload is a reinterpret-cast of the packet bytes, and the handler normally delegates to a service. Outbound packet templates live in `send_actions.h`. Packet payload structs are in `business/api/messaging.h`, and enums/limits are in `business/api/constants.h`.
- `service/`: the game logic (`UserService`, `RoomService`, `ChannelService`, `AccountService`, `ServerService`).
- `entity/`: `Account` → `Player` (game profile), `User` (a connected session, subclass of `Carom3DUserSession`), and `Channel`/`Room`, which are both `UserSpot`s. A user is always in one spot: logged off, channel, room or in-game. `Match` belongs to a room.
- `util/action_dispatcher` + `destination`: the standard way to send. `ActionDispatcher::prepare().action(data).send(UserDestination(u) | RoomDestination(r) | ChannelDestination(c) | ServerDestination(s))`.

**Adding a new client packet handler**:
1. Define the payload struct in `api/messaging.h`.
2. Add a `GameServerAction<T>` in `recv_actions.h`.
3. Register it with its id in the `actions` map in `game_server.cpp`.
4. Put the logic in a service method.

## Encoding gotcha

Many source files (`game_server.cpp`, `user_service.cpp`, `room_service.cpp`, `carom3d_message_parser.h`, …) and `README_Build.md` use **GBK-encoded Chinese comments with CRLF line endings**. Preserve the existing encoding and line endings when you edit these files. Don't re-save them as UTF-8, which corrupts the comments in MSVC. If text looks garbled, view it with `iconv -f GBK -t UTF-8 <file>`.
