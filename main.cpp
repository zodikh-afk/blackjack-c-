#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <chrono>
#include <iomanip>
using namespace std;

void dealerDrawMessage() {
    cout << endl << "Ділер витягує карту....\n" << endl;
    this_thread::sleep_for(chrono::seconds(2)); 
}
void startGame() {
    cout << endl << "Нова партія, витягуються карти....\n" << endl;
    this_thread::sleep_for(chrono::seconds(3)); 
}

int streakWins = 0;
void printStreakMessage() {
    if (streakWins > 1) {
        cout << "🔥 Серія перемог: " << streakWins << " підряд! 🔥\n";
    }
}

class PlayerBalance {
    int balance;
public:
    PlayerBalance(int initialBalance = 1000) : balance(initialBalance) {}
    int getBalance() const { return balance; }
    void add(int amount) { balance += amount; }
    bool deduct(int amount) {
        if (amount > balance) return false;
        balance -= amount;
        return true;
    }
    void printBalance() const {
        cout << "💰 Баланс: " << balance << endl;
    }
};

class Bet {
    int amount;
public:
    Bet() : amount(0) {}
    bool place(PlayerBalance &player, int betAmount) {
        if (betAmount <= 0) {
            cout << "Ставка повинна бути більше 0!\n";
            return false;
        }
        if (!player.deduct(betAmount)) {
            cout << "Недостатньо грошей для ставки!\n";
            return false;
        }
        amount = betAmount;
        cout << "Ставка зроблена: " << amount << endl;
        return true;
    }
    void win(PlayerBalance &player, float multiplier = 2.0) { 
        int winnings = static_cast<int>(amount * multiplier);
        cout << "Ви виграли: " << winnings << "!\n";
        player.add(winnings);
        amount = 0;
    }
    void lose() {
        cout << "Ви програли ставку: " << amount << "!\n";
        amount = 0;
    }
    int getAmount() const { return amount; }
};

class Card {
    int value;
    string suit;
public:
    Card() {
        value = 2 + rand() % 13;
        static const string suits[4] = {"♠", "♥", "♦", "♣"};
        suit = suits[rand() % 4];
    }
    int getValue() const {
        if (value >= 11 && value <= 13) return 10;
        if (value == 14) return 11;
        return value;
    }
    bool isAce() const { return value == 14; }
    string getLabel() const {
        if (value <= 10) return to_string(value);
        if (value == 11) return "J";
        if (value == 12) return "Q";
        if (value == 13) return "K";
        return "A";
    }
    string getSuit() const { return suit; }
    void print() const { cout << getLabel() << suit; }
};

class CardRenderer {
public:
    static void printCard(const Card &c) {
        string label = c.getLabel();
        string suit = c.getSuit();
        cout << "┌─────┐\n";
        cout << "│" << setw(2) << left << label << "   │\n";
        cout << "│     │\n";
        cout << "│   " << suit << " │\n";
        cout << "└─────┘\n";
    }
    static void printHand(const vector<Card>& cards, bool hideFirst=false) {
        vector<vector<string>> rendered;
        for (size_t i = 0; i < cards.size(); i++) {
            if (i==0 && hideFirst) {
                rendered.push_back({
                    "┌─────┐",
                    "│░░░░░│",
                    "│░░░░░│",
                    "│░░░░░│",
                    "└─────┘"
                });
            } else {
                string label = cards[i].getLabel();
                string suit = cards[i].getSuit();
                rendered.push_back({
                    "┌─────┐",
                    "│" + (label.size()==1 ? label+" " : label) + "   │",
                    "│     │",
                    "│   "s + suit + " │",
                    "└─────┘"
                });
            }
        }
        for (int row=0; row<5; row++) {
            for (size_t i=0; i<rendered.size(); i++) cout << rendered[i][row] << " ";
            cout << "\n";
        }
    }
};

class Hand {
    vector<Card> cards;
public:
    void clear(){ cards.clear(); }
    void addCard(){ cards.push_back(Card()); }
    int getTotal() const {
        int total = 0, aceCount = 0;
        for (auto &c : cards){
            total += c.getValue();
            if (c.isAce()) aceCount++;
        }
        while (total > 21 && aceCount > 0) {
            total -= 10;
            aceCount--;
        }
        return total;
    }
    void printHand(bool hideFirst = false) const {
        CardRenderer::printHand(cards, hideFirst);
        cout << "(сума: " << (hideFirst ? "??" : to_string(getTotal())) << ")\n";
    }
    bool isBust() const { return getTotal() > 21; }
};

Hand player, dealer;
bool playerBlackjack = false; 
PlayerBalance playerBalance;
Bet currentBet;

void playerTurn() {
    if (player.getTotal() == 21) {           
        playerBlackjack = true;              
        cout << endl << "БЛЕКДЖЕК! Ви виграли!\n";
        streakWins++;
        printStreakMessage();
        currentBet.win(playerBalance);
        return;
    }
    while (true) {
        if (player.isBust()) {
            cout << "Перебір! Ви програли.\n";
            streakWins = 0;
            currentBet.lose();
            return;
        }
        cout << "Ваш вибір (w = ще, s = стоп, q = вихід): ";
        char choice;
        cin >> choice;
        if (choice == 'w') {
            player.addCard();
            dealerDrawMessage();
            cout << "Карти гравця:\n";
            player.printHand();
            if (player.getTotal() == 21) {    
                cout << endl <<"БЛЕКДЖЕК! Ви виграли!\n";
                streakWins++;
                printStreakMessage();
                playerBlackjack = true;
                currentBet.win(playerBalance);
                return;
            }
        } 
        else if (choice == 's') break;
        else if (choice == 'q') exit(0);
    }
}

void dealerTurn() {
    cout << "\nКарти дилера:\n";
    dealer.printHand();
    while (dealer.getTotal() < 17) {
        dealerDrawMessage(); cout << endl;
        dealer.addCard();
        cout << "Карти дилера:\n";
        dealer.printHand();
    }
}

void playRound() {
    player.clear();
    dealer.clear();
    playerBlackjack = false;  

    playerBalance.printBalance();
    int betAmount;
    cout << "Введіть вашу ставку: ";
    cin >> betAmount;
    while (!currentBet.place(playerBalance, betAmount)) {
        cout << "Спробуйте ще раз: ";
        cin >> betAmount;
    }

    player.addCard();
    dealer.addCard();
    player.addCard();
    dealer.addCard();

    startGame(); cout << endl;
    cout << "\n=== Нова партія ===\n";
    cout << "Карти гравця:\n";
    player.printHand();
    cout << "Карти дилера:\n";
    dealer.printHand(true);

    playerTurn();
    if (player.isBust() || playerBlackjack) return;

    dealerTurn();
    if (dealer.isBust()) {
        cout << endl << "Дилер перебрав! Ви виграли!\n";
        streakWins++;
        printStreakMessage();
        currentBet.win(playerBalance);
        return;
    }

    int playerTotal = player.getTotal();
    int dealerTotal = dealer.getTotal();
    cout << "\nВаш результат: " << playerTotal << endl;
    cout << "Результат дилера: " << dealerTotal << endl;

    if (playerTotal > dealerTotal) {
        cout << endl << "Ви виграли!\n";
        streakWins++;
        printStreakMessage();
        currentBet.win(playerBalance);
    }
    else if (playerTotal < dealerTotal) {
        cout << endl << "Ви програли!\n";
        streakWins = 0;
        currentBet.lose();
    }
    else {
        cout << endl << "Нічия!\n";
        playerBalance.add(currentBet.getAmount()); // повертаємо ставку при нічиї
        currentBet = Bet(); // обнуляємо ставку
    }
}

int main(){
    setlocale(LC_ALL, "ukr");
    srand(time(0));
    while (true){
        system("clear");    
        playRound();
        this_thread::sleep_for(chrono::seconds(4)); 
    }
    return 0;
}
