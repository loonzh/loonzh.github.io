#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>

#define HEAD 1001
#define BODY 1002
#define WALL 1003
#define FOOD 1004
#define BLANK 1005
#define UP 2001
#define DOWN 2002
#define LEFT 2003
#define RIGHT 2004
#define RESET "\033[0m"
#define GREEN "\033[32m"
#define WHITE "\033[37m"
#define WIDTH 40
#define HEIGHT 30
#define MAX_LENGTH 1024

typedef struct {
  int x, y;
} Point;

typedef struct {
  int length;
  Point body[MAX_LENGTH];
} Snake; //蛇结构体包含点结构体数组和长度

void HideCursor() { //隐藏光标
  HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
  CONSOLE_CURSOR_INFO info = {1, FALSE};
  SetConsoleCursorInfo(consoleHandle, &info);
}

void SetWindowSize(int width, int height) { //设置控制台窗口大小
  HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
  SMALL_RECT DisplayArea = {0, 0, width - 1, height - 1};
  SetConsoleWindowInfo(hOut, TRUE, &DisplayArea);
}

void MoveCursor(int x, int y) { //移动光标到特定位置
  COORD pos = {x, y};
  SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void InitMap(int map[HEIGHT][WIDTH]) { //初始化地图上点的状态
  for (int i = 0; i < HEIGHT; ++i)
    for (int j = 0; j < WIDTH; ++j)
      map[i][j] = (i == 0 || i == HEIGHT - 1 || j == 0 || j == WIDTH - 1) ? WALL : BLANK;
}

void PrintMap(int map[HEIGHT][WIDTH]) { //打印地图上点的状态
  MoveCursor(0, 0);
  for (int i = 0; i < HEIGHT; ++i) {
    for (int j = 0; j < WIDTH; ++j) {
      if (map[i][j] == BLANK) printf("  ");
      else if (map[i][j] == WALL) printf(WHITE "■" RESET);
      else if (map[i][j] == HEAD) printf(WHITE "■" RESET);
      else if (map[i][j] == BODY) printf(WHITE "□" RESET);
      else if (map[i][j] == FOOD) printf(GREEN "●" RESET);
    }
    printf("\n");
  }
}

void PrintScore(int score) { //打印分数
  MoveCursor(0, HEIGHT);
  printf("得分: %d\n", score);
}

void InitSnake(int map[HEIGHT][WIDTH], Snake *snake) { //初始化蛇的长度和状态
  snake->length = 3;
  snake->body[0] = (Point){WIDTH / 2, HEIGHT / 2};
  snake->body[1] = (Point){WIDTH / 2 - 1, HEIGHT / 2};
  snake->body[2] = (Point){WIDTH / 2 - 2, HEIGHT / 2};

  map[HEIGHT / 2][WIDTH / 2] = HEAD;
  map[HEIGHT / 2][WIDTH / 2 - 1] = BODY;
  map[HEIGHT / 2][WIDTH / 2 - 2] = BODY;
}

void GenerateFood(int map[HEIGHT][WIDTH]) { //在随机空白处制造食物
  int x, y;
  srand(time(0));
  do {
    x = rand() % (WIDTH - 2) + 1;
    y = rand() % (HEIGHT - 2) + 1;
  } while (map[y][x] != BLANK);
  map[y][x] = FOOD;
}

int MoveSnake(int map[HEIGHT][WIDTH], Snake *snake, int direction, int score) { //移动蛇
  Point new_head = snake->body[0];

  switch (direction) {
    case UP: new_head.y -= 1; break;
    case DOWN: new_head.y += 1; break;
    case LEFT: new_head.x -= 1; break;
    case RIGHT: new_head.x += 1; break;
  }

  if (map[new_head.y][new_head.x] == WALL || map[new_head.y][new_head.x] == BODY) exit(0);
  if (map[new_head.y][new_head.x] == FOOD) {
    snake->length++;
    score++;
    GenerateFood(map);
    PrintScore(score);
  } else map[snake->body[snake->length - 1].y][snake->body[snake->length - 1].x] = BLANK;

  for (int i = snake->length - 1; i > 0; i--)
    snake->body[i] = snake->body[i - 1];

  snake->body[0] = new_head;
  map[new_head.y][new_head.x] = HEAD;
  map[snake->body[1].y][snake->body[1].x] = BODY;

  return score;
}

int main() {
  int map[HEIGHT][WIDTH];
  Snake snake;
  int direction = RIGHT;
  int score = 0;

  SetWindowSize(WIDTH * 2, HEIGHT + 2);
  HideCursor();
  system("cls");

  InitMap(map);
  InitSnake(map, &snake);
  GenerateFood(map);
  PrintMap(map);
  PrintScore(score);

  while (1) { //持续移动蛇直至死亡
    if (GetAsyncKeyState(VK_UP) & 0x8000 && direction != DOWN) direction = UP;
    if (GetAsyncKeyState(VK_DOWN) & 0x8000 && direction != UP) direction = DOWN;
    if (GetAsyncKeyState(VK_LEFT) & 0x8000 && direction != RIGHT) direction = LEFT;
    if (GetAsyncKeyState(VK_RIGHT) & 0x8000 && direction != LEFT) direction = RIGHT;
    score = MoveSnake(map, &snake, direction, score);
    PrintMap(map);
    PrintScore(score);

    Sleep(10); // 每10毫秒移动一次
  }
  return 0;
}
