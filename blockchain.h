#ifndef BLOCKCHAIN_H
#define BLOCKCHAIN_H

#include <iostream>
#include <vector>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>
#include <fstream>
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

// Structure to store additional block data
struct Data {
    string app_id;
    string data;
};

// Structure to store transactions within the block
struct Transaction {
    string sender;
    string receiver;
    double amount;
};

// Block class representing a blockchain block
class Block {
public:
    string previousHash; // Previous block hash
    string hash; // Current block hash
    vector<Data> data; // List of stored data in the block
    vector<Transaction> transactions; // List of transactions in the block

    Block(string prevHash); // Constructor
    void addTransaction(string sender, string receiver, double amount); // Add transaction
    void addData(string app_id, string data); // Add data to the block
    string calculateHash() const; // Compute the block hash
    void printBlock() const; // Display block information (for debugging)
    json toJSON() const; // Convert block to JSON format
    std::string getHash() const { return hash; }
    std::string getPreviousHash() const { return previousHash; }


private:
    static string sha256(const string& input); // Helper function to compute SHA-256
};

// Blockchain class representing the chain of blocks
class Blockchain {
public:
    Blockchain(); // Constructor
    void addBlock(); // Add a new empty block
    void addTransaction(string sender, string receiver, double amount); // Add transaction to last block
    void addData(string app_id, string data); // Add data to last block
    void printChain(); // Display the entire blockchain
    void saveToFile(const string& filename); // Save blockchain to JSON file
    void loadFromFile(const string& filename); // Load blockchain from JSON file
    vector<Block>& getChain() { return chain; }
    double getBalance(const string& wallet_id) const;
    bool isChainValid() const;

private:
    vector<Block> chain; // List of blocks
};

#endif // BLOCKCHAIN_H

