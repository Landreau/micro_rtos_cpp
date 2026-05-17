#include "scheduler.h"
#include <iostream>
#include <algorithm>

void Scheduler::addTask(std::shared_ptr<Task> task)
{
    if (!task)
        return;

    task->setState(TaskState::READY);
    readyQueue.push(task);
    registerTask(task);

    std::cout << "[SCHEDULER] Tâche " << task->getId()
              << " ajoutée (priorité: " << task->getPriority() << ")" << std::endl;

    if (task->isPeriodic())
    {
        std::cout << "[SCHEDULER]   → Périodique (période: " << task->getPeriod() << " ms)" << std::endl;
    }
}

void Scheduler::registerTask(std::shared_ptr<Task> task)
{
    // Ajouter à la liste de toutes les tâches pour suivi
    auto it = std::find(allTasks.begin(), allTasks.end(), task);
    if (it == allTasks.end())
    {
        allTasks.push_back(task);
    }
}

std::shared_ptr<Task> Scheduler::getNextTask()
{
    if (readyQueue.empty())
    {
        return nullptr;
    }

    auto task = readyQueue.top();
    readyQueue.pop();

    return task;
}

void Scheduler::updateDelayedTasks()
{
    // Vérification des tâches qui sont en délai
    auto it = delayedTasks.begin();
    while (it != delayedTasks.end())
    {
        auto task = *it;

        if (task->hasDelayExpired())
        {
            task->setState(TaskState::READY);
            readyQueue.push(task);
            it = delayedTasks.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void Scheduler::updatePeriodicTasks()
{
    auto now = std::chrono::steady_clock::now();

    for (auto &task : allTasks)
    {
        if (task->isPeriodic() && task->getState() == TaskState::WAITING)
        {
            if (now >= task->getNextWakeup())
            {
                task->setState(TaskState::READY);
                readyQueue.push(task);
            }
        }
    }
}

void Scheduler::runOnce()
{
    updateDelayedTasks();

    auto task = getNextTask();

    if (!task)
    {
        updatePeriodicTasks();

        task = getNextTask();

        if (!task)
        {
            return;
        }
    }

    currentTask = task;
    std::cout << "[SCHEDULER] Exécution de la tâche " << task->getId() << std::endl;

    task->execute();

    if (task->isPeriodic())
    {
        task->updateNextWakeup();
        task->setState(TaskState::WAITING);
    }
    else
    {
        // Fix #6 : état TERMINATED (était incorrectement READY dans task.cpp).
        task->setState(TaskState::TERMINATED);
    }

    currentTask = nullptr;
}

bool Scheduler::hasReadyTasks() const
{
    if (!readyQueue.empty() || !delayedTasks.empty())
    {
        return true;
    }

    auto now = std::chrono::steady_clock::now();
    for (const auto &task : allTasks)
    {
        if (task->isPeriodic() && task->getState() == TaskState::WAITING)
        {
            if (now >= task->getNextWakeup())
            {
                return true;
            }
            return true;
        }
    }

    return false;
}

void Scheduler::printStatus() const
{
    std::cout << "\n=== État du Scheduler ===" << std::endl;
    std::cout << "Tâches prêtes: " << readyQueue.size() << std::endl;
    if (currentTask)
    {
        std::cout << "Tâche en cours: " << currentTask->getId() << std::endl;
    }
    else
    {
        std::cout << "Aucune tâche en cours" << std::endl;
    }
    std::cout << "========================\n"
              << std::endl;
}
