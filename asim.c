/* sim.c*/ 
#include "asim.h"

#include <stdio.h>
#include <math.h>
#include <unistd.h> // For usleep (Unix-based systems)

// Constants
#define GRAVITY 9.81 // m/s²

// Define pump, pipe, and tank parameters
typedef struct {
    double flow_rate;    // Pump flow rate (m³/s)
    double head;         // Pump head (m)
    double power;        // Pump power required (W)
} Pump;

typedef struct {
    double length;       // Pipe length (m)
    double diameter;     // Pipe diameter (m)
    double roughness;    // Pipe roughness (dimensionless)
    double velocity;     // Velocity of water in the pipe (m/s)
    double density;      // Liquid density (kg/m³)
} Pipe;

typedef struct {
    double height;       // Tank height (m)
    double radius;       // Tank radius (m)
    double water_level;  // Current water level (m)
} Tank;

// Function to calculate the Darcy friction factor (using approximation for turbulent flow)
double calculate_friction_factor(double reynolds_number) {
    if (reynolds_number < 2000) {
        // Laminar flow
        return 64.0 / reynolds_number;
    } else {
        // Turbulent flow (using Colebrook-White approximation for simplicity)
        return 0.02; // Typical value for turbulent flows
    }
}

// Function to calculate head loss in the pipe
double calculate_head_loss(Pipe *pipe, double flow_rate) {
    // Velocity of water
    double area = M_PI * pow(pipe->diameter / 2.0, 2);
    pipe->velocity = flow_rate / area;

    // Reynolds number (assuming dynamic viscosity = 0.001 Pa·s)
    double reynolds_number = (pipe->density * pipe->velocity * pipe->diameter) / 0.001;

    // Friction factor
    double friction_factor = calculate_friction_factor(reynolds_number);

    // Head loss (Darcy-Weisbach equation)
    return friction_factor * (pipe->length / pipe->diameter) * pow(pipe->velocity, 2) / (2 * GRAVITY);
}

// Function to calculate pump power
double calculate_pump_power(Pump *pump, Pipe *pipe) {
    return pipe->density * GRAVITY * pump->flow_rate * pump->head; // Watts
}

// Function to update the tank water level
void update_tank(Tank *tank, Pump *pump, Pipe *pipe, double time_step) {
    // Calculate head loss in the pipe
    double head_loss = calculate_head_loss(pipe, pump->flow_rate);

    // Check if the pump can overcome the head loss
    if (pump->head < head_loss) {
        printf("Pump cannot overcome the head loss. No flow occurs.\n");
        return;
    }

    // Effective flow rate (considering losses)
    double effective_flow_rate = pump->flow_rate; // Simplification (losses already considered in pipe head)

    // Update water level in the tank
    double tank_area = M_PI * pow(tank->radius, 2);
    tank->water_level += (effective_flow_rate / tank_area) * time_step;

    // Ensure water level does not exceed tank height
    if (tank->water_level > tank->height) {
        tank->water_level = tank->height;
        printf("Tank is full! Overflow occurs.\n");
    }

    // Update pump power
    pump->power = calculate_pump_power(pump, pipe);
}

// Real-time simulation loop
void simulate_system(Pump *pump, Pipe *pipe, Tank *tank, double simulation_duration, double time_step) {
    double current_time = 0.0;

    printf("Time(s)   Water Level(m)   Flow Rate(m³/s)   Pump Power(W)\n");
    printf("---------------------------------------------------------\n");

    while (current_time < simulation_duration) {
        // Update tank water level
        update_tank(tank, pump, pipe, time_step);

        // Print results
        printf("%.2f       %.4f          %.4f          %.2f\n",
               current_time, tank->water_level, pump->flow_rate, pump->power);

        // Increment time
        current_time += time_step;

        // Real-time delay
        usleep(time_step * 1e6); // Sleep in microseconds
    }
}

int main() {
    // Initialize pump parameters
    Pump pump = {
        .flow_rate = 0.01, // m³/s (10 L/s)
        .head = 10.0,      // m
        .power = 0.0       // Calculated during simulation
    };

    // Initialize pipe parameters
    Pipe pipe = {
        .length = 50.0,     // m
        .diameter = 0.1,    // m (10 cm)
        .roughness = 0.015, // Typical value for steel pipes
        .velocity = 0.0,    // Initially zero
        .density = 1000.0   // Density of water (kg/m³)
    };

    // Initialize tank parameters
    Tank tank = {
        .height = 5.0,       // m
        .radius = 1.0,       // m (1m radius)
        .water_level = 0.0   // Initial water level
    };

    double simulation_duration = 60.0; // 60 seconds
    double time_step = 1.0;            // 1-second time step

    printf("Starting real-time tank filling simulation...\n");
    simulate_system(&pump, &pipe, &tank, simulation_duration, time_step);
    printf("Simulation complete.\n");

    return 0;
}
