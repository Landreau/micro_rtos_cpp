#include "./kernel/rtos.h"
#include "./kernel/sensor_simulator.h"
#include <iostream>
#include <iomanip>
#include <ctime>

// ============ CAPTEURS GLOBAUX ============
TemperatureSensor tempSensor(22.0f, 1.5f);
HumiditySensor humiditySensor(55.0f);
LightSensor lightSensor(300.0f);
MotionSensor motionSensor;

// Kernel global
Kernel *globalKernel = nullptr;

// Compteurs
uint32_t sensorReadCount = 0;
uint32_t messagesSent = 0;
uint32_t messagesReceived = 0;

// ============ TÂCHE PRODUCTEUR: Capteur Température ============
void temperature_sensor_task()
{
    SensorData data = tempSensor.read(sensorReadCount++);

    // Créer un message pour envoyer les données
    Message msg;
    msg.senderId = 1;
    msg.recipientId = -1;
    msg.messageType = 100;

    // Copier les données dans le message
    msg.dataSize = sizeof(SensorData);
    std::memcpy(msg.data, &data, sizeof(SensorData));

    // Envoyer le message
    if (globalKernel->getMessageQueue().send(msg))
    {
        messagesSent++;
        std::cout << "  [TEMP SENSOR] Envoyé: " << std::fixed << std::setprecision(2)
                  << data.value << "°C (quality: " << (int)data.quality << "%)" << std::endl;
    }
}

// ============ TÂCHE PRODUCTEUR: Capteur Humidité ============
void humidity_sensor_task()
{
    SensorData data = humiditySensor.read(sensorReadCount++);

    Message msg;
    msg.senderId = 2;
    msg.recipientId = -1;
    msg.messageType = 101;
    msg.dataSize = sizeof(SensorData);
    std::memcpy(msg.data, &data, sizeof(SensorData));

    if (globalKernel->getMessageQueue().send(msg))
    {
        messagesSent++;
        std::cout << "  [HUM SENSOR] Envoyé: " << std::fixed << std::setprecision(2)
                  << data.value << "% (quality: " << (int)data.quality << "%)" << std::endl;
    }
}

// ============ TÂCHE PRODUCTEUR: Capteur Luminosité ============
void light_sensor_task()
{
    SensorData data = lightSensor.read(sensorReadCount++);

    Message msg;
    msg.senderId = 3;      // Capteur lumière
    msg.recipientId = -1;  // Broadcast
    msg.messageType = 102; // Type: luminosité
    msg.dataSize = sizeof(SensorData);
    std::memcpy(msg.data, &data, sizeof(SensorData));

    if (globalKernel->getMessageQueue().send(msg))
    {
        messagesSent++;
        std::cout << "  [LIGHT SENSOR] Envoyé: " << std::fixed << std::setprecision(1)
                  << data.value << " lux" << std::endl;
    }
}

// ============ TÂCHE PRODUCTEUR: Capteur Mouvement ============
void motion_sensor_task()
{
    SensorData data = motionSensor.read(sensorReadCount++);

    if (data.value > 0)
    { // Seulement si mouvement détecté
        Message msg;
        msg.senderId = 4;      // Capteur mouvement
        msg.recipientId = -1;  // Broadcast
        msg.messageType = 103; // Type: mouvement
        msg.dataSize = sizeof(SensorData);
        std::memcpy(msg.data, &data, sizeof(SensorData));

        if (globalKernel->getMessageQueue().send(msg))
        {
            messagesSent++;
            std::cout << "  [MOTION SENSOR] ⚠️  MOUVEMENT DÉTECTÉ!" << std::endl;
        }
    }
}

// ============ TÂCHE CONSOMMATEUR: Logger ============
void logger_task()
{
    Message msg;

    if (globalKernel->getMessageQueue().tryReceive(msg))
    {
        messagesReceived++;
        SensorData *data = reinterpret_cast<SensorData *>(msg.data);

        std::cout << "    [LOGGER] Message reçu du capteur " << (int)msg.senderId << ": ";

        switch (data->type)
        {
        case SensorType::TEMPERATURE:
            std::cout << std::fixed << std::setprecision(2) << data->value << "°C";
            break;
        case SensorType::HUMIDITY:
            std::cout << std::fixed << std::setprecision(2) << data->value << "%";
            break;
        case SensorType::LIGHT:
            std::cout << std::fixed << std::setprecision(1) << data->value << " lux";
            break;
        case SensorType::MOTION:
            std::cout << (data->value > 0 ? "MOUVEMENT" : "Pas de mouvement");
            break;
        default:
            std::cout << "Données inconnues";
        }
        std::cout << std::endl;
    }
}

// ============ TÂCHE CONSOMMATEUR: Contrôleur ============
void controller_task()
{
    Message msg;

    if (globalKernel->getMessageQueue().tryReceive(msg))
    {
        SensorData *data = reinterpret_cast<SensorData *>(msg.data);

        // Exemple : Déclencher une alerte si température trop haute
        if (data->type == SensorType::TEMPERATURE && data->value > 25.0f)
        {
            std::cout << "    [CONTRÔLEUR] 🌡️  ALERTE TEMPÉRATURE: " << std::fixed
                      << std::setprecision(2) << data->value << "°C" << std::endl;
        }

        // Exemple : Alerte si humidité trop basse
        if (data->type == SensorType::HUMIDITY && data->value < 30.0f)
        {
            std::cout << "    [CONTRÔLEUR] 💧 ALERTE HUMIDITÉ: " << std::fixed
                      << std::setprecision(2) << data->value << "%" << std::endl;
        }

        // Exemple : Mouvement détecté
        if (data->type == SensorType::MOTION && data->value > 0)
        {
            std::cout << "    [CONTRÔLEUR] 📹 Mouvement détecté - Activating alarm!" << std::endl;
        }
    }
}

// ============ TÂCHE MONITORING: Statistiques ============
static int stats_counter = 0;
void stats_task()
{
    stats_counter++;
    if (stats_counter % 5 == 0)
    { // Afficher tous les 5 appels
        std::cout << "\n    ═══ STATISTIQUES ═══" << std::endl;
        std::cout << "    Messages envoyés: " << messagesSent << std::endl;
        std::cout << "    Messages reçus: " << messagesReceived << std::endl;
        std::cout << "    Messages en queue: " << globalKernel->getMessageQueue().getMessageCount() << std::endl;
        std::cout << "    Lectures capteur: " << sensorReadCount << std::endl;
        std::cout << "    ═════════════════════\n"
                  << std::endl;
    }
}

// ============ MAIN ============
int main()
{
    std::cout << "\n  ╔════════════════════════════════════════════╗" << std::endl;
    std::cout << "  ║  Micro-RTOS - Simulateur de Capteurs       ║" << std::endl;
    std::cout << "  ║  (Système IoT Intelligent)                 ║" << std::endl;
    std::cout << "  ╚════════════════════════════════════════════╝\n"
              << std::endl;

    Kernel kernel;
    globalKernel = &kernel;

    std::cout << "Configuration du système:\n"
              << std::endl;
    std::cout << "  Capteurs:" << std::endl;
    std::cout << "    • Température    (ID: 1) - Période: 100ms" << std::endl;
    std::cout << "    • Humidité       (ID: 2) - Période: 150ms" << std::endl;
    std::cout << "    • Luminosité     (ID: 3) - Période: 200ms" << std::endl;
    std::cout << "    • Mouvement      (ID: 4) - Période: 50ms" << std::endl;
    std::cout << "\n  Consommateurs:" << std::endl;
    std::cout << "    • Logger        - Affiche tous les messages" << std::endl;
    std::cout << "    • Contrôleur    - Applique la logique métier" << std::endl;
    std::cout << "    • Stats         - Affiche les statistiques" << std::endl;
    std::cout << "\n════════════════════════════════════════════\n"
              << std::endl;

    // Créer les tâches PRODUCTEURS (capteurs)
    std::cout << "[KERNEL] Création des capteurs...\n"
              << std::endl;
    kernel.createPeriodicTask(temperature_sensor_task, 3, 100); // Haute priorité, 100ms
    kernel.createPeriodicTask(humidity_sensor_task, 3, 150);
    kernel.createPeriodicTask(light_sensor_task, 3, 200);
    kernel.createPeriodicTask(motion_sensor_task, 3, 50);

    // Créer les tâches CONSOMMATEURS
    std::cout << "[KERNEL] Création des consommateurs...\n"
              << std::endl;
    kernel.createPeriodicTask(logger_task, 2, 100); // Priorité moyenne
    kernel.createPeriodicTask(controller_task, 2, 100);
    kernel.createPeriodicTask(stats_task, 1, 500); // Faible priorité

    std::cout << "[KERNEL] Démarrage du système...\n"
              << std::endl;
    std::cout << "════════════════════════════════════════════\n"
              << std::endl;

    // Lancer le kernel
    // Avec une pause de 1ms entre les itérations (déjà dans le run())
    kernel.run(200); // 200 itérations ~ 2-3 secondes

    std::cout << "\n════════════════════════════════════════════" << std::endl;
    std::cout << "\nRESULTATS FINAUX:" << std::endl;
    std::cout << "  Messages envoyés:   " << messagesSent << std::endl;
    std::cout << "  Messages reçus:     " << messagesReceived << std::endl;
    std::cout << "  Lectures capteur:   " << sensorReadCount << std::endl;
    std::cout << "  Messages restants:  " << globalKernel->getMessageQueue().getMessageCount() << std::endl;

    std::cout << "\n  ╔════════════════════════════════════════════╗" << std::endl;
    std::cout << "  ║      Démonstration Terminée avec Succès    ║" << std::endl;
    std::cout << "  ╚════════════════════════════════════════════╝\n"
              << std::endl;

    return 0;
}
