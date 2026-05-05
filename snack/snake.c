#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

// 游戏区域大小
#define WIDTH 20
#define HEIGHT 20

// 方向枚举
enum Direction { UP, DOWN, LEFT, RIGHT };

// 蛇的节点结构
typedef struct Node {
    int x, y;
    struct Node* next;
} Node;

// 游戏状态结构
typedef struct {
    Node* head;
    Node* tail;
    enum Direction dir;
    int foodX, foodY;
    int score;
    bool gameOver;
    bool started;
} GameState;

// 全局游戏状态
GameState game;

// SDL图形相关常量
#define CELL_SIZE 30
#define WINDOW_WIDTH (WIDTH * CELL_SIZE)
#define WINDOW_HEIGHT (HEIGHT * CELL_SIZE)

// 颜色定义 (RGBA)
#define COLOR_BACKGROUND 0x00, 0x00, 0x00, 0xFF  // 黑色
#define COLOR_SNAKE_HEAD 0xFF, 0x00, 0x00, 0xFF  // 红色
#define COLOR_SNAKE_BODY 0x00, 0xFF, 0x00, 0xFF  // 绿色
#define COLOR_FOOD 0xFF, 0xFF, 0x00, 0xFF        // 黄色
#define COLOR_WALL 0x80, 0x80, 0x80, 0xFF        // 灰色

// SDL全局变量
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

// 函数声明
void initGame();
void freeSnake();
bool isOnSnake(int x, int y);
void generateFood();
void draw();
void processInput();
void update();

// 初始化游戏
void initGame() {
    game.gameOver = false;
    game.started = false;
    printf("initGame: game.gameOver = false, game.started = false\n");
    game.score = 0;
    game.dir = RIGHT;

    // 初始化SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL初始化失败: %s\n", SDL_GetError());
        exit(1);
    }

    window = SDL_CreateWindow("贪吃蛇游戏",
                              100,
                              100,
                              WINDOW_WIDTH,
                              WINDOW_HEIGHT,
                              SDL_WINDOW_SHOWN | SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_ALWAYS_ON_TOP);
    if (window == NULL) {
        fprintf(stderr, "窗口创建失败: %s\n", SDL_GetError());
        SDL_Quit();
        exit(1);
    }
    printf("SDL window created successfully\n");

    // 调试信息：窗口尺寸和位置
    int winW, winH, winX, winY;
    SDL_GetWindowSize(window, &winW, &winH);
    SDL_GetWindowPosition(window, &winX, &winY);
    printf("Window: size=%dx%d, position=(%d,%d), flags=0x%08X\n",
           winW, winH, winX, winY, SDL_GetWindowFlags(window));
    fflush(stdout);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if (renderer == NULL) {
        fprintf(stderr, "渲染器创建失败: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        exit(1);
    }

    // 调试信息：渲染器信息
    SDL_RendererInfo rendererInfo;
    if (SDL_GetRendererInfo(renderer, &rendererInfo) == 0) {
        printf("Renderer: %s, max texture size=%dx%d, flags=0x%08X\n",
               rendererInfo.name, rendererInfo.max_texture_width,
               rendererInfo.max_texture_height, rendererInfo.flags);
    } else {
        printf("Renderer created but info unavailable: %s\n", SDL_GetError());
    }
    fflush(stdout);

    // 创建初始蛇身（3个节点）
    game.head = malloc(sizeof(Node));
    game.head->x = WIDTH / 2;
    game.head->y = HEIGHT / 2;
    game.head->next = NULL;
    game.tail = game.head;

    // 添加第二个节点
    Node* node2 = malloc(sizeof(Node));
    node2->x = game.head->x - 1;
    node2->y = game.head->y;
    node2->next = NULL;
    game.head->next = node2;
    game.tail = node2;

    // 添加第三个节点
    Node* node3 = malloc(sizeof(Node));
    node3->x = node2->x - 1;
    node3->y = node2->y;
    node3->next = NULL;
    node2->next = node3;
    game.tail = node3;

    // 生成食物
    srand(time(NULL));
    generateFood();
}

// 释放蛇节点内存
void freeSnake() {
    Node* current = game.head;
    while (current != NULL) {
        Node* next = current->next;
        free(current);
        current = next;
    }
    game.head = NULL;
    game.tail = NULL;
}

// 检查坐标是否在蛇身上
bool isOnSnake(int x, int y) {
    Node* current = game.head;
    while (current != NULL) {
        if (current->x == x && current->y == y) {
            return true;
        }
        current = current->next;
    }
    return false;
}

// 生成新食物
void generateFood() {
    do {
        game.foodX = rand() % (WIDTH - 2) + 1;  // 避开边界 (1 到 WIDTH-2)
        game.foodY = rand() % (HEIGHT - 2) + 1; // 避开边界 (1 到 HEIGHT-2)
    } while (isOnSnake(game.foodX, game.foodY));
}

// 绘制游戏界面
void draw() {
    static int frameCount = 0;
    frameCount++;
    if (frameCount % 10 == 0) {
        printf("draw() called: frame=%d, started=%d\n", frameCount, game.started);
        fflush(stdout);
    }

    // 清除屏幕为黑色
    SDL_SetRenderDrawColor(renderer, COLOR_BACKGROUND);
    SDL_RenderClear(renderer);

    // 如果游戏未开始，显示提示信息
    if (!game.started) {
        // 绘制半透明覆盖层
        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x80); // 半透明黑色
        SDL_Rect overlay = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
        SDL_RenderFillRect(renderer, &overlay);

        // 这里可以添加文字提示，但需要SDL_ttf库
        // 窗口变暗提示游戏暂停
    }

    // 绘制网格背景（可选）
    SDL_SetRenderDrawColor(renderer, 0x20, 0x20, 0x20, 0xFF); // 深灰色网格
    for (int x = 0; x <= WIDTH; x++) {
        SDL_RenderDrawLine(renderer, x * CELL_SIZE, 0, x * CELL_SIZE, WINDOW_HEIGHT);
    }
    for (int y = 0; y <= HEIGHT; y++) {
        SDL_RenderDrawLine(renderer, 0, y * CELL_SIZE, WINDOW_WIDTH, y * CELL_SIZE);
    }

    // 绘制边框
    SDL_SetRenderDrawColor(renderer, COLOR_WALL);
    SDL_Rect border = {0, 0, WINDOW_WIDTH, CELL_SIZE}; // 上边框
    SDL_RenderFillRect(renderer, &border);
    border.y = WINDOW_HEIGHT - CELL_SIZE; // 下边框
    SDL_RenderFillRect(renderer, &border);
    border.y = 0; border.w = CELL_SIZE; border.h = WINDOW_HEIGHT; // 左边框
    SDL_RenderFillRect(renderer, &border);
    border.x = WINDOW_WIDTH - CELL_SIZE; // 右边框
    SDL_RenderFillRect(renderer, &border);

    // 绘制蛇头
    SDL_SetRenderDrawColor(renderer, COLOR_SNAKE_HEAD);
    SDL_Rect headRect = {
        game.head->x * CELL_SIZE + 1,
        game.head->y * CELL_SIZE + 1,
        CELL_SIZE - 2,
        CELL_SIZE - 2
    };
    SDL_RenderFillRect(renderer, &headRect);

    // 绘制蛇身
    SDL_SetRenderDrawColor(renderer, COLOR_SNAKE_BODY);
    Node* current = game.head->next;
    while (current != NULL) {
        SDL_Rect bodyRect = {
            current->x * CELL_SIZE + 1,
            current->y * CELL_SIZE + 1,
            CELL_SIZE - 2,
            CELL_SIZE - 2
        };
        SDL_RenderFillRect(renderer, &bodyRect);
        current = current->next;
    }

    // 绘制食物
    SDL_SetRenderDrawColor(renderer, COLOR_FOOD);
    SDL_Rect foodRect = {
        game.foodX * CELL_SIZE + 1,
        game.foodY * CELL_SIZE + 1,
        CELL_SIZE - 2,
        CELL_SIZE - 2
    };
    SDL_RenderFillRect(renderer, &foodRect);

    // 显示渲染结果
    SDL_RenderPresent(renderer);
}

// 处理输入
void processInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                printf("processInput: SDL_QUIT received, setting gameOver = true\n");
                game.gameOver = true;
                break;
            case SDL_KEYDOWN:
                printf("processInput: Key pressed: %d\n", event.key.keysym.sym);
                switch (event.key.keysym.sym) {
                    case SDLK_w:
                    case SDLK_UP:
                        if (!game.started) {
                            printf("processInput: Game started! Setting started = true\n");
                            game.started = true;
                        }
                        if (game.dir != DOWN) game.dir = UP;
                        break;
                    case SDLK_s:
                    case SDLK_DOWN:
                        if (!game.started) {
                            printf("processInput: Game started! Setting started = true\n");
                            game.started = true;
                        }
                        if (game.dir != UP) game.dir = DOWN;
                        break;
                    case SDLK_a:
                    case SDLK_LEFT:
                        if (!game.started) {
                            printf("processInput: Game started! Setting started = true\n");
                            game.started = true;
                        }
                        if (game.dir != RIGHT) game.dir = LEFT;
                        break;
                    case SDLK_d:
                    case SDLK_RIGHT:
                        if (!game.started) {
                            printf("processInput: Game started! Setting started = true\n");
                            game.started = true;
                        }
                        if (game.dir != LEFT) game.dir = RIGHT;
                        break;
                    case SDLK_q:
                    case SDLK_ESCAPE:
                        printf("processInput: Q or ESC pressed, setting gameOver = true\n");
                        game.gameOver = true;
                        break;
                }
                break;
        }
    }
}

// 更新游戏状态
void update() {
    if (!game.started) {
        printf("update: game not started yet, skipping update\n");
        return; // 游戏未开始，不更新
    }
    printf("update: game is started, updating...\n");
    // 计算新的蛇头位置
    // printf("update: head at (%d, %d), dir=%d\n", game.head->x, game.head->y, game.dir);
    int newX = game.head->x;
    int newY = game.head->y;

    switch (game.dir) {
        case UP: newY--; break;
        case DOWN: newY++; break;
        case LEFT: newX--; break;
        case RIGHT: newX++; break;
    }

    // 检查撞墙
    if (newX < 0 || newX >= WIDTH || newY < 0 || newY >= HEIGHT) {
        printf("update: wall collision at (%d, %d), setting gameOver = true\n", newX, newY);
        game.gameOver = true;
        return;
    }

    // 检查撞到自己
    if (isOnSnake(newX, newY)) {
        printf("update: self collision at (%d, %d), setting gameOver = true\n", newX, newY);
        game.gameOver = true;
        return;
    }

    // 创建新蛇头
    Node* newHead = malloc(sizeof(Node));
    newHead->x = newX;
    newHead->y = newY;
    newHead->next = game.head;
    game.head = newHead;

    // 检查是否吃到食物
    if (newX == game.foodX && newY == game.foodY) {
        game.score += 10;
        printf("Score: %d\n", game.score);  // 只在分数变化时输出
        generateFood();
    } else {
        // 没吃到食物，移除蛇尾
        Node* current = game.head;
        while (current->next->next != NULL) {
            current = current->next;
        }
        free(game.tail);
        current->next = NULL;
        game.tail = current;
    }
}

int main() {
    initGame();
    printf("Main loop starting. Game is paused. Press any arrow key to start, then use arrow keys to control, Q or ESC to quit.\n");

    // 时间控制变量
    Uint32 lastUpdateTime = SDL_GetTicks();  // 上次游戏更新时间
    Uint32 lastRenderTime = SDL_GetTicks();  // 上次渲染时间
    const Uint32 UPDATE_INTERVAL = 150;      // 游戏更新间隔 (ms) - 控制蛇移动速度
    const Uint32 RENDER_INTERVAL = 16;       // 渲染间隔 (ms) ~60 FPS

    while (!game.gameOver) {
        Uint32 currentTime = SDL_GetTicks();

        // 总是处理输入（最频繁）
        processInput();

        // 按UPDATE_INTERVAL更新游戏状态
        if (currentTime - lastUpdateTime >= UPDATE_INTERVAL) {
            update();
            lastUpdateTime = currentTime;
        }

        // 按RENDER_INTERVAL渲染（保持流畅显示）
        if (currentTime - lastRenderTime >= RENDER_INTERVAL) {
            draw();
            lastRenderTime = currentTime;
        }

        // 小延迟避免CPU占用过高
        SDL_Delay(1);
    }

    freeSnake();

    // 清理SDL资源
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = NULL;
    }
    if (window) {
        SDL_DestroyWindow(window);
        window = NULL;
    }
    SDL_Quit();

    printf("Game Over! Final Score: %d\n", game.score);

    return 0;
}