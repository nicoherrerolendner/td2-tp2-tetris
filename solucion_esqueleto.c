// Uso:  ./tetris           corre los tests y despues el juego
//       ./tetris --test    corre solo los tests (valgrind --leak-check=full ./tetris --test)

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

// ========= CONSTANTES DEL JUEGO =========
#define SCREEN_WIDTH 500
#define SCREEN_HEIGHT 800
#define GRID_ROWS 20
#define GRID_COLS 10
#define CELL_SIZE 35
#define OFFSET_X 75
#define OFFSET_Y 50
#define TICK_RATE 30

const int TETROMINOS[7][4][4][4] = {
    { {{0,0,0,0},{1,1,1,1},{0,0,0,0},{0,0,0,0}}, {{0,0,1,0},{0,0,1,0},{0,0,1,0},{0,0,1,0}}, {{0,0,0,0},{0,0,0,0},{1,1,1,1},{0,0,0,0}}, {{0,1,0,0},{0,1,0,0},{0,1,0,0},{0,1,0,0}} },
    { {{2,0,0,0},{2,2,2,0},{0,0,0,0},{0,0,0,0}}, {{0,2,2,0},{0,2,0,0},{0,2,0,0},{0,0,0,0}}, {{0,0,0,0},{2,2,2,0},{0,0,2,0},{0,0,0,0}}, {{0,2,0,0},{0,2,0,0},{2,2,0,0},{0,0,0,0}} },
    { {{0,0,3,0},{3,3,3,0},{0,0,0,0},{0,0,0,0}}, {{0,3,0,0},{0,3,0,0},{0,3,3,0},{0,0,0,0}}, {{0,0,0,0},{3,3,3,0},{3,0,0,0},{0,0,0,0}}, {{3,3,0,0},{0,3,0,0},{0,3,0,0},{0,0,0,0}} },
    { {{0,4,4,0},{0,4,4,0},{0,0,0,0},{0,0,0,0}}, {{0,4,4,0},{0,4,4,0},{0,0,0,0},{0,0,0,0}}, {{0,4,4,0},{0,4,4,0},{0,0,0,0},{0,0,0,0}}, {{0,4,4,0},{0,4,4,0},{0,0,0,0},{0,0,0,0}} },
    { {{0,5,5,0},{5,5,0,0},{0,0,0,0},{0,0,0,0}}, {{0,5,0,0},{0,5,5,0},{0,0,5,0},{0,0,0,0}}, {{0,0,0,0},{0,5,5,0},{5,5,0,0},{0,0,0,0}}, {{5,0,0,0},{5,5,0,0},{0,5,0,0},{0,0,0,0}} },
    { {{0,6,0,0},{6,6,6,0},{0,0,0,0},{0,0,0,0}}, {{0,6,0,0},{0,6,6,0},{0,6,0,0},{0,0,0,0}}, {{0,0,0,0},{6,6,6,0},{0,6,0,0},{0,0,0,0}}, {{0,6,0,0},{6,6,0,0},{0,6,0,0},{0,0,0,0}} },
    { {{7,7,0,0},{0,7,7,0},{0,0,0,0},{0,0,0,0}}, {{0,0,7,0},{0,7,7,0},{0,7,0,0},{0,0,0,0}}, {{0,0,0,0},{7,7,0,0},{0,7,7,0},{0,0,0,0}}, {{0,7,0,0},{7,7,0,0},{7,0,0,0},{0,0,0,0}} }
};

SDL_Color colors[8] = {
    {30, 30, 30, 255}, {0, 255, 255, 255}, {0, 0, 255, 255}, {255, 165, 0, 255},
    {255, 255, 0, 255}, {0, 255, 0, 255}, {128, 0, 128, 255}, {255, 0, 0, 255}
};

// ========= ESTRUCTURAS DE DATOS =========

// 1. Nodo para la fila del tablero (Lista Enlazada)
typedef struct RowNode {
    int cells[GRID_COLS]; 
    struct RowNode* next; 
} RowNode;

// 2. Nodo para la cola de próximas piezas (Queue)
typedef struct PieceQueueNode {
    int type; 
    struct PieceQueueNode* next;
} PieceQueueNode;

// 3. Nodo para el sistema de partículas 
typedef struct ParticleNode {
    float x, y;
    float vel_x, vel_y;
    int color;
    int life_timer;
    struct ParticleNode* next;
} ParticleNode;

typedef struct ActivePiece {
    int col, row;
    int type;      
    int rotation;  
    int active;
} ActivePiece;

typedef struct GameBoard {
    RowNode* first_row;             
    PieceQueueNode* queue_head;     
    PieceQueueNode* queue_tail;     
    ParticleNode* first_particle;   
    ActivePiece current_piece;
    int score;
    int tick_timer;
} GameBoard;

// ========= VARIABLES GLOBALES =========
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
GameBoard* game_board = NULL;

// ========= EJERCICIO 1: STRINGS (NO USAR string.h) =========

char* strDuplicate(char* src) {
    int longitud = 0;

    // Contamos los caracteres hasta el final del string.
    while (src[longitud] != '\0') {
        longitud++;
    }

    // Reservamos espacio para el texto y su terminador.
    char* copia = malloc((longitud + 1) * sizeof(char));

    if (copia == NULL) {
        return NULL;
    }

    // Copiamos tambien el terminador '\0'.
    for (int i = 0; i <= longitud; i++) {
        copia[i] = src[i];
    }

    return copia;
}

int strCompare(char* s1, char* s2) {
    int i = 0;

    // Avanzamos mientras los caracteres coincidan y no termine s1.
    while (s1[i] != '\0' && s1[i] == s2[i]) {
        i++;
    }

    unsigned char c1 = (unsigned char) s1[i];
    unsigned char c2 = (unsigned char) s2[i];

    if (c1 == c2) {
        return 0;
    }

    if (c1 < c2) {
        return 1;
    }

    return -1;
}

char* strConcatenate(char* src1, char* src2) {
    int longitud1 = 0;
    int longitud2 = 0;

    while (src1[longitud1] != '\0') {
        longitud1++;
    }

    while (src2[longitud2] != '\0') {
        longitud2++;
    }

    // Reservamos espacio para ambos textos y un solo terminador.
    char* resultado = malloc(
        (longitud1 + longitud2 + 1) * sizeof(char)
    );

    if (resultado == NULL) {
        free(src1);
        free(src2);
        return NULL;
    }

    // Copiamos el primer texto sin su terminador.
    for (int i = 0; i < longitud1; i++) {
        resultado[i] = src1[i];
    }

    // Copiamos el segundo texto e incluimos su terminador.
    for (int i = 0; i <= longitud2; i++) {
        resultado[longitud1 + i] = src2[i];
    }

    // La funcion consume los dos strings originales.
    free(src1);
    free(src2);

    return resultado;
}

// ========= EJERCICIO 2: FUNCIONES A IMPLEMENTAR (EL TP) =========

GameBoard* gameBoardNew() {
    GameBoard* board = malloc(sizeof(GameBoard));

    if (board == NULL) {
        return NULL;
    }

    // Inicializamos todos los campos: numeros en 0 y punteros en NULL.
    *board = (GameBoard){0};

    for (int i = 0; i < GRID_ROWS; i++) {
        RowNode* nueva = malloc(sizeof(RowNode));

        if (nueva == NULL) {
            // Si falla una reserva, liberamos las filas ya creadas.
            RowNode* actual = board->first_row;

            while (actual != NULL) {
                RowNode* siguiente = actual->next;
                free(actual);
                actual = siguiente;
            }

            free(board);
            return NULL;
        }

        for (int j = 0; j < GRID_COLS; j++) {
            nueva->cells[j] = 0;
        }

        // Insertamos la nueva fila al principio de la lista.
        nueva->next = board->first_row;
        board->first_row = nueva;
    }

    return board;
}

void gameBoardDelete(GameBoard* board) {
    if (board == NULL) {
        return;
    }

    // Liberamos todas las filas.
    RowNode* fila = board->first_row;

    while (fila != NULL) {
        RowNode* siguiente = fila->next;
        free(fila);
        fila = siguiente;
    }

    // Liberamos todas las piezas pendientes en la cola.
    PieceQueueNode* pieza = board->queue_head;

    while (pieza != NULL) {
        PieceQueueNode* siguiente = pieza->next;
        free(pieza);
        pieza = siguiente;
    }

    // Liberamos todas las particulas.
    ParticleNode* particula = board->first_particle;

    while (particula != NULL) {
        ParticleNode* siguiente = particula->next;
        free(particula);
        particula = siguiente;
    }

    // Finalmente liberamos la estructura principal.
    free(board);
}

void gameBoardAddParticle(GameBoard* board, float x, float y, int color) {
    ParticleNode* nueva = malloc(sizeof(ParticleNode));

    if (nueva == NULL) {
        return;
    }

    nueva->x = x;
    nueva->y = y;
    nueva->color = color;

    // Mantenemos las velocidades y duracion del juego base.
    nueva->vel_x = ((rand() % 100) / 10.0f) - 5.0f;
    nueva->vel_y = ((rand() % 50) / -10.0f) - 2.0f;
    nueva->life_timer = 30 + (rand() % 30);

    // La nueva particula pasa a ser la cabeza de la lista.
    nueva->next = board->first_particle;
    board->first_particle = nueva;
}

int gameBoardCheckAndClearLines(GameBoard* board) {
    RowNode* actual = board->first_row;
    RowNode* anterior = NULL;
    int indiceFila = 0;
    int eliminadas = 0;

    while (actual != NULL) {
        int completa = 1;

        for (int j = 0; j < GRID_COLS; j++) {
            if (actual->cells[j] == 0) {
                completa = 0;
                break;
            }
        }

        if (!completa) {
            anterior = actual;
            actual = actual->next;
            indiceFila++;
            continue;
        }

        // Reservamos el reemplazo antes de modificar la lista.
        // Si falla, el tablero conserva sus 20 filas.
        RowNode* nueva = malloc(sizeof(RowNode));

        if (nueva == NULL) {
            return eliminadas;
        }

        for (int j = 0; j < GRID_COLS; j++) {
            nueva->cells[j] = 0;
        }

        // Generamos una particula en el centro de cada celda.
        for (int j = 0; j < GRID_COLS; j++) {
            float x = OFFSET_X + j * CELL_SIZE + CELL_SIZE / 2.0f;
            float y = OFFSET_Y + indiceFila * CELL_SIZE
                      + CELL_SIZE / 2.0f;

            gameBoardAddParticle(board, x, y, actual->cells[j]);
        }

        RowNode* siguiente = actual->next;

        // Desenlazamos la fila completa.
        if (anterior == NULL) {
            board->first_row = siguiente;
        } else {
            anterior->next = siguiente;
        }

        free(actual);

        // Insertamos el reemplazo vacio arriba del tablero.
        nueva->next = board->first_row;
        board->first_row = nueva;

        // Si eliminamos la cabeza, el reemplazo es ahora
        // el nodo anterior a la siguiente fila por revisar.
        if (anterior == NULL) {
            anterior = nueva;
        }

        actual = siguiente;
        eliminadas++;
        indiceFila++;
    }

    return eliminadas;
}

void gameBoardEnqueuePiece(GameBoard* board, int type) {
    PieceQueueNode* nueva = malloc(sizeof(PieceQueueNode));

    if (nueva == NULL) {
        return;
    }

    nueva->type = type;
    nueva->next = NULL;

    if (board->queue_tail == NULL) {
        // La cola estaba vacia: la nueva pieza es tambien la primera.
        board->queue_head = nueva;
    } else {
        // Enlazamos la ultima pieza con la nueva.
        board->queue_tail->next = nueva;
    }

    board->queue_tail = nueva;
}

int gameBoardDequeuePiece(GameBoard* board, int* out_type) {
    if (board->queue_head == NULL) {
        // Si esta vacia, no modificamos *out_type.
        return 0;
    }

    PieceQueueNode* primera = board->queue_head;

    // Escribimos el tipo de pieza en la variable del llamador.
    *out_type = primera->type;

    board->queue_head = primera->next;

    if (board->queue_head == NULL) {
        // Salio la ultima pieza: ambos extremos quedan en NULL.
        board->queue_tail = NULL;
    }

    free(primera);

    return 1;
}

// ========= LOGICA DE DIBUJADO Y COLISIONES (NO MODIFICAR) =========

RowNode* getRowAt(GameBoard* board, int index) {
    RowNode* curr = board->first_row;
    for(int i = 0; i < index && curr != NULL; i++) curr = curr->next;
    return curr;
}

int canMove(GameBoard* board, int target_col, int target_row, int target_rot) {
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            if (TETROMINOS[board->current_piece.type][target_rot][r][c] != 0) {
                int board_col = target_col + c;
                int board_row = target_row + r;
                if (board_col < 0 || board_col >= GRID_COLS || board_row >= GRID_ROWS) return 0;
                if (board_row >= 0) {
                    RowNode* row_node = getRowAt(board, board_row);
                    if (row_node && row_node->cells[board_col] != 0) return 0;
                }
            }
        }
    }
    return 1;
}

void lockPiece(GameBoard* board) {
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            int val = TETROMINOS[board->current_piece.type][board->current_piece.rotation][r][c];
            if (val != 0) {
                RowNode* row_node = getRowAt(board, board->current_piece.row + r);
                if (row_node) row_node->cells[board->current_piece.col + c] = val;
            }
        }
    }
    board->current_piece.active = 0;
    board->score += gameBoardCheckAndClearLines(board) * 100;
}

// Devuelve 0 si la pieza nueva no entra (GAME OVER), 1 en otro caso.
int spawnNextPiece(GameBoard* board) {
    int type;
    if (gameBoardDequeuePiece(board, &type)) {
        board->current_piece.type = type;
        board->current_piece.rotation = 0;
        board->current_piece.col = GRID_COLS / 2 - 2;
        board->current_piece.row = 0;
        board->current_piece.active = 1;
        gameBoardEnqueuePiece(board, rand() % 7);
        if (!canMove(board, board->current_piece.col, board->current_piece.row, board->current_piece.rotation)) {
            printf("GAME OVER. Puntaje final: %d\n", board->score);
            board->current_piece.active = 0;
            return 0;   // main sale del loop y libera tablero y SDL
        }
    }
    return 1;
}

// Devuelve 0 cuando termina la partida.
int gameBoardUpdate(GameBoard* board) {
    if (!board) return 0;
    if (!board->current_piece.active && !spawnNextPiece(board)) return 0;

    board->tick_timer++;
    if (board->tick_timer >= TICK_RATE) {
        board->tick_timer = 0;
        if (canMove(board, board->current_piece.col, board->current_piece.row + 1, board->current_piece.rotation)) {
            board->current_piece.row++;
        } else {
            lockPiece(board);
        }
    }

    ParticleNode* curr_part = board->first_particle;
    ParticleNode* prev_part = NULL;
    while (curr_part) {
        curr_part->x += curr_part->vel_x;
        curr_part->y += curr_part->vel_y;
        curr_part->vel_y += 0.5f;
        curr_part->life_timer--;

        if (curr_part->life_timer <= 0) {
            ParticleNode* to_delete = curr_part;
            if (prev_part == NULL) {
                board->first_particle = curr_part->next;
                curr_part = board->first_particle;
            } else {
                prev_part->next = curr_part->next;
                curr_part = prev_part->next;
            }
            free(to_delete);
        } else {
            prev_part = curr_part;
            curr_part = curr_part->next;
        }
    }
    return 1;
}

void gameBoardDraw(GameBoard* board) {
    if (!board) return;
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Rect board_rect = {OFFSET_X - 2, OFFSET_Y - 2, GRID_COLS * CELL_SIZE + 4, GRID_ROWS * CELL_SIZE + 4};
    SDL_RenderDrawRect(renderer, &board_rect);

    RowNode* current_row = board->first_row;
    int r = 0;
    while (current_row) {
        for (int c = 0; c < GRID_COLS; c++) {
            if (current_row->cells[c] > 0) {
                SDL_Color col = colors[current_row->cells[c]];
                SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, 255);
                SDL_Rect cell_rect = {OFFSET_X + c * CELL_SIZE, OFFSET_Y + r * CELL_SIZE, CELL_SIZE - 1, CELL_SIZE - 1};
                SDL_RenderFillRect(renderer, &cell_rect);
            }
        }
        current_row = current_row->next;
        r++;
    }

    if (board->current_piece.active) {
        for(int pr = 0; pr < 4; pr++) {
            for(int pc = 0; pc < 4; pc++) {
                int cell_val = TETROMINOS[board->current_piece.type][board->current_piece.rotation][pr][pc];
                if (cell_val != 0) {
                    SDL_Color col = colors[cell_val];
                    SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, 255);
                    SDL_Rect p_rect = {OFFSET_X + (board->current_piece.col + pc) * CELL_SIZE, 
                                       OFFSET_Y + (board->current_piece.row + pr) * CELL_SIZE, 
                                       CELL_SIZE - 1, CELL_SIZE - 1};
                    SDL_RenderFillRect(renderer, &p_rect);
                }
            }
        }
    }

    ParticleNode* curr_part = board->first_particle;
    while (curr_part) {
        SDL_Color col = colors[curr_part->color];
        int alpha = curr_part->life_timer * 8;   // se satura: SDL recibe un Uint8
        if (alpha > 255) alpha = 255;
        SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, alpha);
        SDL_Rect part_rect = {(int)curr_part->x, (int)curr_part->y, 6, 6};
        SDL_RenderFillRect(renderer, &part_rect);
        curr_part = curr_part->next;
    }
    SDL_RenderPresent(renderer);
}

int inicializar() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return 0;
    window = SDL_CreateWindow("Tetris Base - TP", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) return 0;
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) return 0;
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    return 1;
}

// ========= TESTS =========

void testStrDuplicate(void) {
    // strDuplicate: vacio, un caracter y distintos caracteres.
    char* vacio = strDuplicate("");
    char* letra = strDuplicate("x");
    char* texto = strDuplicate("Hola 123!");

    assert(vacio != NULL);
    assert(letra != NULL);
    assert(texto != NULL);

    assert(vacio[0] == '\0');
    assert(letra[0] == 'x' && letra[1] == '\0');

    char esperado[] = "Hola 123!";
    for (int i = 0; i <= 9; i++) {
        assert(texto[i] == esperado[i]);
    }

    // La copia debe poder modificarse sin cambiar el original.
    char original[] = "hola";
    char* copia = strDuplicate(original);
    assert(copia != NULL);
    copia[0] = 'H';
    assert(original[0] == 'h');

    free(vacio);
    free(letra);
    free(texto);
    free(copia);

    printf("strDuplicate: OK\n");
    }

void testStrCompare(void) {

    // strCompare: igualdad, diferencias y prefijos.
    assert(strCompare("", "") == 0);
    assert(strCompare("a", "a") == 0);
    assert(strCompare("a", "b") == 1);
    assert(strCompare("b", "a") == -1);
    assert(strCompare("casa", "caso") == 1);
    assert(strCompare("caso", "casa") == -1);
    assert(strCompare("casa", "casas") == 1);
    assert(strCompare("casas", "casa") == -1);
    assert(strCompare("", "a") == 1);
    assert(strCompare("a", "") == -1);

    printf("strCompare: OK\n");
    }

void testStrConcatenate(void) {

    // strConcatenate: argumentos dinamicos y distintos.
    char* primero = strDuplicate("");
    char* segundo = strDuplicate("abc");
    assert(primero != NULL && segundo != NULL);

    char* unido = strConcatenate(primero, segundo);
    assert(unido != NULL);
    assert(unido[0] == 'a');
    assert(unido[1] == 'b');
    assert(unido[2] == 'c');
    assert(unido[3] == '\0');
    free(unido);

    primero = strDuplicate("hola ");
    segundo = strDuplicate("mundo");
    assert(primero != NULL && segundo != NULL);

    unido = strConcatenate(primero, segundo);
    assert(unido != NULL);

    char frase[] = "hola mundo";
    for (int i = 0; i <= 10; i++) {
        assert(unido[i] == frase[i]);
    }
    free(unido);

    printf("strConcatenate: OK\n");
    printf("Tests de strings completados.\n");
    }

void testTablero(void) {
    // Verificamos el estado inicial del tablero.
    GameBoard* tablero = gameBoardNew();
    assert(tablero != NULL);

    assert(tablero->queue_head == NULL);
    assert(tablero->queue_tail == NULL);
    assert(tablero->first_particle == NULL);
    assert(tablero->score == 0);
    assert(tablero->tick_timer == 0);
    assert(tablero->current_piece.active == 0);

    int cantidadFilas = 0;
    RowNode* fila = tablero->first_row;

    while (fila != NULL) {
        cantidadFilas++;
        assert(cantidadFilas <= GRID_ROWS);

        for (int j = 0; j < GRID_COLS; j++) {
            assert(fila->cells[j] == 0);
        }

        fila = fila->next;
    }

    assert(cantidadFilas == GRID_ROWS);

    gameBoardDelete(tablero);

    // Borrar un tablero NULL tambien debe ser valido.
    gameBoardDelete(NULL);

    printf("Creacion y eliminacion del tablero: OK\n");
    }

void testCola(void) {
    // Insertamos cinco piezas y comprobamos el orden FIFO.
    GameBoard* tableroCola = gameBoardNew();
    assert(tableroCola != NULL);

    for (int i = 0; i < 5; i++) {
        gameBoardEnqueuePiece(tableroCola, i);
    }

    int tipoExtraido = -1;

    for (int i = 0; i < 5; i++) {
        int pudoExtraer = gameBoardDequeuePiece(
            tableroCola, &tipoExtraido
        );

        assert(pudoExtraer == 1);
        assert(tipoExtraido == i);
    }

    // La sexta extraccion falla y deja el valor de salida intacto.
    tipoExtraido = 99;
    int pudoExtraer = gameBoardDequeuePiece(tableroCola, &tipoExtraido);
    assert(pudoExtraer == 0);
    assert(tipoExtraido == 99);
    assert(tableroCola->queue_head == NULL);
    assert(tableroCola->queue_tail == NULL);

    // La cola se puede reutilizar despues de vaciarla.
    gameBoardEnqueuePiece(tableroCola, 6);

    assert(tableroCola->queue_head != NULL);
    assert(tableroCola->queue_head == tableroCola->queue_tail);
    pudoExtraer = gameBoardDequeuePiece(tableroCola, &tipoExtraido);
    assert(pudoExtraer == 1);
    assert(tipoExtraido == 6);
    assert(tableroCola->queue_head == NULL);
    assert(tableroCola->queue_tail == NULL);

    // Dejamos piezas pendientes para probar que Delete las libere.
    gameBoardEnqueuePiece(tableroCola, 2);
    gameBoardEnqueuePiece(tableroCola, 4);
    assert(tableroCola->queue_head != NULL);
    assert(tableroCola->queue_tail != NULL);
    assert(tableroCola->queue_head != tableroCola->queue_tail);

    gameBoardDelete(tableroCola);

    printf("Cola de piezas: OK\n");
    }

void testParticulas(void) {
    GameBoard* tableroParticulas = gameBoardNew();
    assert(tableroParticulas != NULL);

    // Primera insercion en una lista vacia.
    gameBoardAddParticle(tableroParticulas, 100.0f, 200.0f, 3);

    ParticleNode* primeraParticula = tableroParticulas->first_particle;
    assert(primeraParticula != NULL);
    assert(primeraParticula->x == 100.0f);
    assert(primeraParticula->y == 200.0f);
    assert(primeraParticula->color == 3);
    assert(primeraParticula->life_timer >= 30);
    assert(primeraParticula->life_timer <= 59);
    assert(primeraParticula->next == NULL);

    // La segunda debe quedar delante de la primera.
    gameBoardAddParticle(tableroParticulas, 150.0f, 250.0f, 5);

    ParticleNode* segundaParticula = tableroParticulas->first_particle;
    assert(segundaParticula != NULL);
    assert(segundaParticula != primeraParticula);
    assert(segundaParticula->x == 150.0f);
    assert(segundaParticula->y == 250.0f);
    assert(segundaParticula->color == 5);
    assert(segundaParticula->next == primeraParticula);
    assert(primeraParticula->next == NULL);

    // Delete debe liberar tambien las particulas pendientes.
    gameBoardDelete(tableroParticulas);

    printf("Lista de particulas: OK\n");
    }

void testLineas(void) {
    // Caso 1: completar y eliminar la ultima fila.
    GameBoard* tableroLineas = gameBoardNew();
    assert(tableroLineas != NULL);

    RowNode* cabezaOriginal = tableroLineas->first_row;
    RowNode* ultima = cabezaOriginal;

    for (int i = 0; i < GRID_ROWS - 1; i++) {
        ultima = ultima->next;
    }

    for (int j = 0; j < GRID_COLS; j++) {
        ultima->cells[j] = 1;
    }

    int eliminadas = gameBoardCheckAndClearLines(tableroLineas);
    assert(eliminadas == 1);

    // La fila nueva esta arriba de la antigua cabeza.
    assert(tableroLineas->first_row != cabezaOriginal);
    assert(tableroLineas->first_row->next == cabezaOriginal);

    for (int j = 0; j < GRID_COLS; j++) {
        assert(tableroLineas->first_row->cells[j] == 0);
    }

    int filasRestantes = 0;
    RowNode* revisarFila = tableroLineas->first_row;

    while (revisarFila != NULL) {
        filasRestantes++;
        assert(filasRestantes <= GRID_ROWS);
        revisarFila = revisarFila->next;
    }

    assert(filasRestantes == GRID_ROWS);

    int cantidadParticulas = 0;
    ParticleNode* revisarParticula = tableroLineas->first_particle;

    while (revisarParticula != NULL) {
        cantidadParticulas++;
        assert(cantidadParticulas <= GRID_COLS);
        revisarParticula = revisarParticula->next;
    }

    assert(cantidadParticulas == GRID_COLS);
    gameBoardDelete(tableroLineas);

    // Caso 2: completar las dos primeras filas.
    tableroLineas = gameBoardNew();
    assert(tableroLineas != NULL);

    RowNode* primeraFila = tableroLineas->first_row;
    RowNode* segundaFila = primeraFila->next;
    RowNode* terceraFila = segundaFila->next;

    for (int j = 0; j < GRID_COLS; j++) {
        primeraFila->cells[j] = 2;
        segundaFila->cells[j] = 3;
    }

    eliminadas = gameBoardCheckAndClearLines(tableroLineas);
    assert(eliminadas == 2);

    // Hay dos filas nuevas vacias antes de la tercera original.
    assert(tableroLineas->first_row->next->next == terceraFila);

    revisarFila = tableroLineas->first_row;
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < GRID_COLS; j++) {
            assert(revisarFila->cells[j] == 0);
        }
        revisarFila = revisarFila->next;
    }

    filasRestantes = 0;
    revisarFila = tableroLineas->first_row;

    while (revisarFila != NULL) {
        filasRestantes++;
        assert(filasRestantes <= GRID_ROWS);
        revisarFila = revisarFila->next;
    }

    assert(filasRestantes == GRID_ROWS);

    cantidadParticulas = 0;
    revisarParticula = tableroLineas->first_particle;

    while (revisarParticula != NULL) {
        cantidadParticulas++;
        assert(cantidadParticulas <= 2 * GRID_COLS);
        revisarParticula = revisarParticula->next;
    }

    assert(cantidadParticulas == 2 * GRID_COLS);

    // Sin filas completas, no debe eliminar ninguna.
    eliminadas = gameBoardCheckAndClearLines(tableroLineas);
    assert(eliminadas == 0);

    gameBoardDelete(tableroLineas);

    printf("Eliminacion de filas completas: OK\n");
}
void correrTests(void) {
    testStrDuplicate();
    testStrCompare();
    testStrConcatenate();
    testTablero();
    testCola();
    testParticulas();
    testLineas();
}

// Compara contra "--test" sin depender de string.h ni del Ejercicio 1.
int esFlagTest(char* arg) {
    char* flag = "--test";
    int i = 0;
    while (arg[i] != '\0' && arg[i] == flag[i]) i++;
    return arg[i] == '\0' && flag[i] == '\0';
}

int main(int argc, char* args[]) {
    srand(time(NULL));

    // Los tests corren antes de SDL. Con --test termina aca, asi valgrind solo
    // reporta la memoria del TP (SDL tiene bloques propios que no se liberan).
    correrTests();
    if (argc > 1 && esFlagTest(args[1])) return 0;

    if (!inicializar()) return 1;

    game_board = gameBoardNew();
    if (game_board == NULL) {
        printf("ADVERTENCIA: Implementa gameBoardNew() para iniciar el juego.\n");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 0;
    }

    for(int i=0; i<3; i++) gameBoardEnqueuePiece(game_board, rand() % 7);

    SDL_Event e;
    int game_over = 0;

    while (!game_over) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) game_over = 1;
            if (e.type == SDL_KEYDOWN && game_board->current_piece.active) {
                if (e.key.keysym.sym == SDLK_LEFT && canMove(game_board, game_board->current_piece.col - 1, game_board->current_piece.row, game_board->current_piece.rotation)) game_board->current_piece.col--;
                if (e.key.keysym.sym == SDLK_RIGHT && canMove(game_board, game_board->current_piece.col + 1, game_board->current_piece.row, game_board->current_piece.rotation)) game_board->current_piece.col++;
                if (e.key.keysym.sym == SDLK_DOWN && canMove(game_board, game_board->current_piece.col, game_board->current_piece.row + 1, game_board->current_piece.rotation)) game_board->current_piece.row++;
                if (e.key.keysym.sym == SDLK_UP) {
                    int next_rot = (game_board->current_piece.rotation + 1) % 4;
                    if (canMove(game_board, game_board->current_piece.col, game_board->current_piece.row, next_rot)) game_board->current_piece.rotation = next_rot;
                }
                if (e.key.keysym.sym == SDLK_SPACE) {
                     while(canMove(game_board, game_board->current_piece.col, game_board->current_piece.row + 1, game_board->current_piece.rotation)) game_board->current_piece.row++;
                     lockPiece(game_board);
                }
            }
        }
        if (!gameBoardUpdate(game_board)) game_over = 1;
        gameBoardDraw(game_board);
        SDL_Delay(16);
    }
    gameBoardDelete(game_board);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
