#include "kernel/rtos.h"
#include <iostream>
#include <thread>
#include <chrono>

// Variables globales
int counter1 = 0, counter2 = 0, counter3 = 0;
int periodic_counter = 0;

// Kernel global (pour delay)
Kernel *globalKernel = nullptr;

//  TÂCHES SIMPLES
void task1_simple()
{
    std::cout << "  [TASK 1] Exécution " << ++counter1 << std::endl;
}

void task2_simple()
{
    std::cout << "  [TASK 2] Exécution " << ++counter2 << std::endl;
}

void task3_simple()
{
    std::cout << "  [TASK 3] Exécution " << ++counter3 << std::endl;
}

//  TÂCHE PÉRIODIQUE
void periodic_task()
{
    std::cout << "  [PERIODIC] Exécution " << ++periodic_counter << std::endl;
}

//  TÂCHE AVEC DÉLAI
void delayed_task()
{
    std::cout << "  [DELAYED] Avant délai" << std::endl;
    Kernel::delay(100);
    std::cout << "  [DELAYED] Après délai" << std::endl;
}

//  TÂCHE AVEC MUTEX
int shared_value = 0;

void task_with_mutex()
{
    std::cout << "  [MUTEX TASK] Début critique" << std::endl;
    globalKernel->getMutex().lock();
    {
        shared_value++;
        std::cout << "  [MUTEX TASK] Valeur partagée = " << shared_value << std::endl;
    }
    globalKernel->getMutex().unlock();
    std::cout << "  [MUTEX TASK] Fin critique" << std::endl;
}

//  TÂCHE AVEC SÉMAPHORE
void task_with_semaphore()
{
    std::cout << "  [SEMAPHORE] En attente du sémaphore..." << std::endl;
    globalKernel->getSemaphore().wait();
    {
        std::cout << "  [SEMAPHORE] Sémaphore acquis!" << std::endl;
    }
    globalKernel->getSemaphore().signal();
    std::cout << "  [SEMAPHORE] Sémaphore libéré" << std::endl;
}

//  TÂCHE AVEC QUEUE DE MESSAGES
void producer_task()
{
    std::cout << "  [PRODUCER] Envoi de message..." << std::endl;
    Message msg;
    msg.senderId = 1;
    msg.recipientId = 2;
    msg.messageType = 100;
    msg.dataSize = 5;
    for (int i = 0; i < 5; i++)
    {
        msg.data[i] = 'A' + i;
    }

    if (globalKernel->getMessageQueue().send(msg))
    {
        std::cout << "  [PRODUCER] Message envoyé avec succès" << std::endl;
    }
}

void consumer_task()
{
    Message msg;
    if (globalKernel->getMessageQueue().tryReceive(msg))
    {
        std::cout << "  [CONSUMER] Message reçu de la tâche " << msg.senderId << std::endl;
        std::cout << "  [CONSUMER] Contenu: ";
        for (size_t i = 0; i < msg.dataSize; i++)
        {
            std::cout << (char)msg.data[i];
        }
        std::cout << std::endl;
    }
    else
    {
        std::cout << "  [CONSUMER] Pas de message dans la queue" << std::endl;
    }
}

int main()
{
    std::cout << "\n╔══════════════════════════════════════════╗" << std::endl;
    std::cout << "   ║  Micro-RTOS Amélioré - Démo Complète     ║" << std::endl;
    std::cout << "  ╚══════════════════════════════════════════╝\n"
              << std::endl;

    // Créer le kernel
    Kernel kernel;
    globalKernel = &kernel;

    //  TEST 1: Tâches Simples
    std::cout << "\n[TEST 1] Tâches simples avec priorité" << std::endl;
    std::cout << "═════════════════════════════════════════\n"
              << std::endl;

    kernel.setExpectedTasks(3);
    kernel.createTask(task1_simple, 1);
    kernel.createTask(task2_simple, 3);
    kernel.createTask(task3_simple, 2);

    kernel.run(10);

    std::cout << "Résultat: Task1=" << counter1 << ", Task2=" << counter2 << ", Task3=" << counter3 << std::endl;

    //  TEST 2: Tâche Périodique
    std::cout << "\n[TEST 2] Tâche périodique" << std::endl;
    std::cout << "═════════════════════════════════════════\n"
              << std::endl;

    Kernel kernel2;
    globalKernel = &kernel2;

    kernel2.createPeriodicTask(periodic_task, 2, 50); // Période 50 ms
    kernel2.run(5);

    std::cout << "Tâche périodique exécutée " << periodic_counter << " fois" << std::endl;

    //  TEST 3: Délai
    std::cout << "\n[TEST 3] Délai (delay)" << std::endl;
    std::cout << "═════════════════════════════════════════\n"
              << std::endl;

    Kernel kernel3;
    globalKernel = &kernel3;

    kernel3.createTask(delayed_task, 1);
    kernel3.run(5);

    //  TEST 4: Mutex
    std::cout << "\n[TEST 4] Mutex (synchronisation)" << std::endl;
    std::cout << "═════════════════════════════════════════\n"
              << std::endl;

    shared_value = 0;
    Kernel kernel4;
    globalKernel = &kernel4;

    kernel4.createTask(task_with_mutex, 1);
    kernel4.createTask(task_with_mutex, 1);
    kernel4.run(5);

    std::cout << "Valeur finale partagée: " << shared_value << std::endl;

    //  TEST 5: Sémaphore
    std::cout << "\n[TEST 5] Sémaphore (contrôle d'accès)" << std::endl;
    std::cout << "═════════════════════════════════════════\n"
              << std::endl;

    Kernel kernel5;
    globalKernel = &kernel5;

    kernel5.createTask(task_with_semaphore, 2);
    kernel5.createTask(task_with_semaphore, 1);
    kernel5.run(5);

    //  TEST 6: Queue de Messages
    std::cout << "\n[TEST 6] Queue de Messages (IPC)" << std::endl;
    std::cout << "═════════════════════════════════════════\n"
              << std::endl;

    Kernel kernel6;
    globalKernel = &kernel6;

    kernel6.createTask(producer_task, 2); // Priorité haute
    kernel6.createTask(consumer_task, 1); // Priorité basse
    kernel6.run(5);

    std::cout << "\n  ╔══════════════════════════════════════════╗" << std::endl;
    std::cout << "  ║         Démo Terminée avec Succès        ║" << std::endl;
    std::cout << "  ╚══════════════════════════════════════════╝\n"
              << std::endl;

    return 0;
}
