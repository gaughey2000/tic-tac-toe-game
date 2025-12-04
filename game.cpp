#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <cstdio>

const int BOARD_SIZE = 25;
char board[BOARD_SIZE][BOARD_SIZE];

int cursorX = 12;  // Start in middle
int cursorY = 12;  // Start in middle
char currentPlayer = 'X';

struct termios origTermios;

void enableRawMode() {
    tcgetattr(STDIN_FILENO, &origTermios);
    struct termios raw = origTermios;

    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
    raw.c_iflag &= ~(IXON | ICRNL);
    raw.c_oflag &= ~(OPOST);

    raw.c_cc[VMIN] = 1;   // Wait for at least 1 character
    raw.c_cc[VTIME] = 0;  // No timeout

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}


void disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &origTermios);

}


void drawBoard() {
    // Disable line wrapping and use simple format
    write(STDOUT_FILENO, "\033[?7l", 5);  // Disable line wrapping
    
    for (int y = 0; y < BOARD_SIZE; y++) {
        for (int x = 0; x < BOARD_SIZE; x++) {
            if (x == cursorX && y == cursorY) {
                if (board[y][x] != 0) {
                    write(STDOUT_FILENO, "[", 1);
                    write(STDOUT_FILENO, &board[y][x], 1);
                    write(STDOUT_FILENO, "]", 1);
                } else {
                    write(STDOUT_FILENO, "[.]", 3);
                }
            } else {
                if (board[y][x] != 0) {
                    write(STDOUT_FILENO, " ", 1);
                    write(STDOUT_FILENO, &board[y][x], 1);
                    write(STDOUT_FILENO, " ", 1);
                } else {
                    write(STDOUT_FILENO, " . ", 3);
                }
            }
        }
        write(STDOUT_FILENO, "\r\n", 2);  // Explicit CRLF
    }
    
    // Re-enable line wrapping
    write(STDOUT_FILENO, "\033[?7h", 5);
}


bool checkWin(int x, int y, char player) {
    int dirs[4][2] = {
        {1, 0},   // horizontal
        {0, 1},   // vertical
        {1, 1},   // diagonal down-right
        {1, -1}   // diagonal up-right
    };
    
    for (int d = 0; d < 4; d++) {
        int dx = dirs[d][0];
        int dy = dirs[d][1];
        
        int count = 1; // the piece just placed
        
        // forward scan
        for (int i = 1; i < 5; i++) {
            int newX = x + i * dx;
            int newY = y + i * dy;
            if (newX >= 0 && newX < BOARD_SIZE && newY >= 0 && newY < BOARD_SIZE && 
                board[newY][newX] == player) {
                count++;
            } else {
                break;
            }
        }
        
        // backward scan
        for (int i = 1; i < 5; i++) {
            int newX = x - i * dx;
            int newY = y - i * dy;
            if (newX >= 0 && newX < BOARD_SIZE && newY >= 0 && newY < BOARD_SIZE && 
                board[newY][newX] == player) {
                count++;
            } else {
                break;
            }
        }
        
        if (count >= 5) {
            return true;
        }
    }
    return false;
}


int main() {
    // Initialize board
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            board[i][j] = 0;  // Empty
        }
    }
    
    enableRawMode();
    
    while (true) {
        // Clear screen and reset cursor
        write(STDOUT_FILENO, "\033[2J\033[H", 7);
        
        drawBoard();
        char msg[100];
        int len = sprintf(msg, "\nCurrent player: %c\nUse arrow keys to move, space to place, 'q' to quit\n", currentPlayer);
        write(STDOUT_FILENO, msg, len);

        char c;
        read(STDIN_FILENO, &c, 1);

        if (c == '\033') {
            char seq1, seq2;
            if (read(STDIN_FILENO, &seq1, 1) == 0) continue;

            if (seq1 == '[') {
                if (read(STDIN_FILENO, &seq2, 1) == 0) continue;

                if (seq2 == 'A' && cursorY > 0) cursorY--;           // up
                else if (seq2 == 'B' && cursorY < BOARD_SIZE-1) cursorY++;     // down
                else if (seq2 == 'C' && cursorX < BOARD_SIZE-1) cursorX++;     // right
                else if (seq2 == 'D' && cursorX > 0) cursorX--;      // left
            }
        }
        else if (c == ' ') {
            if (board[cursorY][cursorX] == 0) {  // Empty spot
                board[cursorY][cursorX] = currentPlayer;

                if (checkWin(cursorX, cursorY, currentPlayer)) {
                    printf("\033[2J\033[H");
                    fflush(stdout);
                    drawBoard();
                    printf("\nPlayer %c wins!\n", currentPlayer);
                    disableRawMode();
                    return 0;
                }

                currentPlayer = (currentPlayer == 'X') ? 'O' : 'X';
            }
        }
        else if (c == 'q') {
            break;
        }
    }

    std::cout << "\033[2J\033[H";
    std::cout.flush();
    disableRawMode();
    return 0;
}
