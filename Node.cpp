#include "Node.h"
#include "Uct.h"

Node::Node() {
    timer.set();
}

Node::Node(Node* _parent, int _M, int _N, int _noX, int _noY, int _posX, int _posY, bool _player): 
    parent(_parent), M(_M), N(_N), noX(_noX), noY(_noY), posX(_posX), posY(_posY), player(_player) {
    
    std::cerr << "[Node::Node] init\n";
    // 初始化
    canMove.clear();
    countCanMove = 0;
    countVisited = 0;
    countWin = 0;
    timer.set();

    // 记录可下子的列编号
    for (int i = 0; i < N; i++) {
        if (UCT::curTop[i]) {
            countCanMove++;
            canMove.push_back(i);
        }
    }

    // 记录子节点
    child = new Node*[N];
    std::cerr << "[Node::Node] init finished\n";
}

Node* Node::bestChild() {
    std::cerr << "[Node::bestChild]\n";
    double tmpScore = -1e20;
    Node* tmpNode = nullptr;
    for (int i = 0; i < N; i++) {
        if (child[i] == nullptr) continue;
        int win = child[i]->countWin;
        int vis = child[i]->countVisited;
        double score = (win * 1.0 / vis + c * sqrt(2.0 * log(countVisited) / vis));
        if (score > tmpScore) {
            tmpScore = score;
            tmpNode = child[i];
        }
    }
    return tmpNode;
}

bool Node::canExpend() {
    return countCanMove > 0;
}

Node* Node::expand() {
    std::cerr << "[Node::expand]\n";
    // 随机选择要下的列
    srand(timer.get());
    int idx = rand() % countCanMove;

    // 对应位置下棋
    int nxtY = canMove[idx];
    int nxtX = --UCT::curTop[nxtY];
    UCT::curBoard[nxtX][nxtY] = ((!player)? 1: 2);

    // 若下棋位置的再下一位是不可下棋点, 则跳过
    if (nxtX - 1 == noX && nxtY == noY) {
        UCT::curTop[nxtY]--;
    }

    // 记录此状态
    Node* tmp = new Node(this, M, N, noX, noY, nxtX, nxtY, !player);
    canMove.erase(canMove.begin() + idx);
    countCanMove--;
    child[nxtY] = tmp;

    // std::cerr << "[Node::expand] Node expend finished\n";
    return tmp;
}

bool Node::end() {
    std::cerr << "[Node::end] Node status check\n";
    // 若尚未下棋
    if (posX == -1 && posY == -1) {
        return false;
    }

    // 否则若到达游戏终止条件
    if ((player && userWin(posX, posY, M, N, UCT::curBoard)) ||
        (!player && machineWin(posX, posY, M, N, UCT::curBoard)) ||
        (isTie(N, UCT::curTop))) {
        return true;
    }
    // std::cerr << "[Node::end] status: game continue\n";
    return false;
}

bool Node::getPlayer() {
    return player;
}

Point Node::getMove() {
    return Point(posX, posY);
}

void Node::clearArray() {
    for (int i = 0; i < N; i++) {
        if (child[i]) {
            child[i]->clearArray();
        }
    }
    delete[] child;
    canMove.clear();
}

Node::~Node() {
    clearArray();
}