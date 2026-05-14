#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

#include "sync.h"
#include <queue>
#include <memory>
#include <cstring>

// Taille maximale d'un message
constexpr size_t MAX_MESSAGE_SIZE = 256;

struct Message
{
    int senderId;
    int recipientId;
    uint32_t messageType;
    uint8_t data[MAX_MESSAGE_SIZE];
    size_t dataSize; // Taille réelle des données

    Message() : senderId(-1), recipientId(-1), messageType(0), dataSize(0)
    {
        std::memset(data, 0, MAX_MESSAGE_SIZE);
    }
};

// ============ MESSAGE QUEUE ============
class MessageQueue
{
public:
    MessageQueue(size_t maxMessages = 10) : maxSize(maxMessages) {}
    ~MessageQueue() = default;

    // Envoyer un message
    bool send(const Message &msg)
    {
        LockGuard lock(queueMutex);

        if (queue.size() >= maxSize)
        {
            return false; // Queue pleine
        }

        queue.push(msg);
        notEmpty.signal();
        return true;
    }

    // Recevoir un message (bloquant)
    bool receive(Message &msg)
    {
        notEmpty.wait();

        LockGuard lock(queueMutex);

        if (queue.empty())
        {
            return false;
        }

        msg = queue.front();
        queue.pop();
        return true;
    }

    // Recevoir sans bloquer
    bool tryReceive(Message &msg)
    {
        LockGuard lock(queueMutex);

        if (queue.empty())
        {
            return false;
        }

        msg = queue.front();
        queue.pop();
        return true;
    }

    // Vérifier si la queue a des messages
    bool hasMessages() const
    {
        return !queue.empty();
    }

    // Nombre de messages en attente
    size_t getMessageCount() const
    {
        return queue.size();
    }

    // Vider la queue
    void clear()
    {
        LockGuard lock(queueMutex);
        while (!queue.empty())
        {
            queue.pop();
        }
    }

private:
    std::queue<Message> queue;
    size_t maxSize;
    Mutex queueMutex;
    Semaphore notEmpty{0}; // Initialisé à 0
};

#endif // MESSAGE_QUEUE_H
