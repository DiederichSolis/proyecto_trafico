
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <omp.h>

// ----- Paso 1: Estructuras -----
typedef enum { RED = 0, GREEN = 1, YELLOW = 2 } LightState;

typedef struct {
    int id;
    LightState state;
    int timer;          // segundos en el estado actual
    int red_dur;        // duración de rojo
    int green_dur;      // duración de verde
    int yellow_dur;     // duración de amarillo
} TrafficLight;

typedef struct {
    int id;
    int position;       // posición discreta en "metros"
    int speed;          // avance por tick cuando puede moverse
    int target_light_id;// id del semáforo que controla su paso
} Vehicle;

// Utilidad para clamp
static inline int clamp(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }

// ----- Paso 2: Inicialización -----
void init_lights(TrafficLight *lights, int n) {
    for (int i = 0; i < n; i++) {
        lights[i].id = i;
        lights[i].state = (i % 2 == 0) ? GREEN : RED;
        lights[i].timer = 0;
        lights[i].green_dur  = 3; // segundos
        lights[i].yellow_dur = 1; // segundos
        lights[i].red_dur    = 3; // segundos
    }
}

void init_vehicles(Vehicle *v, int n, int num_lights) {
    for (int i = 0; i < n; i++) {
        v[i].id = i;
        v[i].position = rand() % 3;            // arranque diverso
        v[i].speed = 1 + rand() % 2;           // 1..2
        v[i].target_light_id = i % num_lights; // cada uno “apunta” a algún semáforo
    }
}

// ----- Paso 3: Semáforos (paralelo) -----
void update_traffic_lights(TrafficLight *lights, int n) {
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < n; i++) {
        TrafficLight *L = &lights[i];
        L->timer++;

        switch (L->state) {
            case GREEN:
                if (L->timer >= L->green_dur) {
                    L->state = YELLOW;
                    L->timer = 0;
                }
                break;
            case YELLOW:
                if (L->timer >= L->yellow_dur) {
                    L->state = RED;
                    L->timer = 0;
                }
                break;
            case RED:
                if (L->timer >= L->red_dur) {
                    L->state = GREEN;
                    L->timer = 0;
                }
                break;
        }
    }
}

// ----- Paso 4: Vehículos (paralelo) -----
void move_vehicles(Vehicle *veh, int nveh, const TrafficLight *lights, int nlights) {
    (void)nlights; // no lo usamos directo aquí, pero se deja por claridad
    #pragma omp parallel for schedule(dynamic, 8)
    for (int i = 0; i < nveh; i++) {
        const TrafficLight *L = &lights[veh[i].target_light_id];
        int delta = 0;
        if (L->state == GREEN)      delta = veh[i].speed;     // avanza normal
        else if (L->state == YELLOW) delta = 1;               // avanza lento
        else                         delta = 0;               // rojo, detenido

        // Evitar números locos
        veh[i].position = clamp(veh[i].position + delta, 0, 1000000);
    }
}

// Impresión amigable de estado de luces
const char* light_name(LightState s) {
    switch (s) {
        case RED: return "ROJO";
        case GREEN: return "VERDE";
        case YELLOW: return "AMARILLO";
        default: return "?";
    }
}

// ----- Paso 5: Simulación con hilos “fijos” -----
void simulate_traffic(int iterations, Vehicle *v, int nveh, TrafficLight *L, int nlights, int print_every) {
    for (int t = 1; t <= iterations; t++) {
        update_traffic_lights(L, nlights);
        move_vehicles(v, nveh, L, nlights);

        if (t % print_every == 0) {
            // Solo un hilo imprime para orden
            printf("\n=== Iteración %d ===\n", t);
            for (int i = 0; i < nlights; i++) {
                printf("Semáforo %d -> %-8s (timer=%d)\n", L[i].id, light_name(L[i].state), L[i].timer);
            }
            int show = (nveh < 20) ? nveh : 20; // muestra primeros 20
            for (int i = 0; i < show; i++) {
                printf("Vehículo %2d: pos=%d (speed=%d, light=%d)\n", v[i].id, v[i].position, v[i].speed, v[i].target_light_id);
            }
        }
        usleep(200000); // 0.2 s entre iteraciones (ajustable)
    }
}

// ----- Paso 6: Simulación con ajuste dinámico de hilos -----
void simulate_traffic_dynamic(int iterations, Vehicle *v, int nveh, TrafficLight *L, int nlights, int print_every) {
    omp_set_dynamic(1); // permitir ajuste dinámico
    for (int t = 1; t <= iterations; t++) {
        // Heurística simple: 1 hilo por cada ~10 vehículos, al menos 2
        int threads = nveh / 10 + 2;
        if (threads < 2) threads = 2;
        omp_set_num_threads(threads);

        #pragma omp parallel sections
        {
            #pragma omp section
            update_traffic_lights(L, nlights);

            #pragma omp section
            move_vehicles(v, nveh, L, nlights);
        }

        if (t % print_every == 0) {
            printf("\n[Dinamico hilos=%d] Iteración %d\n", threads, t);
            for (int i = 0; i < nlights; i++) {
                printf("Semáforo %d -> %-8s (timer=%d)\n", L[i].id, light_name(L[i].state), L[i].timer);
            }
            int show = (nveh < 20) ? nveh : 20;
            for (int i = 0; i < show; i++) {
                printf("Vehículo %2d: pos=%d (speed=%d, light=%d)\n", v[i].id, v[i].position, v[i].speed, v[i].target_light_id);
            }
        }
        usleep(200000);
    }
}

// ----- Paso 7: Main con parámetros sencillos -----
int main(int argc, char **argv) {
    int iterations   = (argc > 1) ? atoi(argv[1]) : 12;
    int num_vehicles = (argc > 2) ? atoi(argv[2]) : 40;
    int num_lights   = (argc > 3) ? atoi(argv[3]) : 4;
    int dynamic_mode = (argc > 4) ? atoi(argv[4]) : 1; // 1 = dinámico, 0 = fijo
    int print_every  = (argc > 5) ? atoi(argv[5]) : 1;

    srand((unsigned)time(NULL));

    TrafficLight *L = (TrafficLight*)malloc(sizeof(TrafficLight) * num_lights);
    Vehicle     *V = (Vehicle*)    malloc(sizeof(Vehicle) * num_vehicles);

    init_lights(L, num_lights);
    init_vehicles(V, num_vehicles, num_lights);

    printf("Simulación: it=%d, veh=%d, lights=%d, modo=%s, print_each=%d\n",
           iterations, num_vehicles, num_lights, dynamic_mode ? "DINAMICO" : "FIJO", print_every);

    if (dynamic_mode) simulate_traffic_dynamic(iterations, V, num_vehicles, L, num_lights, print_every);
    else              simulate_traffic(iterations, V, num_vehicles, L, num_lights, print_every);

    free(V);
    free(L);
    return 0;
}
