#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <iostream>
#include <fstream>
#include <string>
#include "blockchain.h"

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;
using namespace std;

// Check if the file exists
bool file_exists(const string& filename) {
    ifstream file(filename);
    return file.good();
}

// Initialize the blockchain if the file does not exist
void initialize_blockchain(const string& filename) {
    Blockchain blockchain;
    blockchain.saveToFile(filename);
}

// Handle transaction requests
void handle_transaction_request(http_request request) {
    request.extract_json().then([request](json::value request_data) {
        try {
            string sender = request_data[U("sender")].as_string();
            string receiver = request_data[U("receiver")].as_string();
            double amount = request_data[U("amount")].as_double();
            string signature = request_data[U("signature")].as_string();

            Blockchain blockchain;
            blockchain.loadFromFile("blockchain.json");

            // Validate balance and signature before adding the transaction
            if (sender != "SYSTEM" && blockchain.getBalance(sender) < amount) {
                request.reply(status_codes::BadRequest, U("Insufficient balance"));
                return;
            }

            if (!isSignatureValid(signature)) {
                request.reply(status_codes::Unauthorized, U("Invalid signature"));
                return;
            }

            blockchain.addTransaction(sender, receiver, amount, signature);
            blockchain.saveToFile("blockchain.json");

            json::value response;
            response[U("status")] = json::value::string(U("Transaction added successfully"));
            request.reply(status_codes::OK, response);
        } catch (...) {
            request.reply(status_codes::BadRequest, U("Error processing transaction"));
        }
    }).wait();
}

// Handle requests for the entire blockchain
void handle_blockchain_request(http_request request) {
    Blockchain blockchain;
    blockchain.loadFromFile("blockchain.json");

    nlohmann::json json_blockchain = nlohmann::json::array();
    for (const auto& block : blockchain.getChain()) {
        json_blockchain.push_back(block.toJSON());
    }

    web::json::value response_data = web::json::value::parse(json_blockchain.dump());
    request.reply(status_codes::OK, response_data);
}

// Handle requests for a specific block
void handle_block_request(http_request request) {
    string path = uri::decode(request.relative_uri().path());
    string prefix = "/block/";

    if (path.find(prefix) != 0) {
        request.reply(status_codes::BadRequest, U("Invalid format for block request"));
        return;
    }

    string block_hash = path.substr(prefix.length());

    Blockchain blockchain;
    blockchain.loadFromFile("blockchain.json");

    for (const auto& block : blockchain.getChain()) {
        if (block.getHash() == block_hash) {
            web::json::value response_data = web::json::value::parse(block.toJSON().dump());
            request.reply(status_codes::OK, response_data);
            return;
        }
    }

    json::value error_response;
    error_response[U("error")] = json::value::string(U("Block not found"));
    request.reply(status_codes::NotFound, error_response);
}

// Handle requests for wallet balance
void handle_balance_request(http_request request) {
    string path = uri::decode(request.relative_uri().path());
    string prefix = "/balance/";

    if (path.find(prefix) != 0) {
        request.reply(status_codes::BadRequest, U("Invalid format for balance request"));
        return;
    }

    string wallet_id = path.substr(prefix.length());

    Blockchain blockchain;
    blockchain.loadFromFile("blockchain.json");

    double balance = blockchain.getBalance(wallet_id);

    json::value response;
    response[U("wallet")] = json::value::string(wallet_id);
    response[U("balance")] = json::value::number(balance);
    request.reply(status_codes::OK, response);
}

// Main function to start the server
int main() {
    const string blockchain_file = "blockchain.json";

    // Initialize the blockchain if the file does not exist
    if (!file_exists(blockchain_file)) {
        initialize_blockchain(blockchain_file);
    }

    try {
        // Create HTTP listener on port 5555
        http_listener listener(U("http://0.0.0.0:5555"));

        // Handle GET requests
        listener.support(methods::GET, [](http_request request) {
            string path = request.relative_uri().path();
            if (path == "/blockchain") {
                handle_blockchain_request(request);
            } else if (path.find("/balance/") == 0) {
                handle_balance_request(request);
            } else if (path.find("/block/") == 0) {
                handle_block_request(request);
            } else {
                request.reply(status_codes::NotFound, U("Route not found"));
            }
        });

        // Handle POST requests
        listener.support(methods::POST, [](http_request request) {
            if (request.relative_uri().path() == "/transactions") {
                handle_transaction_request(request);
            } else {
                request.reply(status_codes::NotFound, U("Route not found"));
            }
        });

        // Open the listener
        listener.open().then([]() { 
            wcout << L"API running at: http://0.0.0.0:5555/" << endl; 
        }).wait();

        // Keep the server running
        string line;
        getline(cin, line);
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
    }

    return 0;
}

