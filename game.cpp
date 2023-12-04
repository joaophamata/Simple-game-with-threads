#include <iostream>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <cmath>
#include <chrono>
#include <unistd.h>
#include <ncurses.h>
#include <random>
#include <string>

#define HEIGHT 40
#define WIDTH 120

int killCount = 0;
int lives = 3;
std::string nebulosa = "roxo";
std::vector<std::vector<bool>> conwayBoard(HEIGHT, std::vector<bool>(WIDTH, false));

void goToXY(int x, int y) {
    std::cout << "\033[" << y + 1 << ";" << x + 1 << "H";
}

void hideCursor() {
    std::cout << "\033[?25l"; // Oculta o cursor
}

void showCursor() {
    std::cout << "\033[?25h"; // Mostra o cursor
}

void drawConwayGame(std::vector<std::vector<bool>>& board) {
    if (nebulosa == "azul") {
        std::cout << "\033[1;35m"; // Roxo
    }
    else if (nebulosa == "vermelho"){
        std::cout << "\033[1;33m"; // Laranja
    }
    else {
        std::cout << "\033[1;32m"; // Verde
    }

    for (int i = 0; i < HEIGHT; ++i) {
        for (int j = 0; j < WIDTH; ++j) {
            goToXY(j, i);
            if (board[i][j]) {
                std::cout << "#"; // Célula viva: '#' roxo
            } else {
                std::cout << " "; // Célula morta
            }
        }
    }
}

void updateConwayGame(std::vector<std::vector<bool>>& board) {
    std::vector<std::vector<bool>> newBoard(HEIGHT, std::vector<bool>(WIDTH, false));

    for (int i = 0; i < HEIGHT; ++i) {
        for (int j = 0; j < WIDTH; ++j) {
            int neighbors = 0;

            for (int di = -1; di <= 1; ++di) {
                for (int dj = -1; dj <= 1; ++dj) {
                    if (di == 0 && dj == 0) continue;

                    int ni = (i + di + HEIGHT) % HEIGHT;
                    int nj = (j + dj + WIDTH) % WIDTH;

                    if (board[ni][nj]) {
                        neighbors++;
                    }
                }
            }

            if (board[i][j]) {
                if (neighbors == 2 || neighbors == 3) {
                    newBoard[i][j] = true;
                }
            } else {
                if (neighbors == 3) {
                    newBoard[i][j] = true;
                }
            }
        }
    }

    board = newBoard;
}


struct UFO {
    int x;
    int y;
    int directionX;
    int directionY;
    bool active;
    bool canShoot;
    std::chrono::steady_clock::time_point lastShootTime;
};

struct Shot {
    int x;
    int y;
    int directionX;
    int directionY;
    bool active;
    std::chrono::steady_clock::time_point startTime;
    std::string color; // Novo atributo para armazenar a cor do tiro do foguete
};

bool checkConwayCollision(int x, int y, const std::vector<std::vector<bool>>& conwayBoard) {
    // Verifica se há colisão com a célula de Conway
    return conwayBoard[y][x];
}

void drawBorder(int width, int height) {
    std::cout << "\033[1;37m"; // Define a cor do texto para branco

    // Desenha a borda superior e inferior
    for (int i = 0; i < width; ++i) {
        goToXY(i, 0);
        std::cout << "-";
        goToXY(i, height - 1);
        std::cout << "-";
    }

    // Desenha a borda esquerda e direita
    for (int i = 1; i < height - 1; ++i) {
        goToXY(0, i);
        std::cout << "|";
        goToXY(width - 1, i);
        std::cout << "|";
    }
    
    // Desenha a legenda do placar
    goToXY(width + 1, 1);
    std::cout << "Placar";
    goToXY(width + 1, 3);
    std::cout << "Kills: " << killCount;

    // Desenha a quantidade de vidas
    goToXY(width + 1, 5);
    std::cout << "Vidas: " << lives;

    // // Desenha a quantidade de vidas
    // goToXY(width + 1, 7);
    // std::cout << "Nebulosa: " << nebulosa;
}

void updateScore(int width, int killCount, int lives) {
    goToXY(width + 8, 3); // Posição para exibir a pontuação
    std::cout << killCount; // Exibe a contagem de kills

    goToXY(width + 8, 5); // Posição para exibir a quantidade de vidas
    std::cout << lives; // Exibe a quantidade de vidas

    // goToXY(width + 8, 7); // Posição para exibir a quantidade de vidas
    // std::cout << nebulosa; // Exibe a quantidade de vidas
}

void drawExplosion(int x, int y) {
    goToXY(x, y);
    std::cout << "\U0001F4A5"; // Emoji de explosão Unicode
    std::cout << "\a" << std::endl;
}

void shoot(std::vector<Shot>& shots, int x, int y, int targetX, int targetY) {
    Shot shot;
    shot.x = x;
    shot.y = y;
    int dx = targetX - x;
    int dy = targetY - y;
    float magnitude = sqrt(dx * dx + dy * dy);
    shot.directionX = static_cast<int>(round(dx / magnitude));
    shot.directionY = static_cast<int>(round(dy / magnitude));
    shot.active = true;
    shot.startTime = std::chrono::steady_clock::now();
    shots.push_back(shot);
}

void initializeConwayGame(std::vector<std::vector<bool>>& board, double percentageAlive) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);

    for (int i = 0; i < HEIGHT; ++i) {
        for (int j = 0; j < WIDTH; ++j) {
            if (dis(gen) < percentageAlive - 0.1) {
                board[i][j] = true;
            }
        }
    }
}

void rocketShoot(std::vector<Shot>& shots, int x, int y, int directionX, int directionY, const std::string& color) {
    Shot shot;
    shot.x = x;
    shot.y = y;
    shot.directionX = directionX;
    shot.directionY = directionY;
    shot.active = true;
    shot.startTime = std::chrono::steady_clock::now();
    shot.color = color; // Define a cor do tiro
    shots.push_back(shot);
}

void moveRocket(int& x, int& y, bool& running, std::vector<Shot>& shots, const std::vector<std::vector<bool>>& conwayBoard, std::string& nebulosa) {
    const int moveStep = 1; // Define o tamanho do passo do movimento
    char direction = getch();
    goToXY(x, y);
    std::cout << " "; // Emoji Unicode do foguete

    // Salva a posição anterior do foguete
    int prevX = x;
    int prevY = y;

    // Verifica se há colisão com a célula de Conway
    bool collided = checkConwayCollision(x, y, conwayBoard);

    if (collided && nebulosa == "roxo") {
        nebulosa = "azul";
    } else if (nebulosa != "vermelho" && !collided){
        nebulosa = "roxo";
    }


    switch (direction) {
        case 'w':
            y = (y > 1) ? y - moveStep : y; // Movimento para cima respeitando a borda superior
            break;
        case 's':
            y = (y < HEIGHT - 2) ? y + moveStep : y; // Movimento para baixo respeitando a borda inferior
            break;
        case 'a':
            x = (x > 1) ? x - moveStep : x; // Movimento para esquerda respeitando a borda esquerda
            break;
        case 'd':
            x = (x < WIDTH - 2) ? x + moveStep : x; // Movimento para direita respeitando a borda direita
            break;
        case 'q':
            running = false;
            break;
        case 'j': // Disparo para esquerda
            rocketShoot(shots, x, y, -1, 0, "green"); // Direção (-1, 0) e cor verde
            break;
        case 'i': // Disparo para cima
            rocketShoot(shots, x, y, 0, -1, "green"); // Direção (0, -1) e cor verde
            break;
        case 'l': // Disparo para direita
            rocketShoot(shots, x, y, 1, 0, "green"); // Direção (1, 0) e cor verde
            break;
        case 'k': // Disparo para baixo
            rocketShoot(shots, x, y, 0, 1, "green"); // Direção (0, 1) e cor verde
            break;
    }

    // Desenha o foguete apenas se a nova posição estiver dentro dos limites
    if (x != prevX || y != prevY) {
        goToXY(x, y);
        std::cout << "\U0001F680"; // Emoji Unicode do foguete
    }

}

void moveShots(std::vector<Shot>& shots, std::vector<UFO>& UFOs) {
    for (size_t i = 0; i < shots.size(); ++i) {
        if (shots[i].active) {
            shots[i].x += shots[i].directionX;
            shots[i].y += shots[i].directionY;

            // Verifica colisão com os UFOs e desativa o tiro se atingir o próprio UFO
            for (auto& ufo : UFOs) {
                if (ufo.active == true && ufo.x == shots[i].x && ufo.y == shots[i].y) {
                    goToXY(ufo.x, ufo.y);
                    std::cout << " "; // Emoji Unicode do foguete
                    if (shots[i].color == "green") { // Verifica se é um tiro verde
                        ufo.active = false;
                        killCount++;
                        shots[i].active = false; // Desativa o tiro
                        drawExplosion(ufo.x, ufo.y);
                    } else {
                        shots[i].active = false; // Desativa apenas o tiro
                    }
                    break; // Um tiro atingiu um UFO, então não é necessário continuar verificando
                }
            }
        }
    }
}

bool hitBorder(int x, int y, int width, int height) {
    return (x <= 0 || x >= width - 1 || y <= 0 || y >= height - 1);
}

int main() {
    int numUFOs;
    std::cout << "Quantos UFOs deseja criar? ";
    std::cin >> numUFOs;

    initscr();
    noecho();
    curs_set(0); // Esconde o cursor
    keypad(stdscr, TRUE);
    timeout(0); // Configura para não esperar por input

    hideCursor(); // Oculta o cursor antes do início da animação

    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    const int width = WIDTH;
    const int height = HEIGHT;

    // Define a porcentagem de células vivas desejada (entre 0.0 e 1.0)
    double percentageAlive = 0.3; // Por exemplo, 30% das células estarão vivas

    initializeConwayGame(conwayBoard, percentageAlive);

    std::vector<UFO> UFOs(numUFOs);
    std::vector<Shot> shots; // Vetor para armazenar os tiros
    std::vector<Shot> rocketShots; // Vetor para armazenar os tiros do foguete

    // Definir posições aleatórias para cada UFO
    for (int i = 0; i < numUFOs; ++i) {
        UFOs[i].x = std::rand() % (width - 2) + 1; // Evita a borda esquerda e direita
        UFOs[i].y = std::rand() % (height - 2) + 1; // Evita a borda superior e inferior
        UFOs[i].active = true;

        // Define direções aleatórias para cada UFO
        UFOs[i].directionX = (std::rand() % 3) - 1; // -1, 0 ou 1
        UFOs[i].directionY = (std::rand() % 3) - 1; // -1, 0 ou 1

        UFOs[i].canShoot = true;
        UFOs[i].lastShootTime = std::chrono::steady_clock::now();
    }

    // Posicionamento inicial do foguete no centro do grid
    int rocketX = width / 2;
    int rocketY = height / 2;

    bool running = true;

    while (running && killCount < numUFOs && lives > 0) {
        std::system("clear");

        auto currentTime = std::chrono::steady_clock::now();

        // Atualiza o placar com a contagem atualizada de kills e a quantidade de vidas
        updateScore(width, killCount, lives);

        drawConwayGame(conwayBoard);
        updateConwayGame(conwayBoard);

        drawBorder(width, height); // Desenha a borda no início de cada iteração

        int encostar = 0;
        for (int i = 0; i < numUFOs; ++i) {
            if (UFOs[i].active == true) {
                moveRocket(rocketX, rocketY, running, rocketShots, conwayBoard, nebulosa);
                // Desenha o foguete na nova posição
                goToXY(rocketX, rocketY);
                std::cout << "\U0001F680"; // Emoji Unicode do foguete

                bool ufoCollided = checkConwayCollision(UFOs[i].x, UFOs[i].y, conwayBoard);
                if (ufoCollided == true) {
                    encostar++;
                }

                // Movimento dos UFOs
                UFOs[i].directionX += (std::rand() % 3) - 1; // Adiciona -1, 0 ou 1 aleatoriamente
                UFOs[i].directionY += (std::rand() % 3) - 1; // Adiciona -1, 0 ou 1 aleatoriamente

                UFOs[i].directionX = std::max(-1, std::min(1, UFOs[i].directionX));
                UFOs[i].directionY = std::max(-1, std::min(1, UFOs[i].directionY));

                UFOs[i].x += UFOs[i].directionX;
                UFOs[i].y += UFOs[i].directionY;

                if (UFOs[i].x < 0 || UFOs[i].x >= width) {
                    UFOs[i].directionX = -UFOs[i].directionX;
                    UFOs[i].x += UFOs[i].directionX * 2;
                }

                if (UFOs[i].y < 0 || UFOs[i].y >= height) {
                    UFOs[i].directionY = -UFOs[i].directionY;
                    UFOs[i].y += UFOs[i].directionY * 2;
                }

                // Verificar se pode disparar
                auto timeDiff = std::chrono::duration_cast<std::chrono::seconds>(currentTime - UFOs[i].lastShootTime).count();
                if (timeDiff >= 1) {
                    UFOs[i].canShoot = true;
                }

                // Disparar continuamente
                if (UFOs[i].canShoot) {
                    shoot(shots, UFOs[i].x, UFOs[i].y, width / 2, height / 2); // Substitua width/2 e height/2 pelo alvo desejado
                    UFOs[i].canShoot = false;
                    UFOs[i].lastShootTime = currentTime;
                }
            }
        }

        if (encostar > 0 && nebulosa == "roxo") {
            nebulosa = "vermelho";
        } else if (nebulosa != "azul"){
            nebulosa = "roxo";
        }

        moveShots(shots, UFOs); // Movimenta os tiros
        moveShots(rocketShots, UFOs); // Movimenta os tiros

        // Verifica colisões com tiros '*' que não são da cor 'green'
        for (auto& shot : shots) {
            if (shot.active && shot.x == rocketX && shot.y == rocketY && shot.color != "green") {
                shot.active = false;
                drawExplosion(rocketX, rocketY);
                lives--; // Diminui uma vida da nave
                if (lives <= 0) {
                    running = false; // Encerra o jogo se as vidas acabarem
                    break;
                }
            }
        }

        // Verifica colisão dos tiros com as bordas e desativa os tiros se atingirem as bordas
        for (auto& shot : shots) {
            if (shot.active) {
                if (hitBorder(shot.x, shot.y, width, height)) {
                    // O tiro atingiu a borda, desativa o tiro
                    shot.active = false;
                }
                // Verifica colisão com células do tabuleiro Conway representadas por '#'
                if (conwayBoard[shot.y][shot.x]) {
                    if (nebulosa == "azul") {
                        // Desativa o tiro vermelho ao colidir com '#'
                        shot.active = false;
                    }
                }
            }
        }
        moveRocket(rocketX, rocketY, running, rocketShots, conwayBoard, nebulosa);
        // Desenha o foguete na nova posição
        goToXY(rocketX, rocketY);
        std::cout << "\U0001F680"; // Emoji Unicode do foguete
        for (auto& shot : shots) {
            if (shot.active) {
                for (auto& ufo : UFOs) {
                    if (ufo.active == true && ufo.x == shot.x && ufo.y == shot.y) {
                        shot.active = false;
                        drawExplosion(shot.x, shot.y);
                        break;  // Um tiro atingiu um UFO, então pare de verificar outros UFOs
                    }
                }
            }
        }

        // Verifica colisão dos tiros com as bordas e desativa os tiros se atingirem as bordas
        for (auto& shot : rocketShots) {
            if (shot.active) {
                if (hitBorder(shot.x, shot.y, width, height)) {
                    // O tiro atingiu a borda, desativa o tiro
                    shot.active = false;
                }
                // Verifica colisão com células do tabuleiro Conway representadas por '#'
                if (conwayBoard[shot.y][shot.x]) {
                    // Verifica colisão com células do tabuleiro Conway representadas por '#'
                    if (conwayBoard[shot.y][shot.x]) {
                        if (nebulosa == "vermelho") {
                            // Desativa o tiro vermelho ao colidir com '#'
                            shot.active = false;
                        }
                    }
                }
            }
        }

        for (auto& shot : rocketShots) {
            if (shot.active) {
                for (auto& ufo : UFOs) {
                    if (ufo.active == true && ufo.x == shot.x && ufo.y == shot.y) {
                        shot.active = false;
                        drawExplosion(shot.x, shot.y);
                        break;  // Um tiro atingiu um UFO, então pare de verificar outros UFOs
                    }
                }
            }
        }

        // Desenha os tiros
        for (const auto& shot : shots) {
            std::cout << "\033[1;31m"; // Define a cor do texto para vermelho (31)
            if (shot.active) {
                goToXY(shot.x, shot.y);
                std::cout << "*";
            }
        }
        // Desenha os tiros
        for (const auto& shot : rocketShots) {
            std::cout << "\033[1;36m"; // Define a cor do texto para ciano (36)
            if (shot.active) {
                goToXY(shot.x, shot.y);
                std::cout << "*";
            }
        }

        // Desenha os UFOs
        for (const auto& ufo : UFOs) {
            std::cout << "\033[1;31m"; // Define a cor do texto para azul

            // Desenha o UFO normal ou o emoji de explosão, dependendo da situação
            if (ufo.active) {
                goToXY(ufo.x, ufo.y);
                std::cout << "\U0001F6F8"; // Código Unicode UFO
            }
        }
        goToXY(rocketX, rocketY);
        std::cout << "\U0001F680"; // Emoji Unicode do foguete
        std::cout << std::flush;
        usleep(100000);
    }
    endwin(); // Restaura o terminal ao estado inicial
    showCursor(); // Mostra o cursor no final da execução
    if (killCount >= numUFOs) {
    printf("Parabéns!\n");
    } else {
    printf("Game Over\n");
    }
    return 0;
}
