#pragma once

#include <cstddef>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

class ClientSession;

class UserManager
{
public:
    bool registerUser(std::string username,
                      const std::shared_ptr<ClientSession>& session);
    bool isUsernameAvailable(std::string_view username) const;
    void unregisterUser(std::string_view username,
                        const std::shared_ptr<ClientSession>& session);
    std::vector<std::shared_ptr<ClientSession>> recipientsExcept(
        std::string_view username);
    std::size_t connectedUserCount() const;

private:
    mutable std::mutex users_mutex_;
    std::unordered_map<std::string, std::weak_ptr<ClientSession>> users_;
};
