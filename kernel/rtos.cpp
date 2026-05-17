#include <iostream>
#include <thread>
#include <string>
#include "rtos.h"

Kernel::Kernel() : taskIdCounter(0), taskCount(0), expectedTasks(0), running(false), globalSemaphore(1)
{
    std::cout << "[KERNEL] Initialisation du Kernel RTOS" << std::endl;
}

void Kernel::printProgress(const std::string &label) const
{
    if (expectedTasks <= 0)
        return;

    const int barWidth = 20;
    float ratio = static_cast<float>(taskCount) / static_cast<float>(expectedTasks);
    int filled = static_cast<int>(ratio * barWidth);

    std::string bar;
    for (int i = 0; i < barWidth; ++i)
        bar += (i < filled) ? "\u2588" : "\u2591";

    std::cout << "[KERNEL] Enregistrement  ["
              << bar << "]  "
              << taskCount << "/" << expectedTasks
              << "  " << label
              << std::endl;
}

int Kernel::createTask(Task::TaskFunc func, uint32_t priority)
{
    int taskId = taskIdCounter++;
    auto task = std::make_shared<Task>(taskId, func, priority);

    tasks[taskId] = task;
    scheduler.addTask(task);
    taskCount++;

    printProgress("tache " + std::to_string(taskId) + " (priorite " + std::to_string(priority) + ")");

    return taskId;
}

int Kernel::createPeriodicTask(Task::TaskFunc func, uint32_t priority, uint32_t periodMs)
{
    int taskId = taskIdCounter++;
    auto task = std::make_shared<Task>(taskId, func, priority);

    task->setPeriod(periodMs);
    task->updateNextWakeup();

    tasks[taskId] = task;
    scheduler.addTask(task);
    taskCount++;

    printProgress("tache " + std::to_string(taskId) + " (priorite " + std::to_string(priority) + ", periode " + std::to_string(periodMs) + " ms)");

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
        std::cout << "[KERNEL] Le kernel est deja en cours d'execution" << std::endl;
        return;
    }

    // Ligne de séparation finale si une barre de progression était active
    if (expectedTasks > 0)
    {
        std::cout << "[KERNEL] " << taskCount << " tache(s) enregistree(s) - demarrage..." << std::endl;
    }

    running = true;
    std::cout << "[KERNEL] Demarrage du scheduler..." << std::endl;
    std::cout << "[KERNEL] Kernel EN COURS D'EXECUTION" << std::endl;

    int iterations = 0;

    while (running && scheduler.hasReadyTasks())
    {
        if (maxIterations > 0 && iterations >= maxIterations)
            break;

        scheduler.runOnce();
        iterations++;

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    running = false;
    std::cout << "[KERNEL] Scheduler arrete apres " << iterations << " iterations" << std::endl;
    std::cout << "[KERNEL] Kernel ARRETE" << std::endl;
}

void Kernel::stop()
{
    running = false;
    std::cout << "[KERNEL] Arret du kernel demande" << std::endl;
}

void Kernel::printStatus() const
{
    std::cout << "\n====================================" << std::endl;
    std::cout << "  Etat du KERNEL" << std::endl;
    std::cout << "====================================" << std::endl;
    std::cout << "  Statut : " << (running ? "EN COURS" : "ARRETE") << std::endl;
    std::cout << "  Taches : " << taskCount << std::endl;
    std::cout << "====================================\n"
              << std::endl;
    scheduler.printStatus();
}
