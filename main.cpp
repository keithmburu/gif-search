#include <iostream>
#include <vector>
#include <list>
#include <string>
#include <sstream>
#include <map>
#include "asio/ip/tcp.hpp"
#include "rapidjson/document.h"
#include "rapidjson/reader.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"


using std::cin;
using std::cout;
using std::endl;
using std::string;
typedef std::vector<std::tuple<string, string>> vec;
typedef std::list<vec> list_vecs;

bool fetch(string query, int per_page, int page_num, list_vecs& results) {
    string parameters = "v1/gifs/search?api_key=gtjH6rap1pcRre0BnP2XdCr91USN8JWu&q=" + query + "&limit=" + std::to_string(per_page) + "&offset=" + std::to_string(page_num*per_page) + "&rating=r&lang=en";
    asio::ip::tcp::iostream s("api.giphy.com", "http");
    // cout << "GET /" << parameters << " HTTP/1.0\r" << endl;
    // cout << "Host: " << host << "\r" << endl;
    // cout << "Accept: */*\r" << endl;";
    // cout << "Connection: close\r\n\r" << endl;
    s.expires_from_now(std::chrono::seconds(5));
    if (!s) {
        cout << "Socket error: " << s.error().message() << endl;
        exit(1);
    }
    s << "GET " << parameters << " HTTP/1.1\r\n";
    s << "Host: " << "api.giphy.com" << "\r\n";
    s << "Accept: */*\r\n";
    s << "Connection: close\r\n\r\n";
    string header;
    while (getline(s, header) && header != "\r") {
        // cout << header << endl;
    }
    std::stringstream json_ss("");
    json_ss << s.rdbuf();
    string json = json_ss.str();
    // cout << json << endl;
    rapidjson::Document doc;
    doc.Parse(json.c_str());
    if (doc.HasParseError()) {
        cout << "Parse error " << doc.GetParseError() << endl;
        exit(2);
    }
    vec page;
    page.reserve(5);
    int num_entries = doc["data"].Size();
    if (num_entries > 0) {
        cout << endl;
        for (unsigned int i = 0; i < num_entries; i++) {
            string url = doc["data"][i]["url"].GetString();
            string rank = doc["data"][i]["rating"].GetString();
            cout << (page_num*per_page) + (i+1) << ") " << url << endl;
            std::tuple<string, string> entry = std::make_tuple(url, rank);
            page.push_back(entry);
            if (i == num_entries-1) {
                results.push_back(page);
            }
        }
        cout << endl;
        return false;
    } else {
        return true;
    }
}


void rank(list_vecs& results) {
    std::map<string, int> rank_map = {{"g", 0}, {"pg", 0}, {"pg-13", 0}, {"r", 0}};
    list_vecs::iterator it;
    for (it = results.begin(); it != results.end(); it++) {
        vec page = *it;
        for (unsigned int i = 0; i < page.size(); i++) {
            string url, rank;
            std::tie(url, rank) = page[i];
            rank_map[rank] += 1;
        }
    }
    cout << endl;
    cout << "g: " << rank_map["g"] << endl;
    cout << "pg: " << rank_map["pg"] << endl;
    cout << "pg-13: " << rank_map["pg-13"] << endl;
    cout << "r: " << rank_map["r"] << endl;
    cout << endl;
}

int main() {
    list_vecs results;
    int page_num = 0;
    const int per_page = 5;
    string command;
    string query;

    cout << "Enter command: ";
    while(getline(cin, command)) { 
        string instruction = command.substr(0, command.find(' '));

        if (instruction == "search") {
            string criteria = command.substr(command.find(' ')+1);
            if (page_num > 0) {
                cout << "Cancel current search!\n" << endl;
            } else if (criteria.length() == 0) {
                cout << "Specify search criteria!" << endl;
            } else {
                std::stringstream criteria_ss(criteria);
                std::vector<string> criteria_vec;
                string token;
                while (std::getline(criteria_ss, token, ' ')) {
                    criteria_vec.push_back(token);
                }
                for (string criterion : criteria_vec) {
                    query += criterion;
                    if (criterion != criteria_vec.back()) {
                        query += "-";
                    }
                }
                cout << "Searching for \"" << criteria << "\" gifs" << endl;
                bool no_data = fetch(query, per_page, page_num, results);
                if (no_data) {
                    cout << "No data\n" << endl;
                    page_num = 0;
                } else {
                    page_num += 1;
                }
            }
        } else if (instruction == "next") {
            if (page_num == 0) {
                cout << "Initiate new search!\n" << endl;
            } else {
                cout << "Loading the next " << per_page << " search results" << endl;
                bool no_data = fetch(query, per_page, page_num, results);
                if (no_data) {
                    cout << "No data\n" << endl;
                    page_num = 0;
                } else {
                    page_num += 1;
                }
            }
        } else if (instruction == "rank") {
            if (results.size() == 0) {
                cout << "Initiate new search!\n" << endl;
            } else {
                cout << "Checking result ranks" << endl;
                rank(results);
            }
        } else if (instruction == "cancel") {
            cout << "Cancelling search\n" << endl;
            page_num = 0;
            query = "";
            results.clear();
        } else if (instruction == "exit") {
            cout << "Closing session\n" << endl;
            page_num = 0;
            query = "";
            results.clear();
            return 0;
        } else {
            cout << "Invalid command!\n" << endl;
        }
        cout << "Enter command: ";
    }
    return 0;
}

