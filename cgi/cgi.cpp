#include <iostream>
#include <vector>
#include <list>
#include <string>
#include <sstream>
#include <map>
#include <string.h>
#include "asio/ip/tcp.hpp"
#include "rapidjson/document.h"
#include "rapidjson/reader.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "cgicc/Cgicc.h"
#include "cgicc/CgiDefs.h"
#include "cgicc/HTTPHTMLHeader.h"
#include "cgicc/HTMLClasses.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;
using namespace cgicc;
typedef std::vector<std::tuple<string, string>> vec;
typedef std::list<vec> list_vecs;


bool fetch(string query, int per_page, int page_num, string& results_str, string& response) {
    response += "Query: " + query + "\n<br>";
    response += "Page: " + std::to_string(page_num+1) + "\n<br>";
    string parameters = "v1/gifs/search?api_key=gtjH6rap1pcRre0BnP2XdCr91USN8JWu&q=" + query + "&limit=" + std::to_string(per_page) + "&offset=" + std::to_string(page_num*per_page) + "&rating=r&lang=en";
    asio::ip::tcp::iostream s("api.giphy.com", "http");
    // cout << "GET /" << parameters << " HTTP/1.0\r" << endl;
    // cout << "Host: " << host << "\r" << endl;
    // cout << "Accept: */*\r" << endl;";
    // cout << "Connection: close\r\n\r" << endl;
    s.expires_from_now(std::chrono::seconds(5));
    if (!s) {
        response += "Socket error: " + s.error().message() + "\n";
        return true;
    } else {
        s << "GET " << parameters << " HTTP/1.1\r\n";
        s << "Host: " << "api.giphy.com" << "\r\n";
        s << "Accept: */*\r\n";
        s << "Connection: close\r\n\r\n";
        string header;
        while (getline(s, header) && header != "\r") {
            // skipping headers;
            // cout << header << endl;
        }
        std::stringstream json_ss("");
        json_ss << s.rdbuf();
        string json = json_ss.str();
        // cout << json << endl;
        rapidjson::Document doc;
        doc.Parse(json.c_str());
        if (doc.HasParseError()) {
            response += "Parse error " + std::to_string(doc.GetParseError()) + "\n";
            return true;
        } else {
            vec page;
            page.reserve(5);
            int num_entries = doc["data"].Size();
            if (num_entries > 0) {
                response += "<br>";
                for (unsigned int i = 0; i < num_entries; i++) {
                    string url = doc["data"][i]["url"].GetString();
                    string img_url = doc["data"][i]["images"]["fixed_height"]["url"].GetString();
                    string title = doc["data"][i]["title"].GetString();
                    string rank = doc["data"][i]["rating"].GetString();
                    response += std::to_string((page_num*per_page) + (i+1)) + ") <a href=\"" + url + "\" style=\"background-color:white;\">" + url + "</a>\n<br>";
                    response += "<img src=\"" + img_url + "\" alt=\"" + title + "\">" + "</img>\n<br>";
                    results_str += rank + ",";
                    std::tuple<string, string> entry = std::make_tuple(url, rank);
                    page.push_back(entry);
                }
                response += "<br>";
                return false;
            } else {
                return true;
            }
        }
    }
}


void rank(string& results_str, string& response) {
    std::map<string, int> rank_map = {{"g", 0}, {"pg", 0}, {"pg-13", 0}, {"r", 0}};
    vec page;
    list_vecs results;
    string results_strcp = results_str;
    char* token = strtok((char*)results_strcp.c_str(), ",");
    while (token != NULL) {
        page.push_back(std::make_tuple("", string(token)));
        token = strtok(NULL, ",");
        if (page.size() == 5 || token == NULL) {
            results.push_back(page);
            page.clear();
        }
    }
    list_vecs::iterator it;
    for (it = results.begin(); it != results.end(); it++) {
        vec page = *it;
        for (unsigned int i = 0; i < page.size(); i++) {
            string url, rank;
            std::tie(url, rank) = page[i];
            rank_map[rank] += 1;
        }
    }
    response += "\n";
    response += "g: " + std::to_string(rank_map["g"])
                + "\n<br>" + "pg: " + std::to_string(rank_map["pg"])
                + "\n<br>" + "pg-13: " + std::to_string(rank_map["pg-13"])
                + "\n<br>" + "r: " + std::to_string(rank_map["r"]);
    response += "\n<br>";
}


int main() {
    Cgicc formData;
    const_cookie_iterator cci;
    string response;
    string query = "";
    int page_num = 0;
    string results_str = "";
    const int per_page = 5;

    const CgiEnvironment& env = formData.getEnvironment();
    for(cci = env.getCookieList().begin(); cci != env.getCookieList().end(); 
    ++cci) {
        if (cci->getName() == "query") {
            query = cci->getValue();
        } else if (cci->getName() == "page_num") {
            page_num = std::stoi(cci->getValue());   
        } else if (cci->getName() == "results") {
            results_str = cci->getValue();   
        }
    }
    
    std::string command;
    form_iterator fi = formData.getElement("command");  
    string instruction = "";
    bool command_entered = (!fi->isEmpty() && fi != (*formData).end());
    if(command_entered) {  
        cout << "Command: " << **fi << "<br>" << endl;  
        command = **fi;
        
        instruction = command.substr(0, command.find(' '));

        if (instruction == "search") {
            string criteria = command.substr(command.find(' ')+1);
            if (page_num > 0) {
                response += "Cancel current search!\n\n";
            } else if (criteria.length() == 0) {
                response += "Specify search criteria!\n";
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
                response += "Searching for \"" + criteria + "\" gifs\n<br>";
                bool no_data = fetch(query, per_page, page_num, results_str, response);
                if (no_data) {
                    response += "No data\n\n";
                    page_num = 0;
                } else {
                    page_num += 1;
                }
            }
        } else if (instruction == "next") {
            if (page_num == 0) {
                response += "Initiate new search!\n\n";
            } else {
                response += "Loading the next " + std::to_string(per_page) + " search results\n<br>";
                bool no_data = fetch(query, per_page, page_num, results_str, response);
                if (no_data) {
                    response += "No data\n\n";
                    page_num = 0;
                } else {
                    page_num += 1;
                }
            }
        } else if (instruction == "rank") {
            if (results_str.length() == 0) {
                response += "Initiate new search!\n\n";
            } else {
                response += "Checking result ranks\n<br>";
                rank(results_str, response);
            }
        } else if (instruction == "cancel") {
            response += "<h2> GIF Search </h2>";
            page_num = 0;
            query = "";
            results_str = "";
        } else if (instruction == "exit") {
            response += "Closing session\n\n";
            page_num = 0;
            query = "";
            results_str = "";
        } else {
            response += "Invalid command!\n\n";
        }

        cout << "Set-Cookie:query = " << query << ";\r\n";
        cout << "Set-Cookie:page_num = " << page_num << ";\r\n";
        cout << "Set-Cookie:results = " << results_str << ";\r\n";        
    } 

    cout << "Content-type:text/html\r\n\r\n";
    cout << "<html>\n";
    cout << "<head>\n";
    cout << "<title> GIF Search </title>\n";
    cout << "<style>\n";
    cout << "body {\n";
    cout << "    font-family: Arial,sans-serif;\n";
    cout << "    color: #fff;\n";
    cout << "    background-color: #000;\n";
    cout << "}\n";
    cout << "</style>\n";
    cout << "</head>\n";
    cout << "<body>\n";
    if(command_entered) {  
        cout << response << endl;
    } else {
        cout << "No command entered!" << endl;   
    }
    if (instruction != "exit") {
        cout << "<form action = \"/gifs.cgi\" method = \"POST\">\n";
        cout << "Enter command: <input type = \"text\" name = \"command\">  <br />";
        cout << "<input type = \"submit\" value = \"Submit\"  style=\"background-color:green;\"/>\n";
        cout << "</form>";
    }
    cout << "</body>\n";
    cout << "</html>\n";


    return 0;
}

