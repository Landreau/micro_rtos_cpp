#ifndef RTOS_H
#define RTOS_H

#include "scheduler.h"
#include "task.h"
#include "sync.h"
#include "message_queue.h"
#include <memory>
#include <map>
#include <chrono>
#include <string>

class Kernel
{
public:
    Kernel();
    ~Kernel() = default;

    // Créer et ajouter une tâche simple
    int createTask(Task::TaskFunc func, uint32_t priority);

    // Créer une tâche périodique
    int createPeriodicTask(Task::TaskFunc func, uint32_t priority, uint32_t periodMs);

    // Délai (ms) - bloquer la tâche courante
    static void delay(uint32_t milliseconds);

    // Obtenir le Mutex global pour synchronisation
    Mutex &getMutex() { return globalMutex; }

    // Obtenir le Sémaphore global
    Semaphore &getSemaphore() { return globalSemaphore; }

    // Obtenir la queue de messages globale
    MessageQueue &getMessageQueue() { return msgQueue; }

    // Démarrer le kernel (lance le scheduler)
    void run(int maxIterations = -1); // -1 = boucle infinie

    // Arrêter le kernel
    void stop();

    // Définir le nombre de tâches attendues (active la barre de progression)
    void setExpectedTasks(int n) { expectedTasks = n; }

    // Obtenir le nombre de tâches
    int getTaskCount() const { return taskCount; }

    // Vérifier si le kernel est en cours d'exécution
    bool isRunning() const { return running; }

    // Afficher l'état du kernel
    void printStatus() const;

private:
    // Affiche une ligne de progression lors de l'enregistrement des tâches
    void printProgress(const std::string &label) const;

    Scheduler scheduler;
    int taskIdCounter = 0;
    int taskCount = 0;
    int expectedTasks = 0; // 0 = pas de barre de progression
    bool running = false;
    std::map<int, std::shared_ptr<Task>> tasks;

    // Synchronisation
    Mutex globalMutex;
    Semaphore globalSemaphore{1}; // Binaire

    // Queue de messages
    MessageQueue msgQueue;
};

#endif
