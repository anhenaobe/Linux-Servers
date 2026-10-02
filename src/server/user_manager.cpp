#include "user_manager.hpp"

#include "client_session.hpp"
#include "protocol.hpp"

bool UserManager::registerUser(
    std::string username, const std::shared_ptr<ClientSession>& session)
{
    if (!protocol::is_valid_username(username) || !session) {
        return false;
    }

    std::lock_guard<std::mutex> lock(users_mutex_);
    const auto existing = users_.find(username);
    if (existing != users_.end()) {
        const auto previous = existing->second.lock();
        if (previous && previous->isConnected()) {
            return false;
        }
        users_.erase(existing);
    }
    const auto [position, inserted] = users_.emplace(std::move(username), session);
    (void)position;
    return inserted;
}

bool UserManager::isUsernameAvailable(std::string_view username) const
{
    std::lock_guard<std::mutex> lock(users_mutex_);
    const auto found = users_.find(std::string(username));
    if (found == users_.end()) {
        return true;
    }
    const auto session = found->second.lock();
    return !session || !session->isConnected();
}

void UserManager::unregisterUser(
    std::string_view username, const std::shared_ptr<ClientSession>& session)
{
    std::lock_guard<std::mutex> lock(users_mutex_);
    const auto user = users_.find(std::string(username));
    if (user == users_.end()) {
        return;
    }

    const std::shared_ptr<ClientSession> registered_session = user->second.lock();
    if (!registered_session || registered_session == session) {
        users_.erase(user);
    }
}

std::vector<std::shared_ptr<ClientSession>> UserManager::recipientsExcept(
    std::string_view username)
{
    std::vector<std::shared_ptr<ClientSession>> recipients;
    std::lock_guard<std::mutex> lock(users_mutex_);

    for (auto user = users_.begin(); user != users_.end();) {
        std::shared_ptr<ClientSession> session = user->second.lock();
        if (!session || !session->isConnected()) {
            user = users_.erase(user);
            continue;
        }

        if (user->first != username) {
            recipients.push_back(std::move(session));
        }
        ++user;
    }

    return recipients;
}

std::size_t UserManager::connectedUserCount() const
{
    std::lock_guard<std::mutex> lock(users_mutex_);
    return users_.size();
}
