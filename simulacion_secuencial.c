// simulacion_secuencial.c
// Versión secuencial de la simulación de tráfico

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

typedef enum { RED = 0, GREEN = 1, YELLOW = 2 } LightState;

typedef struct {
    int id;
    LightState state;
    int timer;
    int red_dur;
    int green_dur;
    int yellow_dur;
} TrafficLight;

typedef struct {
    int id;
    int position;
    int speed;
    int target_light_id;
} Vehicle;

const char* light_name(LightState s) {
    switch (s) {
        case RED: return "ROJO";
        case GREEN: return "VERDE";
        case YELLOW: return "AMARILLO";
        default: return "?";
    }
}

void init_lights(TrafficLight *lights, int n) {
    for (int i = 0; i < n; i++) {
        lights[i].id = i;
        lights[i].state = (i % 2 == 0) ? GREEN : RED;
        lights[i].timer = 0;
        lights[i].green_dur = 3;
        lights[i].yellow_dur = 1;
        lights[i].red_dur = 3;
    }
}

void init_vehicles(Vehicle *v, int n, int num_lights) {
    for (int i = 0; i < n; i++) {
        v[i].id = i;
        v[i].position = rand() % 3;
        v[i].speed = 1 + rand() % 2;
        v[i].target_light_id = i % num_lights;
    }
}

void update_traffic_lights(TrafficLight *lights, int n) {
    for (int i = 0; i < n; i++) {
        TrafficLight *L = &lights[i];
        L->timer++;
        switch (L->state) {
            case GREEN:
                if (L->timer >= L->green_dur) { L->state = YELLOW; L->timer = 0; }
                break;
            case YELLOW:
                if (L->timer >= L->yellow_dur) { L->state = RED; L->timer = 0; }
                break;
            case RED:
                if (L->timer >= L->red_dur) { L->state = GREEN; L->timer = 0; }
                break;
        }
    }
}

void move_vehicles(Vehicle *veh, int nveh, const TrafficLight *lights) {
    for (int i = 0; i < nveh; i++) {
        const TrafficLight *L = &lights[veh[i].target_light_id];
        if (L->state == GREEN) veh[i].position += veh[i].speed;
        else if (L->state == YELLOW) veh[i].position += 1;
    }
}

void simulate_traffic(int iterations, Vehicle *v, int nveh, TrafficLight *L, int nlights, int print_every) {
    for (int t = 1; t <= iterations; t++) {
        update_traffic_lights(L, nlights);
        move_vehicles(v, nveh, L);
        if (t % print_every == 0) {
            printf("\nIteración %d:\n", t);
            for (int i = 0; i < nlights; i++) {
                printf("Semáforo %d -> %-8s\n", L[i].id, light_name(L[i].state));
            }
        }
        usleep(200000);
    }
}

int main(int argc, char **argv) {
    int iterations = (argc > 1) ? atoi(argv[1]) : 12;
    int num_vehicles = (argc > 2) ? atoi(argv[2]) : 40;
    int num_lights = (argc > 3) ? atoi(argv[3]) : 4;

    srand((unsigned)time(NULL));
    TrafficLight *L = malloc(sizeof(TrafficLight) * num_lights);
    Vehicle *V = malloc(sizeof(Vehicle) * num_vehicles);

    init_lights(L, num_lights);
    init_vehicles(V, num_vehicles, num_lights);

    simulate_traffic(iterations, V, num_vehicles, L, num_lights, 1);

    free(V);
    free(L);
    return 0;
}
