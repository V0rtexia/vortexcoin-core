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

// Function to check if the file exists
bool file_exists(const string& filename) {
    ifstream file(filename);
    return file.good();
}

// Function to initialize the blockchain if the file doesn't exist
void initialize_blockchain(const string& filename) {
    Blockchain blockchain;
    blockchain.saveToFile(filename);  // Save the blockchain to the file
}

// Handle transaction requests
void handle_transaction_request(http_request request) {
    request.extract_json().then([request](json::value request_data) {
        // Extract data from the request
        string sender = request_data[U("sender")].as_string();
        string receiver = request_data[U("receiver")].as_string();
        double amount = request_data[U("amount")].as_double();

        // Add transaction to the blockchain
        Blockchain blockchain;
        blockchain.loadFromFile("blockchain.json");
        blockchain.addTransaction(sender, receiver, amount);

        // Save blockchain after the transaction
        blockchain.saveToFile("blockchain.json");

        // Prepare the response
        json::value response_data;
        response_data[U("status")] = json::value::string(U("Transaction added successfully"));
        request.reply(status_codes::OK, response_data);
    }).wait();  // Wait for the lambda to complete
}

// Handle requests for the blockchain data
void handle_blockchain_request(http_request request) {
    Blockchain blockchain;
    blockchain.loadFromFile("blockchain.json");

    // Convert blockchain data to nlohmann::json for easier manipulation
    nlohmann::json nlohmann_json_blockchain = nlohmann::json::array();

    // Add blocks to the JSON array
    for (const auto& block : blockchain.getChain()) {
        nlohmann_json_blockchain.push_back(block.toJSON());
    }

    // Convert nlohmann::json to web::json::value
    web::json::value response_data = web::json::value::parse(nlohmann_json_blockchain.dump());
    
    // Return blockchain data as a response
    request.reply(status_codes::OK, response_data);
}

void handle_block_request(http_request request) {
    string path = uri::decode(request.relative_uri().path());

    // Define the prefix "/block/"
    std::string prefix = "/block/";

    // Ensure the path starts with "/block/"
    if (path.find(prefix) != 0) {
        request.reply(status_codes::BadRequest, U("Invalid block request format"));
        return;
    }

    // Extract only the hash part
    string block_hash = path.substr(prefix.length());

    Blockchain blockchain;
    blockchain.loadFromFile("blockchain.json");

    // Find the block with the matching hash
    nlohmann::json response_json;
    bool block_found = false;
    for (const auto& block : blockchain.getChain()) {
        if (block.getHash() == block_hash) {
            response_json = block.toJSON();
            block_found = true;
            break;
        }
    }

    // If block is found, return it, otherwise return an error
    if (block_found) {
        web::json::value response_data = web::json::value::parse(response_json.dump());
        request.reply(status_codes::OK, response_data);
    } else {
        json::value error_response;
        error_response[U("error")] = json::value::string(U("Block not found"));
        request.reply(status_codes::NotFound, error_response);
    }
}

void handle_balance_request(http_request request) {
    string path = uri::decode(request.relative_uri().path());

    // Define the prefix "/balance/"
    std::string prefix = "/balance/";

    // Ensure the path starts with "/balance/"
    if (path.find(prefix) != 0) {
        request.reply(status_codes::BadRequest, U("Invalid balance request format"));
        return;
    }

    // Extract wallet_id from URL
    string wallet_id = path.substr(prefix.length());

    // Load blockchain data
    Blockchain blockchain;
    blockchain.loadFromFile("blockchain.json");

    // Calculate balance
    double balance = blockchain.getBalance(wallet_id);

    // Prepare the response
    json::value response_data;
    response_data[U("wallet")] = json::value::string(wallet_id);
    response_data[U("balance")] = json::value::number(balance);

    // Respond with the correct balance
    request.reply(status_codes::OK, response_data);
}




int main() {
    const string blockchain_file = "blockchain.json";

    if (!file_exists(blockchain_file)) {
        initialize_blockchain(blockchain_file);
    }

    try {
        // Set up the HTTP listener
        web::http::experimental::listener::http_listener listener(U("http://0.0.0.0:5555"));

        // Define the request handlers for each endpoint
        listener.support(web::http::methods::GET, [](http_request request) {
            // Route for /blockchain
            if (request.relative_uri().path() == "/blockchain") {
                handle_blockchain_request(request);
            }
            // Route for /balance/{wallet_id}
            else if (request.relative_uri().path().find("/balance/") == 0) {
                handle_balance_request(request);
            }
            // Route for /block/{hash}
            else if (request.relative_uri().path().find("/block/") == 0) {
                handle_block_request(request);
            }
        });

        listener.support(web::http::methods::POST, [](http_request request) {
            // Route for /transactions
            if (request.relative_uri().path() == "/transactions") {
                handle_transaction_request(request);
            }
        });

        // Start the listener
        listener
            .open()
            .then([](){ std::wcout << L"API is running at: http://0.0.0.0:5555/" << std::endl; })
            .wait();

        std::string line;
        std::getline(std::cin, line); // Keeps the server running
    } catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
    }

    return 0;
}

