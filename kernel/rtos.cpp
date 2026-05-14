#include "rtos.h"
#include <iostream>
#include <thread>

Kernel::Kernel() : taskIdCounter(0), taskCount(0), running(false), globalSemaphore(1)
{
    std::cout << "[KERNEL] Initialisation du Kernel RTOS" << std::endl;
}

int Kernel::createTask(Task::TaskFunc func, uint32_t priority)
{
    int taskId = taskIdCounter++;
    auto task = std::make_shared<Task>(taskId, func, priority);

    tasks[taskId] = task;
    scheduler.addTask(task);
    taskCount++;

    std::cout << "[KERNEL] Tâche créée avec ID: " << taskId
              << " (priorité: " << priority << ")" << std::endl;

    return taskId;
}

int Kernel::createPeriodicTask(Task::TaskFunc func, uint32_t priority, uint32_t periodMs)
{
    int taskId = taskIdCounter++;
    auto task = std::make_shared<Task>(taskId, func, priority);

    // Configurer la période
    task->setPeriod(periodMs);
    task->updateNextWakeup();

    tasks[taskId] = task;
    scheduler.addTask(task);
    taskCount++;

    std::cout << "[KERNEL] Tâche périodique créée avec ID: " << taskId
              << " (priorité: " << priority << ", période: " << periodMs << " ms)" << std::endl;

    return taskId;
}

void Kernel::delay(uint32_t milliseconds)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

void Kernel::run(int maxIterations)
{
    if (running)
    {
        std::cout << "[KERNEL] Le kernel est déjà en cours d'exécution" << std::endl;
        return;
    }

    running = true;
    std::cout << "[KERNEL] Démarrage du scheduler..." << std::endl;

    int iterations = 0;

    while (running && scheduler.hasReadyTasks())
    {
        if (maxIterations > 0 && iterations >= maxIterations)
        {
            break;
        }

        scheduler.runOnce();
        iterations++;

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    std::cout << "[KERNEL] Scheduler arrêté après " << iterations << " itérations" << std::endl;
    running = false;
}

void Kernel::stop()
{
    running = false;
    std::cout << "[KERNEL] Arrêt du kernel demandé" << std::endl;
}

void Kernel::printStatus() const
{
    std::cout << "\n========== État du KERNEL ==========" << std::endl;
    std::cout << "Statut: " << (running ? "EN COURS" : "ARRÊTÉ") << std::endl;
    std::cout << "Nombre de tâches: " << taskCount << std::endl;
    scheduler.printStatus();
    std::cout << "===================================\n"
              << std::endl;
}
