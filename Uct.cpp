#include "Uct.h"

// 全局变量初始化
int *UCT::curTop = nullptr;
int **UCT::curBoard = nullptr;

UCT::UCT() {}

UCT::UCT(int _M, int _N, int _noX, int _noY, int _lastX, int _lastY, const int* _top, int** _board):
    M(_M), N(_N), noX(_noX), noY(_noY), lastX(_lastX), lastY(_lastY) {

    std::cerr << "[UCT::UCT] init\n";
    // 初始化
    root = nullptr;
    timer.set();

    // 储存列顶信息
    top = new int[N];
    for (int i = 0; i < N; i++) {
        top[i] = _top[i];
    }

    // 记录当前棋盘信息
    board = new int*[M];
    for (int i = 0; i < M; i++) {
        board[i] = new int[N];
        for (int j = 0; j < N; j++) {
            board[i][j] = _board[i][j];
        }
    }
    // std::cerr << "[UCT::UCT] init finished\n";

}

void UCT::boardClear() {
    delete[] curTop;
    for (int i = 0; i < M; i++) delete[] curBoard[i];
    delete[] curBoard;
}

void UCT::boardReset() {
    curTop = new int[N];
    for (int i = 0; i < N; i++) {
        curTop[i] = top[i];
    }

    // 记录当前棋盘信息
    curBoard = new int*[M];
    for (int i = 0; i < M; i++) {
        curBoard[i] = new int[N];
        for (int j = 0; j < N; j++) {
            curBoard[i][j] = board[i][j];
        }
    }
}

Point UCT::uctSearch() {
    std::cerr << "[UCT::uctSearch] search started\n";
    boardReset();
    root = new Node(nullptr, M, N, noX, noY, lastX, lastY, true);
    while (timer.get() < TIME_LIMIT) {
        // std::cerr << "[UCT::uctSearch] check next node\n";
        std::cerr << "[UCT::uctSearch] check time: " << timer.get() << "\n";
        boardReset();
        Node* vl = treePolicy(root);
        int delta = defaultPolicy(vl);
        backup(vl, delta);
        boardClear();
    }

    return root->bestChild()->getMove();
}

Node* UCT::treePolicy(Node* v) {
    std::cerr << "[UCT::treePolicy]\n";
    while (!v->end()) {
        if (v->canExpend()) {
            return v->expand();
        } else {
            v = v->bestChild();
        }
    }
    return v;
}

int UCT::defaultPolicy(Node* v) {
    std::cerr << "[UCT::defaultPolicy]\n";
    Point pt = v->getMove();

    int x = pt.x;
    int y = pt.y;

    int* tmpTop = new int[N];
    for (int i = 0; i < N; i++) {
        tmpTop[i] = curTop[i];
    }

    int** tmpBoard = new int*[M];
    for (int i = 0; i < M; i++) {
        tmpBoard[i] = new int[N];
        for (int j = 0; j < N; j++) {
            tmpBoard[i][j] = curBoard[i][j];
        }
    }

    // std::cerr << "[UCT::defaultPolicy] copy finished\n";

    bool player = v->getPlayer();
    int score = getScore(x, y, tmpTop, tmpBoard, player);

    // std::cerr << "[UCT::defaultPolicy] simulation\n";
    while (score > 1) {
        // 回合切换
        player = !player;

        // 找出下子点
        srand(timer.get());
        int idx = rand() % N;
        while (!tmpTop[idx]) {
            idx = rand() % N;
        }

        // 下棋
        tmpTop[idx]--;
        int x = tmpTop[idx], y = idx;
        tmpBoard[x][y] = (player? 1: 2);
        // std::cerr << x << " " << y << ": " << tmpBoard[x][y] << "\n";

        // 若下棋位置的再下一位是不可下棋点, 则跳过
        if (x - 1 == noX && y == noY) {
            tmpTop[y]--;
        }

        score = getScore(x, y, tmpTop, tmpBoard, player);
    }

    delete[] tmpTop;
    for (int i = 0; i < M; i++) delete[] tmpBoard[i];
    delete[] tmpBoard;

    return score;
}

int UCT::getScore(int _x, int _y, int* _top, int** _board, bool player) {
    // std::cerr << "[UCT::getScore]\n";

    // std::cerr << _x << " " << _y << ": " << 3 - int(player) << "\n";
    // for (int i = 0; i < M; i++) {
    //     for (int j = 0; j < N; j++) {
    //         std::cerr << _board[i][j] << " ";
    //     }
    //     std::cerr << "\n";
    // }
    // std::cerr << "\n";
    if (_x < 0 || _y < 0) return 5;
    if (player && userWin(_x, _y, M, N, _board)) {
        // for (int i = 0; i < M; i++) {
        //     for (int j = 0; j < N; j++) {
        //         std::cerr << _board[i][j] << " ";
        //     }
        //     std::cerr << "\n";
        // }
        // std::cerr << "\n";
        // std::cerr << "Score: -1\n";
        return -1;
    }
    if (!player && machineWin(_x, _y, M, N, _board)) {
        // for (int i = 0; i < M; i++) {
        //     for (int j = 0; j < N; j++) {
        //         std::cerr << _board[i][j] << " ";
        //     }
        //     std::cerr << "\n";
        // }
        // std::cerr << "\n";

        // std::cerr << "Score: 1\n";
        return 1;
    }
    if (isTie(N, top)) return 0;
    return 10;
}

void UCT::backup(Node* v, int status) {
    // std::cerr << "[UCT::backup]\n";
    while (v) {
        v->countVisited ++;
        v->countWin += status;
        status = 1 - status;
        v = v->parent;
    }
}

UCT::~UCT() {
    delete[] top;
    for (int i = 0; i < M; i++) delete board[i];
    delete[] board;
}