#ifndef SENSOR_SIMULATOR_H
#define SENSOR_SIMULATOR_H

#include <cstdint>
#include <cmath>
#include <random>
#include <chrono>

// Types de capteurs
enum class SensorType
{
    TEMPERATURE,
    HUMIDITY,
    PRESSURE,
    LIGHT,
    MOTION
};

// Structure pour les données capteur
struct SensorData
{
    SensorType type;
    float value;
    uint32_t timestamp; // ms
    uint32_t sensorId;  // ID du capteur
    uint8_t quality;    // 0-100 (qualité du signal)
};

// ============ TEMPÉRATURE ============
class TemperatureSensor
{
public:
    TemperatureSensor(float baseTemp = 20.0f, float variation = 2.0f)
        : baseTemperature(baseTemp), variation(variation),
          generator(std::random_device{}()),
          distribution(-variation, variation) {}

    SensorData read(uint32_t timestamp)
    {
        // Simulation d'une légère variation sinusoïdale + bruit
        static int counter = 0;
        float sine_component = 5.0f * std::sin(counter * 0.1f);
        float noise = distribution(generator);
        float temperature = baseTemperature + sine_component + noise;

        counter++;

        return {
            SensorType::TEMPERATURE,
            temperature,
            timestamp,
            1, // Sensor ID
            95 // Quality
        };
    }

private:
    float baseTemperature;
    float variation;
    std::mt19937 generator;
    std::uniform_real_distribution<float> distribution;
};

// ============ HUMIDITÉ ============
class HumiditySensor
{
public:
    HumiditySensor(float baseHumidity = 50.0f)
        : baseHumidity(baseHumidity),
          generator(std::random_device{}()),
          distribution(-5.0f, 5.0f) {}

    SensorData read(uint32_t timestamp)
    {
        static int counter = 0;
        float sine_component = 15.0f * std::sin(counter * 0.08f);
        float noise = distribution(generator);
        float humidity = baseHumidity + sine_component + noise;

        // Clamp entre 0 et 100
        if (humidity < 0)
            humidity = 0;
        if (humidity > 100)
            humidity = 100;

        counter++;

        return {
            SensorType::HUMIDITY,
            humidity,
            timestamp,
            2, // Sensor ID
            90};
    }

private:
    float baseHumidity;
    std::mt19937 generator;
    std::uniform_real_distribution<float> distribution;
};

// ============ LUMINOSITÉ ============
class LightSensor
{
public:
    LightSensor(float baseLux = 500.0f)
        : baseLux(baseLux),
          generator(std::random_device{}()),
          distribution(-50.0f, 50.0f) {}

    SensorData read(uint32_t timestamp)
    {
        static int counter = 0;
        // Simulation d'un cycle jour/nuit
        float daylight = 1000.0f * (0.5f + 0.5f * std::sin(counter * 0.05f));
        float noise = distribution(generator);
        float light = baseLux + daylight + noise;

        if (light < 0)
            light = 0;

        counter++;

        return {
            SensorType::LIGHT,
            light,
            timestamp,
            3, // Sensor ID
            85};
    }

private:
    float baseLux;
    std::mt19937 generator;
    std::uniform_real_distribution<float> distribution;
};

// ============ MOTION (PIR) ============
class MotionSensor
{
public:
    MotionSensor() : generator(std::random_device{}()),
                     distribution(0, 100) {}

    SensorData read(uint32_t timestamp)
    {
        static int counter = 0;
        // Mouvement aléatoire toutes les 5 lectures
        uint8_t motion = (distribution(generator) > 80) ? 1 : 0;

        counter++;

        return {
            SensorType::MOTION,
            static_cast<float>(motion),
            timestamp,
            4, // Sensor ID
            100};
    }

private:
    std::mt19937 generator;
    std::uniform_int_distribution<int> distribution;
};

#endif
