#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <chrono>
#include <iomanip>
#include <string>

using namespace std;

void DealerDrawMessage() {
    cout << endl << "Ділер витягує карту....\n" << endl;
    this_thread::sleep_for(chrono::seconds(2));
}
void StartGame() {
    cout << endl << "Нова партія, витягуються карти....\n" << endl;
    this_thread::sleep_for(chrono::seconds(1));
}

int streakWins = 0;
void PrintStreakMessage() {
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
    void PrintBalance() const {
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
    int GetAmount() const { return amount; }
    void Clear() { amount = 0; }
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
    Card(int v, const string &s) {
        value = v;
        suit = s;
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
};

class CardRenderer {
public:
    static void PrintHand(const vector<Card>& cards, bool hideFirst=false) {
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
                    string("│") + (label.size()==1 ? label+" " : label) + "   │",
                    "│     │",
                    string("│   ") + suit + " │",
                    "└─────┘"
                });
            }
        }
        for (int row = 0; row < 5; row++) {
            for (size_t i = 0; i < rendered.size(); i++) cout << rendered[i][row] << " ";
            cout << "\n";
        }
    }
};

class Hand {
    vector<Card> cards;
public:
    void clear() { cards.clear(); }
    void addCard() { cards.push_back(Card()); }
    void addCard(const Card &c) { cards.push_back(c); }
    Card removeCard(size_t idx) {
        Card c = cards[idx];
        cards.erase(cards.begin() + idx);
        return c;
    }
    const vector<Card>& getCards() const { return cards; }
    int getTotal() const {
        int total = 0, aceCount = 0;
        for (auto &c : cards) {
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
        CardRenderer::PrintHand(cards, hideFirst);
        cout << "(сума: " << (hideFirst ? "??" : to_string(getTotal())) << ")\n";
    }
    bool isBust() const { return getTotal() > 21; }
};

Hand player, dealer;
bool playerBlackjack = false;
PlayerBalance playerBalance;
Bet currentBet;

void PlayerTurn() {
    if (player.getTotal() == 21) {
        playerBlackjack = true;
        cout << endl << "БЛЕКДЖЕК! Ви виграли!\n";
        streakWins++;
        PrintStreakMessage();
        int amt = currentBet.GetAmount();
        playerBalance.add(amt * 2); 
        currentBet.Clear();
        return;
    }
    while (true) {
        if (player.isBust()) {
            cout << "Перебір! Ви програли.\n";
            streakWins = 0;
            currentBet.Clear();
            return;
        }
        cout << "Ваш вибір (w = ще, s = стоп, q = вихід): ";
        char choice;
        cin >> choice;
        if (choice == 'w') {
            player.addCard();
            DealerDrawMessage();
            cout << "Карти гравця:\n";
            player.printHand();
            if (player.getTotal() == 21) {
                cout << endl << "БЛЕКДЖЕК! Ви виграли!\n";
                streakWins++;
                PrintStreakMessage();
                int amt = currentBet.GetAmount();
                playerBalance.add(amt * 2);
                currentBet.Clear();
                playerBlackjack = true;
                return;
            }
        }
        else if (choice == 's') break;
        else if (choice == 'q') exit(0);
    }
}


void DealerTurn() {
    cout << "\nКарти дилера:\n";
    dealer.printHand();
    while (dealer.getTotal() < 17) {
        DealerDrawMessage();
        dealer.addCard();
        cout << "Карти дилера:\n";
        dealer.printHand();
    }
}


bool CanSplit(const Hand& h) {
    const auto &cards = h.getCards();
    return (cards.size() == 2 && cards[0].getValue() == cards[1].getValue());
}

void HandleSplit(PlayerBalance& playerBalance, Bet& bet, Hand& playerHand, Hand& dealerHand) {
    if (!CanSplit(playerHand)) {
        cout << "Спліт неможливий (не дві однакові карти).\n";
        return;
    }

    int baseBet = bet.GetAmount();
    if (!playerBalance.deduct(baseBet)) {
        cout << "Недостатньо грошей для спліту!\n";
        return;
    }

    const auto &pcards = playerHand.getCards();
    Hand hand1, hand2;
    hand1.clear(); hand2.clear();
    hand1.addCard(pcards[0]);
    hand2.addCard(pcards[1]);

    playerHand.clear();

    hand1.addCard();
    hand2.addCard();

    cout << "Спліт виконано — граємо двома руками. (ставка дублюється)\n\n";

    auto playSingleHand = [&](Hand &h, const string &name) {
        cout << "=== " << name << " ===\n";
        h.printHand();
        while (!h.isBust()) {
            cout << "Ваш вибір для " << name << " (w = ще, s = стоп): ";
            char c; cin >> c;
            if (c == 'w') {
                h.addCard();
                DealerDrawMessage();
                h.printHand();
            } else break;
        }
        if (h.isBust()) cout << "Перебір у " << name << "!\n";
    };

    playSingleHand(hand1, "Рука 1");
    playSingleHand(hand2, "Рука 2");

    DealerTurn();

    int dealerTotal = dealerHand.getTotal();

    auto evaluateAndPayout = [&](Hand &h, int handNumber) {
        cout << "\nРезультат руки " << handNumber << ": (сума " << h.getTotal() << ")\n";
        if (h.isBust()) {
            cout << "Рука " << handNumber << " — програш.\n";
            return;
        }
        if (dealerHand.isBust()) {
            cout << "Дилер перебрав — рука " << handNumber << " виграла!\n";
            playerBalance.add(baseBet * 2); 
            streakWins++;
            return;
        }
        int tot = h.getTotal();
        if (tot > dealerTotal) {
            cout << "Рука " << handNumber << " — виграш!\n";
            playerBalance.add(baseBet * 2);
            streakWins++;
        } else if (tot < dealerTotal) {
            cout << "Рука " << handNumber << " — програш.\n";
        } else {
            cout << "Рука " << handNumber << " — нічия (push).\n";
            playerBalance.add(baseBet); 
        }
    };

    evaluateAndPayout(hand1, 1);
    evaluateAndPayout(hand2, 2);

    PrintStreakMessage();

    bet.Clear();
}

void PlayRound() {
    player.clear();
    dealer.clear();
    playerBlackjack = false;

    playerBalance.PrintBalance();
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

    StartGame();
    cout << "\n=== Нова партія ===\n";
    cout << "Карти гравця:\n";
    player.printHand();
    cout << "Карти дилера:\n";
    dealer.printHand(true);

    if (CanSplit(player)) {
        cout << "У вас дві однакові карти! Бажаєте спліт? (y/n): ";
        char choice;
        cin >> choice;
        if (choice == 'y' || choice == 'Y') {
            HandleSplit(playerBalance, currentBet, player, dealer);
            return;
        }
    }

    PlayerTurn();

    if (player.isBust() || playerBlackjack) return;

    DealerTurn();

    if (dealer.isBust()) {
        cout << endl << "Дилер перебрав! Ви виграли!\n";
        streakWins++;
        PrintStreakMessage();
        int amt = currentBet.GetAmount();
        playerBalance.add(amt * 2);
        currentBet.Clear();
        return;
    }

    int playerTotal = player.getTotal();
    int dealerTotal = dealer.getTotal();
    cout << "\nВаш результат: " << playerTotal << endl;
    cout << "Результат дилера: " << dealerTotal << endl;

    if (playerTotal > dealerTotal) {
        cout << endl << "Ви виграли!\n";
        streakWins++;
        PrintStreakMessage();
        int amt = currentBet.GetAmount();
        playerBalance.add(amt * 2);
        currentBet.Clear();
    }
    else if (playerTotal < dealerTotal) {
        cout << endl << "Ви програли!\n";
        streakWins = 0;
        currentBet.Clear();
    }
    else {
        cout << endl << "Нічия!\n";
        playerBalance.add(currentBet.GetAmount());
        currentBet.Clear();
    }
}

int main(){
    setlocale(LC_ALL, "");
    srand(static_cast<unsigned>(time(0)));
    while (true){
        system("clear");
        PlayRound();
        this_thread::sleep_for(chrono::seconds(4));
    }
    return 0;
}
