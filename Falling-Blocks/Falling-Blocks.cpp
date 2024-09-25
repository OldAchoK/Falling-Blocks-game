#include <iostream>
#include <Windows.h>
#include <cstdlib>
#include <time.h>
#include <vector>

void gotoxy(int x, int y) {
    COORD pos = { x, y };
    HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleCursorPosition(output, pos);
}

const short HEIGHT = 20;
const short WIDTH = 20;
const short MAXSIZE = HEIGHT * WIDTH;
const short SPEED = 1;
const short HITBOXLENGTH = 5;
const short HITBOXWIDTH = 5;
const double STRAFETIME = 0.1;
const double FALLTIME = 0.5;

const enum KEYS
{
    LEFT = -1,
    STOP,
    RIGHT,
    DEFAULT,
    FALL
};

struct Dot
{
public:
    int x, y;
    Dot()
    {
        this->x = NULL;
        this->y = NULL;
    }
    Dot(int i, int j)
    {
        this->x = i;
        this->y = j;
    }
};

class Rasterize
{
private:
    std::string map;
public:
    Rasterize()
    {
        map = "";
    }
    std::string DrawMap()
    {
        return this->map;
    }
    void EndScreen()
    {
        this->map = "";
        int i = 0;
        char cache = '-';
        for (int i = 0; i < HEIGHT; i++)
        {
            if (i == HEIGHT / 2)
                map += "#-----GAME-OVER----#\n";
            else
            for (int j = 0; j < WIDTH + 3; j++)
            {
                cache = '-';
                if (j == 0 || j == WIDTH + 1)
                    cache = '#';
                if (j == WIDTH + 2)
                    cache = '\n';
                map += cache;
            }
        }
    }
    void FillMap()
    {
        this->map = "";
        int i = 0;
        char cache = '-';
        for (int i = 0; i < HEIGHT; i++)
        {
            for (int j = 0; j < WIDTH + 3; j++)
            {
                cache = '-';
                if (j == 0 || j == WIDTH + 1)
                    cache = '#';
                if (j == WIDTH + 2)
                    cache = '\n';
                map += cache;
            }
        }
    }
    void DrawDrop(std::vector<Dot> pos)
    {
        if (pos.size() > 0)
        {
            for (int i = 0; i < pos.size(); i++)
            {
                map[pos[i].y * (WIDTH + 3) + pos[i].x + 1] = '1';
            }
        }
    }
    void DrawBottle(bool** walls)
    {
        for (int i = 0; i < HEIGHT; i++)
        {
            for (int j = 0; j < WIDTH; j++)
            {
                if (walls[i][j])
                    map[i * (WIDTH + 3) + j + 1] = '#';
            }
        }
    }
};

class Bottle
{
private:
    bool** walls;
public:
    Bottle()
    {
        walls = new bool* [HEIGHT];
        for (int i = 0; i < HEIGHT; ++i) {
            walls[i] = new bool[WIDTH];
        }
        for (int i = 0; i < HEIGHT; i++) {
            for (int j = 0; j < WIDTH; j++) {
                walls[i][j] = false;
                if (i == HEIGHT - 1)
                    walls[i][j] = true;
            }
        }
    }
    ~Bottle()
    {
        for (size_t i = 0; i < HEIGHT; ++i) {
            delete[] walls[i];
        }
        delete[] walls;
    }

    bool** Walls()
    {
        return walls;
    }

    short CheckLevel(Dot pos)
    {
        if (walls[pos.y + 1][pos.x])
        {
            return STOP;
        }
        return DEFAULT;
    }
    short CheckStrafe(Dot pos)
    {
        short answer = DEFAULT;
        if (!walls[pos.y][pos.x - 1] && walls[pos.y][pos.x + 1] || pos.x == WIDTH - 1)
        {
            answer = RIGHT;
        }
        if (!walls[pos.y][pos.x + 1] && walls[pos.y][pos.x - 1] || pos.x == 0)
        {
            answer = LEFT;
        }
        return answer;
    }
    bool CheckMatches()
    {
        short index = -1;
        for (int i = 0; i < HEIGHT - 1; i++)
        {
            index = 0;
            for (int j = 0; j < WIDTH - 2; j++)
            {

                if (walls[i][j] == walls[i][j + 1] && walls[i][j])
                {
                    index++;
                    if (index == WIDTH - 2)
                    {
                        this->ClearLine(i);
                        return true;
                    }
                }
            }
        }
        return false;
    }
    void ClearLine(short row)
    {
        for (int i = row; i > 0; i--)
        {
            for (int j = 0; j < WIDTH; j++)
            {
                if (i == 1)
                {
                    this->walls[i][j] = false;
                }
                else
                {
                    this->walls[i][j] = this->walls[i - 1][j];
                }
            }
        }
    }
    bool CheckEnd()
    {
        for (int i = 0; i < WIDTH; i++)
        {
            if (walls[2][i])
                return true;
        }
        return false;
    }
    void AddElement(std::vector<Dot> figure)
    {
        for (int i = 0; i < figure.size(); i++)
        {
            walls[figure[i].y][figure[i].x] = true;
        }
    }
};

Bottle level;

class Drop
{
public:
    short type;
    bool form[HITBOXLENGTH][HITBOXWIDTH];
    std::vector<Dot> figurePos;
    Dot HitboxPos;
    short speed;
    short dir;
    short length;
    short swipelength;

    Drop()
    {
        this->figurePos.clear();
        this->type = -1;
        this->HitboxPos = Dot(WIDTH / 2, 0);
        this->dir = DEFAULT;
        this->speed = SPEED;
        this->swipelength = -1;
        this->length = -1;
        for (int i = 0; i < HITBOXLENGTH; i++)
        {
            for (int j = 0; j < HITBOXWIDTH; j++)
            {
                this->form[i][j] = false;
            }
        }
    }
    ~Drop()
    {
        /*this->figurePos.clear();
        this->type = -1;
        this->HitboxPos = Dot(WIDTH / 2, 0);
        this->dir = DEFAULT;
        this->speed = SPEED;
        for (int i = 0; i < HITBOXLENGTH; i++)
        {
            for (int j = 0; j < HITBOXWIDTH; j++)
            {
                this->form[i][j] = false;
            }
        }*/
    }
    bool DefineType(int num)
    {

        switch (num)
        {
        case 0:
        {
            figurePos.clear();
            this->type = num;
            form[HITBOXLENGTH / 2][HITBOXWIDTH / 2 - 1] = 1;
            form[HITBOXLENGTH / 2][HITBOXWIDTH / 2] = 1;
            form[HITBOXLENGTH / 2][HITBOXWIDTH / 2 + 1] = 1;
            figurePos.push_back(Dot(HitboxPos.x - 1, HitboxPos.y + HITBOXLENGTH / 2));
            figurePos.push_back(Dot(HitboxPos.x, HitboxPos.y + HITBOXLENGTH / 2));
            figurePos.push_back(Dot(HitboxPos.x + 1, HitboxPos.y + HITBOXLENGTH / 2));
            this->swipelength = 3;
            break;
        }
        case 1:
        {
            this->type = num;
            form[HITBOXLENGTH / 2 - 1][HITBOXWIDTH / 2] = 1;
            form[HITBOXWIDTH / 2][HITBOXWIDTH / 2 - 1] = 1;
            form[HITBOXWIDTH / 2][HITBOXWIDTH / 2] = 1;
            form[HITBOXWIDTH / 2][HITBOXWIDTH / 2 + 1] = 1;
            figurePos.push_back(Dot(HitboxPos.x - 1, HitboxPos.y + HITBOXLENGTH / 2));
            figurePos.push_back(Dot(HitboxPos.x, HitboxPos.y + HITBOXLENGTH / 2));
            figurePos.push_back(Dot(HitboxPos.x + 1, HitboxPos.y + HITBOXLENGTH / 2));
            figurePos.push_back(Dot(HitboxPos.x, HitboxPos.y + HITBOXLENGTH / 2 - 1));
            this->swipelength = 3;
            break;
        }
        case 2:
        {
            this->type = num;
            form[HITBOXLENGTH / 2][HITBOXWIDTH / 2 - 2] = 1;
            form[HITBOXLENGTH / 2][HITBOXWIDTH / 2 - 1] = 1;
            form[HITBOXLENGTH / 2][HITBOXWIDTH / 2] = 1;
            form[HITBOXLENGTH / 2][HITBOXWIDTH / 2 + 1] = 1;
            form[HITBOXLENGTH / 2][HITBOXWIDTH / 2 + 2] = 1;
            figurePos.push_back(Dot(HitboxPos.x - 2, HitboxPos.y + HITBOXLENGTH / 2));
            figurePos.push_back(Dot(HitboxPos.x - 1, HitboxPos.y + HITBOXLENGTH / 2));
            figurePos.push_back(Dot(HitboxPos.x, HitboxPos.y + HITBOXLENGTH / 2));
            figurePos.push_back(Dot(HitboxPos.x + 1, HitboxPos.y + HITBOXLENGTH / 2));
            figurePos.push_back(Dot(HitboxPos.x + 2, HitboxPos.y + HITBOXLENGTH / 2));
            this->swipelength = 5;
            break;
        }
        case 3:
        {
            this->type = num;
            form[HITBOXLENGTH / 2 - 1][HITBOXWIDTH / 2 + 1] = 1;
            form[HITBOXLENGTH / 2 - 1][HITBOXWIDTH / 2] = 1;
            form[HITBOXLENGTH / 2][HITBOXWIDTH / 2] = 1;
            form[HITBOXLENGTH / 2][HITBOXWIDTH / 2 + 1] = 1;
            figurePos.push_back(Dot(HitboxPos.x + 1, HitboxPos.y + HITBOXLENGTH / 2));
            figurePos.push_back(Dot(HitboxPos.x, HitboxPos.y + HITBOXLENGTH / 2));
            figurePos.push_back(Dot(HitboxPos.x, HitboxPos.y + HITBOXLENGTH / 2 - 1));
            figurePos.push_back(Dot(HitboxPos.x + 1, HitboxPos.y + HITBOXLENGTH / 2 - 1));
            this->swipelength = 2;
            break;
        }
        case 4:
        {
            this->type = num;
            form[HITBOXLENGTH / 2 - 1][HITBOXWIDTH / 2 - 1] = 1;
            form[HITBOXLENGTH / 2 - 1][HITBOXWIDTH / 2] = 1;
            form[HITBOXLENGTH / 2][HITBOXWIDTH / 2] = 1;
            form[HITBOXLENGTH / 2][HITBOXWIDTH / 2 + 1] = 1;
            figurePos.push_back(Dot(HitboxPos.x - 1, HitboxPos.y + HITBOXLENGTH / 2));
            figurePos.push_back(Dot(HitboxPos.x, HitboxPos.y + HITBOXLENGTH / 2));
            figurePos.push_back(Dot(HitboxPos.x, HitboxPos.y + HITBOXLENGTH / 2 - 1));
            figurePos.push_back(Dot(HitboxPos.x + 1, HitboxPos.y + HITBOXLENGTH / 2 - 1));
            this->swipelength = 3;
            break;
        }
        case 5:
        {
            this->type = num;
            form[HITBOXLENGTH / 2 - 1][HITBOXWIDTH / 2 + 1] = 1;
            form[HITBOXLENGTH / 2][HITBOXWIDTH / 2] = 1;
            form[HITBOXLENGTH / 2][HITBOXWIDTH / 2 - 1] = 1;
            form[HITBOXLENGTH / 2][HITBOXWIDTH / 2 + 1] = 1;
            figurePos.push_back(Dot(HitboxPos.x - 1, HitboxPos.y + HITBOXLENGTH / 2));
            figurePos.push_back(Dot(HitboxPos.x, HitboxPos.y + HITBOXLENGTH / 2));
            figurePos.push_back(Dot(HitboxPos.x + 1, HitboxPos.y + HITBOXLENGTH / 2));
            figurePos.push_back(Dot(HitboxPos.x + 1, HitboxPos.y + HITBOXLENGTH / 2 - 1));
            this->swipelength = 3;
            break;
        }
        default:
        {
            break;
        }
        }
        return level.CheckMatches();
    }
    bool Swipe()
    {
        if (this->type != -1)
        {
            bool copy[HITBOXLENGTH][HITBOXWIDTH];
            for (int i = 0; i < HITBOXLENGTH; i++)
            {
                for (int j = 0; j < HITBOXWIDTH; j++)
                {
                    copy[i][j] = this->form[HITBOXWIDTH - 1 - j][i];
                }
            }
            for (int i = 0; i < HITBOXLENGTH; i++)
            {
                for (int j = 0; j < HITBOXWIDTH; j++)
                {
                    if (copy[i][j] == 1 && level.CheckLevel(Dot(HitboxPos.x + j, HitboxPos.y + i)) == STOP || level.CheckStrafe(Dot(HitboxPos.x + this->swipelength / 2 - 1, HitboxPos.y + this->swipelength / 2)) != DEFAULT)
                        return true;
                }
            }
            for (int i = 0; i < HITBOXLENGTH; i++)
            {
                for (int j = 0; j < HITBOXWIDTH; j++)
                {
                    this->form[i][j] = copy[i][j];
                }
            }
            return false;
        }
        else return true;
    }
    void Rotate(bool check)
    {
        if (!check)
        {
            this->figurePos.clear();
            for (int i = 0; i < HITBOXLENGTH; i++)
            {
                for (int j = 0; j < HITBOXWIDTH; j++)
                {
                    if (this->form[i][j] == 1)
                    {
                        this->figurePos.push_back(Dot(HitboxPos.x + j, HitboxPos.y + i));
                    }
                }
            }
        }
    }
    void Strafe(short input)
    {
        for (int i = 0; i < figurePos.size(); i++)
        {
            if (input != FALL)
            {
                if (level.CheckStrafe(this->figurePos[i]) == input)
                {
                    this->dir = DEFAULT;
                    break;
                }
                else
                    this->dir = input;
            }
            else
            {
                if (level.CheckLevel(this->figurePos[i]) == STOP)
                {
                    this->dir = STOP;
                    break;
                }
                else
                    this->dir = input;
            }
        }
    }
    void Fall()
    {
        switch (this->dir)
        {
        case DEFAULT:
        {
            this->dir = FALL;
            break;
        }
        case STOP:
        {
            break;
        }
        }
    }
    void Move()
    {
        switch (dir)
        {
        case FALL:
        {
            for (int i = 0; i < figurePos.size(); i++)
            {
                level.CheckLevel(figurePos[i]);
                this->figurePos[i].y += speed;
            }
            this->HitboxPos.y += speed;
            this->dir = DEFAULT;
            break;
        }
        case DEFAULT:
        {
            break;
        }
        default:
        {
            for (int i = 0; i < figurePos.size(); i++)
            {
                if (this->dir != FALL)
                    this->figurePos[i].x += this->speed * this->dir;
            }
            for (int i = 0; i < figurePos.size(); i++)
            {
                if (level.CheckLevel(this->figurePos[i]) == STOP)
                {
                    this->dir = STOP;
                    break;
                }
            }
            if (this->dir != FALL)
                this->HitboxPos.x += this->speed * this->dir;
            this->dir = DEFAULT;
            break;
        }
        }
    }
    bool Status()
    {
        bool answer = false;
        for (int i = 0; i < figurePos.size(); i++)
        {
            if (level.CheckLevel(figurePos[i]) == STOP)
            {
                level.AddElement(figurePos);
                answer = true;
                break;
            }
        }
        return answer;
    }
};

class Game
{
public:
    Drop* figure;
    Rasterize map();

    bool kill()
    {
        if (level.CheckEnd())
            return false;
        return true;
    }


};



int main()
{
    bool isRunning = true;
    int score = 0;
    Game start;
    Drop* figure = new Drop();
    Rasterize mape;
    figure->DefineType(2);
    double strafeTime = clock();
    double time = clock();
    std::vector<Dot> ass;
    while (isRunning)
    {

        if ((clock() - strafeTime) / CLOCKS_PER_SEC >= STRAFETIME)
        {

            strafeTime = clock();
            mape.FillMap();
            ass = figure->figurePos;
            mape.DrawDrop(ass);
            mape.DrawBottle(level.Walls());
            std::cout << mape.DrawMap();
            if (GetKeyState('A') & 0x8000)
            {
                figure->Strafe(LEFT);
            }
            if (GetKeyState('R') & 0x8000)
            {
                figure->Rotate(figure->Swipe());
            }
            if (GetKeyState('D') & 0x8000)
            {
                figure->Strafe(RIGHT);
            }
            if (GetKeyState('S') & 0x8000)
            {
                figure->Strafe(FALL);
            }
            figure->Move();

            std::cout << " Score: " << score;
            if (figure->Status())
            {
                if (level.CheckEnd())
                {
                    isRunning = false;
                    break;
                }
                delete figure;
                figure = new Drop();
                srand(time);
                if (figure->DefineType(rand() % 5))
                    score += WIDTH;
            }
            gotoxy(0, 0);
        }
        if ((clock() - time) / CLOCKS_PER_SEC >= FALLTIME)
        {
            time = clock();
            figure->Fall();
            figure->Move();
        }
    }
    if (!isRunning)
        mape.EndScreen();
    gotoxy(0, 0);
    std::cout << mape.DrawMap();
    system("pause");
    return 0;
    }