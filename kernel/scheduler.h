#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "task.h"
#include <queue>
#include <vector>
#include <memory>

// Comparateur pour la priority queue
struct TaskCompare
{
    bool operator()(const std::shared_ptr<Task> &a, const std::shared_ptr<Task> &b) const
    {
        // Les tâches avec priorité PLUS BASSE viennent en premier dans la queue
        // car priority_queue est un max-heap par défaut
        // Donc on inverse : priorité plus basse = ordre plus élevé dans la queue
        return a->getPriority() < b->getPriority();
    }
};

class Scheduler
{
public:
    Scheduler() = default;
    ~Scheduler() = default;

    // Ajouter une tâche au scheduler
    void addTask(std::shared_ptr<Task> task);

    // Ajouter une tâche pour suivi complet
    void registerTask(std::shared_ptr<Task> task);

    // Obtenir la prochaine tâche à exécuter
    std::shared_ptr<Task> getNextTask();

    // Exécuter une itération du scheduler
    void runOnce();

    // Vérification, si des tâches sont prêtes
    bool hasReadyTasks() const;

    // Affichage de l'état du scheduler (pour debug)
    void printStatus() const;

    // Gestion des tâches avec délai
    void updateDelayedTasks();

    // Gestion des tâches périodiques
    void updatePeriodicTasks();

private:
    // Priority queue : tâches ordonnées par priorité
    std::priority_queue<std::shared_ptr<Task>,
                        std::vector<std::shared_ptr<Task>>,
                        TaskCompare>
        readyQueue;

    std::shared_ptr<Task> currentTask;

    // Tâches avec délai
    std::vector<std::shared_ptr<Task>> delayedTasks;

    // Toutes les tâches (pour gestion périodique)
    std::vector<std::shared_ptr<Task>> allTasks;
};

#endif
