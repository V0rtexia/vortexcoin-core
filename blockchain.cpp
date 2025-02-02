#include "blockchain.h"

// Block constructor
Block::Block(string prevHash) : previousHash(prevHash) {
    cout << "Creating new block with previous hash: " << prevHash << endl;
    hash = calculateHash();
    cout << "New block hash: " << hash << endl;
}

// Add transaction to the block
void Block::addTransaction(string sender, string receiver, double amount) {
    cout << "Adding transaction: " << sender << " -> " << receiver << " : " << amount << endl;
    transactions.push_back({sender, receiver, amount});
    hash = calculateHash(); // Recalculate hash after modification
    cout << "Updated block hash: " << hash << endl;
}

// Add data to the block
void Block::addData(string app_id, string data) {
    cout << "Adding data to block: App ID: " << app_id << " | Data: " << data << endl;
    this->data.push_back({app_id, data});
    hash = calculateHash(); // Recalculate hash after modification
    cout << "Updated block hash: " << hash << endl;
}

// Compute block hash by concatenating its information
string Block::calculateHash() const {
    stringstream ss;
    ss << previousHash;
    for (const auto& d : data) ss << d.app_id << d.data;
    for (const auto& t : transactions) ss << t.sender << t.receiver << t.amount;
    return sha256(ss.str());
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
    for (const auto& t : transactions) j["transactions"].push_back({{"sender", t.sender}, {"receiver", t.receiver}, {"amount", t.amount}});
    return j;
}

// Blockchain Validation System
bool Blockchain::isChainValid() const {
    if (chain.empty()) {
        std::cout << "Blockchain is empty." << std::endl;
        return false;
    }

    for (size_t i = 1; i < chain.size(); i++) {
        const Block& currentBlock = chain[i];
        const Block& previousBlock = chain[i - 1];

        // Verifica se o hash do bloco atual está correto
        if (currentBlock.getHash() != currentBlock.calculateHash()) {
            std::cerr << "Block " << i << " has an invalid hash!" << std::endl;
            return false;
        }

        // Verifica se o previousHash está correto
        if (currentBlock.getPreviousHash() != previousBlock.getHash()) {
            std::cerr << "Block " << i << " has an incorrect previous hash!" << std::endl;
            return false;
        }
    }

    std::cout << "Blockchain is valid." << std::endl;
    return true;
}


// Blockchain constructor, initializes with a genesis block
Blockchain::Blockchain() {
    cout << "Initializing blockchain with the genesis block..." << endl;
    chain.emplace_back("0");
}

// Add a new empty block to the blockchain
void Blockchain::addBlock() {
    cout << "Adding new block..." << endl;
    chain.emplace_back(chain.back().hash);
    saveToFile("blockchain.json");
}

// Add transaction to the last block
void Blockchain::addTransaction(string sender, string receiver, double amount) {
    if (!chain.empty()) {
        cout << "Adding transaction to last block..." << endl;
        chain.back().addTransaction(sender, receiver, amount);
	saveToFile("blockchain.json");
    }
}

// Add data to the last block
void Blockchain::addData(string app_id, string data) {
    if (!chain.empty()) {
        cout << "Adding data to last block..." << endl;
        chain.back().addData(app_id, data);
	saveToFile("blockchain.json");
    }
}

// Display the current blockchain state
void Blockchain::printChain() {
    cout << "\nCurrent Blockchain State:" << endl;
    for (const auto& block : chain) {
        cout << block.toJSON().dump(4) << endl;
        cout << "----------------------" << endl;
    }
}

// Save blockchain to JSON file
void Blockchain::saveToFile(const string& filename) {
    json j;
    j["chain"] = json::array();
    for (const auto& block : chain) j["chain"].push_back(block.toJSON());
    ofstream file(filename);
    file << j.dump(4);
    file.close();
    cout << "Blockchain saved to " << filename << endl;
}

// Load blockchain from JSON file
void Blockchain::loadFromFile(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "Failed to open file: " << filename << endl;
        return;
    }
    json j;
    file >> j;
    file.close();
    chain.clear();
    for (const auto& blk : j["chain"]) {
        Block b(blk["previousHash"]);
        b.hash = blk["hash"];
        for (const auto& d : blk["data"]) b.data.push_back({d["app_id"], d["data"]});
        for (const auto& t : blk["transactions"]) b.transactions.push_back({t["sender"], t["receiver"], t["amount"]});
        chain.push_back(b);
    }
    cout << "Blockchain loaded from " << filename << endl;
}

// Get balance for Wallet
double Blockchain::getBalance(const string& wallet_id) const {
    double balance = 0.0;

    // Iterate through each block and its transactions
    for (const auto& block : chain) {
        for (const auto& transaction : block.transactions) {
            // If the wallet is the sender, subtract the amount
            if (transaction.sender == wallet_id) {
                balance -= transaction.amount;
            }
            // If the wallet is the receiver, add the amount
            if (transaction.receiver == wallet_id) {
                balance += transaction.amount;
            }
        }
    }

    return balance;
}
