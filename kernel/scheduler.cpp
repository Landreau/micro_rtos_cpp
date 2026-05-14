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
        if (task->isPeriodic() && task->getState() == TaskState::READY)
        {
            // Vérification, si c'est l'heure de réexécuter la tâche périodique
            if (now >= task->getNextWakeup())
            {
                // Remettre dans la queue si elle n'y est pas déjà
                readyQueue.push(task);
            }
        }
    }
}

void Scheduler::runOnce()
{
    // Mettre à jour les tâches en délai et périodiques
    updateDelayedTasks();
    updatePeriodicTasks();

    auto task = getNextTask();

    if (!task)
    {
        std::cout << "[SCHEDULER] Aucune tâche à exécuter" << std::endl;
        return;
    }

    currentTask = task;
    std::cout << "[SCHEDULER] Exécution de la tâche " << task->getId() << std::endl;

    task->execute();

    // Gestion de l'état après exécution
    if (task->getState() != TaskState::TERMINATED)
    {
        if (task->isPeriodic())
        {
            // Tâche périodique : remettre en queue
            task->setState(TaskState::READY);
            readyQueue.push(task);
        }
        else
        {
            // Tâche normale : remettre en queue
            task->setState(TaskState::READY);
            readyQueue.push(task);
        }
    }

    currentTask = nullptr;
}

bool Scheduler::hasReadyTasks() const
{
    return !readyQueue.empty() || !delayedTasks.empty();
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
