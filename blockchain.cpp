#include "blockchain.h"
#include <fstream>
#include <set>

using namespace std;

// Helper function to check if the signature exists in keys file
bool isSignatureValid(const string& signature) {
    ifstream file("keys");
    if (!file.is_open()) {
        cout << "[ERROR] 'keys' file not found." << endl;
        return false;
    }

    string line;
    while (getline(file, line)) {
        if (line == signature) {
            return true;
        }
    }
    return false;
}

// Block constructor
Block::Block(string prevHash) : previousHash(prevHash), isDirty(true) {
    cout << "[INFO] Creating new block with previous hash: " << prevHash << endl;
    hash = calculateHash();
    cout << "[INFO] New block hash: " << hash << endl;
}

// Add transaction to the block
void Block::addTransaction(const string& sender, const string& receiver, double amount, const string& signature) {
    cout << "[INFO] Adding transaction: " << sender << " -> " << receiver << " : " << amount << endl;

    // Verify balance
    Blockchain blockchain;
    double senderBalance = blockchain.getBalance(sender);
    if (sender != "SYSTEM" && senderBalance < amount) {
        cout << "[ERROR] Insufficient balance for transaction from " << sender << endl;
        return;
    }

    if (!isSignatureValid(signature)) {
        cout << "[ERROR] Invalid transaction signature." << endl;
        return;
    }

    transactions.push_back({sender, receiver, amount, signature});
    isDirty = true; // Mark block as modified
}

// Add data to the block
void Block::addData(const string& app_id, const string& data) {
    cout << "[INFO] Adding data to block: App ID: " << app_id << " | Data: " << data << endl;
    this->data.push_back({app_id, data});
    isDirty = true; // Mark block as modified
}

// Compute block hash by concatenating its information
string Block::calculateHash() const {
    stringstream ss;
    ss << previousHash;
    for (const auto& d : data) ss << d.app_id << d.data;
    for (const auto& t : transactions) ss << t.sender << t.receiver << t.amount << t.signature;
    return sha256(ss.str());
}

// Get cached hash or recalculate if dirty
string Block::getHash() const {
    if (isDirty) {
        hash = calculateHash();
        isDirty = false;
    }
    return hash;
}

// SHA-256 function to generate the hash
string Block::sha256(const string& input) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, input.c_str(), input.size());
    SHA256_Final(hash, &sha256);
    stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) ss << hex << setw(2) << setfill('0') << (int)hash[i];
    return ss.str();
}

// Convert block to JSON format
json Block::toJSON() const {
    json j;
    j["previousHash"] = previousHash;
    j["hash"] = hash;
    j["data"] = json::array();
    for (const auto& d : data) j["data"].push_back({{"app_id", d.app_id}, {"data", d.data}});
    j["transactions"] = json::array();
    for (const auto& t : transactions) j["transactions"].push_back({{"sender", t.sender}, {"receiver", t.receiver}, {"amount", t.amount}, {"signature", t.signature}});
    return j;
}

// Blockchain constructor
Blockchain::Blockchain() {
    ifstream file("blockchain.json");
    if (file.good()) {
        loadFromFile("blockchain.json");
    } else {
        cout << "[INFO] Initializing blockchain with the genesis block..." << endl;
        chain.emplace_back("0");
    }
}

// Blockchain Validation System
bool Blockchain::isChainValid() const {
    if (chain.empty()) {
        cout << "[ERROR] Blockchain is empty." << endl;
        return false;
    }

    for (size_t i = 1; i < chain.size(); i++) {
        const Block& currentBlock = chain[i];
        const Block& previousBlock = chain[i - 1];

        if (currentBlock.getHash() != currentBlock.calculateHash()) {
            cerr << "[ERROR] Block " << i << " has an invalid hash!" << endl;
            return false;
        }

        if (currentBlock.getPreviousHash() != previousBlock.getHash()) {
            cerr << "[ERROR] Block " << i << " has an incorrect previous hash!" << endl;
            return false;
        }
    }

    cout << "[INFO] Blockchain is valid." << endl;
    return true;
}

// Add a new empty block to the blockchain
void Blockchain::addBlock() {
    cout << "[INFO] Adding new block..." << endl;
    chain.emplace_back(chain.back().getHash());
    saveToFile("blockchain.json");
}

// Add transaction to the last block
void Blockchain::addTransaction(const string& sender, const string& receiver, double amount, const string& signature) {
    if (!chain.empty()) {
        cout << "[INFO] Adding transaction to last block..." << endl;
        chain.back().addTransaction(sender, receiver, amount, signature);
        saveToFile("blockchain.json");
    }
}

// Add data to the last block
void Blockchain::addData(const string& app_id, const string& data) {
    if (!chain.empty()) {
        cout << "[INFO] Adding data to last block..." << endl;
        chain.back().addData(app_id, data);
        saveToFile("blockchain.json");
    }
}

// Display the current blockchain state
void Blockchain::printChain() const {
    cout << "\n[INFO] Current Blockchain State:" << endl;
    for (const auto& block : chain) {
        cout << block.toJSON().dump(4) << endl;
        cout << "----------------------" << endl;
    }
}

// Save blockchain to JSON file
void Blockchain::saveToFile(const string& filename) const {
    json j;
    j["chain"] = json::array();
    for (const auto& block : chain) j["chain"].push_back(block.toJSON());
    ofstream file(filename);
    file << j.dump(4);
    file.close();
    cout << "[INFO] Blockchain saved to " << filename << endl;
}

// Load blockchain from JSON file
void Blockchain::loadFromFile(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "[ERROR] Failed to open file: " << filename << endl;
        return;
    }
    json j;
    file >> j;
    file.close();
    chain.clear();

    for (const auto& blk : j["chain"]) {
        Block b(blk["previousHash"]);
        b.hash = b.calculateHash();
        if (b.hash != blk["hash"]) {
            cout << "[ERROR] Data integrity check failed for a block!" << endl;
            return;
        }

        if (blk.contains("data")) {
            for (const auto& d : blk["data"]) b.data.push_back({d["app_id"], d["data"]});
        }

        if (blk.contains("transactions")) {
            for (const auto& t : blk["transactions"]) b.transactions.push_back({t["sender"], t["receiver"], t["amount"], t["signature"]});
        }

        chain.push_back(b);
    }
    cout << "[INFO] Blockchain loaded from " << filename << endl;
}

// Get balance for Wallet
double Blockchain::getBalance(const string& wallet_id) const {
    double balance = 0.0;
    for (const auto& block : chain) {
        for (const auto& transaction : block.transactions) {
            if (transaction.sender == wallet_id) {
                balance -= transaction.amount;
            }
            if (transaction.receiver == wallet_id) {
                balance += transaction.amount;
            }
        }
    }
    return balance;
}

