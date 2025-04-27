#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QInputDialog>
#include <QStackedWidget>
#include <QRandomGenerator>
#include <QDebug>
#include <QTextEdit>
#include <QScrollBar>
#include <QComboBox>

// ------------------------------------------------------------------
// Helper functions for minimax evaluation (operate on a given board state)

static bool evalIsWinner(const std::vector<std::vector<char>> &board, char player) {
    for (int i = 0; i < 3; i++) {
        if (board[i][0] == player && board[i][1] == player && board[i][2] == player)
            return true;
        if (board[0][i] == player && board[1][i] == player && board[2][i] == player)
            return true;
    }
    if (board[0][0] == player && board[1][1] == player && board[2][2] == player)
        return true;
    if (board[0][2] == player && board[1][1] == player && board[2][0] == player)
        return true;
    return false;
}

static bool evalIsFull(const std::vector<std::vector<char>> &board) {
    for (const auto &row : board)
        for (char cell : row)
            if (cell == ' ')
                return false;
    return true;
}

// ------------------------------------------------------------------
// GameBoard Implementation

GameBoard::GameBoard(QWidget *parent, int mode)
    : QWidget(parent), currentPlayer('X'), gameActive(true), gameMode(mode)
{
    mainLayout = new QGridLayout(this);
    mainLayout->setSpacing(0);
    initializeBoard();
    if (gameMode == 2 && currentPlayer == 'O')
        QTimer::singleShot(100, this, &GameBoard::triggerAiMove);
}

GameBoard::~GameBoard()
{
    for (auto& row : buttons) {
        for (auto& button : row)
            delete button;
        row.clear();
    }
    buttons.clear();
    delete mainLayout;
}

void GameBoard::initializeBoard()
{
    board.assign(3, std::vector<char>(3, ' '));
    buttons.assign(3, std::vector<QPushButton*>(3, nullptr));
    for (int row = 0; row < 3; ++row)
    {
        for (int col = 0; col < 3; ++col)
        {
            buttons[row][col] = new QPushButton(this);
            buttons[row][col]->setFixedSize(80, 80);
            buttons[row][col]->setStyleSheet("font: 24px;");
            mainLayout->addWidget(buttons[row][col], row, col);
            connect(buttons[row][col], &QPushButton::clicked, this, &GameBoard::onCellClicked);
        }
    }
    resetBoard();
    if (gameMode == 2 && currentPlayer == 'O')
        QTimer::singleShot(100, this, &GameBoard::triggerAiMove);
}

void GameBoard::resetBoard()
{
    for (int row = 0; row < 3; ++row)
        for (int col = 0; col < 3; ++col)
        {
            board[row][col] = ' ';
            buttons[row][col]->setText("");
            buttons[row][col]->setEnabled(true);
            QString style = "QPushButton { background-color: #f0f0f0; border: 1px solid #ccc; }";
            buttons[row][col]->setStyleSheet(style);
        }
    currentPlayer = 'X';
    gameActive = true;
    if (gameMode == 2 && currentPlayer == 'O')
        QTimer::singleShot(100, this, &GameBoard::triggerAiMove);
}

bool GameBoard::makeMove(int row, int col, char player)
{
    if (row >= 0 && row < 3 && col >= 0 && col < 3 &&
        board[row][col] == ' ' && gameActive)
    {
        board[row][col] = player;
        updateButtonText(row, col, player);
        return true;
    }
    return false;
}

bool GameBoard::checkWinner(char player)
{
    for (int i = 0; i < 3; ++i)
    {
        if (board[i][0] == player &&
            board[i][1] == player &&
            board[i][2] == player)
            return true;
        if (board[0][i] == player &&
            board[1][i] == player &&
            board[2][i] == player)
            return true;
    }
    if (board[0][0] == player &&
        board[1][1] == player &&
        board[2][2] == player)
        return true;
    if (board[0][2] == player &&
        board[1][1] == player &&
        board[2][0] == player)
        return true;
    return false;
}

bool GameBoard::isFull()
{
    for (const auto &row : board)
        for (char cell : row)
            if (cell == ' ')
                return false;
    return true;
}

void GameBoard::switchPlayer()
{
    currentPlayer = (currentPlayer == 'X') ? 'O' : 'X';
    qDebug() << "switchPlayer: Current player is now" << currentPlayer;
    if (gameMode == 2 && currentPlayer == 'O' && gameActive)
        triggerAiMove();
}

char GameBoard::getCurrentPlayer() const { return currentPlayer; }

std::vector<std::vector<char>> GameBoard::getBoard() const { return board; }

bool GameBoard::isEmpty(int row, int col) const
{
    return row >= 0 && row < 3 && col >= 0 && col < 3 && board[row][col] == ' ';
}

void GameBoard::updateButtonText(int row, int col, char text)
{
    buttons[row][col]->setText(QString(QChar(text)));
    buttons[row][col]->setEnabled(false);
    QString style;
    if (text == 'X')
        style = "QPushButton { background-color: #87CEFA; border: 1px solid #ccc; font: 24px; }";
    else
        style = "QPushButton { background-color: #FFA07A; border: 1px solid #ccc; font: 24px; }";
    buttons[row][col]->setStyleSheet(style);
}

void GameBoard::disableBoard()
{
    for (int row = 0; row < 3; ++row)
        for (int col = 0; col < 3; ++col)
            buttons[row][col]->setEnabled(false);
    gameActive = false;
}

void GameBoard::enableBoard()
{
    for (int row = 0; row < 3; ++row)
        for (int col = 0; col < 3; ++col)
            buttons[row][col]->setEnabled(true);
    gameActive = true;
}

void GameBoard::onCellClicked()
{
    QPushButton* clickedButton = qobject_cast<QPushButton*>(sender());
    if (!clickedButton || !gameActive)
        return;

    int row = -1, col = -1;
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            if (buttons[i][j] == clickedButton)
            {
                row = i;
                col = j;
                break;
            }
        }
        if (row != -1)
            break;
    }
    if (row == -1 || col == -1)
        return;

    if (makeMove(row, col, currentPlayer))
    {
        emit moveMade(row, col, currentPlayer);
        if (checkWinner(currentPlayer))
        {
            QString winnerName = (currentPlayer == 'X') ? "You" : "AI";
            if (gameMode == 1)
                winnerName = (currentPlayer == 'X') ? "Player 1" : "Player 2";
            emit gameOver(winnerName);
            disableBoard();
            gameActive = false;
        }
        else if (isFull())
        {
            emit gameOver("Draw");
            disableBoard();
            gameActive = false;
        }
        else
        {
            switchPlayer();
        }
    }
}

void GameBoard::triggerAiMove()
{
    if (!gameActive || currentPlayer != 'O' || gameMode != 2)
        return;
    qDebug() << "triggerAiMove: AI's turn, calling aiMove";
    QTimer::singleShot(100, this, &GameBoard::aiMove);
}

void GameBoard::aiMove()
{
    if (!gameActive || currentPlayer != 'O' || gameMode != 2)
        return;
    qDebug() << "aiMove: AI is making a move";

    QPoint bestMove = findBestMove();
    if (bestMove.x() != -1 && bestMove.y() != -1)
    {
        makeMove(bestMove.x(), bestMove.y(), currentPlayer);
        emit moveMade(bestMove.x(), bestMove.y(), currentPlayer);
        if (checkWinner(currentPlayer))
        {
            emit gameOver("AI");
            disableBoard();
            gameActive = false;
        }
        else if (isFull())
        {
            emit gameOver("Draw");
            disableBoard();
            gameActive = false;
        }
        else
        {
            switchPlayer();
        }
    }
    else
    {
        qDebug() << "aiMove: No valid move found!";
    }
}

// ------------------------------------------------------------------
// Enhanced AI using minimax

int GameBoard::minimax(std::vector<std::vector<char>> currentBoard, char player) {
    if (evalIsWinner(currentBoard, 'O'))
        return 10;
    if (evalIsWinner(currentBoard, 'X'))
        return -10;
    if (evalIsFull(currentBoard))
        return 0;

    if (player == 'O') { // Maximizing
        int bestScore = -1000;
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (currentBoard[i][j] == ' ') {
                    currentBoard[i][j] = 'O';
                    int score = minimax(currentBoard, 'X');
                    currentBoard[i][j] = ' ';
                    bestScore = std::max(bestScore, score);
                }
            }
        }
        return bestScore;
    } else { // Minimizing (player 'X')
        int bestScore = 1000;
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (currentBoard[i][j] == ' ') {
                    currentBoard[i][j] = 'X';
                    int score = minimax(currentBoard, 'O');
                    currentBoard[i][j] = ' ';
                    bestScore = std::min(bestScore, score);
                }
            }
        }
        return bestScore;
    }
}

QPoint GameBoard::findBestMove() {
    int bestScore = -1000;
    QPoint bestMove = { -1, -1 };
    std::vector<std::vector<char>> boardCopy = board;

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (boardCopy[i][j] == ' ') {
                boardCopy[i][j] = 'O'; // AI move candidate
                int score = minimax(boardCopy, 'X');
                boardCopy[i][j] = ' ';
                if (score > bestScore) {
                    bestScore = score;
                    bestMove = { i, j };
                }
            }
        }
    }
    qDebug() << "findBestMove: Chosen move at" << bestMove.x() << bestMove.y()
             << "with score" << bestScore;
    return bestMove;
}

// ------------------------------------------------------------------
// GameDialog Implementation

GameDialog::GameDialog(QWidget *parent)
    : QDialog(parent), gameBoard(nullptr), gameMode(0)
{
    this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mainLayout = new QGridLayout(this);
    verticalLayout = new QVBoxLayout();
    buttonLayout = new QHBoxLayout();

    pvpButton = new QPushButton("PvP (Two Players)", this);
    pvaiButton = new QPushButton("PvAI (Play against AI)", this);
    replayButton = new QPushButton("Replay Game", this);

    // ComboBox will display only game numbers.
    comboBoxGameList = new QComboBox(this);
    comboBoxGameList->addItem("Select a game...");
    connect(comboBoxGameList, QOverload<int>::of(&QComboBox::activated),
            this, &GameDialog::onComboBoxActivated);

    verticalLayout->addWidget(comboBoxGameList);
    verticalLayout->addWidget(replayButton);
    verticalLayout->addLayout(buttonLayout);
    buttonLayout->addWidget(pvpButton);
    buttonLayout->addWidget(pvaiButton);
    mainLayout->addLayout(verticalLayout, 0, 0);

    // Connect these buttons manually only once.
    connect(pvpButton, &QPushButton::clicked, this, &GameDialog::on_pvpButton_clicked);
    connect(pvaiButton, &QPushButton::clicked, this, &GameDialog::on_pvaiButton_clicked);
    connect(replayButton, &QPushButton::clicked, this, &GameDialog::on_replayButton_clicked);

    player1Name = "Player 1";
    player2Name = "Player 2";
}

GameDialog::~GameDialog()
{
    if (gameBoard)
        delete gameBoard;
    delete pvpButton;
    delete pvaiButton;
    delete replayButton;
    delete comboBoxGameList;
    delete buttonLayout;
    delete verticalLayout;
    delete mainLayout;
}

void GameDialog::on_pvpButton_clicked()
{
    player1Name = QInputDialog::getText(this, "Player 1 Name", "Enter name for Player 1:", QLineEdit::Normal, "Player 1");
    if (player1Name.isEmpty())
        player1Name = "Player 1";
    player2Name = QInputDialog::getText(this, "Player 2 Name", "Enter name for Player 2:", QLineEdit::Normal, "Player 2");
    if (player2Name.isEmpty())
        player2Name = "Player 2";
    startGame(1);
}

void GameDialog::on_pvaiButton_clicked()
{
    player1Name = QInputDialog::getText(this, "Player Name", "Enter your name:", QLineEdit::Normal, "Player");
    if (player1Name.isEmpty())
        player1Name = "Player";
    startGame(2);
}

void GameDialog::startGame(int mode)
{
    gameMode = mode;
    if (gameBoard)
        delete gameBoard;
    gameBoard = new GameBoard(this, gameMode);
    connect(gameBoard, &GameBoard::moveMade, this, &GameDialog::recordMove);
    connect(gameBoard, &GameBoard::gameOver, this, &GameDialog::onGameOver);
    mainLayout->addWidget(gameBoard, 1, 0);
    gameBoard->show();
    moves.clear();
    this->adjustSize();
}

void GameDialog::recordMove(int row, int col, char player)
{
    Move m;
    m.row = row;
    m.col = col;
    m.player = player;
    moves.push_back(m);
}

void GameDialog::onGameOver(const QString& winner)
{
    QString message = (winner == "Draw") ? "It's a draw!" : winner + " win!";
    QMessageBox::information(this, "Game Over", message);

    GameRecord record;
    record.mode = (gameMode == 1) ? "PvP" : "PvAI";
    record.winner = winner.toStdString();
    record.moves = moves;
    MainWindow::gameHistory.push_back(record);
    MainWindow::saveGameHistory();

    gameBoard->resetBoard();
    gameBoard->enableBoard();
    moves.clear();
}

void GameDialog::on_replayButton_clicked()
{
    comboBoxGameList->clear();
    if (MainWindow::gameHistory.empty())
    {
        QMessageBox::information(this, "Replay", "No games have been played yet.");
        return;
    }
    comboBoxGameList->addItem("Select a game...");
    for (size_t i = 0; i < MainWindow::gameHistory.size(); ++i)
    {
        QString itemText = QString("Game %1").arg(i + 1);
        comboBoxGameList->addItem(itemText);
    }
    comboBoxGameList->showPopup();
}

void GameDialog::onComboBoxActivated(int index)
{
    if (index <= 0 || index > static_cast<int>(MainWindow::gameHistory.size()))
    {
        QMessageBox::warning(this, "Replay", "Please select a valid game number.");
        return;
    }
    int selectedGameIndex = index - 1;
    const GameRecord &record = MainWindow::gameHistory[selectedGameIndex];
    if (record.moves.empty())
    {
        QMessageBox::warning(this, "Replay", "No move data available for this game.");
        return;
    }
    ReplayDialog* replayDialog = new ReplayDialog(record.moves, this);
    replayDialog->exec();
    delete replayDialog;
}

// ------------------------------------------------------------------
// HistoryDialog Implementation

HistoryDialog::HistoryDialog(QWidget *parent)
    : QDialog(parent), isReplaying(false)
{
    this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint & ~Qt::WindowMinimizeButtonHint);
    mainLayout = new QVBoxLayout(this);
    historyTextEdit = new QTextEdit(this);
    historyTextEdit->setReadOnly(true);
    historyTextEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    closeButton = new QPushButton("Close", this);
    titleLabel = new QLabel("Game History", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold;");
    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(historyTextEdit);
    mainLayout->addWidget(closeButton);
    connect(closeButton, &QPushButton::clicked, this, &HistoryDialog::on_closeButton_clicked);
    displayGameHistory();
}

HistoryDialog::~HistoryDialog()
{
    delete historyTextEdit;
    delete closeButton;
    delete mainLayout;
    delete titleLabel;
}

void HistoryDialog::setGameHistory(const std::vector<GameRecord>& history)
{
    gameHistory = history;
    displayGameHistory();
}

void HistoryDialog::displayGameHistory()
{
    historyTextEdit->clear();
    if (gameHistory.empty())
    {
        historyTextEdit->append("No games have been played yet.");
        return;
    }
    for (size_t i = 0; i < gameHistory.size(); ++i)
    {
        const GameRecord &record = gameHistory[i];
        QString gameInfo = QString("Game %1: Mode: %2, Winner: %3")
                               .arg(i + 1)
                               .arg(QString::fromStdString(record.mode))
                               .arg(QString::fromStdString(record.winner));
        historyTextEdit->append(gameInfo);
    }
    historyTextEdit->verticalScrollBar()->setValue(historyTextEdit->verticalScrollBar()->maximum());
}

void HistoryDialog::on_closeButton_clicked()
{
    this->close();
}

// ------------------------------------------------------------------
// ReplayDialog Implementation

ReplayDialog::ReplayDialog(const std::vector<Move>& moves, QWidget *parent)
    : QDialog(parent), movesToReplay(moves), moveIndex(0)
{
    this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setWindowTitle("Animated Replay");
    boardLayout = new QGridLayout(this);
    initializeBoard();
    closeButton = new QPushButton("Close", this);
    boardLayout->addWidget(closeButton, 3, 0, 1, 3);
    connect(closeButton, &QPushButton::clicked, this, &ReplayDialog::on_closeButton_clicked);
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &ReplayDialog::playNextMove);
    timer->start(500); // 500 ms interval between moves
}

ReplayDialog::~ReplayDialog()
{
    delete timer;
}

void ReplayDialog::initializeBoard()
{
    cellLabels.resize(9);
    for (int i = 0; i < 9; ++i)
    {
        cellLabels[i] = new QLabel("", this);
        cellLabels[i]->setFixedSize(80, 80);
        cellLabels[i]->setFrameStyle(QFrame::Box | QFrame::Plain);
        cellLabels[i]->setAlignment(Qt::AlignCenter);
        cellLabels[i]->setStyleSheet("font: 24px; background-color: #f0f0f0;");
    }
    int index = 0;
    for (int row = 0; row < 3; ++row)
        for (int col = 0; col < 3; ++col)
            boardLayout->addWidget(cellLabels[index++], row, col);
}

void ReplayDialog::playNextMove()
{
    if (moveIndex >= static_cast<int>(movesToReplay.size()))
    {
        timer->stop();
        return;
    }
    Move m = movesToReplay[moveIndex];
    int index = m.row * 3 + m.col;
    if (index >= 0 && index < cellLabels.size())
    {
        cellLabels[index]->setText(QString(QChar(m.player)));
        if (m.player == 'X')
            cellLabels[index]->setStyleSheet("font: 24px; background-color: #87CEFA; border: 1px solid #ccc;");
        else
            cellLabels[index]->setStyleSheet("font: 24px; background-color: #FFA07A; border: 1px solid #ccc;");
    }
    moveIndex++;
}

void ReplayDialog::on_closeButton_clicked()
{
    this->close();
}

// ------------------------------------------------------------------
// MainWindow Implementation with Password Reset Feature

std::vector<GameRecord> MainWindow::gameHistory;
QString MainWindow::currentUser = "";
QString MainWindow::getHistoryFilePath() {
    return currentUser + "_history.txt";
}

void MainWindow::saveGameHistory()
{
    QFile file(getHistoryFilePath());
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&file);
        for (const auto& record : gameHistory)
        {
            // Save as: mode|winner|row-col-player;row-col-player;...
            QString line = QString::fromStdString(record.mode) + "|" +
                           QString::fromStdString(record.winner) + "|";
            for (size_t i = 0; i < record.moves.size(); ++i)
            {
                const Move &m = record.moves[i];
                line += QString::number(m.row) + "-" +
                        QString::number(m.col) + "-" +
                        QString(m.player);
                if (i < record.moves.size() - 1)
                    line += ";";
            }
            out << line << "\n";
        }
        file.close();
    }
    else
        QMessageBox::critical(nullptr, "Error", "Could not write to history file.");
}

void MainWindow::loadGameHistory()
{
    gameHistory.clear();
    QFile file(getHistoryFilePath());
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&file);
        while (!in.atEnd())
        {
            QString line = in.readLine();
            QStringList fields = line.split("|");
            if (fields.size() < 2)
                continue;
            GameRecord record;
            record.mode = fields[0].toStdString();
            record.winner = fields[1].toStdString();
            if (fields.size() == 3 && !fields[2].isEmpty())
            {
                QStringList moveTokens = fields[2].split(";");
                for (const QString &token : moveTokens)
                {
                    QStringList parts = token.split("-");
                    if (parts.size() == 3)
                    {
                        Move m;
                        m.row = parts[0].toInt();
                        m.col = parts[1].toInt();
                        m.player = parts[2].at(0).toLatin1();
                        record.moves.push_back(m);
                    }
                }
            }
            gameHistory.push_back(record);
        }
        file.close();
    }
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), gameDialog(nullptr), historyDialog(nullptr)
{
    ui = new Ui::MainWindow();
    ui->setupUi(this);
    // Do not call manual connect on these UI buttons, as their slot names
    // no longer follow the auto‑connection pattern.
    connect(ui->SignIn, &QPushButton::clicked, this, &MainWindow::signInButtonClicked);
    connect(ui->SignUp, &QPushButton::clicked, this, &MainWindow::signUpButtonClicked);
    connect(ui->playGameButton, &QPushButton::clicked, this, &MainWindow::playGameButtonClicked);
    connect(ui->viewHistoryButton, &QPushButton::clicked, this, &MainWindow::viewHistoryButtonClicked);

    gameDialog = new GameDialog(this);
    historyDialog = new HistoryDialog(this);
}

MainWindow::~MainWindow()
{
    delete ui;
    if (gameDialog)
        delete gameDialog;
    if (historyDialog)
        delete historyDialog;
}

// New function: Update user password in users.txt.
void MainWindow::updateUserPassword(const QString &username, const QString &newPassword)
{
    QFile file("users.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::critical(this, "Error", "Cannot open users.txt for reading.");
        return;
    }
    QStringList lines;
    QTextStream in(&file);
    while (!in.atEnd())
        lines << in.readLine();
    file.close();

    bool found = false;
    for (int i = 0; i < lines.size(); i++)
    {
        QStringList parts = lines[i].split(' ');
        if (parts.size() == 2 && parts[0] == username)
        {
            lines[i] = username + " " + newPassword;
            found = true;
            break;
        }
    }
    if (!found)
    {
        QMessageBox::critical(this, "Error", "User not found in file.");
        return;
    }

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::critical(this, "Error", "Cannot open users.txt for writing.");
        return;
    }
    QTextStream out(&file);
    for (const QString &line : lines)
        out << line << "\n";
    file.close();
    QMessageBox::information(this, "Password Updated", "Your password has been updated successfully.");
}

// Modified sign-in logic with three-attempt limit and password reset option.
void MainWindow::signInButtonClicked()
{
    static int signInAttempts = 0;
    QString username = ui->Username->text();
    QString password = ui->Password->text();
    if (checkCredentials(username, password))
    {
        signInAttempts = 0;
        QMessageBox::information(this, "Sign In", "Sign in successful!");
        currentUser = username;
        loadGameHistory();
        ui->stackedWidget->setCurrentIndex(1);
    }
    else
    {
        signInAttempts++;
        if (signInAttempts >= 3)
        {
            QMessageBox::StandardButton reply = QMessageBox::question(
                this, "Reset Password",
                " Would you like to reset your password?",
                QMessageBox::Yes | QMessageBox::No);
            if (reply == QMessageBox::Yes)
            {
                bool ok;
                QString newPassword = QInputDialog::getText(this, "Reset Password",
                                                            "Enter new password:", QLineEdit::Password, "", &ok);
                if (ok && !newPassword.isEmpty())
                {
                    updateUserPassword(username, newPassword);
                    signInAttempts = 0;
                    currentUser = username;
                    loadGameHistory();
                    ui->stackedWidget->setCurrentIndex(1);
                }
                else
                {
                    QMessageBox::warning(this, "Warning", "Password was not updated. Please try signing in again.");
                }
            }
            else
            {
                QMessageBox::warning(this, "Sign In", "Incorrect username or password.");
            }
        }
        else
        {
            QMessageBox::warning(this, "Sign In", "Incorrect username or password.");
        }
    }
}

void MainWindow::signUpButtonClicked()
{
    QString username = ui->Username->text();
    QString password = ui->Password->text();
    if (username.isEmpty() || password.isEmpty())
    {
        QMessageBox::warning(this, "Sign Up", "Username and password cannot be empty.");
        return;
    }
    std::unordered_map<QString, QString> users = loadUsers();
    if (users.find(username) != users.end())
    {
        QMessageBox::warning(this, "Sign Up", "This username is already used, please choose another.");
        return;
    }
    saveUser(username, password);
    QMessageBox::information(this, "Sign Up", "Account created successfully!");
    currentUser = username;
    loadGameHistory();
    ui->stackedWidget->setCurrentIndex(1);
}

bool MainWindow::checkCredentials(const QString &username, const QString &password)
{
    std::unordered_map<QString, QString> users = loadUsers();
    auto it = users.find(username);
    return (it != users.end() && it->second == password);
}

std::unordered_map<QString, QString> MainWindow::loadUsers()
{
    std::unordered_map<QString, QString> users;
    QFile file("users.txt");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&file);
        while (!in.atEnd())
        {
            QString line = in.readLine();
            QStringList parts = line.split(' ');
            if (parts.size() == 2)
                users[parts[0]] = parts[1];
        }
        file.close();
    }
    return users;
}

void MainWindow::saveUser(const QString& username, const QString& password)
{
    QFile file("users.txt");
    if (file.open(QIODevice::Append | QIODevice::Text))
    {
        QTextStream out(&file);
        out << username << " " << password << "\n";
        file.close();
    }
    else
        QMessageBox::critical(this, "Error", "Could not write to users.txt");
}

void MainWindow::playGameButtonClicked()
{
    if (!gameDialog)
        gameDialog = new GameDialog(this);
    gameDialog->exec();
}

void MainWindow::viewHistoryButtonClicked()
{
    if (!historyDialog)
        historyDialog = new HistoryDialog(this);
    historyDialog->setGameHistory(gameHistory);
    historyDialog->exec();
}
