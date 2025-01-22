#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <chrono>
#include <limits>
#include <algorithm>
#include<conio.h>
using namespace std;

// عدد الخانات على اللوحة
const int BOARD_SIZE = 56;
const int SAFE_SPOTS[] = { 0, 8, 13, 21, 26, 34, 39, 47, 51, 52, 53, 54, 55 };

// هيكل لتمثيل الحجر
struct Piece {
    int position;    // -1 إذا كان في القاعدة
    bool isSafe;     // إذا كان الحجر في خانة آمنة
};

// هيكل لتمثيل اللاعب
struct Player {
    vector<Piece> pieces;  // الأحجار الخاصة باللاعب
    bool isComputer;       // هل اللاعب هو الكمبيوتر
};

// التحقق إذا كانت الخانة آمنة
bool isSafeSpot(int position) {
    for (int spot : SAFE_SPOTS) {
        if (position == spot) return true;
    }
    return false;
}

// دالة لرمي النرد
int rollDice() {
    return rand() % 6 + 1;
}
// طباعة حالة اللوحة
void printBoard(const vector<Player> & players) {
    cout << "Board Status:\n";
    for (size_t i = 0; i < players.size(); i++) {
        cout << "Player " << (i + 1) << ": ";
        for (const auto& piece : players[i].pieces) {
            if (piece.position == -1)
                cout << "[Base] ";
            else
                cout << "[" << piece.position << "] ";
        }
        cout << endl;
    }
    cout << endl;
}
void printSafeSpots() {
    cout << "Safe Spots: ";
    for (int spot : SAFE_SPOTS) {
        cout << spot << " ";
    }
    cout << endl;
}
// تحقق ما إذا كان هناك قتل

    bool checkKill(vector<Player>& players, int currentPlayerIndex, int pieceIndex) {
    Player& currentPlayer = players[currentPlayerIndex];
    Piece& currentPiece = currentPlayer.pieces[pieceIndex];
    bool killed = false; // مؤشر لتحديد حدوث القتل

    for (size_t i = 0; i < players.size(); i++) {
        if (i == currentPlayerIndex) continue; // تجاوز اللاعب الحالي
        Player & opponent = players[i];
        for (Piece& opponentPiece : opponent.pieces) {
            // حساب الفارق مع التعامل مع الرقعة الدائرية
            if(opponentPiece.position != -1 ){
            int difference = abs(currentPiece.position - opponentPiece.position);
            if (difference == 13  && !isSafeSpot(opponentPiece.position) && currentPiece.position < 50) {
                cout << "Player " << (currentPlayerIndex + 1) << " killed a piece of Player " << (i + 1) << "!\n";
                opponentPiece.position = -1; // إعادة القطعة إلى نقطة البداية
                killed = true; // تم القتل

            }
        } }
    }
    return killed; // إرجاع حالة القتل

}


bool isWall(const vector<Player>& players, int currentPlayerIndex, int position) {
    for (size_t i = 0; i < players.size(); i++) {
        if (i == currentPlayerIndex) continue; // تجاوز اللاعب الحالي
        int count = 0;
        for (const Piece& piece : players[i].pieces) {
            if (piece.position == position) {
                count++;
                if (count >= 2) return true; // الجدار موجود
            }
        }
    }
    return false; // لا يوجد جدار
}
bool canMoveToPosition(const vector<Player>& players, int currentPlayerIndex, int currentPosition, int diceRoll) {
    int targetPosition = currentPosition + diceRoll;
    if (targetPosition >= BOARD_SIZE) {
        targetPosition %= BOARD_SIZE; // التعامل مع الالتفاف في الرقعة
    }
    return !isWall(players, currentPlayerIndex, targetPosition); // تحقق من وجود الجدار
}



// كلاس GetNextState
class GetNextState {
public:
    vector<Player> getNextState(const vector<Player>& players, int currentPlayerIndex, int diceRoll) {
        vector<Player> nextPlayers = players;  // نسخ اللاعبين الحاليين

        // نحاول تحريك كل قطعة لللاعب الحالي بناءً على رمي النرد
        for (size_t i = 0; i < nextPlayers[currentPlayerIndex].pieces.size(); i++) {
            Piece& piece = nextPlayers[currentPlayerIndex].pieces[i];
            if (piece.position == -1 && diceRoll == 6) {
                piece.position = 0;  // إذا كانت في القاعدة وظهرت 6، نخرجها
            }
            else if (piece.position != -1 && piece.position<= 50 &&  piece.position + diceRoll <= BOARD_SIZE) {
                piece.position += diceRoll;  // نتحرك بالقطعة حسب قيمة النرد
            }
            else if(piece.position >50 && piece.position + diceRoll == BOARD_SIZE){
                piece.position += diceRoll;
            }
        }

        return nextPlayers;  // إرجاع الحالة الجديدة
    }
};
int  evaluateState(vector<Player>& players, int currentPlayerIndex) {
    const Player& player = players[currentPlayerIndex];
    //Player& currentPlayer = players[currentPlayerIndex];

    int score = 0;

    for (const auto& piece : player.pieces) {
        if (piece.position >= BOARD_SIZE) {
            score += 100; // نقاط إضافية للقطع التي وصلت للنهاية
        }
        else if (piece.isSafe) {
            score += 50; // نقاط للقطع الآمنة
        }
        else if (checkKill(players, currentPlayerIndex, piece.position)) {
            score -= 20;  // خصم النقاط إذا كانت القطعة معرضة للقتل
        }
        else {
            score += piece.position; // نقاط بناءً على مدى التقدم

        }
    }

    // تقييم وضع الخصم: نخسر إذا كان الخصم قريب من الفوز
    const Player& opponent = players[(currentPlayerIndex + 1) % players.size()];
    for (const auto& piece : opponent.pieces) {
        if (piece.position >= BOARD_SIZE) {
            score -= 100; // خصم النقاط إذا وصل الخصم إلى النهاية
        }
        else {
            // إذا كانت قطعة الخصم معرضة للقتل، نزيد النقاط
            if (checkKill(players, (currentPlayerIndex + 1) % players.size(), piece.position)) {
                score += 30;  // إضافة نقاط إذا كانت قطعة الخصم معرضة للقتل
            }
        }
    }

    return score;
}

// خوارزمية expectiminimax
int expectiminimax(vector<Player> & players, int currentPlayerIndex, int depth, bool isMaxPlayer,
                   GetNextState & stateGenerator, int diceRoll) {
    if (depth == 0) {
        return evaluateState(players, currentPlayerIndex);
    }
    // إذا كانت عقدة Max (الكمبيوتر)
    if (isMaxPlayer) {
        int bestScore = numeric_limits<int>::min();
        // تقييم كل حركة ممكنة للكمبيوتر
        for (size_t i = 0; i < players[currentPlayerIndex].pieces.size(); i++) {
            Piece& piece = players[currentPlayerIndex].pieces[i];
            if (piece.position == -1) { // إذا كانت القطعة في القاعدة
                if (diceRoll == 6) {
                    piece.position = 0;
                    vector<Player> nextPlayers = stateGenerator.getNextState(players, currentPlayerIndex, 6);
                    bestScore = max(bestScore, expectiminimax(nextPlayers, currentPlayerIndex, depth - 1, false, stateGenerator, diceRoll));
                    piece.position = -1; // إعادة القطعة إلى القاعدة
                }
            }
            else if (piece.position <= BOARD_SIZE) {
                piece.position += diceRoll;
                vector<Player> nextPlayers = stateGenerator.getNextState(players, currentPlayerIndex, 6);
                bestScore = max(bestScore, expectiminimax(nextPlayers, currentPlayerIndex, depth - 1, false, stateGenerator, diceRoll));
                piece.position -= diceRoll; // التراجع عن الحركة
            }
        }
        return bestScore;
    }
        // إذا كانت عقدة Min (الخصم)
    else {
        int worstScore = numeric_limits<int>::max();
        for (size_t i = 0; i < players[currentPlayerIndex].pieces.size(); i++) {
            Piece& piece = players[currentPlayerIndex].pieces[i];
            if (piece.position == -1) { // إذا كانت القطعة في القاعدة
                if (rollDice() == 6) {
                    piece.position = 0;
                    vector<Player> nextPlayers = stateGenerator.getNextState(players, currentPlayerIndex, 6);
                    worstScore = min(worstScore, expectiminimax(nextPlayers, currentPlayerIndex, depth - 1, true,
                                                                stateGenerator, diceRoll));
                    piece.position = -1; // إعادة القطعة إلى القاعدة
                }
            }
            else if (piece.position <= BOARD_SIZE && piece.position<=50 && piece.position + diceRoll <= BOARD_SIZE) {
                piece.position += diceRoll;
                vector<Player> nextPlayers = stateGenerator.getNextState(players, currentPlayerIndex, 6);
                worstScore = min(worstScore, expectiminimax(nextPlayers, currentPlayerIndex, depth - 1, true
                        , stateGenerator, diceRoll));
                piece.position -= diceRoll; // التراجع عن الحركة
            }
            else if(piece.position >50 && piece.position + diceRoll == BOARD_SIZE){
                piece.position += diceRoll;
                vector<Player> nextPlayers = stateGenerator.getNextState(players, currentPlayerIndex, 6);
                worstScore = min(worstScore, expectiminimax(nextPlayers, currentPlayerIndex, depth - 1, true
                        , stateGenerator, diceRoll));
                piece.position -= diceRoll; // التراجع عن الحركة
            }
        }

        return worstScore;
    }
}
    bool computerMove(vector<Player>& players, int currentPlayerIndex, int diceRoll) {
    cout << "Computer " << currentPlayerIndex + 1 << " rolled a " << diceRoll << ".\n";
    GetNextState stateGenerator; // كائن من GetNextState
    int bestScore = numeric_limits<int>::min();
    int bestPieceIndex = -1;
    for (size_t i = 0; i < players[currentPlayerIndex].pieces.size(); i++) {
        Piece& piece = players[currentPlayerIndex].pieces[i];
        int currentPosition = piece.position;

        if ((currentPosition == -1 && diceRoll == 6) ||
            (currentPosition != -1   && piece.position<=50 && currentPosition + diceRoll <= BOARD_SIZE ||
            piece.position >50 && piece.position + diceRoll == BOARD_SIZE &&
             canMoveToPosition(players, currentPlayerIndex, currentPosition, diceRoll))) {
            vector<Player> nextPlayers = stateGenerator.getNextState(players, currentPlayerIndex, diceRoll);
            int eval = expectiminimax(nextPlayers, currentPlayerIndex, 3, false, stateGenerator, diceRoll);
            if (eval > bestScore) {
            bestScore = eval;
            bestPieceIndex = i;}
            bestPieceIndex = i;
            break;
        }
        else {
            vector<Player> nextPlayers = stateGenerator.getNextState(players, currentPlayerIndex, diceRoll);
            int eval = expectiminimax(nextPlayers, currentPlayerIndex, 3, false, stateGenerator, diceRoll);
            if (eval > bestScore) {
                bestScore = eval;
                bestPieceIndex = i;
            }}}
        if (bestPieceIndex != -1) {
        Piece& bestPiece = players[currentPlayerIndex].pieces[bestPieceIndex];
        if (bestPiece.position == -1 && diceRoll == 6) {
            bestPiece.position = 0;
            cout << "Computer moved piece " << bestPieceIndex + 1 << " to position " << bestPiece.position << ".\n";
        }
        else if(bestPiece.position != -1 && bestPiece.position + diceRoll <= BOARD_SIZE) {
            bestPiece.position += diceRoll;
            cout << "Computer moved piece " << bestPieceIndex + 1 << " to position " << bestPiece.position << ".\n";
        }
        // تحقق من القتل
        return checkKill(players, currentPlayerIndex, bestPieceIndex);
    }
        else {
        cout << "Computer has no valid moves.\n";
        return false;
    }
}

// تحريك حجر للاعب البشري
bool humanMove(Player& player, int currentPlayerIndex, vector<Player>& players, int diceRoll) {
    vector<int> validPieces;

    for (size_t i = 0; i < player.pieces.size(); i++) {
        int currentPosition = player.pieces[i].position;
        if ((currentPosition == -1 && diceRoll == 6) ||
            (currentPosition != -1 && currentPosition + diceRoll <= BOARD_SIZE &&
             canMoveToPosition(players, currentPlayerIndex, currentPosition, diceRoll))) {
            validPieces.push_back(i);
        }
    }
    if (validPieces.empty()) {
        cout << "No valid moves available.\n";
        return false;
    }

    cout << "Choose a piece to move (1-" << validPieces.size() << "): ";
    int choice;
    cin >> choice;

    if (choice < 1 || choice > validPieces.size()) {
        cout << "Invalid choice.\n";
        return false;
    }

    int pieceIndex = validPieces[choice - 1];
    if (player.pieces[pieceIndex].position == -1 && diceRoll == 6) {
        player.pieces[pieceIndex].position = 0;
    }
    else if (player.pieces[pieceIndex].position != -1 && player.pieces[pieceIndex].position<=50
    &&  player.pieces[pieceIndex].position + diceRoll <= BOARD_SIZE) {
        player.pieces[pieceIndex].position += diceRoll;
    }
    else if(player.pieces[pieceIndex].position >50 && player.pieces[pieceIndex].position + diceRoll == BOARD_SIZE){
        player.pieces[pieceIndex].position += diceRoll;

    }


    // تحقق من القتل

    return checkKill(players, currentPlayerIndex, pieceIndex);
}

// تحريك حجر للكمبيوتر
// تحريك حجر للكمبيوتر بناءً على قيمة النرد باستخدام expectiminimax
int main() {
    srand(static_cast<unsigned int>(time(0)));

    int numPlayers;
    cout << "Enter number of players (1 for human, (1-3) for computer): ";
    cin >> numPlayers;
    while (numPlayers > 4 || numPlayers < 2) {
        cout << "The number must be from 2 to 4)";
        cin >> numPlayers;
    }
    int numComputers = numPlayers - 1;

    vector<Player> players(numPlayers);

    // إنشاء اللاعبين
    for (int i = 0; i < numPlayers; i++) {
        players[i].isComputer = (i > 0);  // أول لاعب هو الإنسان، البقية هم الكمبيوتر
        for (int j = 0; j < 4; j++) {
            players[i].pieces.push_back({ -1, false });
        }
    }

    cout << "Welcome to Ludo Game with AI!\n";
    printSafeSpots();
    printBoard(players);

    bool gameOver = false;
    int currentPlayerIndex = 0;
    int consecutiveSixes = 0; // عداد لتتبع عدد مرات الحصول على 6
    bool extraTurn=false;
    int diceRoll;
    while (!gameOver) {
        Player& currentPlayer = players[currentPlayerIndex];
        cout << "Player " << (currentPlayerIndex + 1) << "'s turn...\n";


        if (currentPlayer.isComputer) {
            diceRoll = rollDice();
            /*bool validInput = false;
            while (!validInput) {
            cout << "Press 'C' to give the role to the Computer ";
            char input = _getch();  // ينتظر إدخال المستخدم
            if (input == 'C' || input == 'c') {
             diceRoll = rollDice();
             cout << "Rolled: " << diceRoll << "\n";
             validInput = true;}// تم رمي النرد
                else {
                    cout << "Invalid input. Please press 'O'.\n";
                }
            }
                GetNextState stateGenerator;*/
            for (size_t i = 0; i < currentPlayer.pieces.size(); i++) {
                extraTurn = computerMove(players, currentPlayerIndex, diceRoll);
                    break;
            }
        }
        else {
            bool validInput = false;
            while (!validInput) {
                diceRoll = 0;
                cout << "Press 'O' to roll the dice ";
                char input = _getch();  // ينتظر إدخال المستخدم
                if (input == 'O' || input == 'o') {
                    diceRoll = rollDice();
                    cout << "Rolled: " << diceRoll << "\n";
                    validInput = true;  // تم رمي النرد
                }
                else {
                    cout << "Invalid input. Please press 'O'.\n";
                }
            }
            GetNextState stateGenerator;
            for (size_t i = 0; i < currentPlayer.pieces.size(); i++) {
                extraTurn = humanMove(players[currentPlayerIndex], currentPlayerIndex, players, diceRoll);
                break;
            }
        }

        printBoard(players);

        if (all_of(currentPlayer.pieces.begin(), currentPlayer.pieces.end(),
                   [](const Piece & piece) { return piece.position >= BOARD_SIZE; })) {
            cout << "Player " << (currentPlayerIndex + 1) << " wins the game!\n";
            gameOver = true;
            break;
        }
        //يحصل اللاعب على دور اضافي عند الحصول على 6
        if (diceRoll == 6 ) {
            consecutiveSixes++;
            if (consecutiveSixes == 3) {
                cout << "Player " << (currentPlayerIndex + 1) << " got three consecutive 6s. Switching turn...\n";
                consecutiveSixes = 0;
                currentPlayerIndex = (currentPlayerIndex + 1) % numPlayers;
            }
            else {
                cout << "Player " << (currentPlayerIndex + 1) << " rolls again...\n";
                continue;
            }
        }
         //يحصل اللاعب على دور اضافي عند قتل الخصم
         else if (extraTurn) {
            cout << "Player " << (currentPlayerIndex + 1) << " rolls again...\n";
            continue;
        }
        else  {
            consecutiveSixes = 0;
            currentPlayerIndex = (currentPlayerIndex + 1) % numPlayers;
        }
        this_thread::sleep_for(chrono::milliseconds(500));
    }

    return 0;
}
