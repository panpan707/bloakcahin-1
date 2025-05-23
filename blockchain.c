// Phiên bản C++ chuyển từ mã nguồn blockchain C
// Dùng class, string, vector, map, và các cấu trúc hiện đại

#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <map>
#include <openssl/sha.h>
#include <sstream>
#include <iomanip>

using namespace std;

const int MAX_TRANSACTIONS = 10;
const int DIFFICULTY = 4;

struct Transaction {
    string sender;
    string recipient;
    double amount;
    time_t timestamp;

    Transaction(string s, string r, double a)
        : sender(s), recipient(r), amount(a), timestamp(time(nullptr)) {}
};

string sha256(const string &data) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((const unsigned char *)data.c_str(), data.size(), hash);
    stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << hex << setw(2) << setfill('0') << (int)hash[i];
    }
    return ss.str();
}

class Block {
public:
    int index;
    vector<Transaction> transactions;
    time_t timestamp;
    string previous_hash;
    string hash;
    int nonce;
    Block *next;

    Block(int idx, const vector<Transaction> &txs, const string &prev_hash)
        : index(idx), transactions(txs), timestamp(time(nullptr)), previous_hash(prev_hash), nonce(0), next(nullptr) {
        mine();
    }

    void mine() {
        string target(DIFFICULTY, '0');
        do {
            hash = calculate_hash();
            nonce++;
        } while (hash.substr(0, DIFFICULTY) != target);
        cout << "Block mined: " << hash << endl;
    }

    string calculate_hash() const {
        stringstream ss;
        ss << index << timestamp << nonce << previous_hash;
        for (const auto &tx : transactions) {
            ss << tx.sender << tx.recipient << tx.amount << tx.timestamp;
        }
        return sha256(ss.str());
    }
};

class Blockchain {
private:
    Block *head;
    Block *tail;
    int size;
    vector<Transaction> pending_transactions;
    double mining_reward;
    map<string, double> wallet_balances;

public:
    Blockchain() : head(nullptr), tail(nullptr), size(0), mining_reward(10.0) {
        create_genesis_block();
    }

    ~Blockchain() {
        Block *curr = head;
        while (curr) {
            Block *next = curr->next;
            delete curr;
            curr = next;
        }
    }

    void create_genesis_block() {
        Block *genesis = new Block(0, {}, "0");
        head = tail = genesis;
        size = 1;
    }

    void add_transaction(const string &sender, const string &recipient, double amount) {
        if (sender != "SYSTEM" && wallet_balances[sender] < amount) {
            cout << "Insufficient funds for " << sender << endl;
            return;
        }
        pending_transactions.emplace_back(sender, recipient, amount);
        cout << "Transaction added: " << sender << " -> " << recipient << ": " << amount << endl;
    }

    void mine_block(const string &miner_address) {
        add_transaction("SYSTEM", miner_address, mining_reward);
        Block *new_block = new Block(size, pending_transactions, tail->hash);
        tail->next = new_block;
        tail = new_block;
        size++;

        for (const auto &tx : pending_transactions) {
            wallet_balances[tx.sender] -= tx.amount;
            wallet_balances[tx.recipient] += tx.amount;
        }
        pending_transactions.clear();
        cout << "Block mined and added to chain.\n";
    }

    void print_chain() const {
        Block *curr = head;
        while (curr) {
            cout << "Block #" << curr->index << " Hash: " << curr->hash << endl;
            for (const auto &tx : curr->transactions) {
                cout << "  " << tx.sender << " -> " << tx.recipient << ": " << tx.amount << endl;
            }
            curr = curr->next;
        }
    }

    void check_balance(const string &wallet) const {
        auto it = wallet_balances.find(wallet);
        double balance = (it != wallet_balances.end()) ? it->second : 0.0;
        cout << "Balance for " << wallet << ": " << balance << endl;
    }
};

int main() {
    Blockchain bc;
    int choice;
    string sender, recipient, miner;
    double amount;

    while (true) {
        cout << "\n===== BLOCKCHAIN MENU =====\n";
        cout << "1. Add Transaction\n2. Mine Block\n3. Print Chain\n4. Check Balance\n5. Exit\nChoice: ";
        cin >> choice;
        cin.ignore();

        switch (choice) {
            case 1:
                cout << "Sender: "; getline(cin, sender);
                cout << "Recipient: "; getline(cin, recipient);
                cout << "Amount: "; cin >> amount; cin.ignore();
                bc.add_transaction(sender, recipient, amount);
                break;
            case 2:
                cout << "Miner address: "; getline(cin, miner);
                bc.mine_block(miner);
                break;
            case 3:
                bc.print_chain();
                break;
            case 4:
                cout << "Wallet address: "; getline(cin, sender);
                bc.check_balance(sender);
                break;
            case 5:
                return 0;
            default:
                cout << "Invalid choice.\n";
        }
    }
}
