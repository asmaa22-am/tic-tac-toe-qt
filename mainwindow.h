#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDialog>
#include <QPushButton>
#include <QGridLayout>
#include <QLabel>
#include <QTimer>
#include <QTextEdit>
#include <QComboBox>
#include <vector>
#include <QString>
#include <string>
#include <unordered_map>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

// --- Move Struct Definition ---
// Records each move's row, column, and the player that moved.
struct Move {
    int row;
    int col;
    char player;
};

// --- GameRecord Struct Definition ---
// Stores the game mode, winner and the full move history.
struct GameRecord {
    std::string mode;
    std::string winner;
    std::vector<Move> moves;
};

// --- GameBoard Class Definition ---
// Handles the board UI and game logic including AI moves with minimax.
class GameBoard : public QWidget
{
    Q_OBJECT

public:
    GameBoard(QWidget *parent = nullptr, int mode = 0);
    ~GameBoard();

    void initializeBoard();
    void resetBoard();
    bool makeMove(int row, int col, char player);
    bool checkWinner(char player);
    bool isFull();
    void switchPlayer();
    char getCurrentPlayer() const;
    std::vector<std::vector<char>> getBoard() const;
    bool isEmpty(int row, int col) const;
    void updateButtonText(int row, int col, char text);
    void disableBoard();
    void enableBoard();

public slots:
    void onCellClicked();
    void aiMove();
    void triggerAiMove();

signals:
    void gameOver(const QString& winner);
    void moveMade(int row, int col, char player); // Emitted whenever a move occurs

private:
    std::vector<std::vector<char>> board;
    std::vector<std::vector<QPushButton*>> buttons;
    QGridLayout* mainLayout;
    char currentPlayer;
    bool gameActive;
    int gameMode;

    QPoint findBestMove();
    int minimax(std::vector<std::vector<char>> currentBoard, char player);
    std::vector<QPoint> getAvailableMoves(const std::vector<std::vector<char>>& b);
};

// --- GameDialog Class Definition ---
// Provides options to start a game (PvP or PvAI) and to replay previous games.
// This class records the moves for the current game.
class GameDialog : public QDialog
{
    Q_OBJECT

public:
    GameDialog(QWidget *parent = nullptr);
    ~GameDialog();

public slots:
    void on_pvpButton_clicked();
    void on_pvaiButton_clicked();
    void on_replayButton_clicked();
    void onComboBoxActivated(int index); // Called when a game number is selected for replay
    void onGameOver(const QString& winner);
    void recordMove(int row, int col, char player); // Records every move

private:
    void startGame(int mode);

    GameBoard* gameBoard;
    QGridLayout* mainLayout;
    QVBoxLayout* verticalLayout;
    QHBoxLayout* buttonLayout;
    QPushButton* pvpButton;
    QPushButton* pvaiButton;
    QPushButton* replayButton;
    QComboBox* comboBoxGameList;  // Displays only game numbers for replay
    QString player1Name;
    QString player2Name;
    int gameMode;
    std::vector<Move> moves;      // Stores the current game's moves
};

// --- HistoryDialog Class Definition ---
// (Optional) Displays a textual summary of game history.
class HistoryDialog : public QDialog
{
    Q_OBJECT

public:
    HistoryDialog(QWidget *parent = nullptr);
    ~HistoryDialog();
    void setGameHistory(const std::vector<GameRecord>& history);

private slots:
    void on_closeButton_clicked();

private:
    void displayGameHistory();

    QGridLayout* replayLayout;
    QVBoxLayout* mainLayout;
    QPushButton* closeButton;
    std::vector<GameRecord> gameHistory;
    QTextEdit* historyTextEdit;
    bool isReplaying;
    QLabel* titleLabel;
};

// --- ReplayDialog Class Definition ---
// Provides animated replay of a selected game record using a 3x3 grid.
class ReplayDialog : public QDialog
{
    Q_OBJECT
public:
    ReplayDialog(const std::vector<Move>& moves, QWidget* parent = nullptr);
    ~ReplayDialog();

private slots:
    void playNextMove();
    void on_closeButton_clicked();

private:
    void initializeBoard();

    QGridLayout* boardLayout;
    std::vector<QLabel*> cellLabels; // 9 labels for the game grid
    QTimer* timer;
    std::vector<Move> movesToReplay;
    int moveIndex;
    QPushButton* closeButton;
};

// --- MainWindow Class Definition ---
// Manages user authentication and account‑specific game history. Also supports
// password reset after three failed sign‑in attempts.
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // gameHistory pertains to the current user.
    static std::vector<GameRecord> gameHistory;
    // Returns the file path for the current user's history.
    static QString getHistoryFilePath();

    // currentUser is set upon successful sign in.
    static QString currentUser;

    static void saveGameHistory();

private slots:
    // Renamed slots to avoid auto‑connection conflicts.
    void signInButtonClicked();
    void signUpButtonClicked();
    void playGameButtonClicked();
    void viewHistoryButtonClicked();

private:
    Ui::MainWindow *ui;
    GameDialog* gameDialog;
    HistoryDialog* historyDialog;

    bool checkCredentials(const QString &username, const QString &password);
    std::unordered_map<QString, QString> loadUsers();
    void saveUser(const QString& username, const QString& password);
    void updateUserPassword(const QString &username, const QString &newPassword); // Updates user's password in users.txt

    static void loadGameHistory();
};

#endif // MAINWINDOW_H
