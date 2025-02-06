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


bool isSignatureValid(const string& signature); // Check if the signature is valid

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
    string signature; // Transaction signature for integrity verification
};

// Block class representing a blockchain block
class Block {
public:
    string previousHash;                   // Previous block hash
    mutable string hash;                           // Current block hash
    vector<Data> data;                     // List of stored data in the block
    vector<Transaction> transactions;      // List of transactions in the block
    mutable bool isDirty;


    Block(string prevHash);                // Constructor

    void addTransaction(const string& sender, const string& receiver, double amount, const string& signature);
    void addData(const string& app_id, const string& data);
    string calculateHash() const;          // Compute the block hash
    void printBlock() const;               // Display block information (for debugging)
    json toJSON() const;                   // Convert block to JSON format

    string getPreviousHash() const { return previousHash; }
    string getHash() const;


private:
    static string sha256(const string& input); // Helper function to compute SHA-256
};

// Blockchain class representing the chain of blocks
class Blockchain {
public:
    Blockchain();                          // Constructor

    void addBlock();                       // Add a new empty block
    void addTransaction(const string& sender, const string& receiver, double amount, const string& signature);
    void addData(const string& app_id, const string& data);
    void printChain() const;               // Display the entire blockchain
    void saveToFile(const string& filename) const;
    void loadFromFile(const string& filename);
    vector<Block>& getChain() { return chain; }

    double getBalance(const string& wallet_id) const;    // Get the balance of a wallet
    bool isChainValid() const;                           // Verify the integrity of the blockchain


private:
    vector<Block> chain;                   // List of blocks
    bool verifySignature(const Transaction& transaction) const; // Verify the transaction signature
};

#endif // BLOCKCHAIN_H

